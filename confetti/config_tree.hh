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

#ifndef CONFETTI_CONFIG_TREE_HH
#define CONFETTI_CONFIG_TREE_HH

#include "config_source.hh"
#include "internal/type_traits.hh"
#include <compare>
#include <filesystem>
#include <tuple>
#include <vector>

namespace confetti {

class ConfigTree;

template <typename T>
class ConfigValue;

class ConfigPath final {
public:
    [[nodiscard]] static constexpr std::string_view getDefaultSeaparators() noexcept
    {
        return "/.\\";
    }

    constexpr ConfigPath()
        : path_{}
        , sep_{getDefaultSeaparators()}
    {
    }

    explicit constexpr ConfigPath(
        std::string_view path, std::string_view separators = getDefaultSeaparators()) noexcept
        : path_{path}
        , sep_{separators}
    {
    }

    [[nodiscard]] ConfigTree getChildNode(ConfigTree tree) const;

    [[nodiscard]] std::tuple<ConfigTree, std::string_view> getValueNode(ConfigTree tree) const;

    [[nodiscard]] constexpr std::string_view getPathString() const noexcept { return path_; }

private:
    template <typename R, typename C>
    [[nodiscard]] R findNodeImpl(ConfigTree tree, const C& handler) const;

    std::string_view path_;
    std::string_view sep_;
};

class ConfigTree final {
public:
    struct EndIterator final {
    };

    template <typename T>
    class ValueIterator final {
    public:
        explicit ValueIterator(const ConfigTree& tree) noexcept
            : tree_{&tree}
            , index_{0}
        {
        }

        bool operator!=(EndIterator) const noexcept
        {
            return tree_->source_ && tree_->source_->hasValueAt(index_);
        }

        ValueIterator& operator++()
        {
            ++index_;
            return *this;
        }

        decltype(auto) operator*() const { return tree_->get<T>(index_); }

    private:
        const ConfigTree* tree_;
        int index_;
    };

    class ChildIterator;

    template <typename IteratorType>
    class Range final {
    public:
        explicit Range(IteratorType it) noexcept
            : it_{std::move(it)}
        {
        }

        [[nodiscard]] IteratorType begin() const noexcept { return it_; }

        static constexpr auto end() noexcept { return EndIterator{}; }

        // NOLINTNEXTLINE(google-explicit-constructor)
        operator std::vector<decltype(*std::declval<IteratorType>())>() const
        {
            std::vector<decltype(*std::declval<IteratorType>())> result;
            for (auto it = begin(); it != end(); ++it) {
                result.emplace_back(*it);
            }
            return result;
        }

    private:
        IteratorType it_;
    };

    ConfigTree() noexcept = default;

    explicit ConfigTree(ConfigSourcePointer source) noexcept
        : source_{std::move(source)}
    {
    }

    ConfigTree(const ConfigTree&) noexcept = default;
    ConfigTree(ConfigTree&&) noexcept = default;

    ~ConfigTree() = default;

    ConfigTree& operator=(const ConfigTree&) noexcept = default;
    ConfigTree& operator=(ConfigTree&&) noexcept = default;

    explicit operator bool() const noexcept { return source_.get() != nullptr; }

    auto operator<=>(const ConfigTree& other) const noexcept
    {
        return source_.get() <=> other.source_.get();
    }

    template <typename K>
    [[nodiscard]] ConfigTree tryGetChild(K key) const
    {
        ConfigSourcePointer result;
        if (source_)
            result = source_->tryGetChild(key);
        return ConfigTree{std::move(result)};
    }

    [[nodiscard]] ConfigTree tryGetChild(const ConfigPath& path) const
    {
        return path.getChildNode(*this);
    }

    template <typename K>
    [[nodiscard]] decltype(auto) getChild(K key) const
    {
        decltype(auto) child = tryGetChild(key);
        if (!child)
            noSuchChild(key);
        return child;
    }

    template <typename K>
    [[nodiscard]] decltype(auto) operator[](K key) const
    {
        return getChild(key);
    }

    template <typename K>
    [[nodiscard]] decltype(auto) tryGetBoolean(K key) const
    {
        return tryGet(&ConfigSource::tryGetBoolean, key);
    }

    template <typename K>
    [[nodiscard]] decltype(auto) getBoolean(K key) const
    {
        return get<bool>(key);
    }

    template <typename K>
    [[nodiscard]] decltype(auto) tryGetNumber(K key) const
    {
        return tryGet(&ConfigSource::tryGetNumber, key);
    }

