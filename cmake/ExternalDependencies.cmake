# Declare all external dependencies and make sure that they are available.

include(FetchContent)
set(FETCH_PACKAGES "")

if(BUILD_MQT_SYREC_BINDINGS)
  # Manually detect the installed mqt-core package.
  execute_process(
    COMMAND "${Python_EXECUTABLE}" -m mqt.core --cmake_dir
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE mqt-core_DIR
    ERROR_QUIET)

  # Add the detected directory to the CMake prefix path.
  if(mqt-core_DIR)
    list(APPEND CMAKE_PREFIX_PATH "${mqt-core_DIR}")
    message(STATUS "Found mqt-core package: ${mqt-core_DIR}")
  endif()

  if(NOT SKBUILD)
    # Manually detect the installed pybind11 package.
    execute_process(
      COMMAND "${Python_EXECUTABLE}" -m pybind11 --cmakedir
      OUTPUT_STRIP_TRAILING_WHITESPACE
      OUTPUT_VARIABLE pybind11_DIR)

    # Add the detected directory to the CMake prefix path.
    list(APPEND CMAKE_PREFIX_PATH "${pybind11_DIR}")
  endif()

  # add pybind11 library
  find_package(pybind11 2.13.6 CONFIG REQUIRED)
endif()

# cmake-format: off
set(MQT_CORE_VERSION 3.0.0
    CACHE STRING "MQT Core version")
set(MQT_CORE_REV "be654c753e98e9062d796143dd3a591366370b2d"
    CACHE STRING "MQT Core identifier (tag, branch or commit hash)")
set(MQT_CORE_REPO_OWNER "cda-tum"
	CACHE STRING "MQT Core repository owner (change when using a fork)")
# cmake-format: on
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.24)
  FetchContent_Declare(
    mqt-core
    GIT_REPOSITORY https://github.com/${MQT_CORE_REPO_OWNER}/mqt-core.git
    GIT_TAG ${MQT_CORE_REV}
    FIND_PACKAGE_ARGS ${MQT_CORE_VERSION})
  list(APPEND FETCH_PACKAGES mqt-core)
else()
  find_package(mqt-core ${MQT_CORE_VERSION} QUIET)
  if(NOT mqt-core_FOUND)
    FetchContent_Declare(
      mqt-core
      GIT_REPOSITORY https://github.com/${MQT_CORE_REPO_OWNER}/mqt-core.git
      GIT_TAG ${MQT_CORE_REV})
    list(APPEND FETCH_PACKAGES mqt-core)
  endif()
endif()

if(BUILD_MQT_SYREC_TESTS)
  set(gtest_force_shared_crt
      ON
      CACHE BOOL "" FORCE)
  set(GTEST_VERSION
      1.14.0
      CACHE STRING "Google Test version")
  set(GTEST_URL https://github.com/google/googletest/archive/refs/tags/v${GTEST_VERSION}.tar.gz)
  if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.24)
    FetchContent_Declare(googletest URL ${GTEST_URL} FIND_PACKAGE_ARGS ${GTEST_VERSION} NAMES GTest)
    list(APPEND FETCH_PACKAGES googletest)
  else()
    find_package(googletest ${GTEST_VERSION} QUIET NAMES GTest)
    if(NOT googletest_FOUND)
      FetchContent_Declare(googletest URL ${GTEST_URL})
      list(APPEND FETCH_PACKAGES googletest)
    endif()
  endif()
endif()

set(FMT_VERSION
    11.0.2
    CACHE STRING "FMT library version")
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.24)
  FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG ${FMT_VERSION})
  list(APPEND FETCH_PACKAGES fmt)
