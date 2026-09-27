#pragma once

#include "shadowrun_ghidra/session/runner_record_access_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

enum class RunnerAssetOperationResult {
    no_change,
    ammo_changed,
    inventory_phase_changed,
    equipment_cleared,
    invalid_equipped_slot,
};

// ROM 0x0D82A reads these values from the active live record while building
// the Attributes/Skills view. Drawing text remains a separate concern.
struct AttributeSkillValues {
    std::uint8_t race_index = 0;
    std::uint8_t species_index = 0;
    std::uint8_t archetype_index = 0;
    std::array<std::uint8_t, kRunnerRecordAttributeCount> attributes{};
    std::array<std::uint8_t, kRunnerRecordSkillCount> skills{};
};

AttributeSkillValues read_attributes_and_skills(
    const RunnerRecordFields& record);

// ROM 0x0D5DC and the inline entry at 0x11950 read the cyberware bitfield.
// Keep all sixteen source bits available to callers; screens choose groups.
struct CyberwareValues {
    std::uint16_t flags = 0;
    std::array<bool, 16U> installed{};
};

CyberwareValues read_cyberware_detail(const RunnerRecordFields& record);

// ROM 0x036FC. `delta` is the signed source meter delta and `ammo_limit` is
// the selected item row's extracted maximum-round value.
RunnerAssetOperationResult advance_entity_asset_meter(
    RunnerRecordFields& record, std::int8_t delta,
    std::uint8_t ammo_limit);

// ROM 0x038AC. Record types below three delegate to 0x036FC; other record
// types consume one round directly.
RunnerAssetOperationResult service_entity_ammo(
    RunnerRecordFields& record, std::int8_t meter_delta,
    std::uint8_t ammo_limit);

// ROM 0x107CE's record mutation, without its VDP/menu side effects.
std::uint8_t adjust_runner_posture(RunnerRecordFields& record,
                                   std::int8_t delta);

struct RunnerPostureRefreshCallbacks {
    std::function<void()> write_grid;
    std::function<void()> render_encoded_text;
    std::function<void()> write_three_grids_then_service;
    std::function<void()> refresh_equipment_action_metric;
    std::function<void()> refresh_equipment_combat_values;
};

// ROM 0x107CE: mutate posture and preserve the complete source refresh order,
// including the two consecutive combat-value refresh calls. These callbacks
// are presentation/service boundaries; posture itself remains typed state.
std::uint8_t adjust_runner_posture_and_refresh(
    RunnerRecordFields& record, std::int8_t delta,
    const RunnerPostureRefreshCallbacks& callbacks = {});

struct MenuDialogPostureCallbacks {
    std::function<void()> write_grid;
    std::function<void()> render_encoded_text;
};

// ROM 0x11AE4: adjust the five-state posture and emit only the dialog grid
// followed by its posture text.
std::uint8_t adjust_menu_dialog_posture(
    RunnerRecordFields& record, std::int8_t delta,
    const MenuDialogPostureCallbacks& callbacks = {});

enum class RunnerPostureLoopResult {
    waiting,
    adjusted,
    finished,
};

struct RunnerPostureLoopCallbacks {
    std::function<void()> wait_for_vblank;
    std::function<void()> latch_controller_events;
    std::function<std::uint16_t()> sample_controller_input;
    std::function<void()> advance_menu_sprite_overlay;
    std::function<void(std::int8_t)> adjust_and_refresh_posture;
};

// ROM 0x10722: the posture sub-loop gives up/down priority to posture
// mutation, while the source confirm/cancel masks finish the loop. Rendering
// and vblank service remain outside this state conversion.
RunnerPostureLoopResult apply_runner_posture_input(
    RunnerRecordFields& record, std::uint16_t input);

// ROM 0x10722: perform the initial neutral posture refresh, then cycle the
// source vblank/latch/sample/overlay sequence. Up/down repeat; confirm,
// cancel, and mode-action masks finish through one final neutral refresh.
RunnerPostureLoopResult run_runner_posture_adjustment(
    const RunnerPostureLoopCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