    template <typename K>
    [[nodiscard]] decltype(auto) getNumber(K key) const
    {
        return get<int64_t>(key);
    }

    template <typename K>
    [[nodiscard]] decltype(auto) tryGetUnsignedNumber(K key) const
    {
        return tryGet(&ConfigSource::tryGetUnsignedNumber, key);
    }

    template <typename K>
    [[nodiscard]] decltype(auto) getUnsignedNumber(K key) const
    {
        return get<uint64_t>(key);
    }

    template <typename K>
    [[nodiscard]] decltype(auto) tryGetDouble(K key) const
    {
        return tryGet(&ConfigSource::tryGetDouble, key);
    }

    template <typename K>
    [[nodiscard]] decltype(auto) getDouble(K key) const
    {
        return get<double>(key);
    }

    template <typename K>
    [[nodiscard]] decltype(auto) tryGetString(K key) const
    {
        return tryGet(&ConfigSource::tryGetString, key);
    }

    template <typename K>
    [[nodiscard]] decltype(auto) getString(K key) const
    {
        return get<std::string>(key);
    }

    template <typename T, typename K>
    [[nodiscard]] std::optional<T> tryGet(K key) const
    {
        static_assert(internal::is_any_of_v<T, std::string, bool, double, int16_t, uint16_t,
                          int32_t, uint32_t, int64_t, uint64_t>,
            "Type not supported");
        if (source_) {
            if constexpr (std::is_same_v<T, std::string>) {
                return source_->tryGetString(key);
            } else if constexpr (std::is_same_v<T, bool>) {
                return source_->tryGetBoolean(key);
            } else if constexpr (std::is_same_v<T, double>) {
                return source_->tryGetDouble(key);
            } else if constexpr (internal::is_any_of_v<T, int16_t, int32_t, int64_t>) {
                return source_->tryGetNumber(key);
            } else if constexpr (internal::is_any_of_v<T, uint16_t, uint32_t, uint64_t>) {
                return source_->tryGetUnsignedNumber(key);
            }
        }

        // GCC 10.2.0 gives false positive around optional below in optimizing build:
        // ‘<anonymous>’ may be used uninitialized in this function [-Werror=maybe-uninitialized]
        // So disable the warning as a workaround. See also:
        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80635
#if defined(__GNUC__) && !defined(__clang__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
        return std::optional<T>{};
#if defined(__GNUC__) && !defined(__clang__)
#    pragma GCC diagnostic pop
#endif
    }

    template <typename T>
    [[nodiscard]] std::optional<T> tryGet(const ConfigPath& path) const
    {
        auto [tree, key] = path.getValueNode(*this);
        return tree.tryGet<T>(key);
    }

    template <typename T, typename K>
    [[nodiscard]] T get(K key) const
    {
        auto result = tryGet<T>(key);
        if (!result.has_value())
            noSuchKey(key);
        return *std::move(result);
    }

    template <typename T, typename K>
    [[nodiscard]] T get(K key, T&& default_value) const
    {
        auto result = tryGet<T>(key);
        if (!result.has_value())
            return std::forward<T>(default_value);
        return *std::move(result);
    }

    template <typename T, typename K, typename U>
    [[nodiscard]] T get(K key, U&& default_value) const
    {
        auto result = tryGet<T>(key);
        if (!result.has_value()) {
            if constexpr (std::is_invocable<U>::value) {
                return default_value();
            } else {
                return std::forward<U>(default_value);
            }
        }
        return *std::move(result);
    }

    template <typename T>
    [[nodiscard]] T at(int index) const
    {
        return get<T>(index);
    }

    template <typename T>
    [[nodiscard]] decltype(auto) values() const
    {
        return Range<ValueIterator<T>>{ValueIterator<T>{*this}};
    }

    [[nodiscard]] decltype(auto) children() const;

    [[nodiscard]] ConfigValue<int> get(int index) const;

    [[nodiscard]] ConfigValue<std::string> get(std::string_view name) const;

    [[nodiscard]] ConfigValue<std::string> get(const ConfigPath& path) const;

    [[nodiscard]] static ConfigTree loadLuaFile(const std::filesystem::path& file);

    [[nodiscard]] static ConfigTree loadIniFile(const std::filesystem::path& file);

    [[nodiscard]] static ConfigTree loadFile(const std::filesystem::path& file);

private:
    template <typename R>
    [[nodiscard]] R tryGet(R (ConfigSource::*getter)(int) const, int key) const
    {
        return source_ ? (source_.get()->*getter)(key) : R{};
    }

