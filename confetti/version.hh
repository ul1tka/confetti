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

#ifndef CONFETTI_VERSION_HH
#define CONFETTI_VERSION_HH

#include <confetti/internal/version.hh>

#include <compare>
#include <iosfwd>

#define CONFETTI_VERSION_MAJOR (CONFETTI_VERSION / 1000000)
#define CONFETTI_VERSION_MINOR (CONFETTI_VERSION / 1000 % 1000)
#define CONFETTI_VERSION_PATCH (CONFETTI_VERSION % 1000)

namespace confetti {

class Version final {
public:
    using ValueType = unsigned int;

    constexpr explicit Version(ValueType value) noexcept
        : value_{value}
    {
    }

    [[nodiscard]] constexpr ValueType getValue() const noexcept { return value_; }

    [[nodiscard]] constexpr ValueType getMajor() const noexcept { return value_ / 1000000; }

    [[nodiscard]] constexpr ValueType getMinor() const noexcept { return value_ / 1000 % 1000; }

    [[nodiscard]] constexpr ValueType getPatch() const noexcept { return value_ % 1000; }

    [[nodiscard]] constexpr auto operator<=>(const Version& other) const noexcept = default;

private:
    ValueType value_;
};

[[nodiscard]] constexpr Version getVersion() noexcept { return Version{CONFETTI_VERSION}; }

[[nodiscard]] Version getRuntimeVersion() noexcept;

std::ostream& operator<<(std::ostream& out, const Version& version);

} // namespace confetti

#endif // CONFETTI_VERSION_HH
