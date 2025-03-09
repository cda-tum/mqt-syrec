# The original CMake configuration in the ANTLR C++ git repository (https://github.com/antlr/antlr4/blob/master/runtime/Cpp/cmake/ExternalAntlr4Cpp.cmake)
# uses the ExternalProject_XX functions to configure the built of the ANTLR runtime and serves as a reference from which this configuration file was built
# using the FetchContent_XX functions instead.

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

set(ANTLR4_GIT_REPOSITORY https://github.com/antlr/antlr4.git)
# ANTLR v4.13.2 - minor version update could include "minor" breaking changes, thus we include only a specific path version.
# https://github.com/antlr/antlr4?tab=readme-ov-file#versioning
set(ANTLR4_TAG "cc82115" CACHE STRING "Antlr4 runtime identifier (tag, branch or commit hash)") 
set(ANTLR_BUILD_CPP_TESTS BOOL OFF) 
#set(CMAKE_POSITION_INDEPENDENT_CODE BOOL ON)
set(DISABLE_WARNINGS BOOL ON) # Do not report compiler warnings for build of ANTLR runtime

option(ANTLR4_BUILD_AS_STATIC_LIBRARY "Build the ANTLR4 runtime as a static library (turned on by default)" ON)

message(STATUS "ANTLR git repo: ${ANTLR4_GIT_REPOSITORY}")
message(STATUS "ANTLR git tag: ${ANTLR4_TAG}")

if (NOT DEFINED WITH_STATIC_CRT AND (MSVC OR WIN32))
    # using /MD flag for antlr4_runtime (for Visual C++ compilers only)
    set(WITH_STATIC_CRT OFF)
endif()

if(ANTLR4_BUILD_AS_STATIC_LIBRARY)
    set(ANTLR_BUILD_STATIC BOOL ON)
    message(STATUS "ANTLR runtime library type: STATIC")

    FetchContent_Declare(
        antlr4_static
        GIT_PROGRESS 0
        GIT_REPOSITORY ${ANTLR4_GIT_REPOSITORY}
        GIT_SHALLOW 1
        GIT_TAG ${ANTLR4_TAG}
        SOURCE_SUBDIR runtime/Cpp
        EXCLUDE_FROM_ALL 1
    )
    FetchContent_MakeAvailable(antlr4_static)
else()
    set(ANTLR_BUILD_SHARED BOOL ON)
    message(STATUS "ANTLR runtime library type: SHARED")

    FetchContent_Declare(
        antlr4_shared
        GIT_PROGRESS 0
        GIT_REPOSITORY ${ANTLR4_GIT_REPOSITORY}
        GIT_SHALLOW 1
        GIT_TAG ${ANTLR4_TAG}
        SOURCE_SUBDIR runtime/Cpp
        EXCLUDE_FROM_ALL 1
    )
  FetchContent_MakeAvailable(antlr4_shared)
endif()