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

#include "levenshtein.hh"
#include <gtest/gtest.h>

using confetti::internal::distance;

TEST(LevenshteinDistance, Basic)
{
    EXPECT_EQ(0, distance("", ""));
    EXPECT_EQ(0, distance("same", "same"));
    EXPECT_EQ(1, distance("meail", "email"));
    EXPECT_EQ(2, distance("mail", "email"));
    EXPECT_EQ(3, distance("xmail", "email"));
    EXPECT_EQ(4, distance("email", "mail"));
    EXPECT_EQ(10, distance("email", "male"));
}
