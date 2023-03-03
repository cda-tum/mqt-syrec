# add MQT::qfr library
add_subdirectory("${PROJECT_SOURCE_DIR}/extern/qfr" "extern/qfr" EXCLUDE_FROM_ALL)

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
  ${PROJECT_SOURCE_DIR}/include/core/syrec/expression.hpp
  ${PROJECT_SOURCE_DIR}/include/core/syrec/grammar.hpp
  ${PROJECT_SOURCE_DIR}/include/core/syrec/module.hpp
  ${PROJECT_SOURCE_DIR}/include/core/syrec/number.hpp
  ${PROJECT_SOURCE_DIR}/include/core/syrec/parser.hpp
  ${PROJECT_SOURCE_DIR}/include/core/syrec/program.hpp
  ${PROJECT_SOURCE_DIR}/include/core/syrec/statement.hpp
  ${PROJECT_SOURCE_DIR}/include/core/syrec/variable.hpp
  ${PROJECT_SOURCE_DIR}/include/core/utils/timer.hpp
  ${PROJECT_SOURCE_DIR}/include/core/truthTable/truth_table.hpp
  ${PROJECT_SOURCE_DIR}/include/core/io/pla_parser.hpp)

# set include directories
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include
                                                  ${PROJECT_BINARY_DIR}/include)

# link to the MQT::qfr DD library
target_link_libraries(${PROJECT_NAME} PUBLIC MQT::qfr_dd)

# add header-only part of the Boost library
set(BOOST_USE_MULTITHREADED ON)
set(BOOST_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.71 REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC Boost::boost)

# add MQT alias
add_library(MQT::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
