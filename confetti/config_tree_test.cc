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

#include "config_tree.hh"
#include <gmock/gmock.h>
#include <sstream>

namespace {

struct EmptySource final : confetti::ConfigSource {
    ~EmptySource() override = default;

    [[nodiscard]] bool hasValueAt(int) const override { return false; }

    [[nodiscard]] confetti::ConfigSourcePointer tryGetChild(int) const override { return {}; }

    [[nodiscard]] confetti::ConfigSourcePointer tryGetChild(std::string_view) const override
    {
        return {};
    }

    [[nodiscard]] std::optional<bool> tryGetBoolean(int) const override { return {}; }

    [[nodiscard]] std::optional<bool> tryGetBoolean(std::string_view) const override { return {}; }

    [[nodiscard]] std::optional<double> tryGetDouble(int) const override { return {}; }

    [[nodiscard]] std::optional<double> tryGetDouble(std::string_view) const override { return {}; }

    [[nodiscard]] std::optional<std::string> tryGetString(int) const override { return {}; }

    [[nodiscard]] std::optional<std::string> tryGetString(std::string_view) const override
    {
        return {};
    }

    [[nodiscard]] std::vector<std::string> getKeyList() const override { return {}; }
};

struct FullSource final : confetti::ConfigSource {
    ~FullSource() override = default;

    [[nodiscard]] bool hasValueAt(int) const override { return false; }

    [[nodiscard]] confetti::ConfigSourcePointer tryGetChild(int) const override
    {
        return std::make_shared<FullSource>();
    }

    [[nodiscard]] confetti::ConfigSourcePointer tryGetChild(std::string_view) const override
    {
        return tryGetChild(0);
    }

    [[nodiscard]] std::optional<bool> tryGetBoolean(int) const override { return true; }

    [[nodiscard]] std::optional<bool> tryGetBoolean(std::string_view) const override
    {
        return tryGetBoolean(0);
    }

    [[nodiscard]] std::optional<double> tryGetDouble(int) const override { return 19.86; }

    [[nodiscard]] std::optional<double> tryGetDouble(std::string_view) const override
    {
        return tryGetDouble(0);
    }

    [[nodiscard]] std::optional<std::string> tryGetString(int) const override { return "Hello!"; }

    [[nodiscard]] std::optional<std::string> tryGetString(std::string_view) const override
    {
        return tryGetString(0);
    }

    [[nodiscard]] std::vector<std::string> getKeyList() const override { return {}; }
};

template <typename T>
class ConfigTreeNumeric : public testing::Test {
protected:
    static constexpr T values[] = {T{1962}, T{1968}, T{1986}, T{2021}};
};

using NumericTestingTypes = testing::Types<int32_t, int64_t, uint32_t, uint64_t>;

TYPED_TEST_SUITE(ConfigTreeNumeric, NumericTestingTypes);

decltype(auto) loadLuaFile()
{
    return confetti::ConfigTree::loadFile(CONFETTI_SOURCE_DIR "/confetti/config_tree_test.lua");
}

decltype(auto) loadIniFile()
{
    return confetti::ConfigTree::loadFile(CONFETTI_SOURCE_DIR "/confetti/config_tree_test.ini");
}

decltype(auto) loadJsonFile()
{
    return confetti::ConfigTree::loadFile(CONFETTI_SOURCE_DIR "/confetti/config_tree_test.json");
}

} // namespace

TEST(ConfigTree, EmptyTree)
{
    confetti::ConfigTree tree1;

    EXPECT_FALSE(tree1);

    confetti::ConfigTree tree2{tree1};

    tree1 = tree2;
    tree2 = std::move(tree1);
    EXPECT_FALSE(tree1 < tree2); // NOLINT(bugprone-use-after-move)
    EXPECT_FALSE(tree1 > tree2);

    confetti::ConfigTree tree3{std::make_shared<EmptySource>()};

    EXPECT_TRUE(tree3);
    EXPECT_TRUE(tree3 > tree1);
    EXPECT_FALSE(tree3 < tree2);

    tree1 = std::move(tree3);

    EXPECT_TRUE(tree1);
    EXPECT_FALSE(tree3); // NOLINT(bugprone-use-after-move)

    tree2 = tree1;

    EXPECT_TRUE(tree1);
    EXPECT_TRUE(tree2);

    tree3 = std::move(tree2);

    EXPECT_TRUE(tree3);
    EXPECT_FALSE(tree2); // NOLINT(bugprone-use-after-move)
}

