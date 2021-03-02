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
#include "internal/lua.hh"
#include "internal/string.hh"
#include <stdexcept>

namespace confetti {

void ConfigTree::noSuchChild(int index)
{
    throw std::runtime_error{
        std::string{"Cannot find child config section at index "}.append(std::to_string(index))};
}

void ConfigTree::noSuchChild(std::string_view name)
{
    throw std::runtime_error{std::string{"Cannot find child config section "}.append(name)};
}

void ConfigTree::noSuchKey(int index)
{
    throw std::runtime_error{
        std::string{"Cannot find config value at index "}.append(std::to_string(index))};
}

void ConfigTree::noSuchKey(std::string_view name)
{
    throw std::runtime_error{std::string{"Cannot find config value for key "}.append(name)};
}

ConfigTree ConfigTree::loadLuaFile(const std::filesystem::path& file)
{
    return ConfigTree{internal::LuaSource::loadFile(file)};
}

ConfigTree ConfigTree::loadIniFile(const std::filesystem::path& file)
{
    const auto code = std::string{"local ini = require 'ini'\n"
                                  "for k, v in pairs(ini.parse_file('"}
                          .append(file.native())
                          .append("')) do confetti[k] = v end");
    return ConfigTree{internal::LuaSource::loadCode(code)};
}

ConfigTree ConfigTree::loadJsonFile(const std::filesystem::path& file)
{
    const auto code = std::string{R"!(
local json = require 'lunajson'
local file = assert(io.open(")!"}
                          .append(file.native())
                          .append(R"!(", "r"))
local content = file:read("*all")
file:close()
for k, v in pairs(json.decode(content)) do confetti[k] = v end)!");
    return ConfigTree{internal::LuaSource::loadCode(code)};
}

ConfigTree ConfigTree::loadFile(const std::filesystem::path& file)
{
    const auto extension = file.extension().native();
    if (internal::strCaseEq(extension, ".lua")) {
        return loadLuaFile(file);
    } else if (internal::strCaseEq(extension, ".json")) {
        return loadJsonFile(file);
    } else if (internal::strCaseEq(extension, ".ini")) {
        return loadIniFile(file);
    }
    throw std::runtime_error{"Unknown configuration file type: " + file.native()};
}

} // namespace confetti
