# set required cmake version
cmake_minimum_required(VERSION 3.19...3.27)

project(
  syrec
  LANGUAGES CXX
  DESCRIPTION "SyReC - A Programming Language for the Synthesis of Reversible Circuits")

# check whether the submodule ``modulename`` is correctly cloned in the ``/extern`` directory.
macro(CHECK_SUBMODULE_PRESENT modulename)
  if(NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/${modulename}/CMakeLists.txt")
    message(
      FATAL_ERROR
        "${modulename} submodule not cloned properly. \
        Please run `git submodule update --init --recursive` \
        from the main project directory")
  endif()
endmacro()

check_submodule_present(mqt-core)

option(BUILD_MQT_SYREC_BINDINGS "Build the MQT SYREC Python bindings" OFF)
if(BUILD_MQT_SYREC_BINDINGS)
  # ensure that the BINDINGS option is set
  set(BINDINGS
      ON
      CACHE BOOL "Enable settings related to Python bindings" FORCE)
  # cmake-lint: disable=C0103
  set(Python_FIND_VIRTUALENV
      FIRST
      CACHE STRING "Give precedence to virtualenvs when searching for Python")
  # cmake-lint: disable=C0103
  set(Python_ARTIFACTS_INTERACTIVE
      ON
      CACHE BOOL "Prevent multiple searches for Python and instead cache the results.")
  # top-level call to find Python
  find_package(
    Python 3.8 REQUIRED
    COMPONENTS Interpreter Development.Module
    OPTIONAL_COMPONENTS Development.SABIModule)
endif()

# add main library code
add_subdirectory(src)

# add test code
option(BUILD_MQT_SYREC_TESTS "Also build tests for the MQT SYREC project" ON)
if(BUILD_MQT_SYREC_TESTS)
  enable_testing()
  include(GoogleTest)
  add_subdirectory(test)
endif()
