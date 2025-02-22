if(NOT TARGET MQT::SyReC)
  # collect headers and source files
  file(GLOB_RECURSE SYREC_HEADERS ${MQT_SYREC_INCLUDE_BUILD_DIR}/*.hpp)
  file(GLOB_RECURSE SYREC_SOURCES **.cpp)
  list(FILTER SYREC_SOURCES EXCLUDE REGEX ".*/python/.*$")
  
  # TODO: Split library into IR and parser target
  # add SyReC Package library
  add_library(${PROJECT_NAME} ${SYREC_HEADERS} ${SYREC_SOURCES})
  target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

  if(BUILD_SHARED_LIBS)
	target_link_libraries(${PROJECT_NAME} PRIVATE antlr4_shared)
	add_custom_command(TARGET ${PROJECT_NAME}
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND}
                           -E copy ${ANTLR4_RUNTIME_LIBRARIES} .
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  else()
	# required if linking to static library
	target_compile_definitions(${PROJECT_NAME} PRIVATE -DANTLR4CPP_STATIC)
	target_link_libraries(${PROJECT_NAME} PRIVATE antlr4_static)
   endif()

  # set include directories
  target_include_directories(${PROJECT_NAME}
        PUBLIC 
            $<BUILD_INTERFACE:${MQT_SYREC_INCLUDE_BUILD_DIR}>
            ${CMAKE_SOURCE_DIR}/include/core/syrec/parser/antlr
		    ${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/antlr
            ${ANTLR4_INCLUDE_DIRS})

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
endif()

if(BUILD_MQT_SYREC_BINDINGS)
  add_subdirectory(python)
endif()
