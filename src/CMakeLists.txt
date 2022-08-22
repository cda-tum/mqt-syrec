add_library(${PROJECT_NAME}
            ${CMAKE_CURRENT_SOURCE_DIR}/algorithms/synthesis/syrec_synthesis.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/algorithms/simulation/simple_simulation.cpp

            ${${PROJECT_NAME}_SOURCE_DIR}/include/algorithms/simulation/simple_simulation.hpp
            ${${PROJECT_NAME}_SOURCE_DIR}/include/algorithms/synthesis/syrec_synthesis.hpp

            ${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/parser.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/program.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/core/syrec/variable.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/core/truthTable/truth_table.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/core/truthTable/dd_synthesis.cpp
			#${CMAKE_CURRENT_SOURCE_DIR}/core/truthTable/pla_parser.cpp

            ${${PROJECT_NAME}_SOURCE_DIR}/include/core/gate.hpp
            ${${PROJECT_NAME}_SOURCE_DIR}/include/core/circuit.hpp
            ${${PROJECT_NAME}_SOURCE_DIR}/include/core/syrec/expression.hpp
            ${${PROJECT_NAME}_SOURCE_DIR}/include/core/syrec/grammar.hpp
            ${${PROJECT_NAME}_SOURCE_DIR}/include/core/syrec/module.hpp
            ${${PROJECT_NAME}_SOURCE_DIR}/include/core/syrec/number.hpp
            ${${PROJECT_NAME}_SOURCE_DIR}/include/core/syrec/parser.hpp
            ${${PROJECT_NAME}_SOURCE_DIR}/include/core/syrec/program.hpp
            ${${PROJECT_NAME}_SOURCE_DIR}/include/core/syrec/statement.hpp
            ${${PROJECT_NAME}_SOURCE_DIR}/include/core/syrec/variable.hpp
            ${${PROJECT_NAME}_SOURCE_DIR}/include/core/utils/timer.hpp
            ${${PROJECT_NAME}_SOURCE_DIR}/include/core/truthTable/truth_table.hpp
			${${PROJECT_NAME}_SOURCE_DIR}/include/core/truthTable/dd_synthesis.hpp)
			#${${PROJECT_NAME}_SOURCE_DIR}/include/core/truthTable/pla_parser.hpp)

# set include directories
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>)

# set required C++ standard and disable compiler specific extensions
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES CMAKE_CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)

# add MQT::qfr library
add_subdirectory("${PROJECT_SOURCE_DIR}/extern/qfr" "extern/qfr")
target_link_libraries(${PROJECT_NAME} PUBLIC MQT::qfr)

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
