#!/usr/bin/env sh
set -eu

prefix="${PREFIX:-$HOME/.local}"
build_dir="${BUILD_DIR:-build}"

cmake -S . -B "$build_dir" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$prefix"

cmake --build "$build_dir"

if [ "${SKIP_TESTS:-0}" != "1" ]; then
  ctest --test-dir "$build_dir" --output-on-failure
fi

cmake --install "$build_dir"

printf '%s\n' "Installed Quick Note to $prefix"
printf '%s\n' "Restart Plasma Shell or log out and back in, then add Quick Note to your panel."
