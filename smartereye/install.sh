#!/usr/bin/env bash

###############################################################################
# Copyright 2020 The Apollo Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
###############################################################################

# Fail on first error.
set -e
cd "$(dirname "${BASH_SOURCE[0]}")"

DEST_DIR="/usr/local/apollo/third_party/smartereye"
if [ $# -ge 1 ]; then
    DEST_DIR="$1"
fi

function create_symlinks() {
    local mylib_dir="lib"
    local orig_libs
    orig_libs=$(find ${mylib_dir} -name "lib*.so.*" -type f)
    for mylib in ${orig_libs}; do
        mylib=$(basename $mylib)
        ver="${mylib##*.so.}"
        if [ -z "$ver" ]; then
            continue
        fi
        libname="${mylib%%.so*}"
        IFS='.' read -ra arr <<< "${ver}"
        IFS=" " # restore IFS
        num=${#arr[@]}
        if [[ num -gt 2 ]]; then
            ln -s "./${mylib}" "${mylib_dir}/${libname}.so.${arr[0]}.${arr[1]}"
        fi
        ln -s "./${mylib}" "${mylib_dir}/${libname}.so.${arr[0]}"
        ln -s "./${mylib}" "${mylib_dir}/${libname}.so"
        echo "Done symlinking ${mylib}"
    done
}

function clean_symlinks() {
    local dest_dir="${DEST_DIR}"
    pushd ${dest_dir}
    local mylib_dir="lib"
    find ${mylib_dir} -name lib*.so* -type l | xargs rm -rf
    popd
}

function install_smartereye() {
    local dest_dir="${DEST_DIR}"
    [ -d "${dest_dir}" ] || mkdir -p "${dest_dir}"
    [ -d "${dest_dir}/lib" ] && rm -rf "${dest_dir}/lib"
    [ -d "${dest_dir}/include" ] && rm -rf "${dest_dir}/include"
    cp -rf include "${dest_dir}"
    cp -rf lib "${dest_dir}"
    cp -f README.md License.txt "${dest_dir}"
    pushd $dest_dir
    create_symlinks
    popd
}

install_smartereye

### QAsioSocket was provided by https://github.com/dushibaiyu/QAsioSocket

