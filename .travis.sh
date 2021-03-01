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

case "${TRAVIS_OS_NAME}" in
    "osx")
        export CMAKE_BUILD_PARALLEL_LEVEL=$(($(sysctl -n hw.ncpu) * 2))
        ;;
    "linux")
        export CMAKE_BUILD_PARALLEL_LEVEL=$(($(nproc) * 2))
        ;;
esac

#
# Build Lua
#
# http://lua.space/general/ci-with-lua
# https://github.com/lzubiaur/ini.lua
#

pip install hererocks

hererocks lua_install -r^ --lua=5.4

export LUA_DIR="${PWD}/lua_install"

export PATH="${LUA_DIR}/bin:${PATH}"

luarocks install path
luarocks install lpeg
luarocks install busted

wget https://raw.githubusercontent.com/lzubiaur/ini.lua/master/ini.lua \
    -O "${LUA_DIR}/share/lua/5.4/ini.lua"

# Build Confetti

command -v ninja > /dev/null && CMAKE_FLAGS="-GNinja ${CMAKE_FLAGS}"

test -n "${CC}" && CMAKE_FLAGS="${CMAKE_FLAGS} -DCMAKE_C_COMPIER=${CC}"
test -n "${CXX}" && CMAKE_FLAGS="${CMAKE_FLAGS} -DCMAKE_CXX_COMPIER=${CXX}"

BUILD_TYPE=${BUILD_TYPE:-Debug}

cmake \
    ${CMAKE_FLAGS} \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DCMAKE_VERBOSE_MAKEFILE=ON \
    -DLUA_INCLUDE_DIR="${LUA_DIR}/include" \
    -DLUA_LIBRARIES="${LUA_DIR}/lib/liblua54.a" \
    ..

cmake --build . -- -j ${CMAKE_BUILD_PARALLEL_LEVEL}

ctest \
    --repeat-until-fail 2 \
    --timeout 600 \
    --parallel ${CMAKE_BUILD_PARALLEL_LEVEL} \
    --schedule-random \
    --output-on-failure

if [ "${BUILD_TYPE}" = "Coverage" ]; then
    cd ..
    coveralls \
        --gcov "${GCOV:=gcov}" \
        --gcov-options '\-lp' \
        --exclude-pattern '.*/build/CMakeFiles/.*' \
        --exclude-pattern '.*/build/_deps/.*'
fi
