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
#include <memory>
#include <optional>
#include <stdexcept>
#include <type_traits>

extern "C" {
struct lua_State;
};

namespace conf {
namespace internal {

template <typename T, typename... O>
constexpr static auto is_any_v = (std::is_same_v<T, O> || ...);

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

class LuaReference final {
public:
    LuaReference();

    explicit LuaReference(std::shared_ptr<LuaState> state) noexcept;

    LuaReference(LuaReference&& other) noexcept;

    ~LuaReference();

    LuaReference(const LuaReference&) = delete;
    LuaReference& operator=(const LuaReference&) = delete;

    void reset() noexcept;

    void set();

    void push() const;

    LuaState* operator->() const noexcept { return state_.get(); }

    operator lua_State*() const noexcept { return *state_; }

    std::shared_ptr<LuaState> getState() const noexcept { return state_; }

private:
    std::shared_ptr<LuaState> state_;
    int ref_;
};

class LuaStackGuard {
public:
    explicit LuaStackGuard(const LuaReference& ref) noexcept;

    ~LuaStackGuard() noexcept;

    LuaStackGuard(const LuaStackGuard&) = delete;
    LuaStackGuard& operator=(const LuaStackGuard&) = delete;

private:
    lua_State* state_;
    int top_;
};

} // namespace internal

class LuaTree {
public:
    static std::shared_ptr<LuaTree> loadFile(const std::filesystem::path& file);

    ~LuaTree();

    LuaTree(const LuaTree&) = delete;
    LuaTree& operator=(const LuaTree&) = delete;

    [[nodiscard]] std::shared_ptr<LuaTree> tryGetChild(std::string_view name) const;

    [[nodiscard]] std::shared_ptr<LuaTree> getChild(std::string_view name) const;

    [[nodiscard]] decltype(auto) operator[](std::string_view name) const { return getChild(name); }

    [[nodiscard]] std::optional<bool> tryGetBoolean(std::string_view name) const;

    [[nodiscard]] bool getBoolean(std::string_view name) const { return get<bool>(name); }

    [[nodiscard]] std::optional<double> tryGetDouble(std::string_view name) const;

    [[nodiscard]] double getDouble(std::string_view name) const { return get<double>(name); }

    [[nodiscard]] std::optional<std::string> tryGetString(std::string_view name) const;

    [[nodiscard]] std::string getString(std::string_view name) const
    {
        return get<std::string>(name);
    }

    template <typename T>
    [[nodiscard]] std::optional<T> tryGet(std::string_view name) const
    {
        static_assert(internal::is_any_v<T, std::string, double, bool>, "Type not supported");
        if constexpr (std::is_same_v<T, std::string>) {
            return tryGetString(name);
        } else if constexpr (std::is_same_v<T, double>) {
            return tryGetDouble(name);
        } else if constexpr (std::is_same_v<T, bool>) {
            return tryGetBoolean(name);
        }
    }

    template <typename T>
    [[nodiscard]] T get(std::string_view name) const
    {
        auto result = tryGet<T>(name);
        if (!result.has_value())
            raiseKeyNotFound(name);
        return std::move(result).value();
    }

private:
    struct SharedConstructTag final {
    };

    [[nodiscard]] int loadField(std::string_view name) const noexcept;

    [[noreturn]] static void raiseKeyNotFound(std::string_view name);

    internal::LuaReference ref_;

public:
    explicit LuaTree(SharedConstructTag, internal::LuaReference&& ref) noexcept;

    explicit LuaTree(SharedConstructTag, std::shared_ptr<internal::LuaState> ref) noexcept;
};

} // namespace conf

#endif
