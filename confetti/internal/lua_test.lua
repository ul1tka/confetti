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

print("Configuring the application...")

local cfg = confetti

cfg.simple_string = "Hello, Lua!"
cfg.simple_number = 12345
cfg.simple_double_number = 19.86
cfg.simple_double_string = "-19.86"
cfg.simple_zero = 0
cfg.simple_yes = true
cfg.simple_no = false

cfg.simple_func = function()
    return 2 + 2
end

cfg.simple_nested_func = function()
    return function()
        return 2 + 2 * 2
    end
end

cfg.simple_nested_math = function()
    return function()
        return 2.5 * 2.5
    end
end

cfg["user"] = {
    name = "Vlad Lazarenko",
    email = "vlad@lazarenko.me"
}

cfg.days = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday" }
