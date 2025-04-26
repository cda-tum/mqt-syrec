if(NOT TARGET MQT::SyReC)
  # collect headers and source files
  file(GLOB_RECURSE SYREC_HEADERS ${MQT_SYREC_INCLUDE_BUILD_DIR}/*.hpp)
  file(GLOB_RECURSE SYREC_SOURCES **.cpp)
  list(FILTER SYREC_SOURCES EXCLUDE REGEX ".*/python/.*$")

  # add SyReC Package library
  add_library(${PROJECT_NAME} ${SYREC_HEADERS} ${SYREC_SOURCES})

  # set include directories
  target_include_directories(${PROJECT_NAME}
                             PUBLIC $<BUILD_INTERFACE:${MQT_SYREC_INCLUDE_BUILD_DIR}>)

  # link to the MQT::Core libraries
  target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC MQT::CoreDD
    PRIVATE MQT::ProjectWarnings MQT::ProjectOptions)

  # add header-only part of the Boost library
  set(BOOST_USE_MULTITHREADED ON)
  set(BOOST_USE_STATIC_RUNTIME OFF)
  find_package(Boost 1.71 REQUIRED)
  target_link_libraries(${PROJECT_NAME} PUBLIC Boost::boost)

  # add MQT alias
  add_library(MQT::SyReC ALIAS ${PROJECT_NAME})
  target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
endif()

if(BUILD_MQT_SYREC_BINDINGS)
  add_subdirectory(python)
endif()
