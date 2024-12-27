cmake_minimum_required(VERSION 3.10.2)

# 设置release版本还是debug版本
if(${CMAKE_BUILD_TYPE} MATCHES "Release")
    message(STATUS "Release版本")
    set(BuildType "Release")
else()
    set(BuildType "Debug")
    message(STATUS "Debug版本")
endif()

# 全局启用调试信息
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

#设置生成的so动态库最后输出的路径
#设置bin和lib库目录
#设置bin和lib库目录
set(RELEASE_DIR ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${RELEASE_DIR}/libs/${ANDROID_ABI})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${RELEASE_DIR}/libs/${ANDROID_ABI})

set(libname UnimicHalJNI)
set(libtiny ${CMAKE_SOURCE_DIR}/tinyalsa)
set(libssp  ${CMAKE_SOURCE_DIR}/ssp_hal)

#定义 全局定义 UNI_ANDROID 宏
add_definitions(-DUNI_ANDROID)
add_definitions(-DDEBUG)  # 启用调试日志

#ssp_hal start
include_directories(
        include
        ${libssp}/include
)
link_directories(
        ${libssp}/libs/${ANDROID_ABI})
#ssp_hal end

include_directories(
        include
        ${libtiny}/include
)
link_directories(
        ${libtiny}/libs/${ANDROID_ABI})

# 添加多个 cpp 文件目录
file(GLOB_RECURSE ALL_SOURCE
        "cpp/*.cpp"
        "cpp/*.h"
        "${libssp}/src/*.c"
        "${libssp}/src/*.h"
#        "${libtiny}/*.c"
#        "${libtiny}/*.h"
#        "${libtiny}/jni/*.c"
#        "${libtiny}/jni/*.h"
        )

add_library( # Sets the name of the library.
        ${libname}

        SHARED

        ${ALL_SOURCE})

set(ssp_hal

        UalFDEnhance)

set(tiny

        tinyalsa)

set(native-libs

        android
        log
        m
        z)

target_link_libraries( # Specifies the target library.
        ${libname}

        ${ssp_hal}
        ${tiny}
        ${native-libs})
