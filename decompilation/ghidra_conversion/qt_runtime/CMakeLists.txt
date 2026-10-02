find_package(Qt6 REQUIRED COMPONENTS Widgets)

# This is the runnable Qt host for the converted source contracts. It is a
# child of ghidra_conversion, not a target in the native-port CMake project.
# It deliberately does not link shadowrun_port, shadowrun_native,
# shadowrun_qt, GenesisBus, or the native asset decoder.
add_executable(shadowrun_ghidra_conversion_qt
    src/ghidra_conversion_qt_main.cpp
    src/field_scene_runtime.cpp
    src/field_clips_menu_runtime.cpp
    src/field_party_roster_runtime.cpp
    src/field_menu_text_catalog.cpp
    src/field_command_overview_bridge.cpp
    src/genesis_image_compositor.cpp
    src/dialogue_panel.cpp
    src/dialogue_text_catalog.cpp
    src/field_dialogue_runtime.cpp
)
target_include_directories(shadowrun_ghidra_conversion_qt
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
set_target_properties(shadowrun_ghidra_conversion_qt PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
target_link_libraries(shadowrun_ghidra_conversion_qt
    PRIVATE
        shadowrun_ghidra_conversion_runtime
        Qt6::Widgets
)

# One end-to-end startup/field QA walk. The executable writes its visual
# checkpoints outside the asset tree so this remains a verification artifact,
# not a runtime resource or another generated asset pile.
add_test(NAME shadowrun_ghidra_conversion_qt_startup_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --startup-qa "${CMAKE_CURRENT_BINARY_DIR}/startup-qa")
set_tests_properties(shadowrun_ghidra_conversion_qt_startup_qa PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=windows")

add_test(NAME shadowrun_ghidra_conversion_qt_field_action_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-action-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-action-qa")
set_tests_properties(shadowrun_ghidra_conversion_qt_field_action_qa PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=windows")

add_test(NAME shadowrun_ghidra_conversion_qt_field_clips_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-clips-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-clips-qa")
set_tests_properties(shadowrun_ghidra_conversion_qt_field_clips_qa PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=windows")

# The plain command route is its own QA entry because it is the shared shell
# consumed by the inventory, posture, clips, cyberware, and status pages. A
# test that only enters a nested action can miss a broken base layer.
add_test(NAME shadowrun_ghidra_conversion_qt_field_command_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-command-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-command-qa.bmp")
set_tests_properties(shadowrun_ghidra_conversion_qt_field_command_qa PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=windows")

# All three Joshua archetypes must reach the field HUD with their own
# extracted face/bar/item families. A single-default dump cannot prove that
# Decker or Gator/Shaman propagation works.
add_test(NAME shadowrun_ghidra_conversion_qt_field_hud_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-hud-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-hud-qa")
set_tests_properties(shadowrun_ghidra_conversion_qt_field_hud_qa PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=windows")

# Keep the source-state and arithmetic contract in the same CTest suite as
# the executable walk. The Qt QA test proves wiring/pixels; this leaf test
# proves selector wrap, cancellation, equipment/ammo math, discard gates, and
# typed transfer without making the Qt presenter the oracle for the rules.
add_executable(shadowrun_ghidra_inventory_action_conversion_test
    ../tests/menus/inventory_action_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_inventory_action_conversion_test
    PRIVATE shadowrun_ghidra_conversion_runtime)
add_test(NAME shadowrun_ghidra_inventory_action_conversion
         COMMAND shadowrun_ghidra_inventory_action_conversion_test)

add_executable(shadowrun_ghidra_field_clips_menu_runtime_test
    tests/field_clips_menu_runtime_test.cpp
    src/field_clips_menu_runtime.cpp
)
target_include_directories(shadowrun_ghidra_field_clips_menu_runtime_test
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(shadowrun_ghidra_field_clips_menu_runtime_test
    PRIVATE shadowrun_ghidra_conversion_runtime)
add_test(NAME shadowrun_ghidra_field_clips_menu_runtime
         COMMAND shadowrun_ghidra_field_clips_menu_runtime_test)

add_executable(shadowrun_ghidra_field_party_roster_runtime_test
    tests/field_party_roster_runtime_test.cpp
    src/field_party_roster_runtime.cpp
)
target_include_directories(shadowrun_ghidra_field_party_roster_runtime_test
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(shadowrun_ghidra_field_party_roster_runtime_test
    PRIVATE shadowrun_ghidra_conversion_runtime)
add_test(NAME shadowrun_ghidra_field_party_roster_runtime
         COMMAND shadowrun_ghidra_field_party_roster_runtime_test)

add_executable(shadowrun_ghidra_genesis_image_compositor_test
    tests/genesis_image_compositor_test.cpp
    src/genesis_image_compositor.cpp
)
target_include_directories(shadowrun_ghidra_genesis_image_compositor_test
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
set_target_properties(shadowrun_ghidra_genesis_image_compositor_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
target_link_libraries(shadowrun_ghidra_genesis_image_compositor_test
    PRIVATE Qt6::Gui)
add_test(NAME shadowrun_ghidra_genesis_image_compositor
         COMMAND shadowrun_ghidra_genesis_image_compositor_test)

add_executable(shadowrun_ghidra_bmp_alpha_load_test
    tests/bmp_alpha_load_test.cpp
)
target_compile_definitions(shadowrun_ghidra_bmp_alpha_load_test PRIVATE
    SHADOWRUN_GHIDRA_ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../assets")
target_link_libraries(shadowrun_ghidra_bmp_alpha_load_test
    PRIVATE Qt6::Gui)
add_test(NAME shadowrun_ghidra_bmp_alpha_load
         COMMAND shadowrun_ghidra_bmp_alpha_load_test)

add_executable(shadowrun_ghidra_field_command_overview_bridge_test
    tests/field_command_overview_bridge_test.cpp
    src/field_command_overview_bridge.cpp
)
target_include_directories(shadowrun_ghidra_field_command_overview_bridge_test
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(shadowrun_ghidra_field_command_overview_bridge_test
    PRIVATE shadowrun_ghidra_conversion_runtime)
add_test(NAME shadowrun_ghidra_field_command_overview_bridge
         COMMAND shadowrun_ghidra_field_command_overview_bridge_test)

add_executable(shadowrun_ghidra_field_menu_text_catalog_test
    tests/field_menu_text_catalog_test.cpp
    src/field_menu_text_catalog.cpp
)
target_include_directories(shadowrun_ghidra_field_menu_text_catalog_test
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_compile_definitions(shadowrun_ghidra_field_menu_text_catalog_test PRIVATE
    SHADOWRUN_GHIDRA_ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../assets")
target_link_libraries(shadowrun_ghidra_field_menu_text_catalog_test
    PRIVATE shadowrun_ghidra_conversion_runtime)
add_test(NAME shadowrun_ghidra_field_menu_text_catalog
         COMMAND shadowrun_ghidra_field_menu_text_catalog_test)

add_executable(shadowrun_ghidra_dialogue_panel_test
    tests/dialogue_panel_test.cpp
    src/dialogue_panel.cpp
    src/dialogue_text_catalog.cpp
)
target_include_directories(shadowrun_ghidra_dialogue_panel_test
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_compile_definitions(shadowrun_ghidra_dialogue_panel_test PRIVATE
    SHADOWRUN_GHIDRA_ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../assets")
target_link_libraries(shadowrun_ghidra_dialogue_panel_test
    PRIVATE shadowrun_ghidra_conversion_runtime)
add_test(NAME shadowrun_ghidra_dialogue_panel
         COMMAND shadowrun_ghidra_dialogue_panel_test)

add_executable(shadowrun_ghidra_field_dialogue_runtime_test
    tests/field_dialogue_runtime_test.cpp
    src/field_dialogue_runtime.cpp
    src/dialogue_panel.cpp
    src/dialogue_text_catalog.cpp
    src/genesis_image_compositor.cpp
)
target_include_directories(shadowrun_ghidra_field_dialogue_runtime_test
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
set_target_properties(shadowrun_ghidra_field_dialogue_runtime_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
target_compile_definitions(shadowrun_ghidra_field_dialogue_runtime_test PRIVATE
    SHADOWRUN_GHIDRA_ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../assets")
target_link_libraries(shadowrun_ghidra_field_dialogue_runtime_test
    PRIVATE shadowrun_ghidra_conversion_runtime Qt6::Gui)
add_test(NAME shadowrun_ghidra_field_dialogue_runtime
         COMMAND shadowrun_ghidra_field_dialogue_runtime_test)

if (WIN32)
    find_program(SHADOWRUN_GHIDRA_QT_WINDEPLOYQT
        NAMES windeployqt windeployqt.exe
        HINTS "${CMAKE_PREFIX_PATH}/bin")
    if (SHADOWRUN_GHIDRA_QT_WINDEPLOYQT)
        # The installed Qt MinGW kit supplies release Qt libraries and the
        # release qwindows platform plugin even for a Debug application build.
        # CMAKE_BUILD_TYPE describes our symbols, not the Qt ABI selected by
        # find_package(Qt6).
        set(SHADOWRUN_GHIDRA_QT_DEPLOY_CONFIGURATION --release)
        add_custom_command(TARGET shadowrun_ghidra_conversion_qt POST_BUILD
            COMMAND "${SHADOWRUN_GHIDRA_QT_WINDEPLOYQT}"
                    ${SHADOWRUN_GHIDRA_QT_DEPLOY_CONFIGURATION}
                    --no-translations --no-compiler-runtime
                    "$<TARGET_FILE:shadowrun_ghidra_conversion_qt>"
            COMMENT "Deploying Qt 6 ${CMAKE_BUILD_TYPE} runtime beside the Ghidra conversion frontend"
            VERBATIM)
    endif()

    get_filename_component(SHADOWRUN_GHIDRA_QT_COMPILER_BIN
        "${CMAKE_CXX_COMPILER}" DIRECTORY)
    foreach(SHADOWRUN_GHIDRA_QT_RUNTIME_DLL
            libstdc++-6.dll libgcc_s_seh-1.dll libwinpthread-1.dll)
        add_custom_command(TARGET shadowrun_ghidra_conversion_qt POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                    "${SHADOWRUN_GHIDRA_QT_COMPILER_BIN}/${SHADOWRUN_GHIDRA_QT_RUNTIME_DLL}"
                    "$<TARGET_FILE_DIR:shadowrun_ghidra_conversion_qt>/${SHADOWRUN_GHIDRA_QT_RUNTIME_DLL}"
            COMMENT "Deploying matching MSYS2 runtime ${SHADOWRUN_GHIDRA_QT_RUNTIME_DLL}"
            VERBATIM)
    endforeach()
    # Pre-RUNTIME_OUTPUT_DIRECTORY builds left a copy under qt_runtime/ that
    # ninja no longer updates. Remove it so a rebuild cannot launch the stale
    # binary.
    add_custom_command(TARGET shadowrun_ghidra_conversion_qt POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E rm -f
                "${CMAKE_CURRENT_BINARY_DIR}/shadowrun_ghidra_conversion_qt.exe"
        COMMENT "Removing leftover qt_runtime copy of the conversion frontend"
        VERBATIM)
endif()
