cmake_minimum_required(VERSION 3.20)
project(shadowrun_ghidra_conversion LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SHADOWRUN_GHIDRA_QT_FRONTEND
       "Build the separate Qt frontend for the Ghidra conversion project" OFF)

enable_testing()

# This is an offline source-understanding library, not a game-runtime
# dependency. The leaf contracts remain traceable, but they are built through
# the reviewed behavioral modules in conversion_modules.cmake rather than as
# one undifferentiated export-shaped target.
include(conversion_modules.cmake)

# Pinned, upstream Lua 5.4 source is built as a static C library so both the
# Qt conversion runtime and script-content QA work without a machine-installed
# Lua DLL or a configure-time network dependency.
add_library(shadowrun_lua_5_4_9 STATIC
    third_party/lua-5.4.9/src/lapi.c
    third_party/lua-5.4.9/src/lcode.c
    third_party/lua-5.4.9/src/lctype.c
    third_party/lua-5.4.9/src/ldebug.c
    third_party/lua-5.4.9/src/ldo.c
    third_party/lua-5.4.9/src/ldump.c
    third_party/lua-5.4.9/src/lfunc.c
    third_party/lua-5.4.9/src/lgc.c
    third_party/lua-5.4.9/src/llex.c
    third_party/lua-5.4.9/src/lmem.c
    third_party/lua-5.4.9/src/lobject.c
    third_party/lua-5.4.9/src/lopcodes.c
    third_party/lua-5.4.9/src/lparser.c
    third_party/lua-5.4.9/src/lstate.c
    third_party/lua-5.4.9/src/lstring.c
    third_party/lua-5.4.9/src/ltable.c
    third_party/lua-5.4.9/src/ltm.c
    third_party/lua-5.4.9/src/lundump.c
    third_party/lua-5.4.9/src/lvm.c
    third_party/lua-5.4.9/src/lzio.c
    third_party/lua-5.4.9/src/lauxlib.c
    third_party/lua-5.4.9/src/lbaselib.c
    third_party/lua-5.4.9/src/lmathlib.c
    third_party/lua-5.4.9/src/lstrlib.c
    third_party/lua-5.4.9/src/ltablib.c
    third_party/lua-5.4.9/src/lutf8lib.c
)
target_include_directories(shadowrun_lua_5_4_9 PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/lua-5.4.9/src")
set_target_properties(shadowrun_lua_5_4_9 PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF)
if (UNIX)
    target_link_libraries(shadowrun_lua_5_4_9 PUBLIC m)
endif()

add_library(shadowrun_gameplay_script_runtime STATIC
    src/scripting/game_script_runtime.cpp
    src/scripting/cyberdeck_menu_bindings.cpp
    src/scripting/world_content_tsv_loader.cpp)
target_include_directories(shadowrun_gameplay_script_runtime PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(shadowrun_gameplay_script_runtime
    PUBLIC shadowrun_ghidra_conversion_modules shadowrun_lua_5_4_9)

# This is the Ghidra-conversion runtime composition layer. Its implementation
# stays in src/integration/, but the runner and any future conversion-runtime
# clients link this target directly. It remains outside the native-port CMake
# graph and does not own native game runtime state.
add_library(shadowrun_ghidra_conversion_runtime STATIC
    src/integration/conversion_workbench.cpp
    src/integration/field_session_bootstrap_conversion.cpp
    src/integration/field_session_source_assets_conversion.cpp
    src/integration/field_runtime_coordinator_conversion.cpp
    src/integration/opening_intro_conversion.cpp
    src/integration/recruited_runner_conversation_conversion.cpp
    src/integration/startup_integration_conversion.cpp
)
target_include_directories(shadowrun_ghidra_conversion_runtime PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(shadowrun_ghidra_conversion_runtime
    PUBLIC shadowrun_ghidra_conversion_modules)

# Keep the previous target names as compatibility aliases. The source files
# remain physically separate from the domain conversion sources.
add_library(shadowrun_ghidra_conversion_integration ALIAS
    shadowrun_ghidra_conversion_runtime)
add_library(shadowrun_ghidra_conversion_workbench ALIAS
    shadowrun_ghidra_conversion_runtime)

add_executable(shadowrun_ghidra_conversion_runner
    tools/conversion_runner_main.cpp
)
target_link_libraries(shadowrun_ghidra_conversion_runner
    PRIVATE shadowrun_ghidra_conversion_runtime)
add_test(NAME shadowrun_ghidra_conversion_runner
         COMMAND shadowrun_ghidra_conversion_runner
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../assets")

# Offline extraction helper for the source records consumed by the converted
# startup/session code. It is intentionally not linked into the Qt frontend or
# any native runtime target.
add_executable(shadowrun_ghidra_startup_record_extract
    tools/extract_startup_record_assets.cpp
)

# The complete asset importer is an explicit offline step. Its decoder support
# is compiled from the archived extraction implementation, and none of these
# targets are linked into the Qt game executable.
find_package(ZLIB REQUIRED)
set(SHADOWRUN_ARCHIVED_ASSET_SOURCE_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/../../archive/legacy_native_port/decompilation")
add_library(shadowrun_ghidra_asset_import_support STATIC
    "${SHADOWRUN_ARCHIVED_ASSET_SOURCE_ROOT}/src/title_intro/boot.cpp"
    "${SHADOWRUN_ARCHIVED_ASSET_SOURCE_ROOT}/src/core/compression.cpp"
    "${SHADOWRUN_ARCHIVED_ASSET_SOURCE_ROOT}/src/entities/entity_projection.cpp"
    "${SHADOWRUN_ARCHIVED_ASSET_SOURCE_ROOT}/src/entities/entity_state.cpp"
    "${SHADOWRUN_ARCHIVED_ASSET_SOURCE_ROOT}/src/rendering/graphics_runtime.cpp"
    "${SHADOWRUN_ARCHIVED_ASSET_SOURCE_ROOT}/src/title_intro/intro_scene.cpp"
    "${SHADOWRUN_ARCHIVED_ASSET_SOURCE_ROOT}/src/core/memory_bus.cpp"
    "${SHADOWRUN_ARCHIVED_ASSET_SOURCE_ROOT}/src/core/native_input.cpp"
    "${SHADOWRUN_ARCHIVED_ASSET_SOURCE_ROOT}/src/field/native_port.cpp"
    "${SHADOWRUN_ARCHIVED_ASSET_SOURCE_ROOT}/src/entities/object_mask.cpp"
    "${SHADOWRUN_ARCHIVED_ASSET_SOURCE_ROOT}/src/entities/object_list.cpp"
    "${SHADOWRUN_ARCHIVED_ASSET_SOURCE_ROOT}/src/entities/object_update.cpp"
    "${SHADOWRUN_ARCHIVED_ASSET_SOURCE_ROOT}/src/rendering/palette.cpp"
    "${SHADOWRUN_ARCHIVED_ASSET_SOURCE_ROOT}/src/session/resource_setup.cpp"
    "${SHADOWRUN_ARCHIVED_ASSET_SOURCE_ROOT}/src/core/runtime.cpp"
    "${SHADOWRUN_ARCHIVED_ASSET_SOURCE_ROOT}/src/session/session.cpp"
    "${SHADOWRUN_ARCHIVED_ASSET_SOURCE_ROOT}/src/field/terrain.cpp"
    "${SHADOWRUN_ARCHIVED_ASSET_SOURCE_ROOT}/src/audio/z80_transport.cpp"
)
target_include_directories(shadowrun_ghidra_asset_import_support PUBLIC
    "${SHADOWRUN_ARCHIVED_ASSET_SOURCE_ROOT}/include")
target_link_libraries(shadowrun_ghidra_asset_import_support PRIVATE ZLIB::ZLIB)

add_library(shadowrun_ghidra_asset_import STATIC
    "${SHADOWRUN_ARCHIVED_ASSET_SOURCE_ROOT}/src/rendering/graphics_import.cpp")
target_include_directories(shadowrun_ghidra_asset_import PUBLIC
    "${SHADOWRUN_ARCHIVED_ASSET_SOURCE_ROOT}/include")
target_link_libraries(shadowrun_ghidra_asset_import
    PUBLIC shadowrun_ghidra_asset_import_support ZLIB::ZLIB)

add_executable(shadowrun_graphics_extract
    tools/rom_research/graphics_extract.cpp
    tools/rom_research/field_mode_profile_import.cpp)
target_include_directories(shadowrun_graphics_extract PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/qt_runtime/src/srx_refactor")
target_link_libraries(shadowrun_graphics_extract PRIVATE
    shadowrun_ghidra_asset_import shadowrun_ghidra_asset_import_support
    ZLIB::ZLIB)

add_executable(shadowrun_genesis_1728_sprite_test
    tools/rom_research/tests/field_mode_profile_import_test.cpp
    tools/rom_research/field_mode_profile_import.cpp)
target_link_libraries(shadowrun_genesis_1728_sprite_test PRIVATE
    shadowrun_ghidra_asset_import)
add_test(NAME shadowrun_genesis_1728_sprite
         COMMAND shadowrun_genesis_1728_sprite_test)

# Qt presets exist to link the host binary. The workbench presets own the
# per-contract test executables; building those 280 targets here makes a
# "Rebuild All" look finished while shadowrun_ghidra_conversion_qt.exe stays
# stale, and CONFIGURE_DEPENDS re-runs CMake on every ninja invocation.
if (NOT SHADOWRUN_GHIDRA_QT_FRONTEND)
    file(GLOB_RECURSE SHADOWRUN_CONVERSION_TEST_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*_test.cpp"
    )

    foreach(test_source IN LISTS SHADOWRUN_CONVERSION_TEST_SOURCES)
        get_filename_component(test_stem "${test_source}" NAME_WE)
        string(REGEX REPLACE "_conversion_test$" "" test_name "${test_stem}")

        # Preserve the historical target/CTest name for the original title-loop
        # harness, whose first target predates the normal naming convention.
        if(test_name STREQUAL "title_loop")
            set(test_name "conversion")
        endif()

        set(test_target "shadowrun_ghidra_${test_name}_test")
        add_executable(${test_target} "${test_source}")
        if(test_name STREQUAL "gameplay_script_menu_route" OR
           test_name STREQUAL "matrix_lua_rules")
            target_link_libraries(${test_target}
                PRIVATE shadowrun_gameplay_script_runtime)
        elseif(test_name STREQUAL "conversion_workbench" OR
           test_name STREQUAL "field_session_bootstrap" OR
           test_name STREQUAL "field_runtime_coordinator" OR
           test_name STREQUAL "field_session_source_assets" OR
           test_name STREQUAL "recruited_runner_conversation" OR
           test_name STREQUAL "startup_integration" OR
           test_name STREQUAL "title_intro_late")
            target_link_libraries(${test_target}
                PRIVATE shadowrun_ghidra_conversion_runtime)
        else()
            target_link_libraries(${test_target}
                PRIVATE shadowrun_ghidra_conversion_modules)
        endif()
        if(test_name STREQUAL "equipment_combat_bar" OR
           test_name STREQUAL "attributes_skills_profile_permutation" OR
           test_name STREQUAL "startup_integration" OR
           test_name STREQUAL "field_session_source_assets")
            target_compile_definitions(${test_target} PRIVATE
                SHADOWRUN_CONVERSION_ASSET_ROOT=\"${CMAKE_CURRENT_SOURCE_DIR}/../assets\")
        endif()
        if(test_name STREQUAL "attributes_skills_profile_permutation")
            target_compile_definitions(${test_target} PRIVATE
                SHADOWRUN_US_ROM_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/../input/shadowrun_genesis_us.bin\")
        endif()
        if(test_name STREQUAL "field_hud_ammo_permutation")
            target_compile_definitions(${test_target} PRIVATE
                SHADOWRUN_US_ROM_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/../input/shadowrun_genesis_us.bin\")
        endif()
        if(test_name STREQUAL "magic_defense_party_permutation")
            target_compile_definitions(${test_target} PRIVATE
                SHADOWRUN_US_ROM_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/../input/shadowrun_genesis_us.bin\")
        endif()
        if(test_name STREQUAL "gameplay_script_menu_route" OR
           test_name STREQUAL "matrix_lua_rules")
            target_compile_definitions(${test_target} PRIVATE
                SHADOWRUN_GAMEPLAY_SCRIPT_ROOT=\"${CMAKE_CURRENT_SOURCE_DIR}/../gameplay/scripts\")
        endif()
        add_test(NAME "shadowrun_ghidra_${test_name}"
                 COMMAND ${test_target})
    endforeach()
endif()

if (SHADOWRUN_GHIDRA_QT_FRONTEND)
    add_subdirectory(qt_runtime)
endif()
