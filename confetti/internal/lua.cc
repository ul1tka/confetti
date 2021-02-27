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
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <string>

namespace confetti::internal {

static bool strCaseEquals(std::string_view lhs, std::string_view rhs) noexcept
{
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
        [](auto x, auto y) noexcept { return std::tolower(x) == std::tolower(y); });
}

template <typename... T>
static bool strCaseAnyOf(std::string_view lhs, T... rhs) noexcept
{
    return (strCaseEquals(lhs, rhs) || ...);
}

LuaException::LuaException(const char* message)
    : std::runtime_error{message}
{
}

LuaException::~LuaException() = default;

void LuaException::raise(const char* message) { throw LuaException{message}; }

void LuaException::raise(lua_State* state)
{
    const char* message;
    raise(state && (message = lua_tostring(state, -1)) ? message : "Lua error");
}

[[noreturn]] static int on_lua_panic(lua_State* state) { LuaException::raise(state); }

LuaState::LuaState()
    : state_{lua_newstate(&alloc, this)}
{
    if (!state_)
        LuaException::raise("Cannot create Lua stack");

    lua_atpanic(state_, &on_lua_panic);

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

LuaReference::LuaReference()
    : state_{std::make_shared<LuaState>()}
    , ref_{LUA_NOREF}
{
}

LuaReference::LuaReference(LuaReference&& other) noexcept
    : state_{std::move(other.state_)}
    , ref_{other.ref_}
{
    other.ref_ = LUA_NOREF;
}

LuaReference::LuaReference(std::shared_ptr<LuaState> state) noexcept
    : state_{std::move(state)}
    , ref_{luaL_ref(*state_, LUA_REGISTRYINDEX)}
{
}

LuaReference::~LuaReference() { reset(); }

void LuaReference::reset() noexcept
{
    if (state_ && ref_ != LUA_NOREF) {
        luaL_unref(*state_, LUA_REGISTRYINDEX, ref_);
        ref_ = LUA_NOREF;
    }
}

void LuaReference::set()
{
    reset();
    ref_ = luaL_ref(*state_, LUA_REGISTRYINDEX);
}

void LuaReference::push() const { lua_rawgeti(*state_, LUA_REGISTRYINDEX, ref_); }

LuaStackGuard::LuaStackGuard(const LuaReference& ref) noexcept
    : state_{ref}
    , top_{lua_gettop(state_)}
{
    ref.push();
}

LuaStackGuard::~LuaStackGuard() noexcept { lua_settop(state_, top_); }

LuaSource::LuaSource(SharedConstructTag, LuaReference&& ref) noexcept
    : ref_{std::move(ref)}
{
}

LuaSource::LuaSource(SharedConstructTag, std::shared_ptr<LuaState> ref) noexcept
    : ref_{std::move(ref)}
{
}

LuaSource::~LuaSource() = default;

int LuaSource::loadField(std::string_view name) const noexcept
{
    auto type = lua_getfield(ref_, -1, name.data());
    while (type == LUA_TFUNCTION) {
        if (lua_pcall(ref_, 0, 1, 0) != LUA_OK)
            LuaException::raise(ref_);
        type = lua_type(ref_, -1);
    }
    return type;
}

std::optional<bool> LuaSource::tryGetBoolean(std::string_view name) const
{
    std::optional<bool> result;
    LuaStackGuard _{ref_};
    switch (loadField(name)) {
        case LUA_TNIL:
        case LUA_TUSERDATA:
        case LUA_TTABLE:
        case LUA_TTHREAD:
            break;
        case LUA_TSTRING: {
            size_t size{};
            if (auto data = lua_tolstring(ref_, -1, &size)) {
                std::string_view value{data, size};
                if (strCaseAnyOf(value, "y", "yes", "true", "1")) {
                    result.emplace(true);
                } else if (strCaseAnyOf(value, "n", "no", "false", "0")) {
                    result.emplace(false);
                } else {
                    errno = 0;
                    char* end_ptr{};
                    const auto n = std::strtod(data, &end_ptr);
                    result.emplace(end_ptr && *end_ptr == '\0' && errno == 0 && n > 0);
                }
            }
            break;
        }
        case LUA_TBOOLEAN:
            result.emplace(lua_toboolean(ref_, -1) != 0);
            break;
        case LUA_TNUMBER:
        default:
            result.emplace(lua_tonumber(ref_, -1) > 0);
            break;
    }
    return result;
}

std::optional<double> LuaSource::tryGetDouble(std::string_view name) const
{
    std::optional<double> result;
    LuaStackGuard _{ref_};
    switch (loadField(name)) {
        case LUA_TNIL:
        case LUA_TUSERDATA:
        case LUA_TTABLE:
        case LUA_TTHREAD:
            break;
        case LUA_TBOOLEAN:
            result.emplace(lua_toboolean(ref_, -1));
            break;
        case LUA_TSTRING: {
            size_t size{};
            if (auto data = lua_tolstring(ref_, -1, &size)) {
                char* end_ptr{};
                errno = 0;
                auto value = std::strtod(data, &end_ptr);
                const auto ec = errno;
                if (end_ptr != nullptr && *end_ptr == '\0' && ec == 0) {
                    result.emplace(value);
                } else {
                    throw std::runtime_error{std::string{"Cannot convert string '"}
                                                 .append(data, size)
                                                 .append("' to double: ")
                                                 .append(std::strerror(ec))};
                }
            }
            break;
        }
        default:
            result.emplace(lua_tonumber(ref_, -1));
            break;
    }
    return result;
}

std::optional<std::string> LuaSource::tryGetString(std::string_view name) const
{
    std::optional<std::string> result;
    LuaStackGuard _{ref_};
    switch (loadField(name)) {
        case LUA_TNIL:
        case LUA_TUSERDATA:
        case LUA_TTABLE:
        case LUA_TTHREAD:
            break;
        case LUA_TBOOLEAN:
            result.emplace(1, '0' + lua_toboolean(ref_, -1));
            break;
        default: {
            size_t size{};
            if (auto data = lua_tolstring(ref_, -1, &size))
                result.emplace(data, size);
            break;
        }
    }
    return result;
}

std::shared_ptr<ConfigSource> LuaSource::tryGetChild(std::string_view name) const
{
    std::shared_ptr<ConfigSource> result;
    LuaStackGuard _{ref_};
    switch (loadField(name)) {
        case LUA_TTABLE:
            result = std::make_shared<LuaSource>(SharedConstructTag{}, ref_.getState());
            break;
    }
    return result;
}

ConfigSourcePointer LuaSource::loadFile(const std::filesystem::path& file)
{
    LuaReference ref;
    lua_newtable(ref);
    lua_pushvalue(ref, -1);
    lua_setglobal(ref, "Confetti");
    ref.set();
    ref->runFile(file);
    return std::make_shared<LuaSource>(SharedConstructTag{}, std::move(ref));
}

} // namespace confetti::internal
