#!/bin/sh
export MOODY_BASE=~/moody
export MOODY_SYSROOTS=~/moody/sysroots
export MOODY_TOOLCHAIN_DIR=~/moody/configs/cmake

PI_DIR=build-aarch_64
PC_DIR=build-x86_64

DEF_TOOLCHAIN="$MOODY_TOOLCHAIN_DIR/x86-default.cmake"
PI_TOOLCHAIN="$MOODY_TOOLCHAIN_DIR/rpi-aarch64-linux.cmake"
INSTALL_DIR="$MOODY_SYSROOTS/rpi-aarch64"
ROOT_DIR=$(pwd)

export PKG_CONFIG_SYSROOT_DIR=$INSTALL_DIR
export PKG_CONFIG_PATH=$INSTALL_DIR/usr/lib/aarch64-linux-gnu/pkgconfig:$SYSROOT/usr/share/pkgconfig
export PKG_CONFIG_LIBDIR=$PKG_CONFIG_PATH

# Build for x86
mkdir -p "$PC_DIR"
[ -f "$PC_DIR/CMakeCache.txt" ] && rm -rf "$PC_DIR/CMakeCache.txt" "$PC_DIR/CMakeFiles"

echo "Configuring and building for x86..."
cd "$PC_DIR" || exit 1
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_TOOLCHAIN_FILE="$DEF_TOOLCHAIN" ..
cmake --build . --parallel 8
echo "Build for x86 done."

cd "$ROOT_DIR" || exit 1

# Build for ARM
mkdir -p "$PI_DIR"
[ -f "$PI_DIR/CMakeCache.txt" ] && rm -rf "$PI_DIR/CMakeCache.txt" "$PI_DIR/CMakeFiles"

echo "Configuring and building for aarch64..."
cd "$PI_DIR" || exit 1
cmake -DRPI=ON \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      -DCMAKE_TOOLCHAIN_FILE="$PI_TOOLCHAIN" \
      -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" ..
cmake --build . --parallel 8
echo "Build for aarch64 done."

cd "$ROOT_DIR" 

exit 1