#!/usr/bin/env bash
# Build main app (cog_soc_env_space) and local test runner (cog_soc_env_space_tests).
# Default is Debug (-g) so gdb/Cursor breakpoints work well.
# Usage: ./build_app_and_test.sh [build-dir]
# Env: CMAKE_BUILD_TYPE (default: Debug). Example release: CMAKE_BUILD_TYPE=Release ./build_app_and_test.sh

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${1:-${ROOT}/build}"
CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-Debug}"

cmake -S "${ROOT}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
cmake --build "${BUILD_DIR}" --target cog_soc_env_space cog_soc_env_space_tests

echo "Built:"
echo "  ${BUILD_DIR}/cog_soc_env_space"
echo "  ${BUILD_DIR}/cog_soc_env_space_tests"