TEST(ConfigTree, EmptySourceGetters)
{
    auto check = [&](auto&& cfg) {
        EXPECT_FALSE(cfg.tryGetChild(0));
        EXPECT_FALSE(cfg.tryGetChild(""));
        EXPECT_FALSE(cfg.tryGetChild(confetti::ConfigPath{}));
        EXPECT_FALSE(cfg.tryGetBoolean(0));
        EXPECT_FALSE(cfg.tryGetBoolean(""));
        EXPECT_FALSE(cfg.tryGetBoolean(confetti::ConfigPath{}));
        EXPECT_FALSE(cfg.tryGetDouble(0));
        EXPECT_FALSE(cfg.tryGetDouble(""));
        EXPECT_FALSE(cfg.tryGetString(0));
        EXPECT_FALSE(cfg.tryGetString(""));
        EXPECT_FALSE(cfg.template tryGet<bool>(confetti::ConfigPath{}));
        EXPECT_FALSE(cfg.template tryGet<bool>(""));
        EXPECT_FALSE(cfg.template tryGet<bool>(0));
        EXPECT_FALSE(cfg.template tryGet<double>(""));
        EXPECT_FALSE(cfg.template tryGet<double>(0));
        EXPECT_FALSE(cfg.template tryGet<std::string>(""));
        EXPECT_FALSE(cfg.template tryGet<std::string>(0));
        EXPECT_ANY_THROW((void)cfg.getChild(""));
        EXPECT_ANY_THROW((void)cfg.getChild(0));
        EXPECT_ANY_THROW((void)cfg.getBoolean(""));
        EXPECT_ANY_THROW((void)cfg.getBoolean(confetti::ConfigPath{}));
        EXPECT_ANY_THROW((void)cfg.getBoolean(0));
        EXPECT_ANY_THROW((void)cfg.getDouble(""));
        EXPECT_ANY_THROW((void)cfg.getDouble(0));
        EXPECT_ANY_THROW((void)cfg.getString(""));
        EXPECT_ANY_THROW((void)cfg.getString(0));
        EXPECT_ANY_THROW((void)cfg.template get<bool>(""));
        EXPECT_ANY_THROW((void)cfg.template get<bool>(0));
        EXPECT_ANY_THROW((void)cfg.template get<double>(""));
        EXPECT_ANY_THROW((void)cfg.template get<double>(0));
        EXPECT_ANY_THROW((void)cfg.template get<std::string>(""));
        EXPECT_ANY_THROW((void)cfg.template get<std::string>(0));

        EXPECT_EQ(1945, cfg.template get("", 1945));
        EXPECT_EQ("lol", cfg.template get<std::string>("", "lol"));
        EXPECT_EQ("callable", cfg.template get<std::string>("", [] { return "callable"; }));

        EXPECT_EQ(1945, cfg.template get(confetti::ConfigPath{}, 1945));
        EXPECT_EQ("lol", cfg.template get<std::string>(confetti::ConfigPath{}, "lol"));
        EXPECT_EQ("callable",
            cfg.template get<std::string>(confetti::ConfigPath{}, [] { return "callable"; }));

        for (auto n : cfg.template values<int>()) {
            ADD_FAILURE() << n;
        }
    };
    check(confetti::ConfigTree{});
    check(confetti::ConfigTree{std::make_shared<EmptySource>()});
}

