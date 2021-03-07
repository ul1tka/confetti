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
#include <algorithm>
#include <memory>

namespace confetti::internal {

size_t distance(std::string_view left, std::string_view right, TransformationCost cost)
{
    std::unique_ptr<size_t[]> buffer{new size_t[(right.size() + 1) * 3]};
    auto r0 = buffer.get();
    auto r1 = r0 + (right.size() + 1);
    auto r2 = r1 + (right.size() + 1);
    for (size_t i = 0; i <= right.size(); ++i)
        r1[i] = i * cost.insert;
    for (size_t i = 0; i < left.size(); ++i) {
        r2[0] = (i + 1) * cost.remove;
        for (size_t j = 0; j < right.size(); ++j) {
            r2[j + 1] = r1[j] + cost.replace * (left[i] != right[j]);
            if (i > 0 && j > 0 && left[i - 1] == right[j] && left[i] == right[j - 1])
                r2[j + 1] = std::min(r2[j + 1], r0[j - 1] + cost.swap);
            r2[j + 1] = std::min(r2[j + 1], r1[j + 1] + cost.remove);
            r2[j + 1] = std::min(r2[j + 1], r2[j] + cost.insert);
        }
        auto tmp = r0;
        r0 = r1;
        r1 = r2;
        r2 = tmp;
    }
    return r1[right.size()];
}

} // namespace confetti::internal
