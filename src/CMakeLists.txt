if(NOT TARGET MQT::SyReC)
  # collect headers and source files
  file(GLOB_RECURSE SYREC_HEADERS ${MQT_SYREC_INCLUDE_BUILD_DIR}/*.hpp)
  file(GLOB_RECURSE SYREC_SOURCES **.cpp)
  list(FILTER SYREC_SOURCES EXCLUDE REGEX ".*/python/.*$")

  # TODO: Split library into IR and parser target and SyReC Package library (recheck .clang-tidy and
  # .clang-format checks)
  add_library(${PROJECT_NAME} ${SYREC_HEADERS} ${SYREC_SOURCES})

  # set public include directories
  target_include_directories(
    ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${MQT_SYREC_INCLUDE_BUILD_DIR}>
                           ${CMAKE_SOURCE_DIR}/include/core/syrec/parser/antlr)
  # set private include directories
  target_include_directories(${PROJECT_NAME} PRIVATE ${ANTLR4_INCLUDE_DIRS})

  # link to the MQT::Core libraries
  target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC MQT::CoreDD
    PRIVATE MQT::ProjectWarnings MQT::ProjectOptions fmt::fmt)

  if(ANTLR4_BUILD_AS_STATIC_LIBRARY)
    target_link_libraries(${PROJECT_NAME} PRIVATE antlr4_static)
  else()
    target_link_libraries(${PROJECT_NAME} PRIVATE antlr4_shared)
  endif()

  # add header-only part of the Boost library
  set(BOOST_USE_MULTITHREADED ON)
  set(BOOST_USE_STATIC_RUNTIME OFF)
  find_package(Boost 1.71 REQUIRED)
  target_link_libraries(${PROJECT_NAME} PUBLIC Boost::boost)

  # add MQT alias
  add_library(MQT::SyReC ALIAS ${PROJECT_NAME})
endif()

if(BUILD_MQT_SYREC_BINDINGS)
  add_subdirectory(python)
endif()
