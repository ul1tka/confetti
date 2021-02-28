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

namespace confetti {

template <typename T>
class ConfigValue;

class ConfigTree final {
public:
    struct EndIterator final {
    };

    template <typename T>
    class Iterator final {
    public:
        explicit Iterator(const ConfigTree& tree) noexcept
            : tree_{&tree}
            , index_{0}
        {
        }

        bool operator!=(EndIterator) const noexcept
        {
            return tree_->source_ && tree_->source_->hasValueAt(index_);
        }

        Iterator& operator++()
        {
            ++index_;
            return *this;
        }

        decltype(auto) operator*() const { return tree_->get<T>(index_); }

    private:
        const ConfigTree* tree_;
        int index_;
    };

    template <typename T>
    class Range {
    public:
        explicit Range(Iterator<T> it) noexcept
            : it_{it}
        {
        }

        Iterator<T> begin() const noexcept { return it_; }

        static constexpr auto end() noexcept { return EndIterator{}; }

    private:
        Iterator<T> it_;
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
        static_assert(internal::is_any_of_v<T, std::string, bool, double, int32_t, uint32_t,
                          int64_t, uint64_t>,
            "Type not supported");
        if (source_) {
            if constexpr (std::is_same_v<T, std::string>) {
                return source_->tryGetString(key);
            } else if constexpr (std::is_same_v<T, bool>) {
                return source_->tryGetBoolean(key);
            } else if constexpr (std::is_same_v<T, double>) {
                return source_->tryGetDouble(key);
            } else if constexpr (internal::is_any_of_v<T, int32_t, int64_t>) {
                return source_->tryGetNumber(key);
            } else if constexpr (internal::is_any_of_v<T, uint32_t, uint64_t>) {
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

    template <typename T, typename K>
    [[nodiscard]] T get(K key) const
    {
        auto result = tryGet<T>(key);
        if (!result.has_value())
            noSuchKey(key);
        return *std::move(result);
    }

    template <typename T>
    [[nodiscard]] T at(int index) const
    {
        return get<T>(index);
    }

    template <typename T>
    Range<T> values() const
    {
        return Range<T>{Iterator<T>{*this}};
    }

    [[nodiscard]] ConfigValue<int> get(int index) const;

    [[nodiscard]] ConfigValue<std::string> get(std::string_view name) const;

    [[nodiscard]] static ConfigTree loadLuaFile(const std::filesystem::path& file);

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

    [[noreturn]] static void noSuchChild(int index);

    [[noreturn]] static void noSuchChild(std::string_view name);

    [[noreturn]] static void noSuchKey(int index);

    [[noreturn]] static void noSuchKey(std::string_view name);

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

} // namespace confetti

#endif // CONFETTI_CONFIG_TREE_HH
