find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia)

set(SHADOWRUN_GHIDRA_QT_RUNTIME_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/app"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/audio"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/dialogue"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/menus"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/rendering")

# This is the runnable Qt host for the converted source contracts. It is a
# child of ghidra_conversion, not a target in the native-port CMake project.
# It deliberately does not link shadowrun_port, shadowrun_native,
# shadowrun_qt, GenesisBus, or the native asset decoder.
add_executable(shadowrun_ghidra_conversion_qt
    src/app/ghidra_conversion_qt_main.cpp
    src/field/field_scene_runtime.cpp
    src/field/elevator_selector_runtime.cpp
    src/menus/attributes_skills_menu_runtime.cpp
    src/field/field_clips_menu_runtime.cpp
    src/menus/shop_menu_runtime.cpp
    src/field/field_party_roster_runtime.cpp
    src/field/field_menu_text_catalog.cpp
    src/field/field_command_overview_bridge.cpp
    src/rendering/genesis_image_compositor.cpp
    src/dialogue/dialogue_panel.cpp
    src/dialogue/dialogue_panel_presenter.cpp
    src/dialogue/dialogue_text_catalog.cpp
    src/dialogue/field_dialogue_runtime.cpp
    src/menus/hotel_menu_runtime.cpp
    src/field/field_mode_text_runtime.cpp
    src/field/field_interaction_runtime.cpp
    src/audio/audio_playback.cpp
)
target_include_directories(shadowrun_ghidra_conversion_qt
    PRIVATE ${SHADOWRUN_GHIDRA_QT_RUNTIME_INCLUDE_DIRS})
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
if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Qt's bundled WinMain shim is built against a different MinGW runtime
    # than this MSYS2 UCRT64 build. Select the GUI subsystem directly while
    # retaining the normal CRT main() startup entry point.
    target_link_options(shadowrun_ghidra_conversion_qt PRIVATE
        "-Wl,--subsystem,windows"
        "-Wl,-e,mainCRTStartup")
endif()

# One end-to-end startup/field QA walk. The executable writes its visual
# checkpoints outside the asset tree so this remains a verification artifact,
# not a runtime resource or another generated asset pile.
add_test(NAME shadowrun_ghidra_conversion_qt_startup_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --startup-qa "${CMAKE_CURRENT_BINARY_DIR}/startup-qa")
set_tests_properties(shadowrun_ghidra_conversion_qt_startup_qa PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=windows")

# Launch from the runtime build directory without --assets. The installed
# executable must locate the checked-in named assets from its own path rather
# than depending on the shell's working directory.
add_test(NAME shadowrun_ghidra_conversion_qt_asset_autodiscovery_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --startup-dump start_options 0 0
                 "${CMAKE_CURRENT_BINARY_DIR}/asset-autodiscovery-start-options.bmp")
set_tests_properties(
    shadowrun_ghidra_conversion_qt_asset_autodiscovery_qa PROPERTIES
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    ENVIRONMENT "QT_QPA_PLATFORM=windows")

# The six airport rotor descriptors advance every six source frames. Capture
# all six whole screens plus the 36-frame repeat and check the animated strip
# against the horizontally translated, otherwise stable aircraft body.
add_test(NAME shadowrun_ghidra_conversion_qt_opening_rotor_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --opening-rotor-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/opening-rotor-qa")
set_tests_properties(shadowrun_ghidra_conversion_qt_opening_rotor_qa PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=windows")

# The landing path uses the ROM-selected movement art until the computed Y
# enters its narrow special-image window. Save whole-screen frames at every
# landing event and across entry/exit from that window.
add_test(NAME shadowrun_ghidra_conversion_qt_opening_landing_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --opening-landing-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/opening-landing-qa")
set_tests_properties(shadowrun_ghidra_conversion_qt_opening_landing_qa PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=windows")

# During 0x59D7C the wilderness body layers pan with FFDD08/FFDD0A while the
# Indian's head SAT advances with DD46/DD48. Verify their relative placement
# and capture complete start/midpoint/settled screens from the Gator Shaman
# route.
add_test(NAME shadowrun_ghidra_conversion_qt_opening_wilderness_pan_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --opening-wilderness-pan-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/opening-wilderness-pan-qa")
set_tests_properties(
    shadowrun_ghidra_conversion_qt_opening_wilderness_pan_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

# The six 0x5A080 updates replace the left wilderness region. At 0x59F8E's
# terminal event, ROM fills that region with the blank tile before drawing the
# death pose. Verify the full-screen lifetime and the cockpit handoff.
add_test(NAME shadowrun_ghidra_conversion_qt_opening_death_layer_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --opening-death-layer-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/opening-death-layer-qa")
set_tests_properties(
    shadowrun_ghidra_conversion_qt_opening_death_layer_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

# The ROM marks the fixed trivid-left and timed SAT lists high priority
# (0xC000/0xE000) while the moving trivid-right list is low priority (0x6000).
# Whole-screen QA checks these layers across the full 0x115-frame scene loop.
add_test(NAME shadowrun_ghidra_conversion_qt_opening_trivid_layer_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --opening-trivid-layer-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/opening-trivid-layer-qa")
set_tests_properties(
    shadowrun_ghidra_conversion_qt_opening_trivid_layer_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

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
add_test(NAME shadowrun_ghidra_conversion_qt_boot_fasa_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --boot-fasa-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/boot-fasa-qa")
set_tests_properties(shadowrun_ghidra_conversion_qt_boot_fasa_qa PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=windows")

add_test(NAME shadowrun_ghidra_conversion_qt_field_action_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-action-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-action-qa")
set_tests_properties(shadowrun_ghidra_conversion_qt_field_action_qa PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=windows")

# Empty magazines consume one shared clip through the typed player action
# path, then update the named live HUD counter tile. The dump is a route
# checkpoint, not an independent Genesis visual reference.
add_test(NAME shadowrun_ghidra_conversion_qt_field_player_action_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-player-action-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-player-action-qa")
set_tests_properties(
    shadowrun_ghidra_conversion_qt_field_player_action_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

# Field input is held across multiple ROM-timed updates so direction slew,
# collision-accepted motion, walk-frame cadence, release-to-idle, and the
# replacement actor draw all have visible checkpoints in this scene.
add_test(NAME shadowrun_ghidra_conversion_qt_field_movement_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-movement-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-movement-qa")
set_tests_properties(shadowrun_ghidra_conversion_qt_field_movement_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

# Resolve a physical collision event into its native story/service/area route.
# Full-screen normal-input route dumps below and under the Stoker QA check the
# resulting dialogue and scene transition; this test protects route priority
# and ensures each map reads its own event table.
add_executable(shadowrun_ghidra_conversion_qt_field_script_route_test
    ../tests/field/field_script_route_conversion_test.cpp)
target_link_libraries(shadowrun_ghidra_conversion_qt_field_script_route_test
    PRIVATE shadowrun_ghidra_conversion_modules)
add_test(NAME shadowrun_ghidra_conversion_qt_field_script_route_qa
         COMMAND shadowrun_ghidra_conversion_qt_field_script_route_test)

# A projected interaction starts a source 0x10 countdown. 0x16396 fades on
# even counts and publishes FFDF3E only after the signed counter falls below
# zero; keep several full-frame captures so timer and screen clearing stay in
# sync.
add_test(NAME shadowrun_ghidra_conversion_qt_field_transition_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-transition-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-transition-qa")
set_tests_properties(shadowrun_ghidra_conversion_qt_field_transition_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

# The active field event byte selects a named resource scene, samples its ROM
# bound into a typed floor index, and carries that state through the existing
# Security Terminal result prompt. Captures also verify full-scene replacement
# and clearing the modal before returning to the field.
add_test(NAME shadowrun_ghidra_conversion_qt_field_world_activity_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-world-activity-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-world-activity-qa")
set_tests_properties(
    shadowrun_ghidra_conversion_qt_field_world_activity_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

# Normal up input from the scene-2 New Game spawn reaches the motel doorway's
# source C2 collision, event 2, and the shared Stoker dialogue. A/B input then
# follows the source manager and Leave records back to a freshly drawn field.
# Every checkpoint is a complete 320x224 screen dump for visual review.
add_test(NAME shadowrun_ghidra_conversion_qt_field_stoker_collision_to_dialogue_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-stoker-collision-to-dialogue-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-stoker-collision-to-dialogue-qa")
set_tests_properties(
    shadowrun_ghidra_conversion_qt_field_stoker_collision_to_dialogue_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

# Ordinary Right/Up input from the New Game spawn contacts scene-2 record 92
# (the physical Ares entrance), publishes event 11, waits through the source
# fade and generic prompt, enters the shop with A, then verifies a clean field
# redraw after Start. No mode/event tuple is injected.
add_test(NAME shadowrun_ghidra_conversion_qt_field_ares_collision_shop_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-ares-collision-shop-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-ares-collision-shop-qa")
set_tests_properties(
    shadowrun_ghidra_conversion_qt_field_ares_collision_shop_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

# Ordinary D-pad input contacts the paired scene-2 C0 doorway records,
# enters scene 0x14, walks to the scene-20 maglock, and opens its source-backed
# choice panel. C/Walk Away returns to a clean field-only redraw. The QA writes
# actual.bmp and clears its dedicated directory before every run.
add_test(NAME shadowrun_ghidra_conversion_qt_field_scene20_physical_entry_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-scene20-physical-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-scene20-physical-entry-qa")
set_tests_properties(
    shadowrun_ghidra_conversion_qt_field_scene20_physical_entry_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

# Normal Right then Up+Right input from New Game contacts scene-2 collision
# record 93 (C1/event 1). Its map event-table row 1 selects resource scene
# 0x1A; this captures the source fade and complete destination scene after the
# typed world-activity resource selection and scene re-entry.
add_test(NAME shadowrun_ghidra_conversion_qt_field_scene26_event1_entry_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-scene26-event1-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-scene26-event1-entry-qa")
set_tests_properties(
    shadowrun_ghidra_conversion_qt_field_scene26_event1_entry_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

# Continue the ordinary-input event-table route into scene 26, cross its ROM
# event-29 elevator trigger, animate the two independently tracked floor
# labels at the 17x8 cadence, then load the selected child scene through its
# extracted profile group. Captures cover the full selector, animation, and
# clean destination field with its HUD.
add_test(NAME shadowrun_ghidra_conversion_qt_field_elevator_physical_entry_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-elevator-physical-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-elevator-physical-entry-qa")
set_tests_properties(
    shadowrun_ghidra_conversion_qt_field_elevator_physical_entry_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

# Scene 25 is the non-member entry scene for the source child-scene group
# 21/22/23/24. Walk from its descriptor spawn into event 29, visit all four
# floor selections, then ride to scene 22 and inspect every captured frame.
add_test(NAME shadowrun_ghidra_conversion_qt_field_elevator_scene25_entry_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-elevator-scene25-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-elevator-scene25-entry-qa")
set_tests_properties(
    shadowrun_ghidra_conversion_qt_field_elevator_scene25_entry_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

# Scene 29 is the non-member entry scene for the source child-scene group
# 26/27/28. Walk from its descriptor spawn into its paired event-29 records,
# verify the group starts at floor index 0, then ride to scene 27 and inspect
# every frame through the clean scene replacement.
add_test(NAME shadowrun_ghidra_conversion_qt_field_elevator_scene29_entry_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-elevator-scene29-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-elevator-scene29-entry-qa")
set_tests_properties(
    shadowrun_ghidra_conversion_qt_field_elevator_scene29_entry_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

# Scene 34 is the non-member entry scene for the source child-scene group
# 30/31/32/33. Walk from its descriptor spawn into event 29, visit all four
# floor selections, then ride to scene 31 and inspect every captured frame.
add_test(NAME shadowrun_ghidra_conversion_qt_field_elevator_scene34_entry_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-elevator-scene34-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-elevator-scene34-entry-qa")
set_tests_properties(
    shadowrun_ghidra_conversion_qt_field_elevator_scene34_entry_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

# Scene 40 is the non-member entry scene for the source child-scene group
# 35/36/37/38/39. Walk north into event 29, visit every floor, then ride to 36.
add_test(NAME shadowrun_ghidra_conversion_qt_field_elevator_scene40_entry_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-elevator-scene40-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-elevator-scene40-entry-qa")
set_tests_properties(
    shadowrun_ghidra_conversion_qt_field_elevator_scene40_entry_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

# Follow the same ordinary-input route, select the visible Frag Grenade option
# with B, and exercise the no-item result and its clean A dismissal. The New
# Game party inventory is checked before the choice; this is not a synthetic
# prompt or injected world event.
add_test(NAME shadowrun_ghidra_conversion_qt_field_scene20_grenade_no_item_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-scene20-grenade-no-item-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-scene20-grenade-no-item-qa")
set_tests_properties(
    shadowrun_ghidra_conversion_qt_field_scene20_grenade_no_item_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

# Follow ordinary D-pad movement to scene-2 collision record 2. The source
# record is a type-2 collision object, not a Stoker identity or event code;
# this QA verifies its observed event-0/no-dialogue result and full-frame route.
add_test(NAME shadowrun_ghidra_conversion_qt_field_stoker_collision_candidate_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-stoker-collision-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-stoker-collision-qa")
set_tests_properties(
    shadowrun_ghidra_conversion_qt_field_stoker_collision_candidate_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

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

# Command-menu Attributes/Skills is the view-only Karma page. Its separate
# Hotel route exposes the upgrade preview and spending prompt; keep the
# captures and verification for those distinct screen variants separate.
add_test(NAME shadowrun_ghidra_conversion_qt_field_attributes_skills_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-attributes-skills-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-attributes-skills-view-only-qa")
set_tests_properties(
    shadowrun_ghidra_conversion_qt_field_attributes_skills_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

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

# The paid-room result must enter its full-screen Hotel status/options route,
# and checkout must redraw both its prompt and the returning field scene.
add_test(NAME shadowrun_ghidra_conversion_qt_field_hotel_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-hotel-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-hotel-qa")
set_tests_properties(shadowrun_ghidra_conversion_qt_field_hotel_qa PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=windows")

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

add_test(NAME shadowrun_ghidra_conversion_qt_field_dialogue_arrival_manager_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-dialogue-dump arrival-manager
                 "${CMAKE_CURRENT_BINARY_DIR}/field-dialogue-qa/arrival-manager/actual.bmp")
set_tests_properties(
    shadowrun_ghidra_conversion_qt_field_dialogue_arrival_manager_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

add_test(NAME shadowrun_ghidra_conversion_qt_field_dialogue_room_paid_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-dialogue-dump room-paid
                 "${CMAKE_CURRENT_BINARY_DIR}/field-dialogue-qa/room-paid/actual.bmp")
set_tests_properties(
    shadowrun_ghidra_conversion_qt_field_dialogue_room_paid_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

add_test(NAME shadowrun_ghidra_conversion_qt_field_dialogue_room_insufficient_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-dialogue-dump room-insufficient
                 "${CMAKE_CURRENT_BINARY_DIR}/field-dialogue-qa/room-insufficient/actual.bmp")
set_tests_properties(
    shadowrun_ghidra_conversion_qt_field_dialogue_room_insufficient_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

# Exercise the source 0x30/0x31 follow-up commands through the Qt dialogue
# controller and verify their mutations feed the dynamic Notebook row gates.
add_test(NAME shadowrun_ghidra_conversion_qt_field_dialogue_debt_followup_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-dialogue-dump debt-followup
                 "${CMAKE_CURRENT_BINARY_DIR}/field-dialogue-qa/debt-followup/actual.bmp")
set_tests_properties(
    shadowrun_ghidra_conversion_qt_field_dialogue_debt_followup_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

add_test(NAME shadowrun_ghidra_conversion_qt_field_dialogue_referral_followup_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-dialogue-dump referral-followup
                 "${CMAKE_CURRENT_BINARY_DIR}/field-dialogue-qa/referral-followup/actual.bmp")
set_tests_properties(
    shadowrun_ghidra_conversion_qt_field_dialogue_referral_followup_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

# Run command 0x05 from the debt question with the New Game wallet. The source
# 20-nuyen balance takes the insufficient-funds branch to the Jump House page.
add_test(NAME shadowrun_ghidra_conversion_qt_field_dialogue_debt_payment_insufficient_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-dialogue-dump debt-payment-insufficient
                 "${CMAKE_CURRENT_BINARY_DIR}/field-dialogue-qa/debt-payment-insufficient/actual.bmp")
set_tests_properties(
    shadowrun_ghidra_conversion_qt_field_dialogue_debt_payment_insufficient_qa
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

# Door and Security Terminal object prompts use the shared dialogue frame and
# the same source-timed A/B/C prompt controller as NPC and mode text.
add_test(NAME shadowrun_ghidra_conversion_qt_field_interaction_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-interaction-dump maglock-choices
                 "${CMAKE_CURRENT_BINARY_DIR}/field-interaction-qa/maglock-choices/actual.bmp")
set_tests_properties(shadowrun_ghidra_conversion_qt_field_interaction_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

add_test(NAME shadowrun_ghidra_conversion_qt_security_terminal_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-interaction-dump security-terminal
                 "${CMAKE_CURRENT_BINARY_DIR}/field-interaction-qa/security-terminal/actual.bmp")
set_tests_properties(shadowrun_ghidra_conversion_qt_security_terminal_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

# Scene 26 collision records 124-125 contain the signed event-0x18 Security
# Terminal region. Start at the ROM descriptor spawn, traverse scene-26
# corridors and A2/82 terrain interactions with ordinary d-pad input, enter
# the terminal, show both choices, and Walk Away.
add_test(NAME shadowrun_ghidra_conversion_qt_security_terminal_physical_entry_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-security-terminal-physical-entry-qa
                 "${CMAKE_CURRENT_BINARY_DIR}/field-security-terminal-physical-entry-qa")
set_tests_properties(
    shadowrun_ghidra_conversion_qt_security_terminal_physical_entry_qa
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

# ROM 0x119CE posture focus: Up/Down on every command row, Left/Right and
# clamping, 0x25 audio requests, static hold, exact redraw, and field return.
add_test(NAME shadowrun_ghidra_conversion_qt_field_posture_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-posture-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-posture-qa.bmp")
set_tests_properties(shadowrun_ghidra_conversion_qt_field_posture_qa
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")

# The page-5 A route has two source branches. Exercise the fallback with
# Samurai's unmodified source record (class 2, +0x7F = 0); the separate spell
# test uses Gator Shaman's source record. These are runtime regression
# artifacts, not independent Genesis goldens.
add_test(NAME shadowrun_ghidra_conversion_qt_field_magic_qa
         COMMAND shadowrun_ghidra_conversion_qt
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../../assets"
                 --field-archetype samurai
                 --field-magic-dump
                 "${CMAKE_CURRENT_BINARY_DIR}/field-magic-nonmagic-qa")
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
    SHADOWRUN_GHIDRA_ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../assets")
target_link_libraries(shadowrun_ghidra_field_menu_text_catalog_test
    PRIVATE shadowrun_ghidra_conversion_runtime)
add_test(NAME shadowrun_ghidra_field_menu_text_catalog
         COMMAND shadowrun_ghidra_field_menu_text_catalog_test)

add_executable(shadowrun_ghidra_dialogue_panel_test
    tests/dialogue/dialogue_panel_test.cpp
    src/dialogue/dialogue_panel.cpp
    src/dialogue/dialogue_text_catalog.cpp
)
target_include_directories(shadowrun_ghidra_dialogue_panel_test
    PRIVATE ${SHADOWRUN_GHIDRA_QT_RUNTIME_INCLUDE_DIRS})
target_compile_definitions(shadowrun_ghidra_dialogue_panel_test PRIVATE
    SHADOWRUN_GHIDRA_ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../assets")
target_link_libraries(shadowrun_ghidra_dialogue_panel_test
    PRIVATE shadowrun_ghidra_conversion_runtime)
add_test(NAME shadowrun_ghidra_dialogue_panel
         COMMAND shadowrun_ghidra_dialogue_panel_test)

add_executable(shadowrun_ghidra_field_dialogue_runtime_test
    tests/dialogue/field_dialogue_runtime_test.cpp
    src/dialogue/field_dialogue_runtime.cpp
    src/dialogue/dialogue_panel_presenter.cpp
    src/dialogue/dialogue_panel.cpp
    src/dialogue/dialogue_text_catalog.cpp
    src/rendering/genesis_image_compositor.cpp
)
target_include_directories(shadowrun_ghidra_field_dialogue_runtime_test
    PRIVATE ${SHADOWRUN_GHIDRA_QT_RUNTIME_INCLUDE_DIRS})
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
    SHADOWRUN_GHIDRA_ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../assets")
target_link_libraries(shadowrun_ghidra_field_mode_text_runtime_test
    PRIVATE shadowrun_ghidra_conversion_runtime Qt6::Gui)
add_test(NAME shadowrun_ghidra_field_mode_text_runtime
         COMMAND shadowrun_ghidra_field_mode_text_runtime_test)

add_executable(shadowrun_ghidra_field_interaction_runtime_test
    tests/field/field_interaction_runtime_test.cpp
    src/field/field_interaction_runtime.cpp
    src/dialogue/dialogue_panel_presenter.cpp
    src/dialogue/dialogue_panel.cpp
    src/dialogue/dialogue_text_catalog.cpp
)
target_include_directories(shadowrun_ghidra_field_interaction_runtime_test
    PRIVATE ${SHADOWRUN_GHIDRA_QT_RUNTIME_INCLUDE_DIRS})
set_target_properties(shadowrun_ghidra_field_interaction_runtime_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
target_compile_definitions(shadowrun_ghidra_field_interaction_runtime_test PRIVATE
    SHADOWRUN_GHIDRA_ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../assets")
target_link_libraries(shadowrun_ghidra_field_interaction_runtime_test
    PRIVATE shadowrun_ghidra_conversion_runtime Qt6::Gui)
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
    tests/menus/shop_menu_runtime_test.cpp
    src/menus/shop_menu_runtime.cpp
)
target_include_directories(shadowrun_ghidra_shop_menu_runtime_test
    PRIVATE ${SHADOWRUN_GHIDRA_QT_RUNTIME_INCLUDE_DIRS})
target_link_libraries(shadowrun_ghidra_shop_menu_runtime_test
    PRIVATE shadowrun_ghidra_conversion_runtime)
add_test(NAME shadowrun_ghidra_shop_menu_runtime
         COMMAND shadowrun_ghidra_shop_menu_runtime_test)

# Verify the source selector and maglock unlock events reach their extracted
# assets, the FLAC decoder loads the resource, and a mapped event starts on the
# host audio output. Unknown, silent, or lifecycle-unresolved commands stay
# unmapped.
add_executable(shadowrun_ghidra_audio_playback_test
    tests/audio/audio_playback_test.cpp
    src/audio/audio_playback.cpp
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
    SHADOWRUN_GHIDRA_AUDIO_PASSKEY_RESOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/sound-effects/resource-075.flac"
    SHADOWRUN_GHIDRA_AUDIO_SPAWN_RESOURCE_030_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/sound-effects/resource-030.flac"
    SHADOWRUN_GHIDRA_AUDIO_SPAWN_RESOURCE_074_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/sound-effects/resource-074.flac"
    SHADOWRUN_GHIDRA_AUDIO_BOOT_SHOCK_RESOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/sound-effects/resource-092.flac"
    SHADOWRUN_GHIDRA_AUDIO_INTRO_STAGE_1_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/intro/title-stage-1.flac"
    SHADOWRUN_GHIDRA_AUDIO_INTRO_STAGE_3_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../assets/audio/intro/title-stage-3.flac")
target_link_libraries(shadowrun_ghidra_audio_playback_test
    PRIVATE Qt6::Core Qt6::Multimedia)
add_test(NAME shadowrun_ghidra_audio_playback
         COMMAND shadowrun_ghidra_audio_playback_test)
set_tests_properties(shadowrun_ghidra_audio_playback PROPERTIES
    SKIP_RETURN_CODE 77)
