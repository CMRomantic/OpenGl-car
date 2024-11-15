#!/bin/sh

# ./build.sh [arm | arm64]
if [ "$1" = "arm" ]; then
    echo "---------start build uni ssp hal armeabi-v7a-------"
    ABI="armeabi-v7a"
elif [ "$1" = "arm64" ] ; then
    echo "---------start build uni ssp hal arm64-v8a-------"
    ABI="arm64-v8a"
else
    echo "---------start build uni ssp hal armeabi-v7a arm64-v8a-------"
fi

NDK=/home/unisound/code/android-ndk-r23c
TOOLCHAIN=$NDK/build/cmake/android.toolchain.cmake
PLATFORM=21

OUTPUT_DIR=build
mkdir $OUTPUT_DIR

# 单独编译 armeabi-v7a
if [ "$ABI" = "armeabi-v7a" ] || [ -z "$ABI" ]; then
  echo "---------start build arm $ABI-------"
    cmake \
        -D CMAKE_SYSTEM_NAME=Android \
        -D CMAKE_SYSTEM_VERSION=$PLATFORM \
        -D ANDROID_PLATFORM=android-$PLATFORM \
        -D CMAKE_BUILD_TYPE=Release \
        -D CMAKE_TOOLCHAIN_FILE=$TOOLCHAIN \
        -D CMAKE_ANDROID_NDK=$NDK \
        -D CMAKE_ANDROID_ARCH_ABI=armeabi-v7a \
        -D ANDROID_ABI=armeabi-v7a \
        -S . \
        -B $OUTPUT_DIR
    cmake --build $OUTPUT_DIR -- -j4
  echo "---------build arm $ABI finished-------"
fi

if [ "$ABI" = "arm64-v8a" ] || [ -z "$ABI" ]; then
  echo "---------start build arm64 $ABI-------"
    # 单独编译 arm64-v8a
    cmake \
        -D CMAKE_SYSTEM_NAME=Android \
        -D CMAKE_SYSTEM_VERSION=$PLATFORM \
        -D ANDROID_PLATFORM=android-$PLATFORM \
        -D CMAKE_BUILD_TYPE=Release \
        -D CMAKE_TOOLCHAIN_FILE=$TOOLCHAIN \
        -D CMAKE_ANDROID_NDK=$NDK \
        -D CMAKE_ANDROID_ARCH_ABI=arm64-v8a \
        -D ANDROID_ABI=arm64-v8a \
        -S . \
        -B $OUTPUT_DIR
    cmake --build $OUTPUT_DIR -- -j4
  echo "---------build arm64 $ABI finished-------"
fi

cp_libs() {
  # 确保 output 目录存在
  if [ ! -d "output" ]; then
    mkdir output
  fi

  # 确保 config 目录存在
  if [ ! -d "output/config" ]; then
    mkdir output/config
  fi

  # 确保 ABI目录存在，存在时先删除再重新创建
  if [ -d "output/$ABI" ]; then
    rm -rf output/$ABI
  fi
  mkdir -p output/$ABI

  cp ssp_hal/config/*.* output/config

  cp ssp_hal/libs/$ABI/*.so output/$ABI

  cp $OUTPUT_DIR/libs/$ABI/*.so output/$ABI
}

cp_libs

echo "---------remove build folder $OUTPUT_DIR-------"
rm -rf $OUTPUT_DIR