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

readonly PWD0="$(readlink -fn "$(dirname "$0")")"
readonly OUTPUT_DIR="${PWD0}"
readonly APOLLO_DIR="/apollo"

###############################################################################
if [[ ! -e "${PROTOC_FILEPATH}" || ! -x "${PROTOC_FILEPATH}" ]]; then
    echo "Failed to get protoc."
    exit 1
fi
if [[ ! -e "${GRPC_CPP_PLUGIN_FILEPATH}" || ! -x "${GRPC_CPP_PLUGIN_FILEPATH}" ]]; then
    echo "Failed to get grpc_cpp_plugin."
    exit 1
fi
if ! ${PROTOC_FILEPATH} --version </dev/null >/dev/null; then
    echo "Failed to execute protoc."
    exit 1
fi
if ! ${GRPC_CPP_PLUGIN_FILEPATH} </dev/null >/dev/null; then
    echo "Failed to execute grpc_cpp_plugin."
    exit 1
fi

set -e
find "${APOLLO_DIR}/modules" -type f -name "*.proto" | while read line; do
    ${PROTOC_FILEPATH} --cpp_out "${OUTPUT_DIR}" -I /apollo "${line}"
    ${PROTOC_FILEPATH} --grpc_out "${OUTPUT_DIR}" -I /apollo "${line}" \
        "--plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_FILEPATH}"
done
