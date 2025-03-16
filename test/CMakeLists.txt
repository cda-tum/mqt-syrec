# collect all test files
file(GLOB_RECURSE TEST_FILES "*.cpp")

# add test executable
package_add_test(${PROJECT_NAME}-test MQT::SyReC ${TEST_FILES})
target_link_libraries(${PROJECT_NAME}-test PRIVATE fmt::fmt)
target_include_directories(${PROJECT_NAME}-test
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/unittests/syrec/parser/utils)

add_custom_command(
  TARGET ${PROJECT_NAME}-test
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE_DIR:${PROJECT_NAME}-test>/${PROJECT_NAME}-test
    ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-test
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/circuits
          $<TARGET_FILE_DIR:${PROJECT_NAME}-test>/circuits
  COMMAND ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE_DIR:${PROJECT_NAME}-test>/circuits
          ${CMAKE_BINARY_DIR}/circuits
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/configs
          $<TARGET_FILE_DIR:${PROJECT_NAME}-test>/configs
  COMMAND ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE_DIR:${PROJECT_NAME}-test>/configs
          ${CMAKE_BINARY_DIR}/configs
  # Solution is sufficient for now but surely there is a better way where this explicit recreation
  # of the folder structure is not necessary I.e. we might only need to update the paths for
  # creation of the symlinks correctly since there files are only copied to the output directory?
  COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}-test>/unittests
  COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}-test>/unittests/syrec
  COMMAND ${CMAKE_COMMAND} -E make_directory
          $<TARGET_FILE_DIR:${PROJECT_NAME}-test>/unittests/syrec/parser
  COMMAND ${CMAKE_COMMAND} -E make_directory
          $<TARGET_FILE_DIR:${PROJECT_NAME}-test>/unittests/syrec/parser/data
  COMMAND ${CMAKE_COMMAND} -E make_directory
          $<TARGET_FILE_DIR:${PROJECT_NAME}-test>/unittests/syrec/parser/success
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/unittests
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/unittests/syrec
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/unittests/syrec/parser
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/unittests/syrec/parser/data
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/unittests/syrec/parser/success
  COMMAND
    ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/unittests/syrec/parser/data/success
    $<TARGET_FILE_DIR:${PROJECT_NAME}-test>/unittests/syrec/parser/data/success
  COMMAND
    ${CMAKE_COMMAND} -E create_symlink
    $<TARGET_FILE_DIR:${PROJECT_NAME}-test>/unittests/syrec/parser/data/success
    ${CMAKE_BINARY_DIR}/unittests/syrec/parser/data/success
  COMMENT "Copying circuits and creating symlinks for ${PROJECT_NAME}-test"
  VERBATIM)
