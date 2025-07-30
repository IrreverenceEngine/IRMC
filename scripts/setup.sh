#!/bin/sh

set -e

mkdir -p build

cd build

echo "[IRMC] - STARTED SETUP"
cmake .. -DCMAKE_BUILD_TYPE=Debug -DSDL_STATIC=ON
echo "[IRMC] - FINISHED SETUP"

cd ..

ln -sf build/compile_commands.json .