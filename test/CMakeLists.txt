find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(NUMSIM_REFERENCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}" CACHE PATH
    "Directory containing the scenario reference outputs")

function(add_reference_test scenario test_id)
    set(input_file "${CMAKE_CURRENT_SOURCE_DIR}/${scenario}/input_parameters.txt")
    set(reference_dir "${NUMSIM_REFERENCE_ROOT}/${scenario}/reference_output")

    add_test(
        NAME Clean${test_id}
        COMMAND "${CMAKE_COMMAND}" -E rm -rf "${CMAKE_BINARY_DIR}/out"
    )

    add_test(
        NAME Run${test_id}
        COMMAND $<TARGET_FILE:numsim> "${input_file}"
    )
    set_tests_properties(Run${test_id} PROPERTIES
        DEPENDS Clean${test_id}
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    )

    add_test(
        NAME Compare${test_id}
        COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/compare_vti_outputs.py"
                "${reference_dir}"
                "${CMAKE_BINARY_DIR}/out"
                --value-tolerance 1e-2
                --mean-dt-relative-tolerance 0.05
                --endpoint-tolerance 1e-8
    )
    set_tests_properties(Compare${test_id} PROPERTIES DEPENDS Run${test_id})
endfunction()

add_reference_test("0_driven_cavity" "Scenario0")
add_reference_test("3_varying_params" "Scenario3")
set_tests_properties(CleanScenario3 PROPERTIES DEPENDS CompareScenario0)