    template <typename R>
    [[nodiscard]] R tryGet(
        R (ConfigSource::*getter)(std::string_view) const, std::string_view key) const
    {
        return source_ ? (source_.get()->*getter)(key) : R{};
    }

    template <typename R>
    [[nodiscard]] R tryGet(
        R (ConfigSource::*getter)(std::string_view) const, const ConfigPath& path) const
    {
        auto [tree, key] = path.getValueNode(*this);
        return tree.source_ ? (tree.source_.get()->*getter)(key) : R{};
    }

    [[noreturn]] static void noSuchChild(int index);

    [[noreturn]] static void noSuchChild(std::string_view name);

    [[noreturn]] static void noSuchChild(const ConfigPath& path)
    {
        noSuchChild(path.getPathString());
    }

    [[noreturn]] static void noSuchKey(int index);

    [[noreturn]] static void noSuchKey(std::string_view name);

    [[noreturn]] static void noSuchKey(const ConfigPath& path) { noSuchKey(path.getPathString()); }

    ConfigSourcePointer source_;
};

template <typename K>
class ConfigValue final {
public:
    template <typename U>
    explicit ConfigValue(ConfigTree tree, U key)
        : tree_{std::move(tree)}
        , key_{std::move(key)}
    {
    }

    template <typename T>
    operator T() const // NOLINT(google-explicit-constructor)
    {
        static_assert(!std::is_pointer_v<T>, "Cannot convert values to pointers");

        if constexpr (std::is_same_v<T, ConfigTree>)
            return tree_.getChild(key_);
        else
            return tree_.get<T>(key_);
    }

    template <typename T>
    operator std::optional<T>() const // NOLINT(google-explicit-constructor)
    {
        static_assert(!std::is_pointer_v<T>, "Cannot convert values to optional pointers");

        if constexpr (std::is_same_v<T, ConfigTree>)
            return tree_.tryGetChild(key_);
        else
            return tree_.tryGet<T>(key_);
    }

private:
    ConfigTree tree_;
    K key_;
};

inline ConfigValue<int> ConfigTree::get(int index) const { return ConfigValue<int>{*this, index}; }

inline ConfigValue<std::string> ConfigTree::get(std::string_view name) const
{
    return ConfigValue<std::string>{*this, name};
}

inline ConfigValue<std::string> ConfigTree::get(const ConfigPath& path) const
{
    auto [tree, key] = path.getValueNode(*this);
    return tree.get(key);
}

class ConfigTree::ChildIterator final {
public:
    explicit ChildIterator(const ConfigTree& tree) noexcept
        : tree_{&tree}
        , child_{tree_->tryGetChild(0)}
        , index_{1}
    {
    }

    bool operator!=(EndIterator) const noexcept { return static_cast<bool>(child_); }

    ChildIterator& operator++()
    {
        child_ = tree_->tryGetChild(index_++);
        return *this;
    }

    decltype(auto) operator*() const { return child_; }

private:
    const ConfigTree* tree_;
    ConfigTree child_;
    int index_;
};

inline decltype(auto) ConfigTree::children() const
{
    return Range<ChildIterator>{ChildIterator{*this}};
}

template <typename R, typename C>
inline R ConfigPath::findNodeImpl(ConfigTree tree, const C& handler) const
{
    std::string_view::size_type begin = 0;
    for (;;) {
        const auto end = path_.find_first_of(sep_, begin);
        if (end == std::string_view::npos)
            return handler(std::move(tree), path_.substr(begin));
        tree = tree.tryGetChild(path_.substr(begin, end - begin));
        if (!tree)
            break;
        begin = end + 1;
    }
    return {};
}

inline std::tuple<ConfigTree, std::string_view> ConfigPath::getValueNode(ConfigTree tree) const
{
    return findNodeImpl<std::tuple<ConfigTree, std::string_view>>(std::move(tree),
        [](auto node, auto key) noexcept -> std::tuple<ConfigTree, std::string_view> {
            return {std::move(node), key};
        });
}

inline ConfigTree ConfigPath::getChildNode(ConfigTree tree) const
{
    return findNodeImpl<ConfigTree>(std::move(tree),
        [](auto node, auto key) noexcept { return std::move(node).tryGetChild(key); });
}

namespace literals {

inline ConfigPath operator""_cp(const char* data, size_t size) noexcept
{
    return ConfigPath{std::string_view{data, size}};
}

} // namespace literals

} // namespace confetti

#endif // CONFETTI_CONFIG_TREE_HH
