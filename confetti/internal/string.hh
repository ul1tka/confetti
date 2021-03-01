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

#ifndef CONFETTI_INTERNAL_STRING_HH
#define CONFETTI_INTERNAL_STRING_HH

#include <algorithm>
#include <cctype>
#include <string_view>

namespace confetti::internal {

inline bool strCaseEq(std::string_view lhs, std::string_view rhs) noexcept
{
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
        [](auto x, auto y) noexcept { return std::tolower(x) == std::tolower(y); });
}

template <typename... T>
inline bool strCaseIsAnyOf(std::string_view lhs, T... rhs) noexcept
{
    return (strCaseEq(lhs, rhs) || ...);
}

} // namespace confetti::internal

#endif // CONFETTI_INTERNAL_STRING_HH
