#!/bin/bash

# Clear build directory contents only; keep 'build' if it is a symlink.
if [ -e build ]; then
  find -L build -mindepth 1 -delete
fi

cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
ln -sf build/compile_commands.json .
