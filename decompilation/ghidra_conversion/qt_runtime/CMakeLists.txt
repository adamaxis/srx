find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia Network)

set(SHADOWRUN_GHIDRA_QT_RUNTIME_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/app"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/audio"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/dialogue"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/menus"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/persistence"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/rendering"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/srx_refactor"
    "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/dr_libs")

# Qt-free audio core: id-keyed resources (dr_flac), frame-locked mixer with
# null/WAV sinks, and the GEMS command layer (doc 26 phases 0-3).
set(SHADOWRUN_GHIDRA_AUDIO_CORE_SOURCES
    src/audio/audio_resources.cpp
    src/audio/audio_mixer.cpp
    src/audio/gems_commands.cpp)

set(SHADOWRUN_GHIDRA_QT_GAME_SOURCES
    src/field/field_scene_runtime.cpp
    src/field/field_map_composition.cpp
    src/field/matrix_map_presenter.cpp
    src/field/elevator_selector_runtime.cpp
    src/menus/attributes_skills_menu_runtime.cpp
    src/field/field_clips_menu_runtime.cpp
    src/menus/shop_menu_runtime.cpp
    src/field/field_party_roster_runtime.cpp
    src/field/field_menu_text_catalog.cpp
    src/field/field_command_overview_bridge.cpp
    src/rendering/genesis_image_compositor.cpp
    src/rendering/numeric_visual_renderer.cpp
    src/dialogue/dialogue_panel.cpp
    src/dialogue/dialogue_panel_presenter.cpp
    src/dialogue/field_dialogue_runtime.cpp
    src/menus/hotel_menu_runtime.cpp
    src/persistence/game_save_snapshot.cpp
    src/persistence/game_save_store.cpp
    src/field/field_mode_text_runtime.cpp
    src/field/field_interaction_runtime.cpp
    src/audio/audio_playback.cpp
    src/audio/qt_audio_sink.cpp
    ${SHADOWRUN_GHIDRA_AUDIO_CORE_SOURCES})

add_executable(shadowrun_frame_debugger_test
    tests/debug/frame_debugger_test.cpp)