TEST(ConfigTree, FullSource)
{
    confetti::ConfigTree cfg{std::make_shared<FullSource>()};

    EXPECT_TRUE(cfg.tryGetChild(""));
    EXPECT_TRUE(cfg.tryGetChild(1));
    EXPECT_TRUE(cfg.tryGetBoolean("").value());
    EXPECT_TRUE(cfg.tryGetBoolean(2).value());
    EXPECT_DOUBLE_EQ(19.86, cfg.tryGetDouble("").value());
    EXPECT_DOUBLE_EQ(19.86, cfg.tryGetDouble(0).value());
    EXPECT_EQ("Hello!", cfg.tryGetString("").value());
    EXPECT_EQ("Hello!", cfg.tryGetString(0).value());
    EXPECT_TRUE(cfg.template tryGet<bool>("").value());
    EXPECT_TRUE(cfg.template tryGet<bool>(1).value());
    EXPECT_TRUE(cfg.template tryGet<double>("").value());
    EXPECT_TRUE(cfg.template tryGet<double>(0).value());
    EXPECT_EQ("Hello!", cfg.template tryGet<std::string>("").value());
    EXPECT_EQ("Hello!", cfg.template tryGet<std::string>(0).value());

    EXPECT_TRUE(cfg.getChild(""));
    EXPECT_TRUE(cfg.getChild(0));
    EXPECT_TRUE(cfg.getBoolean(""));
    EXPECT_TRUE(cfg.getBoolean(0));
    EXPECT_DOUBLE_EQ(19.86, cfg.getDouble(""));
    EXPECT_DOUBLE_EQ(19.86, cfg.getDouble(0));
    EXPECT_EQ("Hello!", cfg.getString(""));
    EXPECT_EQ("Hello!", cfg.getString(0));
    EXPECT_TRUE(cfg.template get<bool>(""));
    EXPECT_TRUE(cfg.template get<bool>(0));
    EXPECT_TRUE(cfg.template get<double>(""));
    EXPECT_TRUE(cfg.template get<double>(0));
    EXPECT_EQ("Hello!", cfg.template get<std::string>(""));
    EXPECT_EQ("Hello!", cfg.template get<std::string>(0));

    for ([[maybe_unused]] auto n : cfg.values<int>()) {
        ADD_FAILURE() << n;
    }
}

TEST(ConfigTree, ConfigValue)
{
    confetti::ConfigTree cfg{std::make_shared<FullSource>()};
    const auto value = cfg.get(confetti::ConfigPath{""});
    {
        double v = value;
        EXPECT_DOUBLE_EQ(19.86, v);
    }
    {
        std::string x = value;
        EXPECT_EQ("Hello!", x);
    }
    confetti::ConfigTree subTree = value;
    EXPECT_DOUBLE_EQ(19.86, subTree.get(""));

    std::optional<double> x = cfg.get("");
    EXPECT_TRUE(x.has_value());
}

TEST(ConfigTree, LuaLoadFile) { ASSERT_TRUE(loadLuaFile()); }

TEST(ConfigTree, LuaEmptyArray)
{
    auto list = loadLuaFile()["empty_list"];
    size_t count = 0;
    for ([[maybe_unused]] auto n : list.values<int>()) {
        ++count;
    }
    EXPECT_EQ(0, count);
}

TEST(ConfigTree, LuaStringArray)
{
    static const char* values[] = {"Moscow", "never", "sleeps"};

    auto list = loadLuaFile()[confetti::ConfigPath{"string_list"}];

    for (int i = 0; i < static_cast<int>(std::size(values)); ++i) {
        EXPECT_EQ(values[i], list.at<std::string>(i));
    }

    int i = 0;
    for (const auto& s : list.values<std::string>()) {
        EXPECT_EQ(values[i], s);
        ++i;
    }
    EXPECT_EQ(std::size(values), i);
}

TYPED_TEST(ConfigTreeNumeric, LuaArray)
{
    auto list = loadLuaFile()["number_list"];

    for (int i = 0; i < static_cast<int>(std::size(TestFixture::values)); ++i) {
        EXPECT_EQ(TestFixture::values[i], list.at<int>(i));
    }

    int i = 0;
    for (auto n : list.values<TypeParam>()) {
        EXPECT_EQ(TestFixture::values[i], n);
        ++i;
    }
    EXPECT_EQ(std::size(TestFixture::values), i);
}

TEST(ConfigTree, LuaSimpleStringMatrixIteration)
{
    auto matrix = loadLuaFile()["string_matrix_array"];
    size_t total_entries = 0;
    for (auto entry : matrix.children()) {
        size_t entries = 0;
        for (auto child : entry.children()) {
            std::vector<std::string> array = child.values<std::string>();
            if (entries % 2) {
                EXPECT_THAT(array, testing::ElementsAre("Lots", "of", "guns", "!"));
            } else {
                EXPECT_THAT(array, testing::ElementsAre("We", "need", "guns."));
            }
            ++entries;
        }
        EXPECT_EQ(2, entries);
        total_entries += entries;
    }
    EXPECT_EQ(4, total_entries);
}

TEST(ConfigTree, LuaRootConfigCountChildren)
{
    auto cfg = loadLuaFile();
    size_t count = 0;
    for ([[maybe_unused]] auto c : cfg.children()) {
        ++count;
    }
    EXPECT_EQ(0, count);
}