else()
  find_package(fmt ${FMT_VERSION} QUIET)
  if(NOT fmt_FOUND)
    FetchContent_Declare(fmt GIT_REPOSITORY https://github.com/fmtlib/fmt.git)
    list(APPEND FETCH_PACKAGES fmt)
  endif()
endif()

# The original CMake configuration in the ANTLR C++ git repository
# (https://github.com/antlr/antlr4/blob/master/runtime/Cpp/cmake/ExternalAntlr4Cpp.cmake) uses the
# ExternalProject_XX functions to configure the built of the ANTLR runtime and serves as a reference
# from which this configuration file was built using the FetchContent_XX functions instead.

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

set(ANTLR4_GIT_REPOSITORY https://github.com/antlr/antlr4.git)
# ANTLR v4.13.2 - minor version update could include "minor" breaking changes, thus we include only
# a specific path version. https://github.com/antlr/antlr4?tab=readme-ov-file#versioning
set(ANTLR4_TAG
    "cc82115"
    CACHE STRING "Antlr4 runtime identifier (tag, branch or commit hash)")
set(ANTLR_BUILD_CPP_TESTS
    OFF
    CACHE BOOL "Should the ANTLR4 C++ runtime tests be built")
set(DISABLE_WARNINGS ON BOOL) # Do not report compiler warnings for build of ANTLR runtime
set(ANTLR4_BUILD_AS_STATIC_LIBRARY
    ON
    CACHE BOOL "Build the ANTLR4 runtime as a static library (turned on by default)")

message(STATUS "ANTLR git repo: ${ANTLR4_GIT_REPOSITORY}")
message(STATUS "ANTLR git tag: ${ANTLR4_TAG}")

if(NOT DEFINED WITH_STATIC_CRT AND (MSVC OR WIN32))
  # using /MD flag for antlr4_runtime (for Visual C++ compilers only)
  set(WITH_STATIC_CRT OFF)
endif()

if(ANTLR4_BUILD_AS_STATIC_LIBRARY)
  set(ANTLR_BUILD_STATIC
      ON
      CACHE INTERNAL BOOL)
  set(ANTLR_BUILD_SHARED
      OFF
      CACHE INTERNAL BOOL)
  message(STATUS "ANTLR runtime library type: STATIC")

  FetchContent_Declare(
    antlr4_static
    GIT_REPOSITORY ${ANTLR4_GIT_REPOSITORY}
    GIT_SHALLOW ON
    GIT_TAG ${ANTLR4_TAG}
    SOURCE_SUBDIR runtime/Cpp)
  list(APPEND FETCH_PACKAGES antlr4_static)
else()
  set(ANTLR_BUILD_STATIC
      OFF
      CACHE INTERNAL BOOL)
  set(ANTLR_BUILD_SHARED
      ON
      CACHE INTERNAL BOOL)
  message(STATUS "ANTLR runtime library type: SHARED")

  FetchContent_Declare(
    antlr4_shared
    GIT_REPOSITORY ${ANTLR4_GIT_REPOSITORY}
    GIT_SHALLOW ON
    GIT_TAG ${ANTLR4_TAG}
    SOURCE_SUBDIR runtime/Cpp)
  list(APPEND FETCH_PACKAGES antlr4_shared)
endif()

# Make all declared dependencies available.
FetchContent_MakeAvailable(${FETCH_PACKAGES})

if(ANTLR4_BUILD_AS_STATIC_LIBRARY)
  set(ANTLR4_INCLUDE_DIRS ${antlr4_static_SOURCE_DIR}/runtime/Cpp/runtime/src)
  # When linking the ANTLR4 static runtime to a shared library or executable, the position
  # independent code compiler options needs to be set otherwise the linker will fail
  # https://github.com/antlr/antlr4/issues/2776
  set_target_properties(antlr4_static PROPERTIES CMAKE_POSITION_INDEPENDENT_CODE ON)

  # Dlls do not use position independent code compiler option
  # (https://github.com/BVLC/caffe/issues/5992)
  if(NOT WIN32)
    target_compile_options(antlr4_static PUBLIC -fPIC)
  endif()
else()
  set(ANTLR4_INCLUDE_DIRS ${antlr4_shared_SOURCE_DIR}/runtime/Cpp/runtime/src)
endif()
