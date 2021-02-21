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

#include "version.hh"
#include <gtest/gtest.h>
#include <string>

TEST(Version, Parts)
{
    constexpr conf::Version version{123456789};
    EXPECT_EQ(123456789, version.getValue());
    EXPECT_EQ(123, version.getMajor());
    EXPECT_EQ(456, version.getMinor());
    EXPECT_EQ(789, version.getPatch());
}

TEST(Version, CompileTimeMatchesRuntime)
{
    EXPECT_EQ(conf::getVersion(), conf::getRuntimeVersion());
}

TEST(Version, ThreeWayComparison)
{
    EXPECT_LT(conf::Version{1}, conf::Version{2});
    EXPECT_LE(conf::Version{1}, conf::Version{1});
    EXPECT_GT(conf::Version{2}, conf::Version{1});
    EXPECT_GE(conf::Version{1}, conf::Version{1});
    EXPECT_EQ(conf::Version{1}, conf::Version{1});
    EXPECT_NE(conf::Version{1}, conf::Version{2});
}

TEST(Version, VersionMatchesMacros)
{
    EXPECT_EQ(CONF_VERSION, conf::getVersion().getValue());
    EXPECT_EQ(CONF_VERSION_MAJOR, conf::getVersion().getMajor());
    EXPECT_EQ(CONF_VERSION_MINOR, conf::getVersion().getMinor());
    EXPECT_EQ(CONF_VERSION_PATCH, conf::getVersion().getPatch());
}

TEST(Version, Output)
{
    for (auto version : {conf::getVersion(), conf::getRuntimeVersion()}) {
        std::ostringstream stream;
        stream << version;
        EXPECT_EQ(std::to_string(version.getMajor()) + '.' + std::to_string(version.getMinor())
                + '.' + std::to_string(version.getPatch()),
            std::move(stream).str());
    }
}
