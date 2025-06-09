
cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
# now build app's shared lib
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
#set(CMAKE_VERBOSE_MAKEFILE on)

set(libname ffmpeg-convert)
set(libffmpeg ${CMAKE_SOURCE_DIR}/libffmpeg)

file(GLOB native_files "cpp/*.cpp" "src/*.h")

add_library(${libname}

           SHARED

           ${native_files})
		   

include_directories(
        include
        ${libffmpeg}/include
)


# 设置 FFmpeg 库路径
set(FFMPEG_LIB_DIR ${libffmpeg}/libs/${ANDROID_ABI})
		
# 添加 FFmpeg 静态库
add_library(avformat STATIC IMPORTED)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION ${FFMPEG_LIB_DIR}/libavformat.a)

add_library(avcodec STATIC IMPORTED)
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION ${FFMPEG_LIB_DIR}/libavcodec.a)

add_library(avfilter STATIC IMPORTED)
set_target_properties(avfilter PROPERTIES IMPORTED_LOCATION ${FFMPEG_LIB_DIR}/libavfilter.a)

add_library(swresample STATIC IMPORTED)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION ${FFMPEG_LIB_DIR}/libswresample.a)

add_library(swscale STATIC IMPORTED)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION ${FFMPEG_LIB_DIR}/libswscale.a)

add_library(postproc STATIC IMPORTED)
set_target_properties(postproc PROPERTIES IMPORTED_LOCATION ${FFMPEG_LIB_DIR}/libpostproc.a)

add_library(avutil STATIC IMPORTED)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION ${FFMPEG_LIB_DIR}/libavutil.a)
		
set(native-libs

        android
        log
        m
        z)


# 链接所有库
target_link_libraries(
    ${libname}
    avformat
    avcodec
    avfilter
    swresample
    swscale
    postproc
    avutil
    ${native-libs}
)
