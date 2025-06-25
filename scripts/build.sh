#!/bin/sh

set -e

mkdir -p build
cd build

echo "[IRMC] - STARTED BUILDING"
cmake --build . -j8
echo "[IRMC] - FINISHED BUILDING"

cd ..