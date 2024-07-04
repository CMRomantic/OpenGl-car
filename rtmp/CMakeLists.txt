cmake_minimum_required(VERSION 3.4.1)

# now build app's shared lib
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
set(libffmpeg ${CMAKE_SOURCE_DIR}/ffmpeg)
set(libyuv ${CMAKE_SOURCE_DIR}/yuv)
set(libname unisound-stream)

# ffmpeg lib include	start
include_directories(
        include
        ${libffmpeg}/include
)
link_directories(
        ${libffmpeg}/libs)
# ffmpeg lib include	end

# libyuv lib include	start
include_directories(
        include
        ${libyuv}/include
)

link_directories(
        ${libyuv}/libs)
# libyuv lib include	end

file(GLOB_RECURSE ALL_SOURCE "cpp/*.cpp" "cpp/*.h")

add_library( # Sets the name of the library.
        ${libname}

        SHARED

        ${ALL_SOURCE})


set(ffmpeg-libs

        avformat
        avcodec
        avfilter
        swresample
        swscale
        postproc
        avutil)

set(yuv-libs

        yuv)

set(native-libs

        android
        log)

target_link_libraries( # Specifies the target library.
        ${libname}

        ${log-lib}
        ${ffmpeg-libs}
        ${yuv-libs}
        ${native-libs})
