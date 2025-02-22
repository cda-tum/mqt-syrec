#
# https://github.com/antlr/antlr4/blob/master/runtime/Cpp/cmake/ExternalAntlr4Cpp.cmake
#
if(POLICY CMP0114)
    cmake_policy(SET CMP0114 NEW)
endif()
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

INCLUDE(ExternalProject)
FIND_PACKAGE(Git REQUIRED)

set(ANTLR4_ROOT ${CMAKE_CURRENT_BINARY_DIR}/antlr4_runtime/src/antlr4_runtime)
set(ANTLR4_INCLUDE_DIRS ${ANTLR4_ROOT}/runtime/Cpp/runtime/src)
set(ANTLR4_GIT_REPOSITORY https://github.com/antlr/antlr4.git)
set(ANTLR4_TAG "master"
    CACHE STRING "Antlr4 runtime identifier (tag, branch or commit hash)")
set(ANTLR4_CPP_LANG_VERSION 17)

# Ensure that the include dir already exists at configure time (to avoid cmake erroring
# on non-existent include dirs)
file(MAKE_DIRECTORY "${ANTLR4_INCLUDE_DIRS}")

if(${CMAKE_GENERATOR} MATCHES "Visual Studio.*")
  set(ANTLR4_OUTPUT_DIR ${ANTLR4_ROOT}/runtime/Cpp/runtime/$(Configuration))
elseif(${CMAKE_GENERATOR} MATCHES "Xcode.*")
  set(ANTLR4_OUTPUT_DIR ${ANTLR4_ROOT}/runtime/Cpp/runtime/$(CONFIGURATION))
else()
  set(ANTLR4_OUTPUT_DIR ${ANTLR4_ROOT}/runtime/Cpp/runtime)
endif()

message(STATUS "ANTLR will use CMake generator: ${CMAKE_GENERATOR}")
message(STATUS "ANLTR libraries output directory was defined based on compiler id: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "ANLTR libraries will use output directory: ${ANTLR4_OUTPUT_DIR}")
message(STATUS "ANTLR will be compiled using compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "ANTLR will be built using C++ language version: ${ANTLR4_CPP_LANG_VERSION}")

if(NOT DEFINED ANTLR4_WITH_STATIC_CRT)
  if (MSVC OR WIN32)
     # using /MD flag for antlr4_runtime (for Visual C++ compilers only)
    set(ANTLR4_WITH_STATIC_CRT OFF)
  else()
    set(ANTLR4_WITH_STATIC_CRT ON)
  endif()
endif()

if(MSVC OR WIN32)
  set(ANTLR4_STATIC_LIBRARIES
      ${ANTLR4_OUTPUT_DIR}/antlr4-runtime-static.lib)
  set(ANTLR4_SHARED_LIBRARIES
      ${ANTLR4_OUTPUT_DIR}/antlr4-runtime.lib)
  set(ANTLR4_RUNTIME_LIBRARIES
      ${ANTLR4_OUTPUT_DIR}/antlr4-runtime.dll)
else()
  set(ANTLR4_STATIC_LIBRARIES
      ${ANTLR4_OUTPUT_DIR}/libantlr4-runtime.a)
  if(MINGW)
    set(ANTLR4_SHARED_LIBRARIES
        ${ANTLR4_OUTPUT_DIR}/libantlr4-runtime.dll.a)
    set(ANTLR4_RUNTIME_LIBRARIES
        ${ANTLR4_OUTPUT_DIR}/libantlr4-runtime.dll)
  elseif(CYGWIN)
    set(ANTLR4_SHARED_LIBRARIES
        ${ANTLR4_OUTPUT_DIR}/libantlr4-runtime.dll.a)
    set(ANTLR4_RUNTIME_LIBRARIES
        ${ANTLR4_OUTPUT_DIR}/cygantlr4-runtime-4.11.1.dll)
  elseif(APPLE)
    set(ANTLR4_RUNTIME_LIBRARIES
        ${ANTLR4_OUTPUT_DIR}/libantlr4-runtime.dylib)
  else()
    set(ANTLR4_RUNTIME_LIBRARIES
        ${ANTLR4_OUTPUT_DIR}/libantlr4-runtime.so)
  endif()
endif()

if(${CMAKE_GENERATOR} MATCHES ".* Makefiles")
  # This avoids
  # 'warning: jobserver unavailable: using -j1. Add '+' to parent make rule.'
  set(ANTLR4_BUILD_COMMAND $(MAKE))
elseif(${CMAKE_GENERATOR} MATCHES "Visual Studio.*")
  set(ANTLR4_BUILD_COMMAND
      ${CMAKE_COMMAND}
          --build .
          --config $(Configuration)
          --target)
elseif(${CMAKE_GENERATOR} MATCHES "Xcode.*")
  set(ANTLR4_BUILD_COMMAND
      ${CMAKE_COMMAND}
          --build .
          --config $(CONFIGURATION)
          --target)
else()
  set(ANTLR4_BUILD_COMMAND
      ${CMAKE_COMMAND}
          --build .
          --target)
endif()

if(DEFINED CMAKE_CXX_COMPILER AND DEFINED CMAKE_C_COMPILER)
    ExternalProject_Add(
      antlr4_runtime
      PREFIX antlr4_runtime
      GIT_PROGRESS 0
      GIT_REPOSITORY ${ANTLR4_GIT_REPOSITORY}
      GIT_SHALLOW 1
      GIT_TAG ${ANTLR4_TAG}
      DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}
      BUILD_COMMAND ""
      BUILD_IN_SOURCE 1
      SOURCE_DIR ${ANTLR4_ROOT}
      SOURCE_SUBDIR runtime/Cpp
      CMAKE_ARGS
         -DCMAKE_CXX_STANDARD:STRING=${ANTLR4_CPP_LANG_VERSION} # if desired, compile the runtime with a different C++ standard
         -DCMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER} # omitting this parameter will cause ANTLR to be compiled with MSVC compiler when generated/built with visual studio instead of another compiler (i.e. clang)
         -DCMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER} # omitting this parameter will cause ANTLR to be compiled with MSVC compiler when generated/built with visual studio instead of another compiler (i.e. clang)
         -DANTLR_BUILD_CPP_TESTS:BOOL=OFF
         -DWITH_DEMO:BOOL=OFF
      CMAKE_CACHE_ARGS
          -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
          -DWITH_STATIC_CRT:BOOL=${ANTLR4_WITH_STATIC_CRT}
          -DDISABLE_WARNINGS:BOOL=ON 
      INSTALL_COMMAND ""
      EXCLUDE_FROM_ALL 1)
