#pragma once

#include "shadowrun_ghidra/entities/entity_behavior_frame_service_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <vector>

namespace shadowrun::ghidra_conversion {

enum class FieldRunnerSlot {
    primary,
    secondary,
};

// The two live field runner records consumed by ROM 0xC802.  These are the
// source fields that control this routine, promoted to named gameplay state;
// the source record offsets remain provenance only.
struct FieldRunnerRefreshCandidate {
    bool active = false;                 // live record +0x10 bit 0
    std::uint8_t effect_gate = 0;        // live record +0x4C
    std::uint8_t animation_gate = 0;     // live record +0x9C
    EntityIdentity identity = 0;         // stable host identity for A0 match
};

struct FieldRunnerRefreshState {
    std::array<FieldRunnerRefreshCandidate, 2> runners{};

    // FFFC0C/FFFC0D are 1-based compact roster markers. FF is the source's
    // signed absent sentinel. The counter table is the typed host form of
    // FFFC12 onward and is indexed by marker - 1.
    std::array<std::uint8_t, 2> compact_record_markers{{0xFFU, 0xFFU}};
    std::vector<std::uint8_t> effect_counters;
};

struct FieldRunnerRefreshCallbacks {
    // Bind this to the already converted 0x59798 roster serialization path.
    std::function<void()> serialize_runner_record_population;

    // These callbacks are host operations corresponding to the source's
    // effect-slot clear, mode-action dispatch, field-asset transfer setup,
    // and three-pass animation service.
    std::function<void(FieldRunnerSlot)> clear_effect_slot_state;
    std::function<void(FieldRunnerSlot)> handle_mode_action_event;
    std::function<void()> upload_field_asset_setup;
    std::function<void()> run_entity_animation_triplet;
};

enum class FieldRunnerCounterUpdate {
    not_processed,
    updated,
    marker_unavailable,
};

struct FieldRunnerRefreshResult {
    std::array<bool, 2> processed{{false, false}};
    std::array<FieldRunnerCounterUpdate, 2> counter_updates{{
        FieldRunnerCounterUpdate::not_processed,
        FieldRunnerCounterUpdate::not_processed}};
};

// ROM 0xC802: serialize roster state, refresh eligible primary/secondary
// field runners, then upload field assets and run the animation triplet.
//
// The generated C incorrectly expresses each source gate with OR. The direct
// disassembly is AND: active && effect_gate == 0 && animation_gate == 0.
// Source A0 comparisons are stable EntityIdentity comparisons here; no ROM or
// host memory address is part of the native contract.
FieldRunnerRefreshResult refresh_field_runner_state_and_assets(
    FieldRunnerRefreshState& state,
    const EntityIdentity active_runner_identity,
    const FieldRunnerRefreshCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