TEST(ConfigTree, ReachStraightIntoSubtree)
{
    auto cfg = loadLuaFile();

    EXPECT_EQ("NJ", cfg.get<std::string>("a.b.c.state"));
    EXPECT_EQ(2018, cfg.get<int>("a.b.c.year"));

    EXPECT_TRUE(cfg.tryGetChild(confetti::ConfigPath{"a.b"}));
    EXPECT_TRUE(cfg.tryGetChild(confetti::ConfigPath{"a/b\\c"}));
    EXPECT_FALSE(cfg.tryGetChild(confetti::ConfigPath{"a/b/c/this_node_should_not_exist"}));

    EXPECT_EQ("CT", cfg.get<std::string>(confetti::ConfigPath{"a.b/c\\state"}));
    EXPECT_EQ(2021, cfg.get<int>(confetti::ConfigPath{"a/b\\c.year"}));
}

static void checkIniFileConfig(const confetti::ConfigTree& cfg)
{
    using namespace confetti::literals;

    EXPECT_EQ("World", cfg.get<std::string>("Hello"));

    EXPECT_EQ("User Name", cfg.get<std::string>("user.name"_cp));
    EXPECT_EQ("User Name", cfg["user"].get<std::string>("name"));

    EXPECT_EQ("info@example.com", cfg.get<std::string>("user.email"_cp));
    EXPECT_EQ("info@example.com", cfg["user"].get<std::string>("email"));

    EXPECT_EQ("127.0.0.1", cfg.get<std::string>("web.server"_cp));
    EXPECT_EQ("127.0.0.1", cfg["web"].get<std::string>("server"));

    EXPECT_EQ(80, cfg.get<unsigned short>("web.port"_cp));
    EXPECT_EQ(80, cfg["web"].get<unsigned short>("port"));

    EXPECT_EQ("index.html", cfg.get<std::string>("web.file"_cp));
    EXPECT_EQ("index.html", cfg["web"].get<std::string>("file"));
}

TEST(ConfigTree, LuaLoadIniFile) { checkIniFileConfig(loadLuaFile()["ini"]); }

TEST(ConfigTree, LuaLoadJsonFile) { checkIniFileConfig(loadLuaFile()["json"]); }

TEST(ConfigTree, LoadIniFile) { checkIniFileConfig(loadIniFile()); }

TEST(ConfigTree, LoadJsonFile) { checkIniFileConfig(loadJsonFile()); }

TEST(ConfigTree, SimpleLuaSequenceValue)
{
    static constexpr std::string_view code = R"(
local n = 0
confetti.sequence = function()
    n = n + 1
    return n
end
)";
    auto tree = confetti::ConfigTree::loadLuaCode(code);
    for (int i = 1; i <= 10; ++i) {
        ASSERT_EQ(i, tree.get<int>("sequence"));
    }
}

TEST(ConfigTree, KeyNotFoundErrorMessageIniSimple)
{
    auto user = loadIniFile()["user"];
    for (auto key : {"mail", "nail"}) {
        try {
            (void)user.get<std::string>(key);
            ADD_FAILURE() << "Expected exception was not thrown.";
        } catch (const std::exception& e) {
            std::ostringstream stream;
            stream << "Cannot find configuration entry '" << key << "'. Did you mean 'email'?";
            EXPECT_EQ(std::move(stream).str(), e.what());
        }
    }
}

TEST(ConfigTree, KeyNotFoundErrorMessageLuaSimple)
{
    try {
        (void)loadLuaFile().get<std::string>("string_array");
        ADD_FAILURE() << "Key should not have been found.";
    } catch (const std::exception& e) {
        EXPECT_STREQ(
            "Cannot find configuration entry 'string_array'. Did you mean 'string_matrix_array'?",
            e.what());
    }
}

TEST(ConfigTree, KeyNotFoundInSubtreeErrorMessage)
{
    using namespace confetti::literals;
    auto tree = loadLuaFile();
    try {
        (void)tree.get<std::string>("some/deep\\subtree.anothre_vaue"_cp);
    } catch (const std::exception& e) {
        EXPECT_STREQ(
            "Cannot find configuration entry 'anothre_vaue'. Did you mean 'another_value'?",
            e.what());
    }
}