else()
    ExternalProject_Add(
      antlr4_runtime
      PREFIX antlr4_runtime
      GIT_PROGRESS 0
      GIT_REPOSITORY ${ANTLR4_GIT_REPOSITORY}
      GIT_SHALLOW 1
      GIT_TAG ${ANTLR4_TAG}
      DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}
      BUILD_COMMAND ""
      BUILD_IN_SOURCE 1
      SOURCE_DIR ${ANTLR4_ROOT}
      SOURCE_SUBDIR runtime/Cpp
      CMAKE_ARGS
         -DCMAKE_CXX_STANDARD:STRING=${ANTLR4_CPP_LANG_VERSION} # if desired, compile the runtime with a different C++ standard
         -DANTLR_BUILD_CPP_TESTS:BOOL=OFF
         -DWITH_DEMO:BOOL=OFF
      CMAKE_CACHE_ARGS
          -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
          -DWITH_STATIC_CRT:BOOL=${ANTLR4_WITH_STATIC_CRT}
          -DDISABLE_WARNINGS:BOOL=ON 
      INSTALL_COMMAND ""
      EXCLUDE_FROM_ALL 1)
endif()

      
# Separate build step as rarely people want both
set(ANTLR4_BUILD_DIR ${ANTLR4_ROOT})
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.14.0")
  # CMake 3.14 builds in above's SOURCE_SUBDIR when BUILD_IN_SOURCE is true
  set(ANTLR4_BUILD_DIR ${ANTLR4_ROOT}/runtime/Cpp)
endif()

ExternalProject_Add_Step(
    antlr4_runtime
    build_static
    COMMAND ${ANTLR4_BUILD_COMMAND} antlr4_static
    # Depend on target instead of step (a custom command)
    # to avoid running dependent steps concurrently
    DEPENDS antlr4_runtime
    BYPRODUCTS ${ANTLR4_STATIC_LIBRARIES}
    EXCLUDE_FROM_MAIN 1
    WORKING_DIRECTORY ${ANTLR4_BUILD_DIR})
ExternalProject_Add_StepTargets(antlr4_runtime build_static)

add_library(antlr4_static STATIC IMPORTED)
add_dependencies(antlr4_static antlr4_runtime-build_static)
set_target_properties(antlr4_static PROPERTIES
                      IMPORTED_LOCATION ${ANTLR4_STATIC_LIBRARIES})
target_include_directories(antlr4_static
    INTERFACE
        ${ANTLR4_INCLUDE_DIRS}
)

ExternalProject_Add_Step(
    antlr4_runtime
    build_shared
    COMMAND ${ANTLR4_BUILD_COMMAND} antlr4_shared
    # Depend on target instead of step (a custom command)
    # to avoid running dependent steps concurrently
    DEPENDS antlr4_runtime
    BYPRODUCTS ${ANTLR4_SHARED_LIBRARIES} ${ANTLR4_RUNTIME_LIBRARIES}
    EXCLUDE_FROM_MAIN 1
    WORKING_DIRECTORY ${ANTLR4_BUILD_DIR})
ExternalProject_Add_StepTargets(antlr4_runtime build_shared)

add_library(antlr4_shared SHARED IMPORTED)
add_dependencies(antlr4_shared antlr4_runtime-build_shared)
set_target_properties(antlr4_shared PROPERTIES
                      IMPORTED_LOCATION ${ANTLR4_RUNTIME_LIBRARIES})
target_include_directories(antlr4_shared
    INTERFACE
        ${ANTLR4_INCLUDE_DIRS}
)

if(ANTLR4_SHARED_LIBRARIES)
  set_target_properties(antlr4_shared PROPERTIES
                        IMPORTED_IMPLIB ${ANTLR4_SHARED_LIBRARIES})
endif()