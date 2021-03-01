--
-- Copyright (C) 2021 Vlad Lazarenko <vlad@lazarenko.me>
--
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
--
--      https://www.apache.org/licenses/LICENSE-2.0
--
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.
--

local cfg = confetti

cfg.empty_list = {}
cfg.string_list = { "Moscow", "never", "sleeps" }
cfg.number_list = { 1962, 1968, 1986, 2021 }
cfg.string_matrix_array = {
    {
        { "We", "need", "guns." },
        { "Lots", "of", "guns", '!' }
    },
    {
        { "We", "need", "guns." },
        { "Lots", "of", "guns", '!' }
    }
}

cfg["a.b.c.state"] = "NJ"
cfg["a.b.c.year"] = "2018"

cfg.a = {
    b = {
        c = {
            state = "CT",
            year = 2021
        }
    }
}

--
-- Parse INI file using ini.lua
-- See https://github.com/lzubiaur/ini.lua
--
local ini = require 'ini'
local path = require 'path'

cfg.ini = ini.parse_file(path.dirname(
        debug.getinfo(1).source:sub(2)) .. '/config_tree_test.ini')
