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

#ifndef CONFETTI_INTERNAL_LEVENSHTEIN_HH
#define CONFETTI_INTERNAL_LEVENSHTEIN_HH

#include <string_view>

namespace confetti::internal {

struct TransformationCost final {
    unsigned int swap{1};
    unsigned int replace{3};
    unsigned int insert{2};
    unsigned int remove{4};
};

size_t distance(
    std::string_view left, std::string_view right, TransformationCost cost = TransformationCost{});

} // namespace confetti::internal

#endif // CONFETTI_INTERNAL_LEVENSHTEIN_HH
