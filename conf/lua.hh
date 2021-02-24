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
#include <filesystem>
#include <optional>
#include <stdexcept>

extern "C" {
struct lua_State;
};

namespace conf {
namespace internal {

class LuaStackGuard {
public:
    explicit LuaStackGuard(lua_State* state) noexcept;

    ~LuaStackGuard() noexcept;

    LuaStackGuard(const LuaStackGuard&) = delete;
    LuaStackGuard& operator=(const LuaStackGuard&) = delete;

private:
    lua_State* state_;
    int top_;
};

class LuaException final : public std::runtime_error {
public:
    explicit LuaException(const char* error_message);

    explicit LuaException(lua_State* state);

    ~LuaException() override;

    [[noreturn]] static int raise(lua_State* state);
};

class LuaState final {
public:
    LuaState();

    LuaState(const LuaState&) = delete;

    LuaState(LuaState&& other) noexcept;

    ~LuaState();

    LuaState& operator=(const LuaState&) = delete;

    LuaState& operator=(LuaState&& other) noexcept;

    void close() noexcept;

    operator lua_State*() const noexcept { return state_; }

    explicit operator bool() const noexcept { return state_ != nullptr; }

    [[noreturn]] void raise() const;

    void check(int result) const;

    void runFile(const std::filesystem::path& file);

    void runCode(std::string_view code);

private:
    lua_State* state_;

    /// @see http://www.lua.org/manual/5.1/manual.html#lua_Alloc
    static void* alloc(void* aux, void* ptr, size_t osize, size_t nsize) noexcept;

    void run();
};

} // namespace internal

class LuaTree {
public:
    LuaTree();

    void loadFile(const std::filesystem::path& file);

    [[nodiscard]] std::optional<std::string> tryGetString(std::string_view name) const;

    [[nodiscard]] std::string getString(std::string_view name) const
    {
        auto result = tryGetString(name);
        if (!result.has_value())
            raiseKeyNotFound(name);
        return result.value();
    }

private:
    [[nodiscard]] int loadField(std::string_view name) const noexcept;

    [[noreturn]] static void raiseKeyNotFound(std::string_view name);

    internal::LuaState state_;
};

} // namespace conf

#endif
