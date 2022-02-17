# TODO: add any header and source files here
add_library(${PROJECT_NAME}
            

  ${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/syrec_synthesis_p.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/syrec_synthesis.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/syrec_synthesis_garbagefree.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/algorithms/simulation/simple_simulation.cpp

  ${${PROJECT_NAME}_SOURCE_DIR}/include/algorithms/simulation/simulation.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/algorithms/simulation/simple_simulation.hpp

  ${${PROJECT_NAME}_SOURCE_DIR}/include/algorithms/synthesis/synthesis.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/algorithms/synthesis/syrec_synthesis.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/algorithms/synthesis/syrec_synthesis_garbagefree.hpp



  ${CMAKE_CURRENT_SOURCE_DIR}/core/circuit.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/gate.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/pattern.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/properties.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/target_tags.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/truth_table.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/version.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/core/io/create_image.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/io/print_circuit.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/io/print_statistics.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/io/read_pattern.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/io/read_realization.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/io/read_specification.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/io/read_pattern.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/io/read_pla.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/io/revlib_parser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/io/revlib_processor.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/io/write_blif.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/io/write_realization.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/io/write_specification.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/io/write_verilog.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/core/functions/active_controls.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/functions/add_circuit.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/functions/add_gates.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/functions/add_line_to_circuit.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/functions/circuit_hierarchy.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/functions/circuit_to_truth_table.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/functions/clear_circuit.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/functions/copy_circuit.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/functions/copy_metadata.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/functions/create_simulation_pattern.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/functions/expand_circuit.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/functions/extend_truth_table.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/functions/find_lines.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/functions/flatten_circuit.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/functions/fully_specified.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/functions/reverse_circuit.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/functions/transposition_to_circuit.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/core/meta/bus_collection.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/expression.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/grammar.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/modified_variables.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/module.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/number.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/program.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/reverse_statements.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/statement.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/variable.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/write_systemc.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/core/utils/costs.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/utils/program_options.cpp


  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/gate.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/circuit.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/pattern.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/properties.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/target_tags.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/truth_table.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/version.hpp



  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/io/create_image.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/io/print_circuit.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/io/print_statistics.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/io/read_pattern.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/io/read_realization.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/io/read_specification.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/io/read_pattern.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/io/read_pla.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/io/revlib_parser.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/io/revlib_processor.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/io/write_blif.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/io/write_realization.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/io/write_specification.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/io/write_verilog.hpp


  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/functions/active_controls.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/functions/add_circuit.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/functions/add_gates.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/functions/add_line_to_circuit.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/functions/circuit_hierarchy.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/functions/circuit_to_truth_table.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/functions/clear_circuit.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/functions/control_lines.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/functions/copy_circuit.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/functions/copy_metadata.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/functions/expand_circuit.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/functions/extend_truth_table.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/functions/find_lines.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/functions/flatten_circuit.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/functions/fully_specified.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/functions/reverse_circuit.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/functions/target_lines.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/functions/transposition_to_circuit.hpp

  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/meta/bus_collection.hpp

  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/syrec/expression.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/syrec/grammar.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/syrec/modified_variables.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/syrec/module.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/syrec/number.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/syrec/parser.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/syrec/program.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/syrec/reverse_statements.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/syrec/statement.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/syrec/variable.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/syrec/write_systemc.hpp



  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/utils/costs.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/utils/program_options.hpp
  ${${PROJECT_NAME}_SOURCE_DIR}/include/core/utils/timer.hpp
            )

# set include directories
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>)

# set required C++ standard and disable compiler specific extensions
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES CMAKE_CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)

# add JKQ::qfr library
add_subdirectory("${PROJECT_SOURCE_DIR}/extern/qfr" "extern/qfr")
target_link_libraries(${PROJECT_NAME} PUBLIC JKQ::qfr)

# add header-only part of the Boost library
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.71 REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC Boost::boost)

# enable interprocedural optimization if it is supported

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported)
if (ipo_supported)
	set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif ()

# set compiler flags depending on compiler
if (MSVC)
	target_compile_options(${PROJECT_NAME} PUBLIC /utf-8)
else ()
	target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wextra $<$<CONFIG:DEBUG>:-Og>)
	if (BINDINGS)
		# adjust visibility settings for building Python bindings
		target_compile_options(${PROJECT_NAME} PUBLIC -fvisibility=hidden)
	endif ()
	if (NOT DEPLOY)
		# only include machine-specific optimizations when building for the host machine
		target_compile_options(${PROJECT_NAME} PUBLIC -mtune=native)
		include(CheckCXXCompilerFlag)
		check_cxx_compiler_flag(-march=native HAS_MARCH_NATIVE)
		if (HAS_MARCH_NATIVE)
			target_compile_options(${PROJECT_NAME} PUBLIC -march=native)
		endif ()
	endif ()
endif ()

if (GENERATE_POSITION_INDEPENDENT_CODE OR BINDINGS)
	include(CheckPIESupported)
	check_pie_supported()
	set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
endif ()

# add coverage compiler and linker flag to the library and all targets that link against it, if COVERAGE is set
if (COVERAGE)
	target_compile_options(${PROJECT_NAME} PUBLIC --coverage)
	target_link_libraries(${PROJECT_NAME} PUBLIC --coverage)
endif ()

# add MQT alias
add_library(MQT::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
