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

#include "source.hh"
#include <type_traits>

namespace conf {

namespace internal {

template <typename T, typename... O>
constexpr static auto is_any_v = (std::is_same_v<T, O> || ...);

} // namespace internal

class Tree final {
public:
    Tree() noexcept = default;

    explicit Tree(SourcePtr source) noexcept
        : source_{std::move(source)}
    {
    }

    Tree(const Tree&) = default;
    Tree(Tree&&) = default;

    ~Tree() = default;

    Tree& operator=(const Tree&) = default;
    Tree& operator=(Tree&&) = default;

    explicit operator bool() const noexcept { return source_.get() != nullptr; }

    [[nodiscard]] Tree tryGetChild(std::string_view name) const
    {
        SourcePtr result;
        if (source_)
            result = source_->tryGetChild(name);
        return Tree{std::move(result)};
    }

    [[nodiscard]] Tree getChild(std::string_view name) const
    {
        auto child = tryGetChild(name);
        if (!child)
            noSuchChild(name);
        return child;
    }

    [[nodiscard]] Tree operator[](std::string_view name) const { return getChild(name); }

    [[nodiscard]] std::optional<bool> tryGetBoolean(std::string_view name) const
    {
        std::optional<bool> result;
        if (source_)
            result = source_->tryGetBoolean(name);
        return result;
    }

    [[nodiscard]] std::optional<double> tryGetDouble(std::string_view name) const
    {
        std::optional<double> result;
        if (source_)
            result = source_->tryGetDouble(name);
        return result;
    }

    [[nodiscard]] std::optional<std::string> tryGetString(std::string_view name) const
    {
        std::optional<std::string> result;
        if (source_)
            result = source_->tryGetString(name);
        return result;
    }

    template <typename T>
    [[nodiscard]] std::optional<T> tryGet(std::string_view name) const
    {
        static_assert(internal::is_any_v<T, std::string, double, bool>, "Type not supported");
        if (!source_)
            return {};
        if constexpr (std::is_same_v<T, std::string>) {
            return source_->tryGetString(name);
        } else if constexpr (std::is_same_v<T, double>) {
            return source_->tryGetDouble(name);
        } else if constexpr (std::is_same_v<T, bool>) {
            return source_->tryGetBoolean(name);
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

    [[nodiscard]] bool getBoolean(std::string_view name) const { return get<bool>(name); }

    [[nodiscard]] double getDouble(std::string_view name) const { return get<double>(name); }

    [[nodiscard]] std::string getString(std::string_view name) const
    {
        return get<std::string>(name);
    }

private:
    [[noreturn]] static void noSuchChild(std::string_view name);

    [[noreturn]] static void noSuchKey(std::string_view name);

    SourcePtr source_;
};

} // namespace conf

#endif