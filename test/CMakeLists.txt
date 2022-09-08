if(NOT TARGET gtest OR NOT TARGET gmock)
  # Prevent overriding the parent project's compiler/linker settings on Windows
  set(gtest_force_shared_crt # cmake-lint: disable=C0103
      ON
      CACHE BOOL "" FORCE)
  add_subdirectory("${PROJECT_SOURCE_DIR}/extern/qfr/extern/dd_package/extern/googletest"
                   "extern/qfr/extern/dd_package/extern/googletest" EXCLUDE_FROM_ALL)
  mark_as_advanced(
    BUILD_GMOCK
    BUILD_GTEST
    BUILD_SHARED_LIBS
    gmock_build_tests
    gtest_build_samples
    gtest_build_tests
    gtest_disable_pthreads
    gtest_force_shared_crt
    gtest_hide_internal_symbols)
  set_target_properties(gtest gtest_main gmock gmock_main PROPERTIES FOLDER extern)
  if(BINDINGS)
    # adjust visibility settings for building Python bindings
    target_compile_options(gtest PUBLIC -fvisibility=hidden)
    target_compile_options(gmock PUBLIC -fvisibility=hidden)
  endif()
endif()

# add a C++ test executable ``testname`` providing all source files as additional arguments.
macro(PACKAGE_ADD_TEST testname)
  # create an executable in which the tests will be stored
  add_executable(${testname} ${ARGN})
  # link the Google test infrastructure and a default main function to the test executable.
  target_link_libraries(${testname} PUBLIC MQT::${PROJECT_NAME} gmock gtest_main)
  # discover tests
  gtest_discover_tests(
    ${testname}
    WORKING_DIRECTORY ${PROJECT_DIR}
    PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}")
  set_target_properties(${testname} PROPERTIES FOLDER tests)
  enable_lto(${testname})
endmacro()

package_add_test(
  ${PROJECT_NAME}_test
  unittests/test_parser.cpp
  unittests/test_synthesis.cpp
  unittests/test_simulation.cpp
  unittests/test_add_lines_synthesis.cpp
  unittests/test_add_lines_simulation.cpp
  unittests/test_tt_to_dd.cpp
  unittests/test_extend_tt.cpp
  unittests/test_huffman.cpp
  unittests/test_pla_parser.cpp
  unittests/test_dd_synthesis.cpp)

add_custom_command(
  TARGET ${PROJECT_NAME}_test
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE_DIR:${PROJECT_NAME}_test>/${PROJECT_NAME}_test
    ${CMAKE_BINARY_DIR}/${PROJECT_NAME}_test
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/circuits
          $<TARGET_FILE_DIR:${PROJECT_NAME}_test>/circuits
  COMMAND ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE_DIR:${PROJECT_NAME}_test>/circuits
          ${CMAKE_BINARY_DIR}/circuits
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/configs
          $<TARGET_FILE_DIR:${PROJECT_NAME}_test>/configs
  COMMAND ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE_DIR:${PROJECT_NAME}_test>/configs
          ${CMAKE_BINARY_DIR}/configs
  COMMENT "Copying circuits and creating symlinks for ${PROJECT_NAME}_test"
  VERBATIM)
