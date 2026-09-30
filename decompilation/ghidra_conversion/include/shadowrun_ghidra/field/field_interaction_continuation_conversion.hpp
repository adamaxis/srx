#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

// These are the prompt descriptors selected by the source interaction
// continuations.  They are typed host identities for source records, not ROM
// pointers.  The text records themselves remain an extraction/data concern.
enum class FieldInteractionPrompt {
    electromagnetic_lock_door,
    electromagnetic_lock_resolution,
    building_security_terminal,
};

// The callback installed in A1 before 0x1F246 runs the common mode-exit
// services.  A native caller will bind these to presentation/resource work;
// it must not dispatch through a source address.
enum class FieldInteractionPreparation {
    none,
    write_electromagnetic_lock_choice_grid,
    prepare_electromagnetic_lock_resolution,
};

struct FieldInteractionContinuationState {
    // Source FFE17F: the interaction-suppression flag tested by 0x20E48.
    bool interaction_suppressed = false;

    // Source FFE16E: event selection written by 0x0B7B8 and consumed by
    // 0x20E92.  The source event values 2..5 select the four lock grids.
    std::uint8_t interaction_selection = 0;
};

struct FieldInteractionPromptPlan {
    FieldInteractionPrompt prompt =
        FieldInteractionPrompt::electromagnetic_lock_door;
    FieldInteractionPreparation preparation =
        FieldInteractionPreparation::none;

    // Source FFF10C.  The common 0x1F246 service invokes its A1 callback
    // only when this marker is set, then clears it before entering 0x1F3C4.
    bool display_prepared = false;

    // Source FFF0B5.  The Security Terminal continuation requests the
    // optional mode resource before running the shared prompt service.
    bool optional_resource_pending = false;

    // Valid only for write_electromagnetic_lock_choice_grid.  This is the
    // zero-based row in the four-entry source choice-grid table.
    std::uint8_t choice_grid_index = 0;
};

// ROM 0x20E48 and its in-line normal branch at 0x20E7A.  It marks the common
// prompt display as prepared, selects the electromagnetic-lock descriptor,
// and installs either the choice-grid callback or the alternate resolution
// callback depending on the source suppression flag.  The host receives the
// selection as typed state instead of an A0/A1/A5 register tuple.
FieldInteractionPromptPlan
prepare_field_interaction_prompt(
    FieldInteractionContinuationState& state);

// ROM 0x20E92.  The source subtracts two from FFE16E, rejects values below
// two into the Security Terminal continuation at 0x20EC2, and writes one of
// four event-specific choice grids for values 2..5.
FieldInteractionPromptPlan
prepare_electromagnetic_lock_choice_grid(std::uint8_t interaction_selection);

// ROM 0x20EC2.  This is the building Security Terminal path, not a generic
// building fallback: it selects the 0x11CD62 prompt record, clears FFF10C,
// and sets FFF0B5 so 0x1F246 loads the optional mode resource.
FieldInteractionPromptPlan prepare_building_security_terminal_prompt();

} // namespace shadowrun::ghidra_conversion
