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
}

#include <gtest/gtest.h>
#include <memory>

TEST(LuaException, RaiseWithoutState)
{
    try {
        conf::internal::LuaException::raise(nullptr);
        ADD_FAILURE() << "Lua exception was not thrown";
    } catch (const conf::internal::LuaException& e) {
        EXPECT_STREQ("Fatal Lua error", e.what());
    }
}

TEST(LuaException, RaiseWithoutMessage)
{
    std::unique_ptr<lua_State, decltype(&lua_close)> state{luaL_newstate(), &lua_close};
    ASSERT_TRUE(state);
    try {
        conf::internal::LuaException::raise(state.get());
        ADD_FAILURE() << "Lua exception was not thrown";
    } catch (const conf::internal::LuaException& e) {
        EXPECT_STREQ("Fatal Lua error", e.what());
    }
}

TEST(LuaException, RaiseWithMessage)
{
    std::unique_ptr<lua_State, decltype(&lua_close)> state{luaL_newstate(), &lua_close};
    ASSERT_TRUE(state);
    lua_pushstring(state.get(), "test message");
    try {
        conf::internal::LuaException::raise(state.get());
        ADD_FAILURE() << "Lua exception was not thrown";
    } catch (const conf::internal::LuaException& e) {
        EXPECT_STREQ("Fatal Lua error: test message", e.what());
    }
}

TEST(LuaState, BasicScript)
{
    conf::internal::LuaState state;
    const auto stackElements = lua_gettop(state);
    EXPECT_EQ(0, luaL_dostring(state, R"!(print("Hello from Lua"))!"));
    EXPECT_EQ(stackElements, lua_gettop(state));
    EXPECT_NE(0, luaL_dostring(state, R"!(wrong syntax)!"));
    EXPECT_EQ(stackElements + 1, lua_gettop(state));
    lua_pop(state, 1);
    EXPECT_EQ(stackElements, lua_gettop(state));
}
