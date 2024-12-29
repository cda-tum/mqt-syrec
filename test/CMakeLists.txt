set(MQT_SYREC_TESTS_TARGET "${PROJECT_NAME}_test")

 # create an executable in which the tests will be stored
add_executable(${MQT_SYREC_TESTS_TARGET} 
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
  unittests/test_dd_synthesis_dc.cpp

  unittests/syrec/parser/test_base_symbol_table.cpp
)

# link the Google test infrastructure and a default main function to the test executable.
target_link_libraries(${MQT_SYREC_TESTS_TARGET} PRIVATE ${MQT_SYREC_TARGET} gmock gtest_main)

# discover tests
gtest_discover_tests(
    ${MQT_SYREC_TESTS_TARGET}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" DISCOVERY_TIMEOUT 60)
set_target_properties(${MQT_SYREC_TESTS_TARGET} PROPERTIES FOLDER tests)

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

#add_custom_command(
#  TARGET ${MQT_SYREC_TESTS_TARGET}
#  POST_BUILD
#  COMMAND
#    ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE_DIR:${MQT_SYREC_TESTS_TARGET}/${MQT_SYREC_TESTS_TARGET}
#    ${CMAKE_BINARY_DIR}/${MQT_SYREC_TESTS_TARGET}
#  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/circuits
#          $<TARGET_FILE_DIR:${MQT_SYREC_TESTS_TARGET}>/circuits
#  COMMAND ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE_DIR:${MQT_SYREC_TESTS_TARGET}>/circuits
#          ${CMAKE_BINARY_DIR}/circuits
#  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/configs
#          $<TARGET_FILE_DIR:${MQT_SYREC_TESTS_TARGET}>/configs
#  COMMAND ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE_DIR:${MQT_SYREC_TESTS_TARGET}>/configs
#          ${CMAKE_BINARY_DIR}/configs
#  COMMENT "Copying circuits and creating symlinks for ${MQT_SYREC_TESTS_TARGET}"
#  VERBATIM)
