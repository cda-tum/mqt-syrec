# add MQT::Core target
set(BUILD_MQT_CORE_TESTS
    OFF
    CACHE BOOL "Build MQT Core tests")

set(MQT_SYREC_TARGET ${PROJECT_NAME} CACHE INTERNAL "")
set(MQT_SYREC_IR_TARGET "${PROJECT_NAME}-ir")
set(MQT_SYREC_PARSER_TARGET "${PROJECT_NAME}-parser")

add_library(${MQT_SYREC_IR_TARGET} INTERFACE)
target_include_directories(${MQT_SYREC_IR_TARGET}
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
  ${MQT_SYREC_TARGET}
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
  ${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/program.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/variable.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/truthTable/truth_table.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/utils/base_syrec_ir_entity_stringifier.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/io/pla_parser.cpp
  ${PROJECT_SOURCE_DIR}/include/core/gate.hpp
  ${PROJECT_SOURCE_DIR}/include/core/circuit.hpp
  ${PROJECT_SOURCE_DIR}/include/core/syrec/program.hpp
  ${PROJECT_SOURCE_DIR}/include/core/syrec/parser/utils/symbolTable/base_symbol_table.hpp
  ${PROJECT_SOURCE_DIR}/include/core/syrec/parser/utils/symbolTable/temporary_variable_scope.hpp
  ${PROJECT_SOURCE_DIR}/include/core/utils/base_syrec_ir_entity_stringifier.hpp
  ${PROJECT_SOURCE_DIR}/include/core/utils/timer.hpp
  ${PROJECT_SOURCE_DIR}/include/core/truthTable/truth_table.hpp
  ${PROJECT_SOURCE_DIR}/include/core/io/pla_parser.hpp)

add_library(${MQT_SYREC_PARSER_TARGET})
target_sources(${MQT_SYREC_PARSER_TARGET}
	PRIVATE
		
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/components/custom_error_listener.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/components/custom_expression_visitor.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/components/custom_module_visitor.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/components/custom_number_and_signal_visitor.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/components/custom_statement_visitor.cpp
		
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/antlr/TSyrecLexer.h
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/antlr/TSyrecParser.h
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/antlr/TSyrecParserBaseVisitor.h
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/antlr/TSyrecParserVisitor.h
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/antlr/TSyrecLexer.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/antlr/TSyrecParser.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/antlr/TSyrecParserBaseVisitor.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/antlr/TSyrecParserVisitor.cpp

		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/utils/symbolTable/base_symbol_table.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/utils/symbolTable/temporary_variable_scope.cpp
		${CMAKE_SOURCE_DIR}/include/core/syrec/parser/utils/parser_messages_container.hpp
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/utils/parser_messages_container.cpp
		${CMAKE_SOURCE_DIR}/include/core/syrec/parser/utils/variable_assignability_check.hpp
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/utils/variable_access_index_check.cpp
		${CMAKE_SOURCE_DIR}/include/core/syrec/parser/utils/variable_access_index_check.hpp
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/utils/variable_overlap_check.cpp
		${CMAKE_SOURCE_DIR}/include/core/syrec/parser/utils/variable_overlap_check.hpp
	PUBLIC
		${CMAKE_SOURCE_DIR}/include/core/syrec/parser/components/custom_base_visitor.hpp
		${CMAKE_SOURCE_DIR}/include/core/syrec/parser/components/custom_error_listener.hpp
		${CMAKE_SOURCE_DIR}/include/core/syrec/parser/components/custom_expression_visitor.hpp
		${CMAKE_SOURCE_DIR}/include/core/syrec/parser/components/custom_module_visitor.hpp
		${CMAKE_SOURCE_DIR}/include/core/syrec/parser/components/custom_number_and_signal_visitor.hpp
		${CMAKE_SOURCE_DIR}/include/core/syrec/parser/components/custom_statement_visitor.hpp
		${CMAKE_SOURCE_DIR}/include/core/syrec/parser/components/generic_visitor_result.hpp
		
)
target_include_directories(${MQT_SYREC_PARSER_TARGET}
	PUBLIC
		${CMAKE_SOURCE_DIR}/include
		${CMAKE_SOURCE_DIR}/include/core/syrec/parser/antlr
		${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser/antlr
		${ANTLR4_INCLUDE_DIRS}
)
target_link_libraries(${MQT_SYREC_PARSER_TARGET} PRIVATE ${MQT_SYREC_IR_TARGET})
target_link_libraries(${MQT_SYREC_PARSER_TARGET} PUBLIC fmt::fmt)

if(BUILD_SHARED_LIBS)
	target_link_libraries(${MQT_SYREC_PARSER_TARGET} PRIVATE antlr4_shared)
	add_custom_command(TARGET ${MQT_SYREC_PARSER_TARGET}
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND}
                           -E copy ${ANTLR4_RUNTIME_LIBRARIES} .
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
else()
	# required if linking to static library
	target_compile_definitions(${MQT_SYREC_PARSER_TARGET} PUBLIC -DANTLR4CPP_STATIC)
	target_link_libraries(${MQT_SYREC_PARSER_TARGET} PUBLIC antlr4_static)
endif()

# set include directories
target_include_directories(${MQT_SYREC_TARGET} PUBLIC ${PROJECT_SOURCE_DIR}/include
                                                  ${PROJECT_BINARY_DIR}/include)

# link to the MQT::qfr DD library
target_link_libraries(${MQT_SYREC_TARGET} PUBLIC MQT::CoreDD)
target_link_libraries(${MQT_SYREC_TARGET} PUBLIC ${MQT_SYREC_IR_TARGET})
target_link_libraries(${MQT_SYREC_TARGET} PUBLIC ${MQT_SYREC_PARSER_TARGET})

# add header-only part of the Boost library
set(BOOST_USE_MULTITHREADED ON)
set(BOOST_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.71 REQUIRED)
target_link_libraries(${MQT_SYREC_TARGET} PUBLIC Boost::boost)

# add MQT alias
add_library(MQT::${MQT_SYREC_TARGET} ALIAS ${MQT_SYREC_TARGET})

if(BUILD_MQT_SYREC_BINDINGS)
  add_subdirectory(python)
endif()
