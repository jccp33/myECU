cmake_minimum_required(VERSION 3.16)

project(myECU LANGUAGES CXX)


# ============================================================
# Opciones del proyecto
# ============================================================

option(
    BUILD_ECU_SIMULATOR
    "Build the Linux ECU simulator"
    ON
)

option(
    ECU_CORE_EMBEDDED_PROFILE
    "Compile ECU Core as a freestanding subset without exceptions or RTTI"
    OFF
)


# ============================================================
# Configuración general
# ============================================================

# Use Debug by default with single-configuration generators while still
# allowing callers to select Release explicitly.
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(
        CMAKE_BUILD_TYPE
        Debug
        CACHE STRING "Build type"
        FORCE
    )
endif()

set_property(
    CACHE CMAKE_BUILD_TYPE
    PROPERTY STRINGS
        Debug
        Release
        RelWithDebInfo
        MinSizeRel
)

set(CMAKE_CXX_EXTENSIONS OFF)


# ============================================================
# Warnings
# ============================================================

function(enable_project_warnings target)

    if(MSVC)

        target_compile_options(
            ${target}
            PRIVATE
                /W4
                /permissive-
        )

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


# ============================================================
# Perfil embedded/freestanding del ECU Core
# ============================================================

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

            target_compile_options(
                ${target}
                PRIVATE
                    /GR-
            )

        endif()

    endif()

endfunction()


# ============================================================
# ECU Core
# ============================================================

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
    src/fault_manager.cpp
    src/ecu_state_machine.cpp

    src/signal_store.cpp

    src/fault_configuration.cpp
    src/diagnostic_status.cpp
)


target_compile_features(
    ecu_core
    PUBLIC
        cxx_std_11
)


target_include_directories(
    ecu_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)


enable_project_warnings(ecu_core)
enable_ecu_core_profile(ecu_core)


# ============================================================
# Simulador Linux
# ============================================================

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


    target_compile_features(
        ecu_simulator
        PRIVATE
            cxx_std_11
    )


    enable_project_warnings(ecu_simulator)

endif()


# ============================================================
# Headers públicos del ECU Core
# ============================================================
#
# Cada header se compila de forma aislada.
#
# Esto NO constituye una suite de tests funcionales.
# Su objetivo es detectar:
#
#   - includes faltantes;
#   - dependencia accidental de includes transitivos;
#   - dependencia accidental de plataforma;
#   - problemas de portabilidad del Core.
#
# signal_sample.hpp y signal_store.hpp se mantienen mientras
# se evalúa la alineación de STM32.
# ============================================================

set(
    ECU_CORE_PUBLIC_HEADERS
    data_types.hpp
    message.hpp
    config.hpp
    mssgmanager.hpp
    getaway.hpp
    control.hpp
    evaluation_rule.hpp
    fault_state_machine.hpp
    fault_manager.hpp
    ecu_state_machine.hpp
    signal_sample.hpp
    signal_store.hpp
    fault_configuration.hpp
    diagnostic_status.hpp
)

foreach(header IN LISTS ECU_CORE_PUBLIC_HEADERS)
    string(
        MAKE_C_IDENTIFIER
        "${header}"
        header_id
    )
    set(
        header_check_source
        "${CMAKE_CURRENT_BINARY_DIR}/${header_id}_check.cpp"
    )
    file(
        GENERATE
        OUTPUT "${header_check_source}"
        CONTENT "#include \"${header}\"\n"
    )
    add_library(
        "ecu_${header_id}_check"
        OBJECT
        "${header_check_source}"
    )
    target_compile_features(
        "ecu_${header_id}_check"
        PRIVATE
            cxx_std_11
    )
    target_include_directories(
        "ecu_${header_id}_check"
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    enable_project_warnings(
        "ecu_${header_id}_check"
    )
    enable_ecu_core_profile(
        "ecu_${header_id}_check"
    )
endforeach()