target_include_directories(shadowrun_frame_debugger_test
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../include")
add_test(NAME shadowrun_frame_debugger
         COMMAND shadowrun_frame_debugger_test)

add_executable(shadowrun_frame_debug_server_test
    tests/debug/frame_debug_server_test.cpp)
target_include_directories(shadowrun_frame_debug_server_test
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/app"
        "${CMAKE_CURRENT_SOURCE_DIR}/../include")
target_link_libraries(shadowrun_frame_debug_server_test
    PRIVATE Qt6::Core Qt6::Network)
add_test(NAME shadowrun_frame_debug_server
         COMMAND shadowrun_frame_debug_server_test)

# This is the runnable Qt host for the converted source contracts. It is a
# child of the root Ghidra project, not the archived native-port project.
# It deliberately does not link shadowrun_port, shadowrun_native,
# shadowrun_qt, GenesisBus, or the native asset decoder.
add_executable(shadowrun_ghidra_conversion_qt
    src/app/ghidra_conversion_qt_main.cpp
    ${SHADOWRUN_GHIDRA_QT_GAME_SOURCES}
)

# The command-line screenshot and behavior harness is built separately from
# the game executable. CTest invokes this target; the release game only
# contains the interactive Qt application and its runtime systems.
add_executable(shadowrun_ghidra_conversion_qt_qa
    tests/qa/ghidra_conversion_qt_qa_main.cpp
    ${SHADOWRUN_GHIDRA_QT_GAME_SOURCES}
)
target_include_directories(shadowrun_ghidra_conversion_qt_qa
    PRIVATE ${SHADOWRUN_GHIDRA_QT_RUNTIME_INCLUDE_DIRS})
set_target_properties(shadowrun_ghidra_conversion_qt_qa PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
target_link_libraries(shadowrun_ghidra_conversion_qt_qa
    PRIVATE
        shadowrun_gameplay_script_runtime
        shadowrun_ghidra_conversion_runtime
        Qt6::Widgets
        Qt6::Multimedia
        Qt6::Network
)
target_compile_definitions(shadowrun_ghidra_conversion_qt_qa
    PRIVATE SHADOWRUN_QT_QA_DRIVER=1)

add_executable(shadowrun_numeric_visual_renderer_test
    tests/rendering/numeric_visual_renderer_test.cpp
    src/rendering/numeric_visual_renderer.cpp)
set_target_properties(shadowrun_numeric_visual_renderer_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
target_include_directories(shadowrun_numeric_visual_renderer_test
    PRIVATE
        ${SHADOWRUN_GHIDRA_QT_RUNTIME_INCLUDE_DIRS}
        "${CMAKE_CURRENT_SOURCE_DIR}/../include")
target_link_libraries(shadowrun_numeric_visual_renderer_test
    PRIVATE shadowrun_ghidra_conversion_runtime Qt6::Widgets)
add_test(NAME shadowrun_numeric_visual_renderer_qa
         COMMAND shadowrun_numeric_visual_renderer_test)
target_include_directories(shadowrun_ghidra_conversion_qt
    PRIVATE ${SHADOWRUN_GHIDRA_QT_RUNTIME_INCLUDE_DIRS})
set_target_properties(shadowrun_ghidra_conversion_qt PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
target_link_libraries(shadowrun_ghidra_conversion_qt
    PRIVATE
        shadowrun_gameplay_script_runtime
        shadowrun_ghidra_conversion_runtime
        Qt6::Widgets
        Qt6::Multimedia
        Qt6::Network
)

add_executable(shadowrun_field_map_composition_test
    tests/field/field_map_composition_test.cpp
    src/field/field_map_composition.cpp)
target_include_directories(shadowrun_field_map_composition_test
    PRIVATE ${SHADOWRUN_GHIDRA_QT_RUNTIME_INCLUDE_DIRS})
set_target_properties(shadowrun_field_map_composition_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
target_compile_definitions(shadowrun_field_map_composition_test PRIVATE
    SHADOWRUN_FIELD_MAP_ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../assets")
target_link_libraries(shadowrun_field_map_composition_test
    PRIVATE Qt6::Gui)
add_test(NAME shadowrun_ghidra_conversion_qt_field_map_composition_qa
         COMMAND shadowrun_field_map_composition_test)

add_executable(shadowrun_gameplay_script_runtime_test
    tests/scripting/game_script_runtime_test.cpp)
target_link_libraries(shadowrun_gameplay_script_runtime_test
    PRIVATE shadowrun_gameplay_script_runtime)
target_compile_definitions(shadowrun_gameplay_script_runtime_test PRIVATE
    SHADOWRUN_GAMEPLAY_SCRIPT_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../gameplay/scripts"
    SHADOWRUN_GHIDRA_ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
    SHADOWRUN_GAMEPLAY_SCRIPT_FIXTURE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/tests/scripting/fixtures")
add_test(NAME shadowrun_gameplay_script_runtime_qa
         COMMAND shadowrun_gameplay_script_runtime_test)

add_executable(shadowrun_vid_phone_script_test
    tests/scripting/vid_phone_script_test.cpp)
target_link_libraries(shadowrun_vid_phone_script_test
    PRIVATE shadowrun_gameplay_script_runtime)
target_compile_definitions(shadowrun_vid_phone_script_test PRIVATE
    SHADOWRUN_GAMEPLAY_SCRIPT_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../gameplay/scripts"
    SHADOWRUN_GHIDRA_ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../assets")
add_test(NAME shadowrun_vid_phone_script_qa
         COMMAND shadowrun_vid_phone_script_test)

add_executable(shadowrun_vid_phone_service_conversion_test
    tests/field/vid_phone_service_conversion_test.cpp)
target_include_directories(shadowrun_vid_phone_service_conversion_test
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../include")
target_link_libraries(shadowrun_vid_phone_service_conversion_test
    PRIVATE shadowrun_ghidra_conversion_runtime)
add_test(NAME shadowrun_vid_phone_service_conversion_qa
         COMMAND shadowrun_vid_phone_service_conversion_test)

add_executable(shadowrun_matrix_field_state_save_test
    tests/persistence/matrix_field_state_save_test.cpp
    src/persistence/game_save_snapshot.cpp)
target_include_directories(shadowrun_matrix_field_state_save_test
    PRIVATE ${SHADOWRUN_GHIDRA_QT_RUNTIME_INCLUDE_DIRS})
set_target_properties(shadowrun_matrix_field_state_save_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
target_link_libraries(shadowrun_matrix_field_state_save_test
    PRIVATE shadowrun_ghidra_conversion_runtime Qt6::Core)
add_test(NAME shadowrun_matrix_field_state_save_qa
         COMMAND shadowrun_matrix_field_state_save_test)
set_tests_properties(shadowrun_matrix_field_state_save_qa PROPERTIES
    ENVIRONMENT "SRX_HEADLESS=1;QT_QPA_PLATFORM=offscreen")

add_executable(shadowrun_run_actor_defeat_test
    tests/scripting/run_actor_defeat_test.cpp)
target_include_directories(shadowrun_run_actor_defeat_test
    PRIVATE src/srx_refactor)
target_link_libraries(shadowrun_run_actor_defeat_test
    PRIVATE shadowrun_gameplay_script_runtime)
target_compile_definitions(shadowrun_run_actor_defeat_test PRIVATE
    SHADOWRUN_GAMEPLAY_SCRIPT_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../gameplay/scripts"
    SHADOWRUN_GHIDRA_ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../assets")
add_test(NAME shadowrun_ghidra_run_actor_defeat
         COMMAND shadowrun_run_actor_defeat_test)

add_executable(shadowrun_gameplay_script_menu_route_test
    ../tests/gameplay_script_menu_route_conversion_test.cpp
    src/field/matrix_map_presenter.cpp)
target_include_directories(shadowrun_gameplay_script_menu_route_test
    PRIVATE
        ${SHADOWRUN_GHIDRA_QT_RUNTIME_INCLUDE_DIRS}
        "${CMAKE_CURRENT_SOURCE_DIR}/../include")
set_target_properties(shadowrun_gameplay_script_menu_route_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
target_link_libraries(shadowrun_gameplay_script_menu_route_test
    PRIVATE shadowrun_gameplay_script_runtime Qt6::Gui)
target_compile_definitions(shadowrun_gameplay_script_menu_route_test PRIVATE
    SHADOWRUN_GAMEPLAY_SCRIPT_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../gameplay/scripts"
    SHADOWRUN_MATRIX_MAP_ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
    SHADOWRUN_MATRIX_MAP_ARTIFACT_DIR="${CMAKE_CURRENT_BINARY_DIR}/matrix-map-route-qa"
    SHADOWRUN_MATRIX_NODE_ACTION_ARTIFACT_DIR="${CMAKE_CURRENT_BINARY_DIR}/matrix-node-action-route-qa"
    SHADOWRUN_MATRIX_CYBER_INFO_ARTIFACT_DIR="${CMAKE_CURRENT_BINARY_DIR}/matrix-cyber-info-qa"
    SHADOWRUN_MATRIX_MAP_QT_TEST=1)
add_test(NAME shadowrun_ghidra_gameplay_script_menu_route
         COMMAND shadowrun_gameplay_script_menu_route_test)

add_executable(shadowrun_world_content_tsv_loader_test
    tests/scripting/world_content_tsv_loader_test.cpp)
target_include_directories(shadowrun_world_content_tsv_loader_test
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../src/scripting")
target_link_libraries(shadowrun_world_content_tsv_loader_test
    PRIVATE shadowrun_gameplay_script_runtime)
target_compile_definitions(shadowrun_world_content_tsv_loader_test PRIVATE
    SHADOWRUN_GAMEPLAY_WORLD_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../gameplay/world")
add_test(NAME shadowrun_world_content_tsv_loader_qa
         COMMAND shadowrun_world_content_tsv_loader_test)

if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Qt's bundled WinMain shim is built against a different MinGW runtime
    # than this MSYS2 UCRT64 build. Select the GUI subsystem directly while
    # retaining the normal CRT main() startup entry point.
    target_link_options(shadowrun_ghidra_conversion_qt PRIVATE
        "-Wl,--subsystem,windows"
        "-Wl,-e,mainCRTStartup")
    target_link_options(shadowrun_ghidra_conversion_qt_qa PRIVATE
        "-Wl,--subsystem,windows"
        "-Wl,-e,mainCRTStartup")
endif()

function(add_shadowrun_qt_qa_case test_name)
    add_test(NAME "${test_name}"
        COMMAND shadowrun_ghidra_conversion_qt_qa ${ARGN})
    set_tests_properties("${test_name}" PROPERTIES
        # Headless rule: QA never shows a window or takes focus. Windows
        # builds deploy platforms/qoffscreen.dll beside the QA executable
        # (windeployqt block below); Linux/macOS use the Qt install's plugin.
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;SRX_HEADLESS=1")
endfunction()

# One end-to-end startup/field QA walk. The executable writes its visual
# checkpoints outside the asset tree so this remains a verification artifact,
# not a runtime resource or another generated asset pile.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_startup_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --startup-qa "${CMAKE_CURRENT_BINARY_DIR}/startup-qa")

# Source no-input timeout returns to the ordinary boot/title cycle instead of
# remaining on the fully faded black title frame.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_title_idle_restart_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --title-idle-restart-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/title-idle-restart-qa")

# Launch from the runtime build directory without --assets. The installed
# executable must locate the checked-in named assets from its own path rather
# than depending on the shell's working directory.
# Discovery walks up from the executable's directory for assets/resources.tsv,
# so it can only succeed when the build tree sits inside the source checkout.
# An out-of-tree build dir (e.g. /tmp/build) skips the case instead of failing.
get_filename_component(_shadowrun_assets_root
    "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
file(RELATIVE_PATH _shadowrun_binary_from_assets_root
    "${_shadowrun_assets_root}" "${CMAKE_BINARY_DIR}")
if(_shadowrun_binary_from_assets_root MATCHES "^\\.\\.")
    message(STATUS
        "asset_autodiscovery_qa skipped: build dir is outside ${_shadowrun_assets_root}")
else()
    add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_asset_autodiscovery_qa
                     --startup-dump start_options 0 0
                     "${CMAKE_CURRENT_BINARY_DIR}/asset-autodiscovery-start-options.bmp")
    set_tests_properties(shadowrun_ghidra_conversion_qt_asset_autodiscovery_qa
        PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
endif()

# The six airport rotor descriptors advance every six source frames. Capture
# all six whole screens plus the 36-frame repeat and check the animated strip
# against the horizontally translated, otherwise stable aircraft body.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_opening_rotor_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --opening-rotor-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/opening-rotor-qa")

# The landing path uses the ROM-selected movement art until the computed Y
# enters its narrow special-image window. Save whole-screen frames at every
# landing event and across entry/exit from that window.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_opening_landing_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --opening-landing-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/opening-landing-qa")

# The source Start edge is sampled and latched during the opening sequence,
# then returns through the New Game field setup without running A/B as skips.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_opening_start_skip_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --opening-start-skip-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/opening-start-skip-qa")

# During 0x59D7C the wilderness body layers pan with FFDD08/FFDD0A while the
# Indian's head SAT advances with DD46/DD48. Verify their relative placement
# and capture complete start/midpoint/settled screens from the Gator Shaman
# route.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_opening_wilderness_pan_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --opening-wilderness-pan-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/opening-wilderness-pan-qa")

# ROM 0x5A2D0 replaces all fifteen colors on palette line 1 with white, then
# 0x5A328 steps them once every nine frames. The head SAT and right actor plate
# share this palette while the six timed body/smoke frames continue separately.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_opening_ambush_flash_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --opening-ambush-flash-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/opening-ambush-flash-qa")

add_executable(shadowrun_ghidra_conversion_qt_title_intro_motion_test
    ../tests/title_intro/title_intro_motion_conversion_test.cpp)
target_link_libraries(shadowrun_ghidra_conversion_qt_title_intro_motion_test
    PRIVATE shadowrun_ghidra_conversion_modules)
add_test(NAME shadowrun_ghidra_conversion_qt_title_intro_motion_qa
         COMMAND shadowrun_ghidra_conversion_qt_title_intro_motion_test)

# The six 0x5A080 updates replace the left wilderness region. At 0x59F8E's
# terminal event, ROM fills that region with the blank tile before drawing the
# death pose. Verify the full-screen lifetime and the cockpit handoff.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_opening_death_layer_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --opening-death-layer-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/opening-death-layer-qa")

# The ROM marks the fixed trivid-left and timed SAT lists high priority
# (0xC000/0xE000) while the moving trivid-right list is low priority (0x6000).
# Whole-screen QA checks these layers across the full 0x115-frame scene loop.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_opening_trivid_layer_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --opening-trivid-layer-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/opening-trivid-layer-qa")

# The Qt preset excludes the broader conversion-test glob, so retain this
# source-contract test here alongside the end-to-end image/cadence QA.
add_executable(shadowrun_ghidra_conversion_qt_title_intro_late_test
    ../tests/title_intro/title_intro_late_conversion_test.cpp)
target_link_libraries(shadowrun_ghidra_conversion_qt_title_intro_late_test
    PRIVATE shadowrun_ghidra_conversion_runtime)
add_test(NAME shadowrun_ghidra_conversion_qt_title_intro_late_qa
         COMMAND shadowrun_ghidra_conversion_qt_title_intro_late_test)

add_executable(shadowrun_ghidra_conversion_qt_title_intro_overlay_test
    ../tests/title_intro/title_intro_overlay_conversion_test.cpp)
target_link_libraries(shadowrun_ghidra_conversion_qt_title_intro_overlay_test
    PRIVATE shadowrun_ghidra_conversion_modules)
add_test(NAME shadowrun_ghidra_conversion_qt_title_intro_overlay_qa
         COMMAND shadowrun_ghidra_conversion_qt_title_intro_overlay_test)

# Keep the pre-title and first title-roll boundaries separate from the longer
# startup walk: the ROM fades SEGA to black, fades FASA in, holds for 0x79
# VBlanks, fades back out, then holds title on black for 0x5A updates before
# revealing the first Shadowrun logo column.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_boot_fasa_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --boot-fasa-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/boot-fasa-qa")

add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_action_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-action-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-action-qa")

# Empty magazines reload through the live FieldSceneRuntime A-button path.
# The current weapon supplies its own capacity while the runner-wide clip
# reserve decreases; this dump is internal route evidence, not an independent
# Genesis visual reference.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_player_action_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-player-action-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-player-action-qa")

# Field input is held across multiple ROM-timed updates so direction slew,
# collision-accepted motion, walk-frame cadence, release-to-idle, and the
# replacement actor draw all have visible checkpoints in this scene.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_movement_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-movement-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-movement-qa")

# Verify the physical Crime Mall route and the post-load ILENE_MET actor gate.
add_shadowrun_qt_qa_case(
    shadowrun_ghidra_conversion_qt_field_crime_mall_ilene_gate_qa
    --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
    --field-crime-mall-ilene-gate-qa
    "${CMAKE_CURRENT_BINARY_DIR}/field-crime-mall-ilene-gate-qa")

# Reuse the same route probe for Riannon's event-3 entrance to shared Scene 13.
add_shadowrun_qt_qa_case(
    shadowrun_ghidra_conversion_qt_field_riannons_magic_shop_entry_qa
    --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
    --field-riannons-magic-shop-entry-qa
    "${CMAKE_CURRENT_BINARY_DIR}/field-riannons-magic-shop-entry-qa")

# The clinic's event-6 doorway is another source-routed entrance to Scene 13.
add_shadowrun_qt_qa_case(
    shadowrun_ghidra_conversion_qt_field_dr_bobs_clinic_entry_qa
    --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
    --field-dr-bobs-clinic-entry-qa
    "${CMAKE_CURRENT_BINARY_DIR}/field-dr-bobs-clinic-entry-qa")

# Every accepted collision is policy-routed through field_events.lua; the
# native event system records the typed result and still supports scoped
# scene-entry and actor-within-scope rules.
add_executable(shadowrun_ghidra_conversion_qt_field_map_event_system_test
    ../tests/field/field_map_event_system_conversion_test.cpp)
target_link_libraries(
    shadowrun_ghidra_conversion_qt_field_map_event_system_test
    PRIVATE shadowrun_ghidra_conversion_modules)
add_test(NAME shadowrun_ghidra_conversion_qt_field_map_event_system_qa
         COMMAND shadowrun_ghidra_conversion_qt_field_map_event_system_test)

# A projected interaction starts a source 0x10 countdown. 0x16396 fades on
# even counts and publishes FFDF3E only after the signed counter falls below
# zero; keep several full-frame captures so timer and screen clearing stay in
# sync.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_transition_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-transition-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-transition-qa")

# The active field event byte selects a named resource scene, samples its ROM
# bound into a typed floor index, and carries that state through the existing
# Security Terminal result prompt. Captures also verify full-scene replacement
# and clearing the modal before returning to the field.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_terrain_mark_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-terrain-mark-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-terrain-mark-qa")
# World-effect FF2700/FF4700 writes repaint the changed 32x32 cell from a
# donor cell with the same word pair (same rule as the terrain mark).
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_world_effect_repaint_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-world-effect-repaint-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-world-effect-repaint-qa")
# Field audio rules: scene-init music (profile +0x40 via audio.lua), the
# Start-menu op-0x20 duck and its restore, music on a scene change.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_audio_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-audio-qa)

add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_world_activity_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-world-activity-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-world-activity-qa")

# Resolve Scene 2's ROM sum-3 actor sequence through Lua and pass its ordered
# templates through source edge placement, Lua actor initialization, and the
# live field renderer. The selector rolls are injected so this QA isolates the
# spawn path; it does not claim to cover the random chance/frame gate.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_random_encounter_spawn_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-random-encounter-spawn-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-random-encounter-spawn-qa")

# Resolve Scene 2's source sum-8 Weenr row, allow its Lua AI to approach the
# runner, and observe the connected 0x1514E spawn, ammo, sound, and 0xB39A/C042
# presentation in full-screen captures.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_npc_item_action_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-npc-item-action-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-npc-item-action-qa")

# Force Scene 0's ROM sum-0 encounter list, place its source Hell Hound inside
# the close-target band, and review the complete screen through phase 4 release.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_hound_close_effect_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-hound-close-effect-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-hound-close-effect-qa")

# Render all four ROM spell-effect phase sets over Scene 1's loaded map, HUD,
# runner, and ROM-selected Lone Star mage. Lua timing and target effect ownership
# are asserted separately by shadowrun_gameplay_script_runtime_qa.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_spell_effect_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-spell-effect-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-spell-effect-qa")

# Start one cell south of the ROM's Jump House trigger, walk into record 85
# using normal movement, dismiss its Lua-owned entry narration with A, verify
# the retained scene-13 route, then walk south into its null-table event-0
# transition and confirm the source-selected scene-0 destination.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_jump_house_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-jump-house-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-jump-house-entry-qa")

# Enter each Scene 14 south-edge event-0 rectangle independently with ordinary
# Down input. Its null event-table pointer selects field Scene 0 through the
# ROM's generic handler.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_wanderer_south_exit_record_7_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-wanderer-south-exit-record-7-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-wanderer-south-exit-record-7-qa")
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_wanderer_south_exit_record_8_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-wanderer-south-exit-record-8-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-wanderer-south-exit-record-8-qa")

# Enter the Big Rhino booth with ordinary movement and verify the ROM-selected
# Mortimer Reed first-visit page, portrait, and three-choice root.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_big_rhino_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-big-rhino-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-big-rhino-entry-qa")

# Enter the Wanderer after FFFC1E bit 16 and verify its blank-profile,
# no-choice booth-vacated page using ordinary movement into event 30.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_wanderer_vacated_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-wanderer-vacated-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-wanderer-vacated-entry-qa")

# Enter Icarus Descending through its scene-12/event-30 rectangle with normal
# Up input and the ROM access flag, then capture Caleb's first meeting.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_icarus_caleb_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-icarus-caleb-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-icarus-caleb-entry-qa")

# Use the same physical event record with FFFC1E bit 10 clear. The Lua link
# must show the ROM bodyguard rejection and leave the quest state untouched.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_icarus_blocked_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-icarus-blocked-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-icarus-blocked-entry-qa")

# Enter scene-1 record 67 through ordinary movement with the source bit-30
# fixture, then verify service 21's first-entry conversation and field return.
# Scene 54 is selected only by the later, quest-gated tomb-departure choice.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_tarislar_first_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-tarislar-first-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-tarislar-first-entry-qa")

# Seed the completed Ito briefing and Renraku map item, then enter record 67
# by normal movement to defend the ROM-gated repeat-service page and map path.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_tarislar_repeat_service_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-tarislar-repeat-service-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-tarislar-repeat-service-qa")

# Seed the pre-Frosty story state and verify service 21's guarded passage
# returns Joshua to scene 1 without applying the first-entry transition.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_tarislar_guarded_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-tarislar-guarded-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-tarislar-guarded-entry-qa")

# Exercise both signed FFF0D5 result pages by reaching record 67 with ordinary
# field movement. Each path must remain on scene 1 after its one-page prompt.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_tarislar_positive_alternate_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-tarislar-positive-alternate-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-tarislar-positive-alternate-qa")
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_tarislar_negative_alternate_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-tarislar-negative-alternate-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-tarislar-negative-alternate-qa")

# Enter scene-1 record 68 through ordinary Up movement with title service 1's
# source flags, then confirm its Lua-owned intro, choices, same-map leave, and
# clean return without taking the table's numeric scene-53 route.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_tarislar_city_inn_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-tarislar-city-inn-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-tarislar-city-inn-qa")

# Walk into Wylie's Gala Inn on Scene 3 and exercise the paid room prompt. Its
# ROM event-table value is scene 13, but the dialog consumes that destination
# and opens Hotel Options on the same map after the 50-nuyen payment.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_wylie_gala_inn_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-wylie-gala-inn-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-wylie-gala-inn-qa")

# Reach Gates Undersound's source event-3 frontage, exercise gated title
# service 15, and verify the one-page Ogami departure result returns to scene 0.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_gates_undersound_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-gates-undersound-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-gates-undersound-qa")

# Enter Renraku's scene-5/event-6 record through ordinary movement and verify
# the ROM-gated entry page resumes the event table's resource activity 35.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_renraku_offices_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-renraku-offices-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-renraku-offices-qa")

# Exercise Fuchi's conditional maglock frontage and title-service-14 entry to
# resource activity 21 using ordinary field movement.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_fuchi_offices_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-fuchi-offices-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-fuchi-offices-qa")

# Enter the gated Fuchi office, then physically reach its event-31 Main Menu
# and check the ROM-seeded Matrix target for resource scene 21.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_fuchi_matrix_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-fuchi-matrix-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-fuchi-matrix-entry-qa")

# Enter resource scene 26, reach its event-31 menu through ordinary movement,
# and verify the ROM-seeded Matrix system/node survive the Lua handoff.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_resource_matrix_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-scene26-event31-matrix-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-resource-matrix-entry-qa")

# Enter the source-confirmed Penumbra Lone Star frontage through its ordinary
# event-6 route and verify the numeric resource-activity destination 41.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_penumbra_lone_star_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-penumbra-lone-star-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-penumbra-lone-star-entry-qa")

# Seed the ROM's completed Harlequin/Ito/map gates, walk into record 67 with
# ordinary Up input, accept the Yes prompt, and verify the three travel pages
# before the route enters scene 54.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_tarislar_tomb_departure_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-tarislar-tomb-departure-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-tarislar-tomb-departure-qa")

# Select No on the same ROM gate set and prove the task-result guard keeps the
# retained scene-1 route instead of loading scene 54.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_tarislar_tomb_decline_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-tarislar-tomb-decline-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-tarislar-tomb-decline-qa")

# The completed New Game opening starts the source heading-zero automatic walk
# from the scene-2 spawn. It reaches the motel doorway's source C2 collision,
# event 2, and indexed Stoker title service without controller input. A opens
# the manager's three-choice Michael-package offer, C opens the debt question,
# B opens the Jump House referral, and A returns to a freshly drawn field.
# Every checkpoint is a complete 320x224 screen dump for visual review.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_stoker_collision_to_dialogue_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-stoker-collision-to-dialogue-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-stoker-collision-to-dialogue-qa")

# The completed 0x59C5E New Game opening must carry its one-shot automatic
# field movement marker into the real field-ready handoff. This source-level
# integration check complements the Qt route QA, which follows the marker
# through scene-2 movement, collision, dialogue, and clean return.
add_executable(shadowrun_ghidra_conversion_qt_startup_integration_test
    ../tests/integration/startup_integration_conversion_test.cpp)
target_link_libraries(
    shadowrun_ghidra_conversion_qt_startup_integration_test
    PRIVATE shadowrun_ghidra_conversion_runtime)
target_compile_definitions(
    shadowrun_ghidra_conversion_qt_startup_integration_test PRIVATE
    SHADOWRUN_CONVERSION_ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../assets")
add_test(NAME shadowrun_ghidra_conversion_qt_startup_integration_qa
         COMMAND shadowrun_ghidra_conversion_qt_startup_integration_test)

# Ordinary Right/Up input from the New Game spawn contacts scene-2 record 92
# (the physical Ares entrance), publishes event 11, waits through the source
# fade and generic prompt, enters the shop with A, then verifies a clean field
# redraw after Start. No mode/event tuple is injected.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_ares_collision_shop_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-ares-collision-shop-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-ares-collision-shop-qa")

# Ordinary D-pad input contacts the paired scene-2 C0 doorway records,
# enters scene 0x14, walks to the scene-20 maglock, and opens its source-backed
# choice panel. C/Walk Away returns to a clean field-only redraw. The QA writes
# actual.bmp and clears its dedicated directory before every run.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_scene20_physical_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-scene20-physical-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-scene20-physical-entry-qa")

# Seed the source's FFFC2C scene-entry marker, reach Scene 20 through the
# ordinary Scene-2 doorway, and verify the host consumes bit 26 and schedules
# the same 0x78-frame timeout as ROM 0x1689A/0x1DBAA.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_scene20_entry_gate_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-scene20-entry-gate-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-scene20-entry-gate-qa")

# Seed service 19's ROM story/tip gates, enter through the paired Scene-2
# doorway with ordinary movement, and check that Lua arms transient FFE182
# while the existing route still enters Scene 20.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_scene20_ilene_prerequisite_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-scene20-ilene-prerequisite-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-scene20-ilene-prerequisite-qa")

# Scene 20 record 110 is a source C0/event-0 contact with no event-table row.
# Enter through Scene 2's physical C0 route, then walk around the adjacent
# 0x7F rectangle with ordinary Left and Up input. Keep Up held through the
# following service update, then verify source-context return and retry latch.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_scene20_c0_exit_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-scene20-c0-exit-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-scene20-c0-exit-qa")

# Normal Right then Up+Right input from New Game contacts scene-2 collision
# record 93 (C1/event 1). Its map event-table row 1 selects resource scene
# 0x1A; this captures the source fade and complete destination scene after the
# typed world-activity resource selection and scene re-entry.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_scene26_event1_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-scene26-event1-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-scene26-event1-entry-qa")

# A ROM-valid type-5 run targeting Little Chiba shows its diversion passage
# before resuming the same event-table transition into resource scene 26.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_scene26_johnson_run_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-scene26-johnson-run-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-scene26-johnson-run-entry-qa")

# The type-4 package-recovery passage uses the same matched-target ordering.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_scene26_package_recovery_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-scene26-package-recovery-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-scene26-package-recovery-entry-qa")

# Scene-2 event 9 is the ROM-valid Halloweeners contract target. Ordinary
# movement must publish event 9, show the ROM response passage, then resume the
# event-table route into resource scene 26.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_scene2_gang_contract_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-scene2-gang-contract-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-scene2-gang-contract-entry-qa")

# ROM gang-contract targets in Puyallup and Penumbra show the same entry
# passage at an exact area/event match; the Scene-2 test covers the third
# tuple. Wrong-target/no-run states use each map's ordinary event-table route.
add_shadowrun_qt_qa_case(
    shadowrun_ghidra_conversion_qt_field_gang_contract_target_entry_qa
    --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
    --field-gang-contract-target-entry-qa
    "${CMAKE_CURRENT_BINARY_DIR}/field-gang-contract-target-entry-qa")

# Continue the ordinary-input event-table route into scene 26, cross its ROM
# event-29 elevator trigger, animate the two independently tracked floor
# labels at the 17x8 cadence, then load the selected child scene through its
# extracted profile group. Captures cover the full selector, animation, and
# clean destination field with its HUD.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_elevator_physical_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-elevator-physical-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-elevator-physical-entry-qa")

# Scene 25 is the non-member entry scene for the source child-scene group
# 21/22/23/24. Walk from its descriptor spawn into event 29, visit all four
# floor selections, then ride to scene 22 and inspect every captured frame.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_elevator_scene25_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-elevator-scene25-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-elevator-scene25-entry-qa")

# Enter the 21/22/23/24 group from scene 25, follow source selector handoffs
# through each member-scene event-29 trigger, then test scene 22's event-31
# Matrix route from its adjacent cell.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_elevator_scene25_group_roundtrip_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-elevator-scene25-group-roundtrip-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-elevator-scene25-group-roundtrip-qa")

# Continue the same elevator chain from Scene 22 to Scene 24, then verify its
# separate ROM-seeded Matrix node through Scene 24 record 136.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_elevator_scene24_matrix_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-elevator-scene25-matrix24-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-elevator-scene24-matrix-entry-qa")

# Continue the Fuchi elevator route to Scene 23 and verify its separate
# event-31 Matrix node from ROM record 159.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_elevator_scene23_matrix_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-elevator-scene25-matrix23-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-elevator-scene23-matrix-entry-qa")

# Scene 29 is the non-member entry scene for the source child-scene group
# 26/27/28. Walk from its descriptor spawn into its paired event-29 records,
# verify the group starts at floor index 0, then ride to scene 27 and inspect
# every frame through the clean scene replacement.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_elevator_scene29_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-elevator-scene29-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-elevator-scene29-entry-qa")

# Scene 34 is the non-member entry scene for the source child-scene group
# 30/31/32/33. Walk from its descriptor spawn into event 29, visit all four
# floor selections, then ride to scene 31 and inspect every captured frame.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_elevator_scene34_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-elevator-scene34-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-elevator-scene34-entry-qa")

# Enter the 30/31/32/33 group from scene 34, then follow source selector
# handoffs through each member-scene event-29 trigger and back to scene 31.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_elevator_scene34_group_roundtrip_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-elevator-scene34-group-roundtrip-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-elevator-scene34-group-roundtrip-qa")

# Scene 40 is the non-member entry scene for the source child-scene group
# 35/36/37/38/39. Walk north into event 29, visit every floor, then ride to 36.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_elevator_scene40_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-elevator-scene40-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-elevator-scene40-entry-qa")

# Enter the 35/36/37/38/39 group from scene 40, then follow native selector
# handoffs through each member-scene event-29 trigger and back to scene 36.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_elevator_scene40_group_roundtrip_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-elevator-scene40-group-roundtrip-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-elevator-scene40-group-roundtrip-qa")

# Scene 43 enters the 41/42 group; continue through both member-scene triggers
# to verify the source current-floor index and the complete group round trip.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_elevator_scene43_group_roundtrip_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-elevator-scene43-group-roundtrip-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-elevator-scene43-group-roundtrip-qa")

# Follow the same ordinary-input route, select the visible Frag Grenade option
# with B, and exercise the no-item result and its clean A dismissal. The New
# Game party inventory is checked before the choice; this is not a synthetic
# prompt or injected world event.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_scene20_grenade_no_item_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-scene20-grenade-no-item-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-scene20-grenade-no-item-qa")

# Follow the same physical map route with one QA-only passkey in the active
# runner's live inventory. Ordinary A must disable record 117's lock, request
# sound 0x4B, preserve the item, update the map counter, and return without a
# leftover modal or HUD layer drawn over the field.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_scene20_passkey_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-scene20-passkey-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-scene20-passkey-qa")

# Follow the same physical route with a QA-only quality-1 passkey. ROM 0x21628
# compares key quality with the current selection (2 here), so ordinary A must
# retain the generic three-choice prompt, leave the key and map untouched, and
# request no unlock sound before Walk Away returns to the field.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_scene20_insufficient_passkey_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-scene20-insufficient-passkey-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-scene20-insufficient-passkey-qa")

# The US ROM's Samurai template at 0x7291A has Electronics 0 at +0x8A. Follow
# its normal C0 doorway route; ROM 0x21692 must show the no-skill result
# without running the random trial, changing the map/inventory, or adding a
# sound.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_scene20_electronics_no_skill_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-archetype samurai
                 --field-scene20-electronics-no-skill-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-scene20-electronics-no-skill-qa")

# Follow the same physical route with the New Game Decker. Seed 0 yields a
# below-target no-kit trial; ROM 0x21692 must show the klaxon text and arm the
# scene-0x14 timeout without changing selection, inventory, map, or audio.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_scene20_electronics_alert_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-archetype decker
                 --field-scene20-electronics-alert-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-scene20-electronics-alert-qa")

# The US Decker template has Electronics 1 and no Electronics Kit. Verify
# one partial no-kit attempt, source collision re-entry resetting selection
# from event 2, then a QA-seeded Electronics Kit trial that opens the lock.
# Pin source PRNG state only at tested action boundaries.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_scene20_electronics_trials_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-archetype decker
                 --field-scene20-electronics-trials-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-scene20-electronics-trials-qa")

# Follow ordinary D-pad movement through a scene-2 obstacle edge and collision
# record 2. The source record is a type-2 collision object, not a Stoker
# identity or event code; this QA checks the ROM's side-step fallback, its
# neutral release, and the record's event-0/no-dialogue result.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_stoker_collision_candidate_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-stoker-collision-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-stoker-collision-qa")

# Follow ordinary New Game movement into the west wall of Stoker's motel,
# hold against the source collision cells, move diagonally along the frontage,
# and retreat. Whole-screen checkpoints and a per-frame coordinate log keep
# the player away from scene-2's C2 doorway event while testing collision.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_scene2_motel_street_collision_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-scene2-motel-street-collision-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-scene2-motel-street-collision-qa")

# First Aid shares the real item cell/action path, then exercises the active
# party list, CANCEL row, RNG-backed medkit effect, result wait, and inventory
# return against directly extracted source text and HUD assets.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_first_aid_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-archetype samurai
                 --field-first-aid-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-first-aid-qa")

# Automatic-use utility items share the real inventory selector/action loop,
# then wait for a fresh controller edge without changing item/equipment state.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_automatic_use_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-automatic-use-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-automatic-use-qa")

# Item Trade scans the typed active party roster, tests recipient capacity,
# lets the player cycle the source Yes/No choice, then mutates both records.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_inventory_trade_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-inventory-trade-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-inventory-trade-qa")

add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_clips_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-clips-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-clips-qa")

# The plain command route is its own QA entry because it is the shared shell
# consumed by the inventory, posture, clips, cyberware, and status pages. A
# test that only enters a nested action can miss a broken base layer.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_command_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-command-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-command-qa.bmp")

add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_taxi_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-taxi-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-taxi-qa.bmp")

# Follow ordinary field-terminal selections into Sharkey's Lua services,
# charge both source passage prices, acknowledge the results, and run the real
# destination fade/map reload through clean Scene 6 and Scene 9 arrivals. Then
# exercise the production item callback for Smart Goggles success and full
# inventory, including clean terminal redraws.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_vid_phone_passage_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-vid-phone-passage-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-vid-phone-passage-qa")

# Follow the ordinary field terminal through System Search and the ROM
# jack-in wait, then capture complete field-runtime frames for the initial
# Cyberdeck, CyberInfo, Run Info, Deck Stats, and Pull Plug screens. Pull Plug
# is checked against the live deck grid/footer and source-position prompt.
# The full framebuffer checks include the bottom scanline so a text-only or
# cropped artifact cannot be mistaken for a screen capture.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_matrix_cyber_info_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-matrix-cyber-info-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-matrix-cyber-info-qa")

# Drive the ROM-timed Matrix entry stream from a deterministic System 2 SPU
# state. Full 320x224 captures at each visual anchor protect the model growth,
# bottom footer, and source-derived setup/entry durations.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_matrix_traversal_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-matrix-traversal-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-matrix-traversal-entry-qa")

# The runner summary uses the equipped item's Lua-backed name/category before
# its ROM-indexed damage and power rows. Keep that shared menu panel covered by
# a full-frame artifact with the default equipped Model 101T fixture.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_runner_summary_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-command-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-runner-summary-qa.bmp"
                 --field-command-page 2)
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_runner_summary_gator_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-command-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-runner-summary-gator-qa.bmp"
                 --field-command-page 2
                 --field-archetype gator)

# Command-menu Attributes/Skills is the view-only Karma page. Its separate
# Hotel route exposes the upgrade preview and spending prompt; keep the
# captures and verification for those distinct screen variants separate.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_attributes_skills_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-attributes-skills-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-attributes-skills-view-only-qa")

add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_inventory_selector_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-command-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-inventory-selector-qa.bmp"
                 --field-archetype gator
                 --field-command-page 1)

# All three Joshua archetypes must reach the field HUD with their own
# extracted face/bar/item families. A single-default dump cannot prove that
# Decker or Gator/Shaman propagation works.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_hud_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-hud-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-hud-qa")

# Follow normal New Game field controls from the physical Stoker C2 doorway
# through the first-visit scripted package offer, room payment, Hotel
# status/options, Hotel Karma spending, and checkout. The 223-frame
# whole-screen capture starts at field spawn and includes a 30-frame stable
# field return. The route does not inject dialogue, payment results, or menu
# state.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_hotel_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-hotel-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-hotel-qa")

# Both menu routes share the ROM's Attributes/Skills grid and current-Karma
# value, but render different detail lines. Compare fresh output from each
# end-to-end route and require only the lower line to differ.
add_executable(shadowrun_ghidra_conversion_qt_karma_screen_distinction_qa
    tests/menus/karma_screen_distinction_test.cpp)
target_link_libraries(
    shadowrun_ghidra_conversion_qt_karma_screen_distinction_qa
    PRIVATE Qt6::Gui)
set_target_properties(
    shadowrun_ghidra_conversion_qt_karma_screen_distinction_qa
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
add_test(NAME shadowrun_ghidra_conversion_qt_karma_screen_distinction_qa
         COMMAND shadowrun_ghidra_conversion_qt_karma_screen_distinction_qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-attributes-skills-view-only-qa/actual.bmp"
                 "${CMAKE_CURRENT_BINARY_DIR}/field-hotel-qa/karma-spend/actual.bmp")
set_tests_properties(
    shadowrun_ghidra_conversion_qt_karma_screen_distinction_qa
    PROPERTIES
        DEPENDS "shadowrun_ghidra_conversion_qt_field_attributes_skills_qa;shadowrun_ghidra_conversion_qt_field_hotel_qa")

# The Pocket Secretary route must open from the command overview's sixth
# tab and render its own full-screen composition.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_secretary_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-secretary-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-secretary-qa")

# Dump the source-backed Stoker package prompt as the dialogue hill's named
# actual image. The focused runtime test checks its panel pixels and branches.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_dialogue_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-dialogue-dump manager-package
                 "${CMAKE_CURRENT_BINARY_DIR}/field-dialogue-qa/manager-package/actual.bmp")

add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_dialogue_arrival_manager_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-dialogue-dump arrival-manager
                 "${CMAKE_CURRENT_BINARY_DIR}/field-dialogue-qa/arrival-manager/actual.bmp")

add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_dialogue_room_paid_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-dialogue-dump room-paid
                 "${CMAKE_CURRENT_BINARY_DIR}/field-dialogue-qa/room-paid/actual.bmp")

add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_dialogue_room_insufficient_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-dialogue-dump room-insufficient
                 "${CMAKE_CURRENT_BINARY_DIR}/field-dialogue-qa/room-insufficient/actual.bmp")

# Exercise the Lua Michael quest choice through its native debt/referral
# services and verify those mutations feed the dynamic Notebook row gates.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_dialogue_debt_followup_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-dialogue-dump debt-followup
                 "${CMAKE_CURRENT_BINARY_DIR}/field-dialogue-qa/debt-followup/actual.bmp")

add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_dialogue_referral_followup_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-dialogue-dump referral-followup
                 "${CMAKE_CURRENT_BINARY_DIR}/field-dialogue-qa/referral-followup/actual.bmp")

# Run command 0x05 from the debt question with the New Game wallet. The source
# 20-nuyen balance takes the insufficient-funds branch to the Jump House page.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_dialogue_debt_payment_insufficient_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-dialogue-dump debt-payment-insufficient
                 "${CMAKE_CURRENT_BINARY_DIR}/field-dialogue-qa/debt-payment-insufficient/actual.bmp")

# Hospital Options is a ROM field-mode screen hosted by the Lua menu task.
# Save both the three-row menu and its No-default leave confirmation as named
# full-frame artifacts after checking the content and selected-row state.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_hospital_options_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-hospital-menu-dump options
                 "${CMAKE_CURRENT_BINARY_DIR}/field-hospital-qa/options/actual.bmp")

add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_hospital_leave_confirmation_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-hospital-menu-dump leave-confirmation
                 "${CMAKE_CURRENT_BINARY_DIR}/field-hospital-qa/leave-confirmation/actual.bmp")

add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_hospital_medical_attention_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-hospital-menu-dump medical-attention
                 "${CMAKE_CURRENT_BINARY_DIR}/field-hospital-qa/medical-attention/actual.bmp")

# Exercise the scene-4 hospital location link through a real collision, then
# select Services and verify the Lua-owned Hospital Options screen opens.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_hospital_physical_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-hospital-physical-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-hospital-physical-entry-qa")

# Enter scene-4 record 88 with ordinary Up and verify the ROM-gated Medicine
# Lodge offer and its Lua-owned Running Stream response on the same field map.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_medicine_lodge_physical_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-medicine-lodge-physical-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-medicine-lodge-physical-entry-qa")

# Follow the quest-gated Wise One choice and verify command 0x1F queues its
# ROM-derived scene-entry placement while returning to the same field map.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_medicine_lodge_wise_one_physical_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-medicine-lodge-wise-one-physical-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-medicine-lodge-wise-one-physical-entry-qa")

# Walk into the Spirit Eyes site through scene-4 record 96/event 27 and check
# the first-contact Lua dialogue against the physical ROM collision route.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_spirit_eyes_physical_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-spirit-eyes-physical-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-spirit-eyes-physical-entry-qa")

# Walk into the quest-gated Owlfeather lodge through Scene-6 record 193 and
# verify the ROM title service opens its Lua dialogue and portrait.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_owlfeather_physical_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-owlfeather-physical-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-owlfeather-physical-entry-qa")

# Walk into Owlfeather's lodge after Aragorn is found, verify trigger 4's
# returned-brother dialogue graph, and confirm its same-map trigger tail.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_owlfeather_return_followup_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-owlfeather-return-followup-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-owlfeather-return-followup-qa")

# Exercise Owlfeather's search follow-up after trigger 2, then take the
# ROM's Later choice and verify the two farewell pages return to Scene 6.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_owlfeather_search_followup_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-owlfeather-search-followup-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-owlfeather-search-followup-qa")

# With the Inn clue absent, record 193 must skip Owlfeather's indexed dialogue
# service and follow the ROM event-table destination into scene 13.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_owlfeather_closed_gate_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-owlfeather-closed-gate-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-owlfeather-closed-gate-qa")

# Set Ilene's story flag with no runner marker 7; the Scene-13 selector must
# select its conditional actor-list entry after the field event-table route.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_owlfeather_ilene_gate_script_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-owlfeather-ilene-gate-script-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-owlfeather-ilene-gate-script-qa")

# Check the two information-only choices on Owlfeather's first prompt, including
# their source farewell pages and same-map return without setting story bit 2.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_owlfeather_opening_land_choice_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-owlfeather-opening-land-choice-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-owlfeather-opening-land-choice-qa")

add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_owlfeather_opening_council_choice_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-owlfeather-opening-council-choice-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-owlfeather-opening-council-choice-qa")

# Walk into Red Buffalo Woman's lodge through Scene-6 record 194 and verify
# title service 18 gates the gift on story bit 17, then updates its source flags.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_red_buffalo_physical_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-red-buffalo-physical-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-red-buffalo-physical-entry-qa")

# Reach the same Scene-6 collision through ordinary Up with the Ritual of
# Binding gate clear, select More with ordinary B, follow the ROM's first
# Harlequin topic chain, and verify the no-Ito return to the source map.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_red_buffalo_no_gift_more_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-red-buffalo-no-gift-more-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-red-buffalo-no-gift-more-qa")

# Enter the ordinary field terminal through Scene-0 record 62/event 31,
# choose Vid-Phone and Captain Quinton, then verify return to the menu.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_vid_phone_physical_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-vid-phone-physical-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-vid-phone-physical-entry-qa")

# The Ares title-only mode-text prompt is a separate presenter from the
# Stoker/NPC route. This diagnostic dump exercises its source-selected text,
# shared text wells, and no-button composition without fabricating a live
# field-event trigger.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_mode_text_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-mode-text-dump introduction
                 "${CMAKE_CURRENT_BINARY_DIR}/field-mode-text-qa/introduction/actual.bmp")

# Door and Security Terminal object prompts use the shared dialogue frame and
# the same source-timed A/B/C prompt controller as NPC and mode text.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_interaction_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-interaction-dump maglock-choices
                 "${CMAKE_CURRENT_BINARY_DIR}/field-interaction-qa/maglock-choices/actual.bmp")

add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_security_terminal_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-interaction-dump security-terminal
                 "${CMAKE_CURRENT_BINARY_DIR}/field-interaction-qa/security-terminal/actual.bmp")

# Scene 26 collision records 124-125 contain the signed event-0x18 Security
# Terminal region. Start at the source-coordinate pre-trigger checkpoint
# (0x1A00,0x0500), approach with ordinary Up input, show both choices, and Walk
# Away. Descriptor-spawn exploration reaches the event-0x1D elevator and then
# the A2/event-2 electromagnetic-lock introduction; its action result and the
# path from there to the D8 terminal remain unverified.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_security_terminal_physical_entry_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-security-terminal-physical-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-security-terminal-physical-entry-qa")

# Enter scene 26 at its real map descriptor spawn, take the event-0x1D elevator
# back to the current floor, use an active-party passkey at both maglocks, and
# follow the collision map with ordinary input to the D8 Security Terminal.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_scene26_maglock_descriptor_route_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-scene26-maglock-descriptor-route-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/scene26-maglock-descriptor-route-qa")

# Keep one FieldSceneRuntime alive across the normal scene-2 event-1 handoff,
# the physical scene-26 route, and the D8 Security Terminal result. This checks
# that the floor selected at 0x167AE survives traversal into 0x20EC2. Only the
# passkey, Electronics skill, and PRNG result are QA fixtures.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_scene26_event1_terminal_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-scene26-event1-terminal-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/scene26-event1-terminal-qa")

# Ares mode 2/event 11 must pass through the generic 30-frame prompt before
# entering its modal service menu. This captures category/item selection,
# insufficient funds, default-No/Yes confirmation, ordinary and E-coded
# purchases, sale/refund (including equipped-weapon ammo cleanup and a 30-frame
# unarmed HUD hold), protected-item feedback, and full-screen return.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_shop_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-shop-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-shop-ares-template-qa")

# Exercise both ROM 0x11950 branches. The normal field smoke record has bit 0
# set; the explicit empty fixture drives the zero-word branch and NONE text.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_cyberware_populated_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-cyberware-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-cyberware-populated/actual.bmp")

add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_cyberware_empty_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-cyberware-empty-qa
                 --field-cyberware-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-cyberware-empty/actual.bmp")

# ROM 0x119CE posture focus: Up/Down on every command row, Left/Right and
# clamping, 0x25 audio requests, static hold, exact redraw, and field return.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_posture_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-posture-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-posture-qa.bmp")

# The page-5 A route has two source branches. Exercise the fallback with
# Samurai's unmodified source record (class 2, +0x7F = 0); the separate spell
# test uses Gator Shaman's source record. These are runtime regression
# artifacts, not independent Genesis goldens.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_magic_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-archetype samurai
                 --field-magic-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-magic-nonmagic-qa")

# This route preserves the Gator field composition's typed magic gate/table
# and walks the spell panel, live selection, four cursor phases, and return.
# Its snapshots are Qt regression artifacts, not independent Genesis goldens.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_magic_spell_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-archetype gator
                 --field-magic-spell-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-magic-spell-qa")

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
    tests/field/field_clips_menu_runtime_test.cpp
    src/field/field_clips_menu_runtime.cpp
)
target_include_directories(shadowrun_ghidra_field_clips_menu_runtime_test
    PRIVATE ${SHADOWRUN_GHIDRA_QT_RUNTIME_INCLUDE_DIRS})
target_link_libraries(shadowrun_ghidra_field_clips_menu_runtime_test
    PRIVATE shadowrun_ghidra_conversion_runtime)
add_test(NAME shadowrun_ghidra_field_clips_menu_runtime
         COMMAND shadowrun_ghidra_field_clips_menu_runtime_test)

add_executable(shadowrun_ghidra_attributes_skills_menu_runtime_test
    tests/menus/attributes_skills_menu_runtime_test.cpp
    src/menus/attributes_skills_menu_runtime.cpp
)
target_include_directories(shadowrun_ghidra_attributes_skills_menu_runtime_test
    PRIVATE ${SHADOWRUN_GHIDRA_QT_RUNTIME_INCLUDE_DIRS})
target_link_libraries(shadowrun_ghidra_attributes_skills_menu_runtime_test
    PRIVATE shadowrun_ghidra_conversion_runtime)
add_test(NAME shadowrun_ghidra_attributes_skills_menu_runtime
         COMMAND shadowrun_ghidra_attributes_skills_menu_runtime_test)

# Keep the ROM-derived selected-cell and cyberware modifier rules in the Qt
# CTest suite as well as the full-screen presenter walk.
add_executable(shadowrun_ghidra_cyberware_detail_conversion_test
    ../tests/menus/cyberware_detail_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_cyberware_detail_conversion_test
    PRIVATE shadowrun_ghidra_conversion_runtime)
add_test(NAME shadowrun_ghidra_cyberware_detail_conversion
         COMMAND shadowrun_ghidra_cyberware_detail_conversion_test)

add_executable(shadowrun_ghidra_field_party_roster_runtime_test
    tests/field/field_party_roster_runtime_test.cpp
    src/field/field_party_roster_runtime.cpp
)
target_include_directories(shadowrun_ghidra_field_party_roster_runtime_test
    PRIVATE ${SHADOWRUN_GHIDRA_QT_RUNTIME_INCLUDE_DIRS})
target_link_libraries(shadowrun_ghidra_field_party_roster_runtime_test
    PRIVATE shadowrun_ghidra_conversion_runtime)
add_test(NAME shadowrun_ghidra_field_party_roster_runtime
         COMMAND shadowrun_ghidra_field_party_roster_runtime_test)

add_executable(shadowrun_ghidra_genesis_image_compositor_test
    tests/rendering/genesis_image_compositor_test.cpp
    src/rendering/genesis_image_compositor.cpp
)
target_include_directories(shadowrun_ghidra_genesis_image_compositor_test
    PRIVATE ${SHADOWRUN_GHIDRA_QT_RUNTIME_INCLUDE_DIRS})
set_target_properties(shadowrun_ghidra_genesis_image_compositor_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
target_link_libraries(shadowrun_ghidra_genesis_image_compositor_test
    PRIVATE Qt6::Gui)
add_test(NAME shadowrun_ghidra_genesis_image_compositor
         COMMAND shadowrun_ghidra_genesis_image_compositor_test)

add_executable(shadowrun_ghidra_bmp_alpha_load_test
    tests/rendering/bmp_alpha_load_test.cpp
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
    tests/field/field_command_overview_bridge_test.cpp
    src/field/field_command_overview_bridge.cpp
)
target_include_directories(shadowrun_ghidra_field_command_overview_bridge_test
    PRIVATE ${SHADOWRUN_GHIDRA_QT_RUNTIME_INCLUDE_DIRS})
target_link_libraries(shadowrun_ghidra_field_command_overview_bridge_test
    PRIVATE shadowrun_ghidra_conversion_runtime)
add_test(NAME shadowrun_ghidra_field_command_overview_bridge
         COMMAND shadowrun_ghidra_field_command_overview_bridge_test)

add_executable(shadowrun_ghidra_field_menu_text_catalog_test
    tests/field/field_menu_text_catalog_test.cpp
    src/field/field_menu_text_catalog.cpp
)
target_include_directories(shadowrun_ghidra_field_menu_text_catalog_test
    PRIVATE ${SHADOWRUN_GHIDRA_QT_RUNTIME_INCLUDE_DIRS})
target_compile_definitions(shadowrun_ghidra_field_menu_text_catalog_test PRIVATE
    SHADOWRUN_GHIDRA_ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
    SHADOWRUN_GAMEPLAY_SCRIPT_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../gameplay/scripts")
target_link_libraries(shadowrun_ghidra_field_menu_text_catalog_test
    PRIVATE shadowrun_gameplay_script_runtime
            shadowrun_ghidra_conversion_runtime)
add_test(NAME shadowrun_ghidra_field_menu_text_catalog
         COMMAND shadowrun_ghidra_field_menu_text_catalog_test)

add_executable(shadowrun_ghidra_dialogue_panel_test
    tests/dialogue/dialogue_panel_test.cpp
    src/dialogue/dialogue_panel.cpp
    src/dialogue/dialogue_panel_presenter.cpp
)
set_target_properties(shadowrun_ghidra_dialogue_panel_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
target_include_directories(shadowrun_ghidra_dialogue_panel_test
    PRIVATE ${SHADOWRUN_GHIDRA_QT_RUNTIME_INCLUDE_DIRS})
target_compile_definitions(shadowrun_ghidra_dialogue_panel_test PRIVATE
    SHADOWRUN_GHIDRA_ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
    SHADOWRUN_GAMEPLAY_SCRIPT_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../gameplay/scripts")
target_link_libraries(shadowrun_ghidra_dialogue_panel_test
    PRIVATE shadowrun_gameplay_script_runtime
            shadowrun_ghidra_conversion_runtime
            Qt6::Gui)
add_test(NAME shadowrun_ghidra_dialogue_panel
         COMMAND shadowrun_ghidra_dialogue_panel_test)

add_executable(shadowrun_ghidra_field_dialogue_runtime_test
    tests/dialogue/field_dialogue_runtime_test.cpp
    src/dialogue/field_dialogue_runtime.cpp
    src/dialogue/dialogue_panel_presenter.cpp
    src/dialogue/dialogue_panel.cpp
    src/rendering/genesis_image_compositor.cpp
)
target_include_directories(shadowrun_ghidra_field_dialogue_runtime_test
    PRIVATE ${SHADOWRUN_GHIDRA_QT_RUNTIME_INCLUDE_DIRS})
set_target_properties(shadowrun_ghidra_field_dialogue_runtime_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
target_compile_definitions(shadowrun_ghidra_field_dialogue_runtime_test PRIVATE
    SHADOWRUN_QT_QA_DRIVER=1
    SHADOWRUN_GHIDRA_ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
    SHADOWRUN_GAMEPLAY_SCRIPT_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../gameplay/scripts")
target_link_libraries(shadowrun_ghidra_field_dialogue_runtime_test
    PRIVATE shadowrun_gameplay_script_runtime
            shadowrun_ghidra_conversion_runtime Qt6::Gui)
add_test(NAME shadowrun_ghidra_field_dialogue_runtime
         COMMAND shadowrun_ghidra_field_dialogue_runtime_test)

add_executable(shadowrun_vid_phone_visual_test
    tests/dialogue/vid_phone_visual_test.cpp
    src/dialogue/field_dialogue_runtime.cpp
    src/dialogue/dialogue_panel_presenter.cpp
    src/dialogue/dialogue_panel.cpp
    src/rendering/genesis_image_compositor.cpp)
target_include_directories(shadowrun_vid_phone_visual_test
    PRIVATE ${SHADOWRUN_GHIDRA_QT_RUNTIME_INCLUDE_DIRS})
set_target_properties(shadowrun_vid_phone_visual_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
target_compile_definitions(shadowrun_vid_phone_visual_test PRIVATE
    SHADOWRUN_GHIDRA_ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
    SHADOWRUN_GAMEPLAY_SCRIPT_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../gameplay/scripts")
target_link_libraries(shadowrun_vid_phone_visual_test
    PRIVATE shadowrun_gameplay_script_runtime
            shadowrun_ghidra_conversion_runtime Qt6::Gui)
add_test(NAME shadowrun_vid_phone_visual_qa
         COMMAND shadowrun_vid_phone_visual_test
                 "${CMAKE_CURRENT_BINARY_DIR}/vid-phone-visual-qa")

add_executable(shadowrun_ghidra_field_mode_text_runtime_test
    tests/field/field_mode_text_runtime_test.cpp
    src/field/field_mode_text_runtime.cpp
    src/dialogue/dialogue_panel_presenter.cpp
    src/dialogue/dialogue_panel.cpp
)
target_include_directories(shadowrun_ghidra_field_mode_text_runtime_test
    PRIVATE ${SHADOWRUN_GHIDRA_QT_RUNTIME_INCLUDE_DIRS})
set_target_properties(shadowrun_ghidra_field_mode_text_runtime_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
target_compile_definitions(shadowrun_ghidra_field_mode_text_runtime_test PRIVATE
    SHADOWRUN_GHIDRA_ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
    SHADOWRUN_GAMEPLAY_SCRIPT_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../gameplay/scripts")
target_link_libraries(shadowrun_ghidra_field_mode_text_runtime_test
    PRIVATE shadowrun_ghidra_conversion_runtime
            shadowrun_gameplay_script_runtime Qt6::Gui)
add_test(NAME shadowrun_ghidra_field_mode_text_runtime
         COMMAND shadowrun_ghidra_field_mode_text_runtime_test)

add_executable(shadowrun_ghidra_field_interaction_runtime_test
    tests/field/field_interaction_runtime_test.cpp
    src/field/field_interaction_runtime.cpp
    src/dialogue/dialogue_panel_presenter.cpp
    src/dialogue/dialogue_panel.cpp
)
target_include_directories(shadowrun_ghidra_field_interaction_runtime_test
    PRIVATE ${SHADOWRUN_GHIDRA_QT_RUNTIME_INCLUDE_DIRS})
set_target_properties(shadowrun_ghidra_field_interaction_runtime_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
target_compile_definitions(shadowrun_ghidra_field_interaction_runtime_test PRIVATE
    SHADOWRUN_QT_QA_DRIVER=1
    SHADOWRUN_GHIDRA_ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
    SHADOWRUN_GAMEPLAY_SCRIPT_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../gameplay/scripts")
target_link_libraries(shadowrun_ghidra_field_interaction_runtime_test
    PRIVATE shadowrun_gameplay_script_runtime
            shadowrun_ghidra_conversion_runtime Qt6::Gui)
add_test(NAME shadowrun_ghidra_field_interaction_runtime
         COMMAND shadowrun_ghidra_field_interaction_runtime_test)

add_executable(shadowrun_ghidra_field_elevator_selector_runtime_test
    tests/field/elevator_selector_runtime_test.cpp
    src/field/elevator_selector_runtime.cpp
)
target_include_directories(shadowrun_ghidra_field_elevator_selector_runtime_test
    PRIVATE ${SHADOWRUN_GHIDRA_QT_RUNTIME_INCLUDE_DIRS})
set_target_properties(shadowrun_ghidra_field_elevator_selector_runtime_test
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
add_test(NAME shadowrun_ghidra_field_elevator_selector_runtime
         COMMAND shadowrun_ghidra_field_elevator_selector_runtime_test)

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
        foreach(SHADOWRUN_GHIDRA_QT_DEPLOY_TARGET IN ITEMS
                shadowrun_ghidra_conversion_qt
                shadowrun_ghidra_conversion_qt_qa
                shadowrun_frame_debug_server_test)
            add_custom_command(TARGET ${SHADOWRUN_GHIDRA_QT_DEPLOY_TARGET} POST_BUILD
                COMMAND "${SHADOWRUN_GHIDRA_QT_WINDEPLOYQT}"
                        ${SHADOWRUN_GHIDRA_QT_DEPLOY_CONFIGURATION}
                        --no-translations --no-compiler-runtime
                        "$<TARGET_FILE:${SHADOWRUN_GHIDRA_QT_DEPLOY_TARGET}>"
                COMMENT "Deploying Qt 6 ${CMAKE_BUILD_TYPE} runtime beside ${SHADOWRUN_GHIDRA_QT_DEPLOY_TARGET}"
                VERBATIM)
            set(SHADOWRUN_GHIDRA_QT_OFFSCREEN_PLUGIN
                "${CMAKE_PREFIX_PATH}/plugins/platforms/qoffscreen.dll")
            if (EXISTS "${SHADOWRUN_GHIDRA_QT_OFFSCREEN_PLUGIN}")
                add_custom_command(TARGET ${SHADOWRUN_GHIDRA_QT_DEPLOY_TARGET} POST_BUILD
                    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                            "${SHADOWRUN_GHIDRA_QT_OFFSCREEN_PLUGIN}"
                            "$<TARGET_FILE_DIR:${SHADOWRUN_GHIDRA_QT_DEPLOY_TARGET}>/platforms/qoffscreen.dll"
                    COMMENT "Deploying Qt offscreen platform for headless replay"
                    VERBATIM)
            endif()
        endforeach()
    endif()

    get_filename_component(SHADOWRUN_GHIDRA_QT_COMPILER_BIN
        "${CMAKE_CXX_COMPILER}" DIRECTORY)
    foreach(SHADOWRUN_GHIDRA_QT_RUNTIME_DLL
            libstdc++-6.dll libgcc_s_seh-1.dll libwinpthread-1.dll)
        foreach(SHADOWRUN_GHIDRA_QT_DEPLOY_TARGET IN ITEMS
                shadowrun_ghidra_conversion_qt
                shadowrun_ghidra_conversion_qt_qa
                shadowrun_frame_debug_server_test)
            add_custom_command(TARGET ${SHADOWRUN_GHIDRA_QT_DEPLOY_TARGET} POST_BUILD
                COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                        "${SHADOWRUN_GHIDRA_QT_COMPILER_BIN}/${SHADOWRUN_GHIDRA_QT_RUNTIME_DLL}"
                        "$<TARGET_FILE_DIR:${SHADOWRUN_GHIDRA_QT_DEPLOY_TARGET}>/${SHADOWRUN_GHIDRA_QT_RUNTIME_DLL}"
                COMMENT "Deploying matching MSYS2 runtime ${SHADOWRUN_GHIDRA_QT_RUNTIME_DLL}"
                VERBATIM)
        endforeach()
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
    tests/menus/shop_menu_runtime_test.cpp
    src/menus/shop_menu_runtime.cpp
)
target_include_directories(shadowrun_ghidra_shop_menu_runtime_test
    PRIVATE ${SHADOWRUN_GHIDRA_QT_RUNTIME_INCLUDE_DIRS})
target_link_libraries(shadowrun_ghidra_shop_menu_runtime_test
    PRIVATE shadowrun_ghidra_conversion_runtime)
add_test(NAME shadowrun_ghidra_shop_menu_runtime
         COMMAND shadowrun_ghidra_shop_menu_runtime_test)

# Keep the native ammo/spawn contracts in the Qt CTest suite too. The field
# action dump proves the live A-button wiring; these tests guard the typed
# source tables and ammo/paired-slot ordering independently of the presenter.
foreach(SHADOWRUN_GHIDRA_ENTITY_TEST IN ITEMS
        entity_combat entity_transition entity_animation_spawn_probe
        entity_proximity_collision movement_metric)
    add_executable(shadowrun_ghidra_${SHADOWRUN_GHIDRA_ENTITY_TEST}_test
        ../tests/entities/${SHADOWRUN_GHIDRA_ENTITY_TEST}_conversion_test.cpp
    )
    target_link_libraries(
        shadowrun_ghidra_${SHADOWRUN_GHIDRA_ENTITY_TEST}_test
        PRIVATE shadowrun_ghidra_conversion_modules)
    add_test(NAME shadowrun_ghidra_${SHADOWRUN_GHIDRA_ENTITY_TEST}
             COMMAND shadowrun_ghidra_${SHADOWRUN_GHIDRA_ENTITY_TEST}_test)
endforeach()

add_executable(shadowrun_ghidra_movement_mode_transition_test
    ../tests/entities/movement_mode_transition_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_movement_mode_transition_test
    PRIVATE shadowrun_ghidra_conversion_modules)
add_test(NAME shadowrun_ghidra_movement_mode_transition
         COMMAND shadowrun_ghidra_movement_mode_transition_test)

# Verify source field, menu, combat, and spell events reach their extracted
# assets, the FLAC decoder loads them, and mapped events start on the host
# output. Unknown, silent, or lifecycle-unresolved commands stay unmapped.
add_executable(shadowrun_ghidra_audio_playback_test
    tests/audio/audio_playback_test.cpp
    src/audio/audio_playback.cpp
    src/audio/qt_audio_sink.cpp
    ${SHADOWRUN_GHIDRA_AUDIO_CORE_SOURCES}
)
target_include_directories(shadowrun_ghidra_audio_playback_test
    PRIVATE ${SHADOWRUN_GHIDRA_QT_RUNTIME_INCLUDE_DIRS})
set_target_properties(shadowrun_ghidra_audio_playback_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
target_compile_definitions(shadowrun_ghidra_audio_playback_test PRIVATE
    SHADOWRUN_GHIDRA_AUDIO_MENU_RESOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/menu/menu-024.flac"
    SHADOWRUN_GHIDRA_AUDIO_FIELD_EFFECT_016_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/field/effect-016.flac"
    SHADOWRUN_GHIDRA_AUDIO_RESOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/sound-effects/resource-037.flac"
    SHADOWRUN_GHIDRA_AUDIO_SPELL_EVENT_32_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/sound-effects/resource-050.flac"
    SHADOWRUN_GHIDRA_AUDIO_SPELL_EVENT_34_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/sound-effects/resource-052.flac"
    SHADOWRUN_GHIDRA_AUDIO_SPELL_EVENT_35_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/sound-effects/resource-053.flac"
    SHADOWRUN_GHIDRA_AUDIO_SPELL_EVENT_37_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/sound-effects/resource-055.flac"
    SHADOWRUN_GHIDRA_AUDIO_SPELL_EVENT_3A_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/sound-effects/resource-058.flac"
    SHADOWRUN_GHIDRA_AUDIO_PASSKEY_RESOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/sound-effects/resource-075.flac"
    SHADOWRUN_GHIDRA_AUDIO_ACTION_RESOURCE_076_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/sound-effects/resource-076.flac"
    SHADOWRUN_GHIDRA_AUDIO_TARGET_RESOURCE_077_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/sound-effects/resource-077.flac"
    SHADOWRUN_GHIDRA_AUDIO_TARGET_RESOURCE_078_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/sound-effects/resource-078.flac"
    SHADOWRUN_GHIDRA_AUDIO_SPAWN_RESOURCE_028_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/sound-effects/resource-028.flac"
    SHADOWRUN_GHIDRA_AUDIO_SPAWN_RESOURCE_029_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/sound-effects/resource-029.flac"
    SHADOWRUN_GHIDRA_AUDIO_SPAWN_RESOURCE_030_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/sound-effects/resource-030.flac"
    SHADOWRUN_GHIDRA_AUDIO_SPAWN_RESOURCE_063_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/sound-effects/resource-063.flac"
    SHADOWRUN_GHIDRA_AUDIO_SPAWN_RESOURCE_074_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/sound-effects/resource-074.flac"
    SHADOWRUN_GHIDRA_AUDIO_SPAWN_RESOURCE_081_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/sound-effects/resource-081.flac"
    SHADOWRUN_GHIDRA_AUDIO_BOOT_SHOCK_SEQUENCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/intro/sega-logo-shock-sequence.flac"
    SHADOWRUN_GHIDRA_AUDIO_TITLE_LOGO_SEQUENCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/intro/shadowrun-logo-reveal-sequence.flac"
    SHADOWRUN_GHIDRA_AUDIO_TITLE_THEME_RESOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/music/resource-000.flac"
    SHADOWRUN_GHIDRA_AUDIO_INTRO_STAGE_1_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/intro/title-stage-1.flac"
    SHADOWRUN_GHIDRA_AUDIO_INTRO_STAGE_3_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/intro/title-stage-3.flac")
target_compile_definitions(shadowrun_ghidra_audio_playback_test PRIVATE
    SHADOWRUN_GHIDRA_ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../assets")
target_link_libraries(shadowrun_ghidra_audio_playback_test
    PRIVATE shadowrun_ghidra_conversion_runtime Qt6::Core Qt6::Multimedia)
# Runs headless through the null and WAV sinks; no audio device needed.
add_test(NAME shadowrun_ghidra_audio_playback
         COMMAND shadowrun_ghidra_audio_playback_test)

# Mixer / GEMS-layer unit tests: determinism, 735-sample frames, loop points,
# release ramp, voice stealing, 0x53126 fade timeline, gates.
add_executable(shadowrun_ghidra_audio_mixer_test
    tests/audio/audio_mixer_test.cpp
    ${SHADOWRUN_GHIDRA_AUDIO_CORE_SOURCES})
target_include_directories(shadowrun_ghidra_audio_mixer_test
    PRIVATE ${SHADOWRUN_GHIDRA_QT_RUNTIME_INCLUDE_DIRS})
set_target_properties(shadowrun_ghidra_audio_mixer_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
target_compile_definitions(shadowrun_ghidra_audio_mixer_test PRIVATE
    SHADOWRUN_GHIDRA_ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../assets")
add_test(NAME shadowrun_ghidra_audio_mixer
         COMMAND shadowrun_ghidra_audio_mixer_test)

# QA-seed FFFC34 bit 13, which the source E7 dialogue action normally writes,
# then enter scene-2/event-7 with ordinary controller movement. This verifies
# the downstream service; it does not exercise the still-unwired E7 dialogue
# route. Capture each modal page, idle hold, one-shot award and clean return.
add_shadowrun_qt_qa_case(shadowrun_ghidra_conversion_qt_field_redmond_vigore_search_qa
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-redmond-vigore-search-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-redmond-vigore-search-qa")
