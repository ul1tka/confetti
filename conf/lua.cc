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
#include <lua.h>
#include <lualib.h>
}

#include <cassert>
#include <string>

namespace conf::internal {

static std::string getErrorMessage(lua_State* state) noexcept
{
    std::string result;
    if (state) {
        const char* message = lua_tostring(state, -1);
        if (message) {
            result.assign("Fatal Lua error: ").append(message);
            return result;
        }
    }
    result.assign("Fatal Lua error");
    return result;
}

LuaException::LuaException(lua_State* state)
    : std::runtime_error{getErrorMessage(state)}
{
}

LuaException::~LuaException() = default;

int LuaException::raise(lua_State* state) { throw LuaException{state}; }

LuaState::LuaState() noexcept
    : state_{}
{
    state_ = lua_newstate(&alloc, this);
    if (!state_)
        return;

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

    lua_atpanic(state_, &LuaException::raise);
}

LuaState::LuaState(LuaState&& other) noexcept
    : state_{other.state_}
{
    other.state_ = nullptr;
}

LuaState::~LuaState() { close(); }

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

} // namespace conf::internal
