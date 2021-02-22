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

#ifndef CONF_LUA_HH
#define CONF_LUA_HH

#include <cstddef>
#include <stdexcept>

extern "C" {
struct lua_State;
};

namespace conf::internal {

class LuaException final : public std::runtime_error {
public:
    explicit LuaException(lua_State* state);

    ~LuaException() override;

    [[noreturn]] static int raise(lua_State* state);
};

class LuaState final {
public:
    LuaState() noexcept;

    LuaState(const LuaState&) = delete;

    LuaState(LuaState&& other) noexcept;

    ~LuaState();

    LuaState& operator=(const LuaState&) = delete;

    LuaState& operator=(LuaState&& other) noexcept;

    void close() noexcept;

    operator lua_State*() const noexcept { return state_; }

private:
    lua_State* state_;

    /// @see http://www.lua.org/manual/5.1/manual.html#lua_Alloc
    static void* alloc(void* aux, void* ptr, size_t osize, size_t nsize) noexcept;
};

} // namespace conf::internal

#endif
