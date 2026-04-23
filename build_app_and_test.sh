#!/usr/bin/env bash
# Build main app (cog_soc_env_space), viewer (cog_soc_env_space_viewer),
# and local test runner (cog_soc_env_space_tests).
# Default is Debug (-g) so gdb/Cursor breakpoints work well.
# Usage: ./build_app_and_test.sh [build-dir]
# Env:
#   CMAKE_BUILD_TYPE (default: Debug)
#   ENABLE_RAYLIB_VIEWER (default: ON)
# Example release: CMAKE_BUILD_TYPE=Release ./build_app_and_test.sh

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${1:-${ROOT}/build}"
CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-Debug}"
ENABLE_RAYLIB_VIEWER="${ENABLE_RAYLIB_VIEWER:-ON}"

cmake -S "${ROOT}" -B "${BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
  -DENABLE_RAYLIB_VIEWER="${ENABLE_RAYLIB_VIEWER}"
cmake --build "${BUILD_DIR}" --target cog_soc_env_space cog_soc_env_space_viewer cog_soc_env_space_tests

echo "Built:"
echo "  ${BUILD_DIR}/cog_soc_env_space"
echo "  ${BUILD_DIR}/cog_soc_env_space_viewer"
echo "  ${BUILD_DIR}/cog_soc_env_space_tests"
