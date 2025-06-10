cmake_minimum_required(VERSION 3.4.1)
add_library(faceUtils SHARED native-lib.cpp)

if (ANDROID_ABI STREQUAL "armeabi-v7a")
    target_compile_options(faceUtils PRIVATE -mfpu=neon -mfloat-abi=softfp)
endif()
set_target_properties(faceUtils PROPERTIES CXX_FLAGS "-mfpu=neon")

target_link_libraries(faceUtils log android)
