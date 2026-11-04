enable_testing()

set(REF_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/reference_data" CACHE PATH "Path to reference data directory")

add_test(
    NAME RunScenario0
    COMMAND numsim ${CMAKE_CURRENT_SOURCE_DIR}/scenario0_lid_driven_cavity/input_parameter.txt
)
set_tests_properties(RunScenario0 PROPERTIES WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")

add_test(
    NAME CompareScenario0
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_vti.py 
            ${REF_DATA_DIR}/scenario0_lid_driven_cavity 
            ${CMAKE_BINARY_DIR}/out
)
set_tests_properties(CompareScenario0 PROPERTIES DEPENDS RunScenario0)

add_test(
    NAME RunScenario3
    COMMAND numsim ${CMAKE_CURRENT_SOURCE_DIR}/scenario3_vary_everything/input_parameter.txt
)
set_tests_properties(RunScenario3 PROPERTIES WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")

add_test(
    NAME CompareScenario3
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/compare_vti.py 
            ${REF_DATA_DIR}/scenario3_vary_everything 
            ${CMAKE_BINARY_DIR}/out
)
set_tests_properties(CompareScenario3 PROPERTIES DEPENDS RunScenario3)