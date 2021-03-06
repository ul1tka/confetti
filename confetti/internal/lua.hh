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

#ifndef CONFETTI_INTERNAL_LUA_HH
#define CONFETTI_INTERNAL_LUA_HH

#include "../config_source.hh"
#include <cstddef>
#include <filesystem>
#include <stdexcept>

extern "C" {
struct lua_State;
};

namespace confetti::internal {

class LuaException final : public std::runtime_error {
    explicit LuaException(const char* message);

public:
    ~LuaException() override;

    [[noreturn]] static void raise(const char* message);

    [[noreturn]] static void raise(lua_State* state);
};

class LuaState final {
public:
    LuaState();

    LuaState(const LuaState&) = delete;

    ~LuaState();

    LuaState& operator=(const LuaState&) = delete;

    void close() noexcept;

    operator lua_State*() const noexcept { return state_; } // NOLINT(google-explicit-constructor)

    [[noreturn]] void raise() const;

    void check(int result) const;

    void run(std::string_view code);

    void run(const std::filesystem::path& file);

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

    operator lua_State*() const noexcept { return *state_; } // NOLINT(google-explicit-constructor)

    [[nodiscard]] std::shared_ptr<LuaState> getState() const noexcept { return state_; }

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

class LuaSource final : public ConfigSource {
public:
    static ConfigSourcePointer loadCode(std::string_view code);

    static ConfigSourcePointer loadFile(const std::filesystem::path& file);

    ~LuaSource() override;

    LuaSource(const LuaSource&) = delete;
    LuaSource& operator=(const LuaSource&) = delete;

    [[nodiscard]] bool hasValueAt(int index) const override;

    [[nodiscard]] ConfigSourcePointer tryGetChild(int index) const override;

    [[nodiscard]] ConfigSourcePointer tryGetChild(std::string_view name) const override;

    [[nodiscard]] std::optional<bool> tryGetBoolean(int index) const override;

    [[nodiscard]] std::optional<bool> tryGetBoolean(std::string_view name) const override;

    [[nodiscard]] std::optional<double> tryGetDouble(int index) const override;

    [[nodiscard]] std::optional<double> tryGetDouble(std::string_view name) const override;

    [[nodiscard]] std::optional<std::string> tryGetString(int index) const override;

    [[nodiscard]] std::optional<std::string> tryGetString(std::string_view name) const override;

    [[nodiscard]] std::vector<std::string> getKeyList() const override;

private:
    struct SharedConstructTag final {
    };

    template <typename T>
    static ConfigSourcePointer load(const T& source);

    [[nodiscard]] int invoke(int type) const;

    [[nodiscard]] int getField(int index) const noexcept;

    [[nodiscard]] int getField(std::string_view name) const noexcept;

    [[nodiscard]] ConfigSourcePointer tryConvertToChild(int type) const;

    [[nodiscard]] std::optional<bool> tryConvertToBoolean(int type) const;

    [[nodiscard]] std::optional<double> tryConvertToDouble(int type) const;

    [[nodiscard]] std::optional<std::string> tryConvertToString(int type) const;

    LuaReference ref_;

public:
    explicit LuaSource(SharedConstructTag, LuaReference&& ref) noexcept;

    explicit LuaSource(SharedConstructTag, std::shared_ptr<LuaState> ref) noexcept;
};

} // namespace confetti::internal

#endif // CONFETTI_INTERNAL_LUA_HH
