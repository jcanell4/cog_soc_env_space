#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN="${SCRIPT_DIR}/cog_soc_env_space"

if [[ ! -x "${BIN}" ]]; then
  echo "Error: simulation executable not found at: ${BIN}" >&2
  exit 1
fi

if [[ "$(uname -s)" == "Linux" ]] && command -v ldd >/dev/null 2>&1; then
  MISSING="$(ldd "${BIN}" 2>/dev/null | grep 'not found' || true)"
  if [[ -n "${MISSING}" ]]; then
    echo "Missing runtime libraries detected for ${BIN}." >&2
    echo >&2
    echo "${MISSING}" >&2
    echo >&2
    echo "On Debian/Ubuntu, install common dependencies with:" >&2
    echo "  sudo apt update" >&2
    echo "  sudo apt install -y libstdc++6 libgcc-s1 libc6 libm6 libx11-6 libxrandr2 libxinerama1 libxcursor1 libxi6 libgl1 libasound2 libudev1 libwayland-client0 libxkbcommon0" >&2
    exit 1
  fi
fi

exec "${BIN}" "$@"
