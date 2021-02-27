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

#include "tree.hh"
#include <gtest/gtest.h>

namespace {

struct EmptySource final : conf::Source {
    ~EmptySource() override = default;

    [[nodiscard]] conf::SourcePtr tryGetChild(std::string_view) const override { return {}; }

    [[nodiscard]] std::optional<bool> tryGetBoolean(std::string_view) const override { return {}; }

    [[nodiscard]] std::optional<double> tryGetDouble(std::string_view) const override { return {}; }

    [[nodiscard]] std::optional<std::string> tryGetString(std::string_view) const override
    {
        return {};
    }
};

struct FullSource final : conf::Source {
    ~FullSource() override = default;

    [[nodiscard]] conf::SourcePtr tryGetChild(std::string_view) const override
    {
        return std::make_shared<FullSource>();
    }

    [[nodiscard]] std::optional<bool> tryGetBoolean(std::string_view) const override
    {
        return true;
    }

    [[nodiscard]] std::optional<double> tryGetDouble(std::string_view) const override
    {
        return 19.86;
    }

    [[nodiscard]] std::optional<std::string> tryGetString(std::string_view) const override
    {
        return "Hello!";
    }
};

} // namespace

TEST(ConfigTree, EmptyTree)
{
    conf::ConfigTree tree1;

    EXPECT_FALSE(tree1);

    conf::ConfigTree tree2{tree1};

    tree1 = tree2;
    tree2 = std::move(tree1);
    EXPECT_FALSE(tree1 < tree2);
    EXPECT_FALSE(tree1 > tree2);

    conf::ConfigTree tree3{std::make_shared<EmptySource>()};

    EXPECT_TRUE(tree3);
    EXPECT_TRUE(tree3 > tree1);
    EXPECT_FALSE(tree3 < tree2);

    tree1 = std::move(tree3);

    EXPECT_TRUE(tree1);
    EXPECT_FALSE(tree3);

    tree2 = tree1;

    EXPECT_TRUE(tree1);
    EXPECT_TRUE(tree2);

    tree3 = std::move(tree2);

    EXPECT_TRUE(tree3);
    EXPECT_FALSE(tree2);
}

TEST(ConfigTree, EmptySourceGetters)
{
    auto check = [&](auto&& cfg) {
        EXPECT_FALSE(cfg.tryGetChild(""));
        EXPECT_FALSE(cfg.tryGetBoolean(""));
        EXPECT_FALSE(cfg.tryGetDouble(""));
        EXPECT_FALSE(cfg.tryGetString(""));
        EXPECT_FALSE(cfg.template tryGet<bool>(""));
        EXPECT_FALSE(cfg.template tryGet<double>(""));
        EXPECT_FALSE(cfg.template tryGet<std::string>(""));
        EXPECT_ANY_THROW((void)cfg.getChild(""));
        EXPECT_ANY_THROW((void)cfg.getBoolean(""));
        EXPECT_ANY_THROW((void)cfg.getDouble(""));
        EXPECT_ANY_THROW((void)cfg.getString(""));
        EXPECT_ANY_THROW((void)cfg.template get<bool>(""));
        EXPECT_ANY_THROW((void)cfg.template get<double>(""));
        EXPECT_ANY_THROW((void)cfg.template get<std::string>(""));
    };
    check(conf::ConfigTree{});
    check(conf::ConfigTree{std::make_shared<EmptySource>()});
}

TEST(ConfigTree, FullSource)
{
    conf::ConfigTree cfg{std::make_shared<FullSource>()};

    EXPECT_TRUE(cfg.tryGetChild(""));
    EXPECT_TRUE(cfg.tryGetBoolean("").value());
    EXPECT_DOUBLE_EQ(19.86, cfg.tryGetDouble("").value());
    EXPECT_EQ("Hello!", cfg.tryGetString("").value());
    EXPECT_TRUE(cfg.template tryGet<bool>("").value());
    EXPECT_TRUE(cfg.template tryGet<double>("").value());
    EXPECT_EQ("Hello!", cfg.template tryGet<std::string>("").value());

    EXPECT_TRUE(cfg.getChild(""));
    EXPECT_TRUE(cfg.getBoolean(""));
    EXPECT_DOUBLE_EQ(19.86, cfg.getDouble(""));
    EXPECT_EQ("Hello!", cfg.getString(""));
    EXPECT_TRUE(cfg.template get<bool>(""));
    EXPECT_TRUE(cfg.template get<double>(""));
    EXPECT_EQ("Hello!", cfg.template get<std::string>(""));
}