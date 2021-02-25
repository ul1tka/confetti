//
// Copyright (C) 2021 Vlad Lazarenko <vlad@lazarenko.me>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "lua.hh"

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

#include <cassert>
#include <string>

namespace conf {
namespace internal {

LuaStackGuard::LuaStackGuard(lua_State* state) noexcept
    : state_{state}
    , top_{lua_gettop(state)}
{
}

LuaStackGuard::~LuaStackGuard() noexcept { lua_settop(state_, top_); }

static std::string getErrorMessage(lua_State* state) noexcept
{
    std::string result;
    if (state) {
        const char* message = lua_tostring(state, -1);
        if (message) {
            result.assign("Fatal Lua error: ").append(message);
            lua_pop(state, 1);
            return result;
        }
    }
    result.assign("Fatal Lua error");
    return result;
}

LuaException::LuaException(const char* error_message)
    : std::runtime_error{error_message}
{
}

LuaException::LuaException(lua_State* state)
    : std::runtime_error{getErrorMessage(state)}
{
}

LuaException::~LuaException() = default;

int LuaException::raise(lua_State* state) { throw LuaException{state}; }

LuaState::LuaState()
    : state_{}
{
    state_ = lua_newstate(&alloc, this);

    if (!state_)
        throw LuaException{"Cannot create Lua stack"};

    lua_atpanic(state_, &LuaException::raise);

    luaopen_base(state_);
    luaopen_coroutine(state_);
    luaopen_table(state_);
    luaopen_io(state_);
    luaopen_os(state_);
    luaopen_string(state_);
    luaopen_utf8(state_);
    luaopen_math(state_);
    luaopen_debug(state_);
    luaopen_package(state_);
}

LuaState::LuaState(LuaState&& other) noexcept
    : state_{other.state_}
{
    other.state_ = nullptr;
}

LuaState::~LuaState() { close(); }

void LuaState::raise() const { LuaException::raise(state_); }

void LuaState::check(int result) const
{
    if (result != LUA_OK)
        raise();
}

void LuaState::close() noexcept
{
    if (state_ != nullptr) {
        lua_close(state_);
        state_ = nullptr;
    }
}

LuaState& LuaState::operator=(LuaState&& other) noexcept
{
    close();
    std::swap(state_, other.state_);
    return *this;
}

void* LuaState::alloc(
    [[maybe_unused]] void* aux, void* ptr, [[maybe_unused]] size_t osize, size_t nsize) noexcept
{
    assert(aux != nullptr);
    if (nsize == 0) {
        free(ptr);
        return nullptr;
    }
    return realloc(ptr, nsize);
}

void LuaState::run()
{
    check(lua_pcall(state_, 0, 1, 0));
    lua_pop(state_, lua_gettop(state_));
}

void LuaState::runFile(const std::filesystem::path& file)
{
    check(luaL_loadfile(state_, file.native().c_str()));
    run();
}

void LuaState::runCode(std::string_view code)
{
    const auto address = std::to_string(reinterpret_cast<std::ptrdiff_t>(code.data()));
    check(luaL_loadbuffer(state_, code.data(), code.size(), address.c_str()));
    run();
}

LuaReference::LuaReference(std::shared_ptr<LuaState> state, int ref) noexcept
    : state_{std::move(state)}
    , ref_{ref}
{
}

LuaReference::LuaReference(std::shared_ptr<LuaState> state) noexcept
    : state_{std::move(state)}
    , ref_{luaL_ref(*state_, LUA_REGISTRYINDEX)}
{
}

LuaReference::~LuaReference()
{
    if (state_ && ref_ != LUA_NOREF) {
        luaL_unref(*state_, LUA_REGISTRYINDEX, ref_);
        ref_ = LUA_NOREF;
    }
}

void LuaReference::push() const { lua_rawgeti(*state_, LUA_REGISTRYINDEX, ref_); }

} // namespace internal

LuaTree::LuaTree(std::shared_ptr<internal::LuaReference> ref) noexcept
    : ref_{std::move(ref)}
{
}

LuaTree::~LuaTree() { }

void LuaTree::raiseKeyNotFound(std::string_view name)
{
    std::string msg = "Key not found: ";
    msg.append(name);
    throw std::runtime_error{std::move(msg)};
}

int LuaTree::loadField(std::string_view name) const noexcept
{
    auto type = lua_getfield(*ref_, -1, name.data());
    while (type == LUA_TFUNCTION) {
        if (lua_pcall(*ref_, 0, 1, 0) != LUA_OK)
            internal::LuaException::raise(*ref_);
        type = lua_type(*ref_, -1);
    }
    return type;
}

std::optional<std::string> LuaTree::tryGetString(std::string_view name) const
{
    std::optional<std::string> result;
    internal::LuaStackGuard _{*ref_};
    ref_->push();
    switch (loadField(name)) {
        case LUA_TNIL:
            break;
        case LUA_TBOOLEAN:
            result.emplace(1, '0' + lua_toboolean(*ref_, -1));
            break;
        default: {
            size_t size{};
            if (auto data = lua_tolstring(*ref_, -1, &size))
                result.emplace(data, size);
            break;
        }
    }
    return result;
}

[[nodiscard]] std::string LuaTree::getString(std::string_view name) const
{
    auto result = tryGetString(name);
    if (!result.has_value())
        raiseKeyNotFound(name);
    return result.value();
}

const std::optional<LuaTree> LuaTree::tryGetChild(std::string_view name) const
{
    std::optional<LuaTree> result;
    internal::LuaStackGuard _{*ref_};
    ref_->push();
    switch (loadField(name)) {
        case LUA_TTABLE:
            result.emplace(LuaTree{std::make_shared<internal::LuaReference>(ref_->getState())});
            break;
    }
    return result;
}

const LuaTree LuaTree::getChild(std::string_view name) const
{
    auto result = tryGetChild(name);
    if (!result.has_value())
        raiseKeyNotFound(name);
    return result.value();
}

LuaTree LuaTree::loadFile(const std::filesystem::path& file)
{
    auto state = std::make_shared<internal::LuaState>();
    lua_newtable(*state);
    lua_pushvalue(*state, -1);
    lua_setglobal(*state, "Confetti");
    auto ref = luaL_ref(*state, LUA_REGISTRYINDEX);
    state->runFile(file);
    return LuaTree{std::make_shared<internal::LuaReference>(std::move(state), ref)};
}

} // namespace conf
