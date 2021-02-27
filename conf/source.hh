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

#ifndef CONF_INTERFACE_HH
#define CONF_INTERFACE_HH

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

namespace conf {

class Source {
public:
    Source() noexcept = default;

    virtual ~Source();

    Source(const Source&) = delete;
    Source& operator=(const Source&) = delete;

    [[nodiscard]] virtual std::shared_ptr<Source> tryGetChild(std::string_view name) const = 0;

    [[nodiscard]] virtual std::optional<bool> tryGetBoolean(std::string_view name) const = 0;

    [[nodiscard]] virtual std::optional<double> tryGetDouble(std::string_view name) const = 0;

    [[nodiscard]] virtual std::optional<int64_t> tryGetNumber(std::string_view name) const;

    [[nodiscard]] virtual std::optional<uint64_t> tryGetUnsignedNumber(std::string_view name) const;

    [[nodiscard]] virtual std::optional<std::string> tryGetString(std::string_view name) const = 0;
};

using SourcePtr = std::shared_ptr<Source>;

} // namespace conf

#endif
