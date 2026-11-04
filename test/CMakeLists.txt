enable_testing()

set(REF_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/reference_data" CACHE PATH "Path to reference data directory")

# Scenario 0: Lid Driven Cavity
add_test(
    NAME RunScenario0
    COMMAND numsim ${CMAKE_CURRENT_SOURCE_DIR}/scenario0_lid_driven_cavity/input_parameter.txt
)

add_test(
    NAME CompareScenario0
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_vti.py 
            ${REF_DATA_DIR}/scenario0_lid_driven_cavity 
            ${CMAKE_BINARY_DIR}/output_scenario0
)

set_tests_properties(CompareScenario0 PROPERTIES DEPENDS RunScenario0)

# Scenario 3: Vary Everything
add_test(
    NAME RunScenario3
    COMMAND numsim ${CMAKE_CURRENT_SOURCE_DIR}/scenario3_vary_everything/input_parameter.txt
)

add_test(
    NAME CompareScenario3
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_vti.py 
            ${REF_DATA_DIR}/scenario3_vary_everything 
            ${CMAKE_BINARY_DIR}/output_scenario3
)

set_tests_properties(CompareScenario3 PROPERTIES DEPENDS RunScenario3)

