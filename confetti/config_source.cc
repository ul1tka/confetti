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

#include "config_source.hh"
#include <cmath>

namespace confetti {

ConfigSource::~ConfigSource() = default;

template <typename T>
std::optional<int64_t> ConfigSource::tryGetNumberT(T key) const
{
    std::optional<int64_t> result;
    if (auto number = tryGetDouble(key)) {
        result.emplace(std::llround(*number));
    }
    return result;
}

std::optional<int64_t> ConfigSource::tryGetNumber(int index) const { return tryGetNumberT(index); }

std::optional<int64_t> ConfigSource::tryGetNumber(std::string_view name) const
{
    return tryGetNumberT(name);
}

template <typename T>
std::optional<uint64_t> ConfigSource::tryGetUnsignedNumberT(T key) const
{
    std::optional<uint64_t> result;
    if (auto number = tryGetDouble(key)) {
        result.emplace(static_cast<uint64_t>(std::llround(*number)));
    }
    return result;
}

std::optional<uint64_t> ConfigSource::tryGetUnsignedNumber(int index) const
{
    return tryGetUnsignedNumberT(index);
}

std::optional<uint64_t> ConfigSource::tryGetUnsignedNumber(std::string_view name) const
{
    return tryGetUnsignedNumberT(name);
}

} // namespace confetti
