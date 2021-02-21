#!/bin/sh
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

set -ex

mkdir build && cd build

export MAKEFLAGS="-j"

command -v ninja > /dev/null && CMAKE_FLAGS="-GNinja"

test -n "${CC}" && CMAKE_FLAGS="${CMAKE_FLAGS} -DCMAKE_C_COMPIER=${CC}"

test -n "${CXX}" && CMAKE_FLAGS="${CMAKE_FLAGS} -DCMAKE_CXX_COMPIER=${CXX}"

BUILD_TYPE=${BUILD_TYPE:-Debug}

cmake \
    ${CMAKE_FLAGS} \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DCMAKE_VERBOSE_MAKEFILE=ON \
    ..

cmake --build .

ctest \
    --repeat-until-fail 2 \
    --timeout 600 \
    --parallel \
    --schedule-random \
    --output-on-failure

if [ "${BUILD_TYPE}" = "Coverage" ]; then
    cd ..
    coveralls \
        --gcov "${GCOV:=gcov}" \
        --gcov-options '\-lp' \
        --exclude-pattern '.*/build/CMakeFiles/.*' \
        --exclude-pattern '.*/build/_deps/.*' \
        --exclude-pattern '.*/bench/.*'
fi
