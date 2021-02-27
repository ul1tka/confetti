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

#ifndef CONFETTI_CONFIG_SOURCE_HH
#define CONFETTI_CONFIG_SOURCE_HH

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

namespace confetti {

class ConfigSource;

using ConfigSourcePointer = std::shared_ptr<ConfigSource>;

class ConfigSource {
public:
    ConfigSource() noexcept = default;

    virtual ~ConfigSource();

    ConfigSource(const ConfigSource&) = delete;
    ConfigSource& operator=(const ConfigSource&) = delete;

    [[nodiscard]] virtual bool hasValueAt(int index) const = 0;

    [[nodiscard]] virtual ConfigSourcePointer tryGetChild(int index) const = 0;

    [[nodiscard]] virtual ConfigSourcePointer tryGetChild(std::string_view name) const = 0;

    [[nodiscard]] virtual std::optional<bool> tryGetBoolean(int index) const = 0;

    [[nodiscard]] virtual std::optional<bool> tryGetBoolean(std::string_view name) const = 0;

    [[nodiscard]] virtual std::optional<double> tryGetDouble(int index) const = 0;

    [[nodiscard]] virtual std::optional<double> tryGetDouble(std::string_view name) const = 0;

    [[nodiscard]] virtual std::optional<int64_t> tryGetNumber(int index) const;

    [[nodiscard]] virtual std::optional<int64_t> tryGetNumber(std::string_view name) const;

    [[nodiscard]] virtual std::optional<uint64_t> tryGetUnsignedNumber(int index) const;

    [[nodiscard]] virtual std::optional<uint64_t> tryGetUnsignedNumber(std::string_view name) const;

    [[nodiscard]] virtual std::optional<std::string> tryGetString(int index) const = 0;

    [[nodiscard]] virtual std::optional<std::string> tryGetString(std::string_view name) const = 0;

private:
    template <typename T>
    [[nodiscard]] std::optional<int64_t> tryGetNumberT(T key) const;

    template <typename T>
    [[nodiscard]] std::optional<uint64_t> tryGetUnsignedNumberT(T key) const;
};

} // namespace confetti

#endif // CONFETTI_CONFIG_SOURCE_HH
