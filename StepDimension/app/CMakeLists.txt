# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

include(ndk-stl-config.cmake)

# Export ANativeActivity_onCreate(),
# Refer to: https://github.com/android-ndk/ndk/issues/381.
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

message(STATUS "ABI ${ANDROID_ABI}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../SFML/SFML/include)
#link_directories(${CMAKE_CURRENT_SOURCE_DIR}/jniLibs/${ANDROID_ABI})

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/native-lib.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/native-lib.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/StateMachine.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/StateMachine.hpp)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

add_library(sfml-system SHARED IMPORTED)
set_target_properties( # Specifies the target library.
        sfml-system

        # Specifies the parameter you want to define.
        PROPERTIES IMPORTED_LOCATION

        # Provides the path to the library you want to import.
        ${CMAKE_CURRENT_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/libsfml-system-d.so )

add_library(sfml-activity SHARED IMPORTED)
set_target_properties( # Specifies the target library.
        sfml-activity

        # Specifies the parameter you want to define.
        PROPERTIES IMPORTED_LOCATION

        # Provides the path to the library you want to import.
        ${CMAKE_CURRENT_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/libsfml-activity-d.so )

add_library(sfml-audio SHARED IMPORTED)
set_target_properties( # Specifies the target library.
        sfml-audio

        # Specifies the parameter you want to define.
        PROPERTIES IMPORTED_LOCATION

        # Provides the path to the library you want to import.
        ${CMAKE_CURRENT_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/libsfml-audio-d.so )

add_library(sfml-window SHARED IMPORTED)
set_target_properties( # Specifies the target library.
        sfml-window

        # Specifies the parameter you want to define.
        PROPERTIES IMPORTED_LOCATION

        # Provides the path to the library you want to import.
        ${CMAKE_CURRENT_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/libsfml-window-d.so )

add_library(sfml-graphics SHARED IMPORTED)
set_target_properties( # Specifies the target library.
        sfml-graphics

        # Specifies the parameter you want to define.
        PROPERTIES IMPORTED_LOCATION

        # Provides the path to the library you want to import.
        ${CMAKE_CURRENT_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/libsfml-graphics-d.so )

add_library(sfml-network SHARED IMPORTED)
set_target_properties( # Specifies the target library.
        sfml-network

        # Specifies the parameter you want to define.
        PROPERTIES IMPORTED_LOCATION

        # Provides the path to the library you want to import.
        ${CMAKE_CURRENT_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/libsfml-network-d.so )

add_library(sfml-main STATIC IMPORTED)
set_target_properties( # Specifies the target library.
        sfml-main

        # Specifies the parameter you want to define.
        PROPERTIES IMPORTED_LOCATION

        # Provides the path to the library you want to import.
        ${CMAKE_CURRENT_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/libsfml-main-d.a )

add_library(openal SHARED IMPORTED)
set_target_properties( # Specifies the target library.
        openal

        # Specifies the parameter you want to define.
        PROPERTIES IMPORTED_LOCATION

        # Provides the path to the library you want to import.
        ${CMAKE_CURRENT_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/libopenal.so )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        native-lib

        # Links the target library to the log library
        # included in the NDK.

        log
        android
        EGL
        GLESv1_CM

        sfml-system

        openal

        sfml-window
        sfml-audio
        sfml-graphics
        sfml-network
        sfml-activity
        -Wl,--whole-archive sfml-main -Wl,--no-whole-archive
        )
