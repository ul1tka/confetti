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
# Lua (https://www.lua.org/)
#

find_package(Lua51 REQUIRED)

message(STATUS "Lua Headers   : ${LUA_INCLUDE_DIR}")
message(STATUS "Lua Libraries : ${LUA_LIBRARIES}")

if ("${LUA_LIBRARIES}" MATCHES "(.*).a$")
    add_library(lua STATIC IMPORTED)
    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_link_options(lua INTERFACE "LINKER:--export-dynamic")
    endif ()
else ()
    add_library(lua SHARED IMPORTED)
endif ()
set_target_properties(lua PROPERTIES IMPORTED_LOCATION ${LUA_LIBRARIES})
target_include_directories(lua SYSTEM INTERFACE ${LUA_INCLUDE_DIR})
if (APPLE)
    target_compile_options(lua INTERFACE -Wno-poison-system-directories)
endif ()

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

set(CONFETTI_VERSION 1)

configure_file(
        confetti/internal/version.hh.in
        ${CMAKE_CURRENT_BINARY_DIR}/include/confetti/internal/version.hh
        @ONLY NEWLINE_STYLE UNIX
)

add_library(
        confetti
        confetti/version.cc
        confetti/config_source.cc
        confetti/config_tree.cc
        confetti/internal/lua.cc
)

target_link_libraries(confetti PUBLIC lua)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(confetti PRIVATE dl m)
endif ()

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
        confetti/version_test.cc
        confetti/config_source_test.cc
        confetti/config_tree_test.cc
        confetti/internal/lua_test.cc
)

target_link_libraries(test-confetti PRIVATE gtest gmock gtest_main confetti)

target_compile_options(test-confetti PRIVATE -Wno-global-constructors)

target_compile_definitions(test-confetti PRIVATE CONFETTI_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\")

include(GoogleTest)

gtest_discover_tests(
        test-confetti
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DISCOVERY_MODE POST_BUILD
)
