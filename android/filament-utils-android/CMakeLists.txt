cmake_minimum_required(VERSION 3.19)
project(filament-utils-android)

set(FILAMENT_DIR ${FILAMENT_DIST_DIR})
set(IMAGEIO_DIR ../../libs/imageio)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../gltfio-android ${CMAKE_CURRENT_BINARY_DIR}/gltfio-android)

add_library(camutils STATIC IMPORTED)
set_target_properties(camutils PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libcamutils.a)

add_library(image STATIC IMPORTED)
set_target_properties(image PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libimage.a)

add_library(viewer STATIC IMPORTED)
set_target_properties(viewer PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libviewer.a)

add_library(civetweb STATIC IMPORTED)
set_target_properties(civetweb PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libcivetweb.a)

add_library(iblprefilter STATIC IMPORTED)
set_target_properties(iblprefilter PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libfilament-iblprefilter.a)

set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/libfilament-utils-jni.map")

add_library(filament-utils-jni SHARED
        src/main/cpp/AutomationEngine.cpp
        src/main/cpp/Bookmark.cpp
        src/main/cpp/HDRLoader.cpp
        src/main/cpp/IBLPrefilterContext.cpp
        src/main/cpp/Utils.cpp
        src/main/cpp/Manipulator.cpp
        src/main/cpp/RemoteServer.cpp

        ${IMAGEIO_DIR}/include/imageio/ImageDecoder.h
        ${IMAGEIO_DIR}/include/imageio/HDRDecoder.h
        ${IMAGEIO_DIR}/src/HDRDecoder.cpp

        ../common/CallbackUtils.cpp
        ../common/NioUtils.cpp
)

target_compile_definitions(filament-utils-jni PUBLIC IMAGEIO_LITE=1)

target_include_directories(filament-utils-jni PRIVATE
        ${FILAMENT_DIR}/include
        ..
        ${IMAGEIO_DIR}/include
        ../../libs/utils/include)

set_target_properties(filament-utils-jni PROPERTIES LINK_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/libfilament-utils-jni.symbols)

# The ordering in the following list is important because CMake does not have dependency information.
target_link_libraries(filament-utils-jni
        gltfio-jni
        civetweb
        camutils
        iblprefilter
        image
        viewer
)
