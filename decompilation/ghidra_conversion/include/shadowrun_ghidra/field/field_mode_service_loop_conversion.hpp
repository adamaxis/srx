#pragma once

#include "shadowrun_ghidra/field/field_mode_navigation_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

// Controller bits consumed by the field-action selection service at ROM
// 0x56AC0.
enum class FieldActionSelectionServiceInput : std::uint16_t {
    action_cursor_decrement = 0x0004U,
    action_panel_decrement = 0x0001U,
    action_cursor_increment = 0x0008U,
    action_panel_increment = 0x0002U,
    restart_setup = 0x0020U,
    confirm = 0x0040U,
    alternate_confirm = 0x0010U,
    return_to_field = 0x0080U,
};

// The source helper at 0x566EC copies exactly eight longwords (sixteen CRAM
// colors) into palette bank 2, the colors 32..47 portion of the full palette
// upload buffer, and publishes the display dirty flag.  The caller chooses
// the source bank from the active runner/entity record's +0x2C
// position/state-derived selector.  The host representation is an ordinary
// typed palette bank instead of an A0/DAT address.
void copy_field_mode_palette_bank(
    const std::array<std::uint32_t, 8>& source,
    std::array<std::uint32_t, 8>& destination,
    std::uint8_t& display_dirty_flags);

struct FieldModeServiceState {
    FieldActionSelectionNavigationState action_selection{};
    std::int8_t service_result = 0; // FFF112
    std::int8_t selection_marker = -1; // low byte of FFF0C2
    bool has_deck = false; // source condition is FF018F bit 0 clear
    std::array<std::uint32_t, 8> palette_bank_words{}; // CRAM colors 32..47
    bool returned_to_field = false;
};

struct FieldModeServiceCallbacks {
    std::function<void(std::uint16_t)> set_display_control;
    std::function<void()> transfer_menu_asset_batch;
    std::function<void()> upload_resource_pair;
    std::function<void()> refresh_mode_palette;
    std::function<void()> write_wrapped_resource_block;
    std::function<void()> refresh_mode_resource_metadata;
    std::function<void()> refresh_mode_resource_state;
    std::function<void()> decode_and_upload_resource;
    std::function<void()> load_font;
    std::function<void()> render_encoded_text;
    std::function<void()> select_numeric_menu_table;
    std::function<void()> reset_upload_state;
    std::function<std::array<std::uint32_t, 8>()> read_mode_palette_bank;
    std::function<void()> draw_menu_panel;
    std::function<void()> prepare_numeric_menu_block;
    std::function<void()> wait_for_controller_event;

    // The source restores the field presentation in this order before
    // returning from the service screen.
    std::function<void()> upload_field_asset_sequence;
    std::function<void()> refresh_field_animation_assets;
    std::function<void()> upload_field_asset_setup;

    // 0x587DA is the converted selection-presentation module. It receives the
    // typed state after the source has saved the current primary selection;
    // the callback keeps the source record table and host renderer separate.
    std::function<void(FieldModeServiceState&)>
        render_field_action_selection_presentation;

    FieldActionSelectionNavigationCallbacks action_selection_callbacks;

    std::function<void()> wait_for_vblank;
    std::function<void()> latch_controller_events;
    std::function<std::uint16_t()> sample_controller_input;
    std::function<void()> service_audio;

    // 0xD4C8 restarts the field-mode setup path.
    std::function<void()> restart_graphics_service;

    // 0x56DC4's top-level result/selection routing is converted by
    // field_action_selection_dispatch_conversion. The selected leaf still
    // owns its own typed state and remains a callback boundary here.
    std::function<void(FieldModeServiceState&)> dispatch_selected_field_action;

    // 0x58D32's post-build call to 0x57C7A. This is distinct from the
    // 0x587DA presentation callback above.
    std::function<void(FieldModeServiceState&)>
        render_field_action_selection;
};

enum class FieldModeServiceLoopAction {
    returned_to_field,
};

// ROM 0x56AC0: initialize the field-action menu presentation, run its action
// panel/selection loop, and restore the field presentation on cancel/return.
// This is a conversion/reference routine only; the native game runtime does
// not depend on Genesis VDP state or ROM addresses.
FieldModeServiceLoopAction run_field_mode_service(
    FieldModeServiceState& state,
    const FieldModeServiceCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
