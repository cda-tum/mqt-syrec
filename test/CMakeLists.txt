if(NOT TARGET gtest OR NOT TARGET gmock)
  # Prevent overriding the parent project's compiler/linker settings on Windows
  set(gtest_force_shared_crt # cmake-lint: disable=C0103
      ON
      CACHE BOOL "" FORCE)
  add_subdirectory("${PROJECT_SOURCE_DIR}/extern/mqt-core/extern/googletest"
                   "extern/mqt-core/extern/googletest" EXCLUDE_FROM_ALL)
  set_target_properties(gtest gtest_main gmock gmock_main PROPERTIES FOLDER extern)
endif()

package_add_test(
  ${PROJECT_NAME}_test
  ${PROJECT_NAME}
  unittests/test_parser.cpp
  unittests/test_line_aware_synthesis.cpp
  unittests/test_line_aware_simulation.cpp
  unittests/test_cost_aware_synthesis.cpp
  unittests/test_cost_aware_simulation.cpp
  unittests/test_tt_to_dd.cpp
  unittests/test_extend_tt.cpp
  unittests/test_huffman.cpp
  unittests/test_pla_parser.cpp
  unittests/test_dd_synthesis.cpp
  unittests/test_dd_synthesis_dc.cpp)

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
