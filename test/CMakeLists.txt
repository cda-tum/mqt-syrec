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

  unittests/syrec/parser/data/error/test_production_assignStatement_errors.cpp
  unittests/syrec/parser/data/error/test_production_binaryExpression_errors.cpp
  unittests/syrec/parser/data/error/test_production_callStatement_errors.cpp
  unittests/syrec/parser/data/error/test_production_forStatement_errors.cpp
  unittests/syrec/parser/data/error/test_production_ifStatement_errors.cpp
  unittests/syrec/parser/data/error/test_production_module_errors.cpp
  unittests/syrec/parser/data/error/test_production_number_errors.cpp
  unittests/syrec/parser/data/error/test_production_parameter_errors.cpp
  unittests/syrec/parser/data/error/test_production_shiftExpression_errors.cpp
  unittests/syrec/parser/data/error/test_production_signalList_errors.cpp
  unittests/syrec/parser/data/error/test_production_signal_errors.cpp
  unittests/syrec/parser/data/error/test_production_swapStatement_errors.cpp
  unittests/syrec/parser/data/error/test_production_unaryStatement_errors.cpp
  unittests/syrec/parser/data/error/test_production_uncallStatement_errors.cpp

  unittests/syrec/parser/test_base_symbol_table.cpp
  unittests/syrec/parser/test_syrec_parser.cpp
  unittests/syrec/parser/test_syrec_parser_module_overload_resolution.cpp
  unittests/syrec/parser/test_syrec_parser_variable_overlap_errors.cpp
  unittests/syrec/parser/test_temporary_variable_scope.cpp
  unittests/syrec/parser/test_variable_access_index_validity.cpp
  unittests/syrec/parser/test_variable_overlap_check.cpp)

target_include_directories(${MQT_SYREC_TESTS_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/unittests/syrec/parser/utils)

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

  # Solution is sufficient for now but surely there is a better way where this explicit recreation of the folder structure is not necessary
  # I.e. we might only need to update the paths for creation of the symlinks correctly since there files are only copied to the ouput directory?
  COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}_test>/unittests
  COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}_test>/unittests/syrec
  COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}_test>/unittests/syrec/parser
  COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}_test>/unittests/syrec/parser/data
  COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}_test>/unittests/syrec/parser/success

  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/unittests
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/unittests/syrec
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/unittests/syrec/parser
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/unittests/syrec/parser/data
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/unittests/syrec/parser/success

  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/unittests/syrec/parser/data/success
          $<TARGET_FILE_DIR:${PROJECT_NAME}_test>/unittests/syrec/parser/data/success
  COMMAND ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE_DIR:${PROJECT_NAME}_test>/unittests/syrec/parser/data/success
          ${CMAKE_BINARY_DIR}/unittests/syrec/parser/data/success

  COMMENT "Copying circuits and creating symlinks for ${PROJECT_NAME}_test"
  VERBATIM)