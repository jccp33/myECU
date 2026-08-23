cmake_minimum_required(VERSION 3.16)

project(myECU LANGUAGES CXX)

option(BUILD_ECU_SIMULATOR "Build the Linux ECU simulator" ON)
option(
    ECU_CORE_EMBEDDED_PROFILE
    "Compile ECU Core as a freestanding subset without exceptions or RTTI"
    OFF
)

# Use Debug by default with single-configuration generators while still
# allowing callers to select Release explicitly.
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set_property(
    CACHE CMAKE_BUILD_TYPE
    PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel
)

set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

function(enable_project_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(
            ${target}
            PRIVATE
                -Wall
                -Wextra
                -Wpedantic
                -Wshadow
                -Wconversion
                -Wsign-conversion
                -Wold-style-cast
                -Wnull-dereference
                -Wdouble-promotion
        )
    endif()
endfunction()

function(enable_ecu_core_profile target)
    if(ECU_CORE_EMBEDDED_PROFILE)
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(
                ${target}
                PRIVATE
                    -ffreestanding
                    -fno-exceptions
                    -fno-rtti
                    -fno-threadsafe-statics
                    -fno-use-cxa-atexit
            )
        elseif(MSVC)
            target_compile_options(${target} PRIVATE /GR-)
        endif()
    endif()
endfunction()

add_library(
    ecu_core
    STATIC
    src/message.cpp
    src/config.cpp
    src/mssgmanager.cpp
    src/getaway.cpp
    src/control.cpp
    src/evaluation_rule.cpp
    src/fault_state_machine.cpp
    src/fault_condition_evaluator.cpp
    src/fault_manager.cpp
    src/ecu_state_machine.cpp
    src/signal_store.cpp
    src/fault_configuration.cpp
    src/diagnostic_status.cpp
)

target_compile_features(ecu_core PUBLIC cxx_std_11)
enable_project_warnings(ecu_core)
enable_ecu_core_profile(ecu_core)

target_include_directories(
    ecu_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(BUILD_ECU_SIMULATOR)
    add_executable(
        ecu_simulator
        main.cpp
        src/utils.cpp
        src/simulations.cpp
        src/sensor_simulation.cpp
        src/linux_platform.cpp
    )

    target_link_libraries(
        ecu_simulator
        PRIVATE
            ecu_core
    )
    enable_project_warnings(ecu_simulator)
endif()

# Compile every public Core header in an isolated translation unit. This makes
# accidental reliance on transitive or platform-specific includes a build
# failure instead of a latent portability problem.
set(
    ECU_CORE_PUBLIC_HEADERS
    data_types.hpp
    message.hpp
    config.hpp
    mssgmanager.hpp
    getaway.hpp
    control.hpp
    fault_types.hpp
    evaluation_rule.hpp
    fault_state_machine.hpp
    fault_condition_evaluator.hpp
    fault_manager.hpp
    ecu_state_machine.hpp
    signal_sample.hpp
    signal_store.hpp
    fault_configuration.hpp
    diagnostic_status.hpp
)

foreach(header IN LISTS ECU_CORE_PUBLIC_HEADERS)
    string(MAKE_C_IDENTIFIER "${header}" header_id)
    set(header_check_source "${CMAKE_CURRENT_BINARY_DIR}/${header_id}_check.cpp")
    file(GENERATE OUTPUT "${header_check_source}" CONTENT "#include \"${header}\"\n")

    add_library("ecu_${header_id}_check" OBJECT "${header_check_source}")
    target_compile_features("ecu_${header_id}_check" PRIVATE cxx_std_11)
    target_include_directories(
        "ecu_${header_id}_check"
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    enable_project_warnings("ecu_${header_id}_check")
    enable_ecu_core_profile("ecu_${header_id}_check")
endforeach()

if(BUILD_TESTING)
    add_executable(
        ecu_gateway_tests
        tests/gateway_tests.cpp
    )

    target_link_libraries(
        ecu_gateway_tests
        PRIVATE
            ecu_core
    )
    enable_project_warnings(ecu_gateway_tests)

    add_test(
        NAME gateway_tests
        COMMAND ecu_gateway_tests
    )

    add_executable(
        ecu_control_tests
        tests/control_tests.cpp
    )

    target_link_libraries(
        ecu_control_tests
        PRIVATE
            ecu_core
    )
    enable_project_warnings(ecu_control_tests)

    add_test(
        NAME control_tests
        COMMAND ecu_control_tests
    )

    add_executable(
        ecu_fault_types_tests
        tests/fault_types_tests.cpp
    )

    target_link_libraries(
        ecu_fault_types_tests
        PRIVATE
            ecu_core
    )
    enable_project_warnings(ecu_fault_types_tests)

    add_test(
        NAME fault_types_tests
        COMMAND ecu_fault_types_tests
    )

    add_executable(
        ecu_evaluation_rule_tests
        tests/evaluation_rule_tests.cpp
    )

    target_link_libraries(
        ecu_evaluation_rule_tests
        PRIVATE
            ecu_core
    )
    enable_project_warnings(ecu_evaluation_rule_tests)

    add_test(
        NAME evaluation_rule_tests
        COMMAND ecu_evaluation_rule_tests
    )

    add_executable(
        ecu_fault_state_machine_tests
        tests/fault_state_machine_tests.cpp
    )

    target_link_libraries(
        ecu_fault_state_machine_tests
        PRIVATE
            ecu_core
    )
    enable_project_warnings(ecu_fault_state_machine_tests)

    add_test(
        NAME fault_state_machine_tests
        COMMAND ecu_fault_state_machine_tests
    )

    add_executable(
        ecu_fault_condition_evaluator_tests
        tests/fault_condition_evaluator_tests.cpp
    )

    target_link_libraries(
        ecu_fault_condition_evaluator_tests
        PRIVATE
            ecu_core
    )
    enable_project_warnings(ecu_fault_condition_evaluator_tests)

    add_test(
        NAME fault_condition_evaluator_tests
        COMMAND ecu_fault_condition_evaluator_tests
    )

    add_executable(
        ecu_fault_manager_tests
        tests/fault_manager_tests.cpp
    )

    target_link_libraries(
        ecu_fault_manager_tests
        PRIVATE
            ecu_core
    )
    enable_project_warnings(ecu_fault_manager_tests)

    add_test(
        NAME fault_manager_tests
        COMMAND ecu_fault_manager_tests
    )

    add_executable(
        ecu_state_machine_tests
        tests/ecu_state_machine_test.cpp
    )

    target_link_libraries(
        ecu_state_machine_tests
        PRIVATE
            ecu_core
    )
    enable_project_warnings(ecu_state_machine_tests)

    add_test(
        NAME ecu_state_machine_tests
        COMMAND ecu_state_machine_tests
    )

    add_executable(
        ecu_signal_store_tests
        tests/signal_store_tests.cpp
    )

    target_link_libraries(
        ecu_signal_store_tests
        PRIVATE
            ecu_core
    )
    enable_project_warnings(ecu_signal_store_tests)

    add_test(
        NAME signal_store_tests
        COMMAND ecu_signal_store_tests
    )

    add_executable(
        ecu_fault_configuration_tests
        tests/fault_configuration_tests.cpp
    )

    target_link_libraries(
        ecu_fault_configuration_tests
        PRIVATE
            ecu_core
    )
    enable_project_warnings(ecu_fault_configuration_tests)

    add_test(
        NAME fault_configuration_tests
        COMMAND ecu_fault_configuration_tests
    )

    add_executable(
        ecu_diagnostic_status_tests
        tests/diagnostic_status_tests.cpp
    )

    target_link_libraries(
        ecu_diagnostic_status_tests
        PRIVATE
            ecu_core
    )
    enable_project_warnings(ecu_diagnostic_status_tests)

    add_test(
        NAME diagnostic_status_tests
        COMMAND ecu_diagnostic_status_tests
    )

    add_executable(
        ecu_integration_tests
        tests/ecu_integration_tests.cpp
    )
    target_link_libraries(ecu_integration_tests PRIVATE ecu_core)
    enable_project_warnings(ecu_integration_tests)
    add_test(NAME ecu_integration_tests COMMAND ecu_integration_tests)

    add_executable(
        ecu_sensor_simulation_tests
        tests/sensor_simulation_tests.cpp
        src/sensor_simulation.cpp
    )
    target_include_directories(ecu_sensor_simulation_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_compile_features(ecu_sensor_simulation_tests PRIVATE cxx_std_11)
    enable_project_warnings(ecu_sensor_simulation_tests)
    add_test(NAME sensor_simulation_tests COMMAND ecu_sensor_simulation_tests)
endif()
