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
#include <gtest/gtest.h>
#include <type_traits>

static_assert(std::is_polymorphic_v<confetti::ConfigSource>);

static_assert(!std::is_copy_constructible_v<confetti::ConfigSource>);
static_assert(!std::is_copy_assignable_v<confetti::ConfigSource>);

static_assert(!std::is_move_constructible_v<confetti::ConfigSource>);
static_assert(!std::is_move_assignable_v<confetti::ConfigSource>);

namespace {

class Source final : public confetti::ConfigSource {
public:
    using ConfigSource::ConfigSource;

    ~Source() override = default;

    [[nodiscard]] bool hasValueAt(int) const override { return false; }

    [[nodiscard]] confetti::ConfigSourcePointer tryGetChild(int) const override { return {}; }

    [[nodiscard]] confetti::ConfigSourcePointer tryGetChild(std::string_view) const override
    {
        return tryGetChild(0);
    }

    [[nodiscard]] std::optional<bool> tryGetBoolean(int) const override { return {}; }

    [[nodiscard]] std::optional<bool> tryGetBoolean(std::string_view) const override
    {
        return tryGetBoolean(0);
    }

    [[nodiscard]] std::optional<double> tryGetDouble(int) const override { return 19.86; }

    [[nodiscard]] std::optional<double> tryGetDouble(std::string_view) const override
    {
        return tryGetDouble(0);
    }

    [[nodiscard]] std::optional<std::string> tryGetString(int) const override { return {}; }

    [[nodiscard]] std::optional<std::string> tryGetString(std::string_view) const override
    {
        return tryGetString(0);
    }

    [[nodiscard]] std::vector<std::string> getKeyList() const override { return {}; }
};

}; // namespace

TEST(ConfigSource, IntFromDouble)
{
    Source source;
    EXPECT_FALSE(source.hasValueAt(0));

    EXPECT_FALSE(source.tryGetChild(0));
    EXPECT_FALSE(source.tryGetChild(""));

    EXPECT_FALSE(source.tryGetBoolean(0).has_value());
    EXPECT_FALSE(source.tryGetBoolean("").has_value());

    EXPECT_DOUBLE_EQ(19.86, source.tryGetDouble(0).value());
    EXPECT_DOUBLE_EQ(19.86, source.tryGetDouble("").value());

    EXPECT_FALSE(source.tryGetString(0).has_value());
    EXPECT_FALSE(source.tryGetString("").has_value());

    EXPECT_EQ(20, source.tryGetNumber("").value());
    EXPECT_EQ(20, source.tryGetUnsignedNumber("").value());
}
