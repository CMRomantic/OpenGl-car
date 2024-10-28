
rm -rf build_32
mkdir -p build_32
cd build_32

cmake \
    -D CMAKE_SYSTEM_NAME=Android \
    -D CMAKE_SYSTEM_VERSION=21 \
    -D ANDROID_PLATFORM=android-21 \
    -D CMAKE_BUILD_TYPE=Release \
    -D CMAKE_TOOLCHAIN_FILE=/home/cmm/code/android-ndk-r21b/build/cmake/android.toolchain.cmake \
    -D CMAKE_ANDROID_NDK=/home/cmm/code/android-ndk-r21b \
    -D CMAKE_ANDROID_ARCH_ABI=armeabi-v7a \
    ..


make -j4

