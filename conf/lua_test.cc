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
#include <type_traits>

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

TEST(LuaState, Basic)
{
    static_assert(!std::is_copy_assignable_v<conf::internal::LuaState>);
    static_assert(!std::is_copy_constructible_v<conf::internal::LuaState>);

    conf::internal::LuaState state1;
    EXPECT_TRUE(state1);

    conf::internal::LuaState state2{std::move(state1)};

    EXPECT_FALSE(state1);
    EXPECT_TRUE(state2);

    state1 = std::move(state2);

    EXPECT_TRUE(state1);
    EXPECT_FALSE(state2);

    EXPECT_THROW(state1.raise(), conf::internal::LuaException);

    EXPECT_NO_THROW(state1.check(LUA_OK));
    EXPECT_THROW(state1.check(LUA_ERRMEM), conf::internal::LuaException);
}

TEST(LuaState, RunCode)
{
    conf::internal::LuaState state;
    EXPECT_NO_THROW(state.runCode(R"!(print("Hello from Lua"))!"));
    EXPECT_THROW(state.runCode(R"!(wrong syntax)!"), conf::internal::LuaException);
}

TEST(LuaTree, Basic)
{
    auto tree
        = conf::LuaTree::loadFile(std::filesystem::path{CONF_SOURCE_DIR} / "conf" / "lua_test.lua");

    EXPECT_FALSE(tree.tryGetString("this_key_should_not_exist").has_value());
    EXPECT_TRUE(tree.tryGetString("simple_string").has_value());

    EXPECT_FALSE(tree.tryGet<std::string>("this_key_should_not_exist").has_value());
    EXPECT_TRUE(tree.tryGet<std::string>("simple_string").has_value());

    EXPECT_EQ("Hello, Lua!", tree.getString("simple_string"));
    EXPECT_EQ("Hello, Lua!", tree.get<std::string>("simple_string"));

    EXPECT_EQ("12345", tree.getString("simple_number"));
    EXPECT_EQ("12345", tree.get<std::string>("simple_number"));

    EXPECT_EQ("1", tree.getString("simple_yes"));
    EXPECT_EQ("0", tree.getString("simple_no"));
    EXPECT_EQ("4", tree.getString("simple_func"));
    EXPECT_EQ("6", tree.getString("simple_nested_func"));

    EXPECT_FALSE(tree.tryGetChild("this_key_should_not_exist").has_value());
    EXPECT_TRUE(tree.tryGetChild("user").has_value());

    auto userTree = tree["user"];

    EXPECT_EQ("Vlad Lazarenko", userTree.getString("name"));
    EXPECT_EQ("vlad@lazarenko.me", tree["user"].getString("email"));
}
