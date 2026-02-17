#!/bin/sh
export MY_OWN_SYSROOTS=~/opt/sysroots

PI_DIR=build-aarch_64
PC_DIR=build-x86_64


INSTALL_DIR="$MY_OWN_SYSROOT/R-Pi5"
ROOT_DIR=$(pwd)

# Build for x86
mkdir -p "$PC_DIR"
[ -f "$PC_DIR/CMakeCache.txt" ] && rm -rf "$PC_DIR/CMakeCache.txt" "$PC_DIR/CMakeFiles"

echo "Configuring and building for x86..."
cd "$PC_DIR" || exit 1
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_TOOLCHAIN_FILE=../toolchain-x86.cmake ..
cmake --build .
echo "Build for x86 done."

cd "$ROOT_DIR" || exit 1

# Build for ARM
mkdir -p "$PI_DIR"
[ -f "$PI_DIR/CMakeCache.txt" ] && rm -rf "$PI_DIR/CMakeCache.txt" "$PI_DIR/CMakeFiles"

echo "Configuring and building for aarch64..."
cd "$PI_DIR" || exit 1
cmake -DRPI=ON \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      -DCMAKE_TOOLCHAIN_FILE=../toolchain-aarch64.cmake \
      -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" ..
cmake --build .
echo "Build for aarch64 done."


cd "$ROOT_DIR" 

# Generate changes to Doxyfile
make regen-docs

exit 1