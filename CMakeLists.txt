#
# Copyright (C) 2021 Vlad Lazarenko <vlad@lazarenko.me>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cmake_minimum_required(VERSION 3.16.3 FATAL_ERROR)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

project(confetti LANGUAGES C CXX DESCRIPTION "C++ Configuration Library")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Coverage Build
if (CMAKE_BUILD_TYPE MATCHES "Coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 --coverage")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 --coverage")
endif ()

include(FetchContent)

#
# Lua (https://github.com/lua/lua)
#

FetchContent_Declare(
        lua
        GIT_REPOSITORY https://github.com/lua/lua.git
        GIT_TAG v5.4.2
)

FetchContent_GetProperties(lua)

if (NOT lua_POPULATED)
    FetchContent_Populate(lua)
    add_library(
            lua
            ${lua_SOURCE_DIR}/lapi.c
            ${lua_SOURCE_DIR}/lcorolib.c
            ${lua_SOURCE_DIR}/ldo.c
            ${lua_SOURCE_DIR}/linit.c
            ${lua_SOURCE_DIR}/lmem.c
            ${lua_SOURCE_DIR}/loslib.c
            ${lua_SOURCE_DIR}/lstrlib.c
            ${lua_SOURCE_DIR}/ltm.c
            ${lua_SOURCE_DIR}/lvm.c
            ${lua_SOURCE_DIR}/lauxlib.c
            ${lua_SOURCE_DIR}/lctype.c
            ${lua_SOURCE_DIR}/ldump.c
            ${lua_SOURCE_DIR}/liolib.c
            ${lua_SOURCE_DIR}/loadlib.c
            ${lua_SOURCE_DIR}/lparser.c
            ${lua_SOURCE_DIR}/ltable.c
            ${lua_SOURCE_DIR}/lzio.c
            ${lua_SOURCE_DIR}/lbaselib.c
            ${lua_SOURCE_DIR}/ldblib.c
            ${lua_SOURCE_DIR}/lfunc.c
            ${lua_SOURCE_DIR}/llex.c
            ${lua_SOURCE_DIR}/lobject.c
            ${lua_SOURCE_DIR}/lstate.c
            ${lua_SOURCE_DIR}/ltablib.c
            ${lua_SOURCE_DIR}/lundump.c
            ${lua_SOURCE_DIR}/lcode.c
            ${lua_SOURCE_DIR}/ldebug.c
            ${lua_SOURCE_DIR}/lgc.c
            ${lua_SOURCE_DIR}/lmathlib.c
            ${lua_SOURCE_DIR}/lopcodes.c
            ${lua_SOURCE_DIR}/lstring.c
            ${lua_SOURCE_DIR}/ltests.c
            ${lua_SOURCE_DIR}/lutf8lib.c
    )
    target_compile_options(
            lua
            PRIVATE
            -mtune=native
            -march=native
    )
    target_include_directories(lua SYSTEM INTERFACE ${lua_SOURCE_DIR})

    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_link_libraries(lua PRIVATE dl m)
    endif ()

    add_executable(lua-interpreter ${lua_SOURCE_DIR}/lua.c)
    set_target_properties(lua-interpreter PROPERTIES OUTPUT_NAME "lua")
    target_link_libraries(lua-interpreter lua)
endif ()

#
# Google Benchmark (https://github.com/google/benchmark)
#

FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.5.2
)

FetchContent_GetProperties(benchmark)

if (NOT benchmark_POPULATED)
    FetchContent_Populate(benchmark)
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Enable testing of the benchmark library." FORCE)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL
        "Enable building the unit tests which depend on gtest" FORCE)
    add_subdirectory(${benchmark_SOURCE_DIR} ${benchmark_BINARY_DIR})
endif ()

get_target_property(BENCHMARK_INCLUDE_DIRECTORIES benchmark INTERFACE_INCLUDE_DIRECTORIES)
set_target_properties(benchmark PROPERTIES
                      INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${BENCHMARK_INCLUDE_DIRECTORIES}")

#
# Google Test (https://github.com/google/googletest)
#

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.10.0
)

FetchContent_GetProperties(googletest)

if (NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif ()

enable_testing()

#
# Confetti Library
#

set(CONF_VERSION 1)

configure_file(
        conf/internal/version.hh.in
        ${CMAKE_CURRENT_BINARY_DIR}/include/conf/internal/version.hh
        @ONLY NEWLINE_STYLE UNIX
)

add_library(
        confetti
        conf/version.cc
        conf/tree.cc
        conf/lua.cc
)

target_link_libraries(confetti PUBLIC lua)

target_include_directories(
        confetti
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/include
)

target_compile_options(
        confetti
        PUBLIC
        -Wall
        -Wextra
        -Werror
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        -Wno-padded
        -Wno-gnu-zero-variadic-macro-arguments
        -Wno-zero-as-null-pointer-constant
        -mtune=native
        -march=native
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(confetti PUBLIC -Weverything)
endif ()

add_executable(
        test-confetti
        conf/version_test.cc
        conf/tree_test.cc
        conf/lua_test.cc
)

target_link_libraries(test-confetti PRIVATE gtest gmock gtest_main confetti)

target_compile_options(test-confetti PRIVATE -Wno-global-constructors)

target_compile_definitions(test-confetti PRIVATE CONF_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\")

include(GoogleTest)

gtest_discover_tests(
        test-confetti
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DISCOVERY_MODE POST_BUILD
)
