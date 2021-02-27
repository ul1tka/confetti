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
        confetti::internal::LuaException::raise(static_cast<lua_State*>(nullptr));
        ADD_FAILURE() << "Lua exception was not thrown";
    } catch (const confetti::internal::LuaException& e) {
        EXPECT_STREQ("Lua error", e.what());
    }
}

TEST(LuaException, RaiseWithoutMessageOnStack)
{
    std::unique_ptr<lua_State, decltype(&lua_close)> state{luaL_newstate(), &lua_close};
    ASSERT_TRUE(state);
    try {
        confetti::internal::LuaException::raise(state.get());
        ADD_FAILURE() << "Lua exception was not thrown";
    } catch (const confetti::internal::LuaException& e) {
        EXPECT_STREQ("Lua error", e.what());
    }
}

TEST(LuaException, RaiseWithMessageOnStack)
{
    static constexpr char msg[] = "test message";
    std::unique_ptr<lua_State, decltype(&lua_close)> state{luaL_newstate(), &lua_close};
    ASSERT_TRUE(state);
    lua_pushstring(state.get(), msg);
    try {
        confetti::internal::LuaException::raise(state.get());
        ADD_FAILURE() << "Lua exception was not thrown";
    } catch (const confetti::internal::LuaException& e) {
        EXPECT_STREQ(msg, e.what());
    }
}

TEST(LuaException, RaiseWithCustomMessage)
{
    static constexpr char msg[] = "custom message";
    try {
        confetti::internal::LuaException::raise(msg);
        ADD_FAILURE() << "Lua exception was not thrown";
    } catch (const confetti::internal::LuaException& e) {
        EXPECT_STREQ(msg, e.what());
    }
}

TEST(LuaState, Basic)
{
    static_assert(!std::is_copy_assignable_v<confetti::internal::LuaState>);
    static_assert(!std::is_copy_constructible_v<confetti::internal::LuaState>);
    static_assert(!std::is_move_assignable_v<confetti::internal::LuaState>);
    static_assert(!std::is_move_constructible_v<confetti::internal::LuaState>);

    confetti::internal::LuaState state;

    EXPECT_TRUE(state);
    EXPECT_THROW(state.raise(), confetti::internal::LuaException);
    EXPECT_NO_THROW(state.check(LUA_OK));
    EXPECT_THROW(state.check(LUA_ERRMEM), confetti::internal::LuaException);
}

TEST(LuaState, RunCode)
{
    confetti::internal::LuaState state;
    EXPECT_NO_THROW(state.runCode(R"!(print("Hello from Lua"))!"));
    EXPECT_THROW(state.runCode(R"!(wrong syntax)!"), confetti::internal::LuaException);
}

static decltype(auto) loadTestFile()
{
    return confetti::internal::LuaSource::loadFile(
        std::filesystem::path{CONF_SOURCE_DIR "/confetti/internal/lua_test.lua"});
}

TEST(LuaTree, Boolean)
{
    auto source = loadTestFile();
    EXPECT_TRUE(source->tryGetBoolean("simple_yes").value());
    EXPECT_FALSE(source->tryGetBoolean("simple_no").value());
    EXPECT_TRUE(source->tryGetBoolean("simple_number").value());
    EXPECT_TRUE(source->tryGetBoolean("simple_double_number").value());
    EXPECT_FALSE(source->tryGetBoolean("simple_zero").value());
    EXPECT_FALSE(source->tryGetBoolean("simple_string").value());
}

TEST(LuaTree, Double)
{
    auto source = loadTestFile();
    EXPECT_DOUBLE_EQ(0.0, source->tryGetDouble("simple_no").value());
    EXPECT_DOUBLE_EQ(1.0, source->tryGetDouble("simple_yes").value());
    EXPECT_FALSE(source->tryGetDouble("this_key_should_not_exist").has_value());
    EXPECT_TRUE(source->tryGetDouble("simple_number"));
    EXPECT_FALSE(source->tryGetDouble("this_key_should_not_exist"));
    EXPECT_TRUE(source->tryGetDouble("simple_number"));
    EXPECT_DOUBLE_EQ(12345, source->tryGetDouble("simple_number").value());
    EXPECT_DOUBLE_EQ(19.86, source->tryGetDouble("simple_double_number").value());
    EXPECT_DOUBLE_EQ(12345.0, source->tryGetDouble("simple_number").value());
    EXPECT_DOUBLE_EQ(19.86, source->tryGetDouble("simple_double_number").value());
    EXPECT_DOUBLE_EQ(19.86, source->tryGetDouble("simple_double_number").value());
    EXPECT_DOUBLE_EQ(-19.86, source->tryGetDouble("simple_double_string").value());
    EXPECT_DOUBLE_EQ(6.25, source->tryGetDouble("simple_nested_math").value());
}

TEST(LuaTree, Number)
{
    auto source = loadTestFile();

    EXPECT_EQ(12345, source->tryGetNumber("simple_number").value());
    EXPECT_EQ(12345, source->tryGetNumber("simple_number").value());

    EXPECT_EQ(12345, source->tryGetUnsignedNumber("simple_number").value());
    EXPECT_EQ(12345, source->tryGetUnsignedNumber("simple_number").value());

    EXPECT_EQ(12345, source->tryGetNumber("simple_number").value_or(0));
    EXPECT_EQ(12345, source->tryGetUnsignedNumber("simple_number").value_or(0));

    EXPECT_EQ(20, source->tryGetNumber("simple_double_number").value());
    EXPECT_EQ(-20, source->tryGetNumber("simple_double_string").value());

    EXPECT_EQ(20, source->tryGetUnsignedNumber("simple_double_number").value());
    EXPECT_EQ(
        static_cast<uint64_t>(-20), source->tryGetUnsignedNumber("simple_double_string").value());

    EXPECT_FALSE(source->tryGetNumber("this_key_should_not_exist"));
    EXPECT_FALSE(source->tryGetUnsignedNumber("this_key_should_not_exist"));
}

TEST(LuaTree, String)
{
    auto source = loadTestFile();

    EXPECT_FALSE(source->tryGetString("this_key_should_not_exist").has_value());
    EXPECT_TRUE(source->tryGetString("simple_string").has_value());

    EXPECT_EQ("Hello, Lua!", source->tryGetString("simple_string").value());

    EXPECT_EQ("12345", source->tryGetString("simple_number").value());
    EXPECT_EQ("12345", source->tryGetString("simple_number").value());

    EXPECT_EQ("1", source->tryGetString("simple_yes").value());
    EXPECT_EQ("0", source->tryGetString("simple_no").value());
    EXPECT_EQ("4", source->tryGetString("simple_func").value());
    EXPECT_EQ("6", source->tryGetString("simple_nested_func").value());
}

TEST(LuaTree, Child)
{
    auto source = loadTestFile();
    EXPECT_FALSE(source->tryGetChild("this_key_should_not_exist"));
    EXPECT_TRUE(source->tryGetChild("user"));
    auto userTree = source->tryGetChild("user");
    ASSERT_TRUE(userTree);
    EXPECT_EQ("Vlad Lazarenko", userTree->tryGetString("name").value());
    EXPECT_EQ("vlad@lazarenko.me", userTree->tryGetString("email").value());
}
