cmake_minimum_required(VERSION 3.16)

project(myECU LANGUAGES CXX)

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

add_library(
    ecu_core
    STATIC
    src/message.cpp
    src/config.cpp
    src/mssgmanager.cpp
    src/getaway.cpp
    src/control.cpp
)

target_compile_features(ecu_core PUBLIC cxx_std_11)
enable_project_warnings(ecu_core)

target_include_directories(
    ecu_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(
    ecu_simulator
    main.cpp
    src/utils.cpp
    src/simulations.cpp
    src/linux_platform.cpp
)

target_link_libraries(
    ecu_simulator
    PRIVATE
        ecu_core
)
enable_project_warnings(ecu_simulator)

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
endif()
