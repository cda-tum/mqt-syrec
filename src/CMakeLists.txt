# add MQT::Core target
set(BUILD_MQT_CORE_TESTS
    OFF
    CACHE BOOL "Build MQT Core tests")

add_library(${PROJECT_NAME}-ir INTERFACE)
target_include_directories(${PROJECT_NAME}-ir
	INTERFACE
		${PROJECT_SOURCE_DIR}/include/core/syrec/expression.hpp
		${PROJECT_SOURCE_DIR}/include/core/syrec/grammar.hpp
		${PROJECT_SOURCE_DIR}/include/core/syrec/module.hpp
		${PROJECT_SOURCE_DIR}/include/core/syrec/number.hpp
		${PROJECT_SOURCE_DIR}/include/core/syrec/parser.hpp
		${PROJECT_SOURCE_DIR}/include/core/syrec/statement.hpp
		${PROJECT_SOURCE_DIR}/include/core/syrec/variable.hpp
)

add_library(
  ${PROJECT_NAME}
  ${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/syrec_synthesis.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/syrec_cost_aware_synthesis.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/syrec_line_aware_synthesis.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/algorithms/simulation/simple_simulation.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/algorithms/simulation/circuit_to_truthtable.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/dd_synthesis.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/encoding.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/algorithms/optimization/esop_minimization.cpp
  ${PROJECT_SOURCE_DIR}/include/algorithms/optimization/esop_minimization.hpp
  ${PROJECT_SOURCE_DIR}/include/algorithms/simulation/simple_simulation.hpp
  ${PROJECT_SOURCE_DIR}/include/algorithms/simulation/circuit_to_truthtable.hpp
  ${PROJECT_SOURCE_DIR}/include/algorithms/synthesis/syrec_synthesis.hpp
  ${PROJECT_SOURCE_DIR}/include/algorithms/synthesis/syrec_cost_aware_synthesis.hpp
  ${PROJECT_SOURCE_DIR}/include/algorithms/synthesis/syrec_line_aware_synthesis.hpp
  ${PROJECT_SOURCE_DIR}/include/algorithms/synthesis/dd_synthesis.hpp
  ${PROJECT_SOURCE_DIR}/include/algorithms/synthesis/encoding.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/program.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/variable.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/truthTable/truth_table.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/io/pla_parser.cpp
  ${PROJECT_SOURCE_DIR}/include/core/gate.hpp
  ${PROJECT_SOURCE_DIR}/include/core/circuit.hpp
  ${PROJECT_SOURCE_DIR}/include/core/syrec/program.hpp
  ${PROJECT_SOURCE_DIR}/include/core/utils/timer.hpp
  ${PROJECT_SOURCE_DIR}/include/core/truthTable/truth_table.hpp
  ${PROJECT_SOURCE_DIR}/include/core/io/pla_parser.hpp)

add_library(${PROJECT_NAME}-parser "")
target_sources(${PROJECT_NAME}-parser
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/components/custom_base_visitor.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/components/custom_error_listener.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/components/custom_expression_visitor.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/components/custom_statement_visitor.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/components/custom_module_visitor.cpp

		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/antlr/TSyrecLexer.h
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/antlr/TSyrecParser.h
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/antlr/TSyrecParserBaseVisitor.h
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/antlr/TSyrecParserVisitor.h
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/antlr/TSyrecLexer.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/antlr/TSyrecParser.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/antlr/TSyrecParserBaseVisitor.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/antlr/TSyrecParserVisitor.cpp
		
	PUBLIC
		${CMAKE_SOURCE_DIR}/include/core/syrec/parser/components/custom_base_visitor.hpp
		${CMAKE_SOURCE_DIR}/include/core/syrec/parser/components/custom_error_listener.hpp
		${CMAKE_SOURCE_DIR}/include/core/syrec/parser/components/custom_expression_visitor.hpp
		${CMAKE_SOURCE_DIR}/include/core/syrec/parser/components/custom_statement_visitor.hpp
		${CMAKE_SOURCE_DIR}/include/core/syrec/parser/components/custom_module_visitor.hpp
)
target_include_directories(${PROJECT_NAME}-parser
	PUBLIC
		${CMAKE_SOURCE_DIR}/include
		${CMAKE_SOURCE_DIR}/include/core/syrec/parser/antlr
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/antlr
		${ANTLR4_INCLUDE_DIRS}
)
target_link_libraries(${PROJECT_NAME}-parser PRIVATE ${PROJECT_NAME}-ir)
target_link_libraries(${PROJECT_NAME}-parser PRIVATE fmt::fmt)

if(ANTLR4_WITH_STATIC_CRT)
	# required if linking to static library
	target_compile_definitions(${PROJECT_NAME}-parser PRIVATE -DANTLR4CPP_STATIC)
	target_link_libraries(${PROJECT_NAME}-parser PRIVATE antlr4_static)
else()
	target_link_libraries(${PROJECT_NAME}-parser PRIVATE antlr4_shared)
	add_custom_command(TARGET ${PROJECT_NAME}-parser
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND}
                           -E copy ${ANTLR4_RUNTIME_LIBRARIES} .
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()

# set include directories
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include
                                                  ${PROJECT_BINARY_DIR}/include)

# link to the MQT::qfr DD library
target_link_libraries(${PROJECT_NAME} PUBLIC MQT::CoreDD)
target_link_libraries(${PROJECT_NAME} PUBLIC ${PROJECT_NAME}-ir)
target_link_libraries(${PROJECT_NAME} PUBLIC ${PROJECT_NAME}-parser)

# add header-only part of the Boost library
set(BOOST_USE_MULTITHREADED ON)
set(BOOST_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.71 REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC Boost::boost)

# add MQT alias
add_library(MQT::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if(BUILD_MQT_SYREC_BINDINGS)
  add_subdirectory(python)
endif()
