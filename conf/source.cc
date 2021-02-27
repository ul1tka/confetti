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

#include "source.hh"
#include <cmath>

namespace conf {

Source::~Source() = default;

std::optional<int64_t> Source::tryGetNumber(std::string_view name) const
{
    std::optional<int64_t> result;
    if (auto number = tryGetDouble(name)) {
        result.emplace(std::llround(*number));
    }
    return result;
}

std::optional<uint64_t> Source::tryGetUnsignedNumber(std::string_view name) const
{
    std::optional<uint64_t> result;
    if (auto number = tryGetDouble(name)) {
        result.emplace(static_cast<uint64_t>(std::llround(*number)));
    }
    return result;
}

} // namespace conf
