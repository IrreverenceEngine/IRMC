#!/bin/sh

set -e

mkdir -p build

cd build

echo "[IRMC] - STARTED SETUP"
cmake ..
echo "[IRMC] - FINISHED SETUP"

cd ..

ln -sf build/compile_commands.json .