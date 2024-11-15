
# now build app's shared lib
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
cmake_minimum_required(VERSION 3.10.2)

set(libmic ${CMAKE_SOURCE_DIR}/libunimic)
set(libname UnimicHalJNI)
set(libtiny ${CMAKE_SOURCE_DIR}/libtinyalsa)

include_directories(
        include
        ${libmic}/include
)
link_directories(
        ${libmic}/${ANDROID_ABI})

include_directories(
        include
        ${libtiny}/include
)
link_directories(
        ${libtiny}/${ANDROID_ABI})

#include_directories(${CMAKE_SOURCE_DIR}/includes/)

file(GLOB_RECURSE ALL_SOURCE "cpp/*.cpp" "cpp/*.h")

add_library( # Sets the name of the library.
        ${libname}

        SHARED

        ${ALL_SOURCE})

set(uni_mic

        uni4michal)

set(tiny

        tinyalsa)

set(native-libs

        android
        log
        m
        z)

#add_library(libuni4michal  SHARED  IMPORTED)
#set_target_properties(libuni4michal PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../../../libs/${ANDROID_ABI}/libuni4michal.so)


target_link_libraries( # Specifies the target library.
        ${libname}

        ${uni_mic}
        ${tiny}
        ${native-libs})
