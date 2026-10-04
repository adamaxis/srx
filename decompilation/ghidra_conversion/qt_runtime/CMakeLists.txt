find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia)

# This is the runnable Qt host for the converted source contracts. It is a
# child of ghidra_conversion, not a target in the native-port CMake project.
# It deliberately does not link shadowrun_port, shadowrun_native,
# shadowrun_qt, GenesisBus, or the native asset decoder.
add_executable(shadowrun_ghidra_conversion_qt
    src/ghidra_conversion_qt_main.cpp
    src/field_scene_runtime.cpp
    src/field_clips_menu_runtime.cpp
    src/shop_menu_runtime.cpp
    src/field_party_roster_runtime.cpp
    src/field_menu_text_catalog.cpp
    src/field_command_overview_bridge.cpp
    src/genesis_image_compositor.cpp
    src/dialogue_panel.cpp
    src/dialogue_panel_presenter.cpp
    src/dialogue_text_catalog.cpp
    src/field_dialogue_runtime.cpp
    src/field_mode_text_runtime.cpp
    src/audio_playback.cpp
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
        Qt6::Multimedia
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

# First Aid shares the real item cell/action path, then exercises the active
# party list, CANCEL row, RNG-backed medkit effect, result wait, and inventory
# return against directly extracted source text and HUD assets.
add_test(NAME shadowrun_ghidra_conversion_qt_field_first_aid_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-archetype samurai
                 --field-first-aid-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-first-aid-qa")
set_tests_properties(shadowrun_ghidra_conversion_qt_field_first_aid_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

# Automatic-use utility items share the real inventory selector/action loop,
# then wait for a fresh controller edge without changing item/equipment state.
add_test(NAME shadowrun_ghidra_conversion_qt_field_automatic_use_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-automatic-use-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-automatic-use-qa")
set_tests_properties(shadowrun_ghidra_conversion_qt_field_automatic_use_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

# Item Trade scans the typed active party roster, tests recipient capacity,
# lets the player cycle the source Yes/No choice, then mutates both records.
add_test(NAME shadowrun_ghidra_conversion_qt_field_inventory_trade_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-inventory-trade-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-inventory-trade-qa")
set_tests_properties(shadowrun_ghidra_conversion_qt_field_inventory_trade_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

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

add_test(NAME shadowrun_ghidra_conversion_qt_inventory_selector_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-command-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-inventory-selector-qa.bmp"
                 --field-archetype gator
                 --field-command-page 1)
set_tests_properties(shadowrun_ghidra_conversion_qt_inventory_selector_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

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

# The Pocket Secretary route must open from the command overview's sixth
# tab and render its own full-screen composition.
add_test(NAME shadowrun_ghidra_conversion_qt_field_secretary_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-secretary-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-secretary-qa")
set_tests_properties(shadowrun_ghidra_conversion_qt_field_secretary_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

# Dump the source-backed Stoker package prompt as the dialogue hill's named
# actual image. The focused runtime test checks its panel pixels and branches.
add_test(NAME shadowrun_ghidra_conversion_qt_field_dialogue_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-dialogue-dump manager-package
                 "${CMAKE_CURRENT_BINARY_DIR}/field-dialogue-qa/manager-package/actual.bmp")
set_tests_properties(shadowrun_ghidra_conversion_qt_field_dialogue_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

# The Ares title-only mode-text prompt is a separate presenter from the
# Stoker/NPC route. This diagnostic dump exercises its source-selected text,
# shared text wells, and no-button composition without fabricating a live
# field-event trigger.
add_test(NAME shadowrun_ghidra_conversion_qt_field_mode_text_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-mode-text-dump introduction
                 "${CMAKE_CURRENT_BINARY_DIR}/field-mode-text-qa/introduction/actual.bmp")
set_tests_properties(shadowrun_ghidra_conversion_qt_field_mode_text_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

# Ares mode 2/event 11 must pass through the generic 30-frame prompt before
# entering its modal service menu. This captures category/item selection,
# insufficient funds, default-No/Yes confirmation, ordinary and E-coded
# purchases, sale/refund, protected-item feedback, and full-screen return.
add_test(NAME shadowrun_ghidra_conversion_qt_field_shop_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-shop-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-shop-ares-qa")
set_tests_properties(shadowrun_ghidra_conversion_qt_field_shop_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

# Exercise both ROM 0x11950 branches. The normal field smoke record has bit 0
# set; the explicit empty fixture drives the zero-word branch and NONE text.
add_test(NAME shadowrun_ghidra_conversion_qt_field_cyberware_populated_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-cyberware-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-cyberware-populated/actual.bmp")
set_tests_properties(
    shadowrun_ghidra_conversion_qt_field_cyberware_populated_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

add_test(NAME shadowrun_ghidra_conversion_qt_field_cyberware_empty_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-cyberware-empty-qa
                 --field-cyberware-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-cyberware-empty/actual.bmp")
set_tests_properties(
    shadowrun_ghidra_conversion_qt_field_cyberware_empty_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

# The 0x119CE posture dialog must raise/lower the typed posture with the
# source clamp and keep the command shell underneath.
add_test(NAME shadowrun_ghidra_conversion_qt_field_posture_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-posture-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-posture-qa.bmp")
set_tests_properties(shadowrun_ghidra_conversion_qt_field_posture_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

# The page-5 A route has two source branches. This integration fixture uses
# Gator's field composition and sets typed +0x7F to zero to exercise the
# static Magical Defense fallback; it writes before/open/held/return frames.
# It is a runtime regression artifact, not an independent Genesis golden.
add_test(NAME shadowrun_ghidra_conversion_qt_field_magic_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-archetype gator
                 --field-magic-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-magic-qa")
set_tests_properties(shadowrun_ghidra_conversion_qt_field_magic_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

# This route preserves the Gator field composition's typed magic gate/table
# and walks the spell panel, live selection, four cursor phases, and return.
# Its snapshots are Qt regression artifacts, not independent Genesis goldens.
add_test(NAME shadowrun_ghidra_conversion_qt_field_magic_spell_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-archetype gator
                 --field-magic-spell-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-magic-spell-qa")
set_tests_properties(shadowrun_ghidra_conversion_qt_field_magic_spell_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

add_executable(shadowrun_ghidra_magic_menu_conversion_test
    ../tests/menus/magic_menu_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_magic_menu_conversion_test
    PRIVATE shadowrun_ghidra_conversion_runtime)
add_test(NAME shadowrun_ghidra_magic_menu_conversion
         COMMAND shadowrun_ghidra_magic_menu_conversion_test)

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

add_executable(shadowrun_ghidra_first_aid_conversion_test
    ../tests/menus/first_aid_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_first_aid_conversion_test
    PRIVATE shadowrun_ghidra_conversion_runtime)
add_test(NAME shadowrun_ghidra_first_aid_conversion
         COMMAND shadowrun_ghidra_first_aid_conversion_test)

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
set_target_properties(shadowrun_ghidra_bmp_alpha_load_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
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
    src/dialogue_panel_presenter.cpp
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

add_executable(shadowrun_ghidra_field_mode_text_runtime_test
    tests/field_mode_text_runtime_test.cpp
    src/field_mode_text_runtime.cpp
    src/dialogue_panel_presenter.cpp
    src/dialogue_panel.cpp
)
target_include_directories(shadowrun_ghidra_field_mode_text_runtime_test
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
set_target_properties(shadowrun_ghidra_field_mode_text_runtime_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
target_compile_definitions(shadowrun_ghidra_field_mode_text_runtime_test PRIVATE
    SHADOWRUN_GHIDRA_ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../assets")
target_link_libraries(shadowrun_ghidra_field_mode_text_runtime_test
    PRIVATE shadowrun_ghidra_conversion_runtime Qt6::Gui)
add_test(NAME shadowrun_ghidra_field_mode_text_runtime
         COMMAND shadowrun_ghidra_field_mode_text_runtime_test)

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

add_executable(shadowrun_ghidra_shop_menu_runtime_test
    tests/shop_menu_runtime_test.cpp
    src/shop_menu_runtime.cpp
)
target_include_directories(shadowrun_ghidra_shop_menu_runtime_test
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(shadowrun_ghidra_shop_menu_runtime_test
    PRIVATE shadowrun_ghidra_conversion_runtime)
add_test(NAME shadowrun_ghidra_shop_menu_runtime
         COMMAND shadowrun_ghidra_shop_menu_runtime_test)

# Source selector event 0x25 is ROM table-2 entry 37. Verify the event reaches
# the direct extracted asset resolver without assigning aliases to unknown
# events.
add_executable(shadowrun_ghidra_audio_playback_test
    tests/audio_playback_test.cpp
    src/audio_playback.cpp
)
target_include_directories(shadowrun_ghidra_audio_playback_test
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
set_target_properties(shadowrun_ghidra_audio_playback_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
target_compile_definitions(shadowrun_ghidra_audio_playback_test PRIVATE
    SHADOWRUN_GHIDRA_AUDIO_RESOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/sound-effects/resource-037.flac")
target_link_libraries(shadowrun_ghidra_audio_playback_test
    PRIVATE Qt6::Core Qt6::Multimedia)
add_test(NAME shadowrun_ghidra_audio_playback
         COMMAND shadowrun_ghidra_audio_playback_test)
