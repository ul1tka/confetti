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

#ifndef CONF_TREE_HH
#define CONF_TREE_HH

#include "internal/type_traits.hh"
#include "source.hh"
#include <compare>

namespace conf {

class ConfigTree final {
public:
    ConfigTree() noexcept = default;

    explicit ConfigTree(SourcePtr source) noexcept
        : source_{std::move(source)}
    {
    }

    ConfigTree(const ConfigTree&) = default;
    ConfigTree(ConfigTree&&) = default;

    ~ConfigTree() = default;

    ConfigTree& operator=(const ConfigTree&) = default;
    ConfigTree& operator=(ConfigTree&&) = default;

    explicit operator bool() const noexcept { return source_.get() != nullptr; }

    auto operator<=>(const ConfigTree& other) const noexcept
    {
        return source_.get() <=> other.source_.get();
    }

    [[nodiscard]] ConfigTree tryGetChild(std::string_view name) const
    {
        SourcePtr result;
        if (source_)
            result = source_->tryGetChild(name);
        return ConfigTree{std::move(result)};
    }

    [[nodiscard]] decltype(auto) getChild(std::string_view name) const
    {
        decltype(auto) child = tryGetChild(name);
        if (!child)
            noSuchChild(name);
        return child;
    }

    [[nodiscard]] decltype(auto) operator[](std::string_view name) const { return getChild(name); }

    [[nodiscard]] decltype(auto) tryGetBoolean(std::string_view name) const
    {
        return tryGet(&Source::tryGetBoolean, name);
    }

    [[nodiscard]] decltype(auto) getBoolean(std::string_view name) const { return get<bool>(name); }

    [[nodiscard]] decltype(auto) tryGetNumber(std::string_view name) const
    {
        return tryGet(&Source::tryGetNumber, name);
    }

    [[nodiscard]] decltype(auto) getNumber(std::string_view name) const
    {
        return get<int64_t>(name);
    }

    [[nodiscard]] decltype(auto) tryGetUnsignedNumber(std::string_view name) const
    {
        return tryGet(&Source::tryGetUnsignedNumber, name);
    }

    [[nodiscard]] decltype(auto) getUnsignedNumber(std::string_view name) const
    {
        return get<uint64_t>(name);
    }

    [[nodiscard]] decltype(auto) tryGetDouble(std::string_view name) const
    {
        return tryGet(&Source::tryGetDouble, name);
    }

    [[nodiscard]] decltype(auto) getDouble(std::string_view name) const
    {
        return get<double>(name);
    }

    [[nodiscard]] decltype(auto) tryGetString(std::string_view name) const
    {
        return tryGet(&Source::tryGetString, name);
    }

    [[nodiscard]] decltype(auto) getString(std::string_view name) const
    {
        return get<std::string>(name);
    }

    template <typename T>
    [[nodiscard]] std::optional<T> tryGet(std::string_view name) const
    {
        static_assert(internal::is_any_v<T, std::string, bool, double, int64_t, uint64_t>,
            "Type not supported");
        if (!source_)
            return {};
        if constexpr (std::is_same_v<T, std::string>) {
            return source_->tryGetString(name);
        } else if constexpr (std::is_same_v<T, bool>) {
            return source_->tryGetBoolean(name);
        } else if constexpr (std::is_same_v<T, double>) {
            return source_->tryGetDouble(name);
        } else if constexpr (std::is_same_v<T, int64_t>) {
            return source_->tryGetNumber(name);
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            return source_->tryGetUnsignedNumber(name);
        }
    }

    template <typename T>
    [[nodiscard]] T get(std::string_view name) const
    {
        auto result = tryGet<T>(name);
        if (!result.has_value())
            noSuchKey(name);
        return *std::move(result);
    }

private:
    template <typename R>
    [[nodiscard]] R tryGet(R (Source::*getter)(std::string_view) const, std::string_view name) const
    {
        return source_ ? (source_.get()->*getter)(name) : R{};
    }

    [[noreturn]] static void noSuchChild(std::string_view name);

    [[noreturn]] static void noSuchKey(std::string_view name);

    SourcePtr source_;
};

} // namespace conf

#endif
