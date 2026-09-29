#pragma once

#include "shadowrun_ghidra/core/word_transfer_conversion.hpp"
#include "shadowrun_ghidra/menus/hud_selection_lookup_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct MenuSlotCommandState {
    std::uint8_t selected_slot = 0;
    std::array<std::uint8_t, 5> slot_codes{};
    std::uint8_t mode_flags = 0;
    std::uint8_t prompt_state = 0;
    std::uint8_t slot_timer = 0;
};

struct MenuSlotCommandCallbacks {
    std::function<void()> write_repeated_grid;
    std::function<void()> write_grid;
    std::function<void()> render_rows;
};

// ROM 0x1962A: apply a selected slot command's flag cleanup and refresh all
// three menu/HUD presentation boundaries.
void apply_menu_slot_command(MenuSlotCommandState& state,
                             const MenuSlotCommandCallbacks& callbacks);

// ROM 0x19736: clear the selected slot timer once its source input gate is
// ready, then submit one grid update.
bool clear_ready_menu_slot_timer(MenuSlotCommandState& state,
                                 std::int8_t input_timer,
                                 const std::function<void()>& write_grid);

void clear_menu_status_at(const std::function<void()>& clear_status,
                          const std::function<void()>& write_grid);

constexpr std::array<std::array<std::uint16_t, 3>, 7> kMenuStatusGridRows{{
    {0x00B4U, 0x00B4U, 0x00B4U},
    {0x00B5U, 0x00B4U, 0x00B4U},
    {0x00B2U, 0x00B4U, 0x00B4U},
    {0x00B2U, 0x00B5U, 0x00B4U},
    {0x00B2U, 0x00B2U, 0x00B4U},
    {0x00B2U, 0x00B2U, 0x00B5U},
    {0x00B2U, 0x00B2U, 0x00B2U},
}};
constexpr std::array<std::uint16_t, 3> kMenuStatusGridRow = kMenuStatusGridRows[0];
constexpr std::uint16_t kMenuStatusGridBase = 0xED14U;

// ROM 0x19762: clr A4[D4], 3x1 grid at ED14+D4*0xC from 0xD84D6, D4=-1.
std::int16_t clear_menu_status_at(
    std::uint16_t index,
    std::vector<std::uint8_t>& status,
    const WordTransferCallbacks& callbacks);

void publish_menu_selection_then_text(
    const std::function<void()>& set_selection,
    const std::function<void()>& render_text);

// ROM 0x19B34: D1=7 into 0x17C84 (caller D0 is the slot code), then 0x17C52.
void publish_menu_selection_then_text(
    HudSelectionState& state,
    std::uint8_t code,
    const std::function<void()>& render_text);
void refresh_mode_metadata_leaf(const std::function<void()>& refresh);
void run_menu_service_then_prepare(
    const std::function<void()>& menu_service,
    const std::function<void()>& prepare_sequence);

struct MenuSlotFinalizeCallbacks {
    std::function<void()> clear_visual;
    std::function<void()> refresh_number_or_grid;
    std::function<void()> clear_status;
};

// ROM 0x19F72 / 0x19F78: locate the last slot carrying the requested code,
// clear its visual, publish its ten-times value, and refresh the slot.
bool finalize_menu_slot_code(MenuSlotCommandState& state,
                             std::uint8_t requested_code,
                             std::int16_t value,
                             const MenuSlotFinalizeCallbacks& callbacks);

struct MenuSelectionCodeFourState {
    // FFF0D3 / FFFBCA: signed selection delta and bounded footer offset.
    std::int8_t selection_delta = 0;
    std::int8_t footer_offset = 0;
    std::array<std::uint8_t, 5> slot_codes{};
    std::array<std::uint8_t, 5> pending_values{};
    std::array<std::int16_t, 5> offsets{};
};

struct MenuSelectionCodeFourCallbacks {
    // 0x19BAE and 0x19B34/0x17C52 remain typed call boundaries.
    std::function<std::uint8_t()> select_hud_mode;
    std::function<void()> publish_default_selection_and_text;
    std::function<void()> publish_fixed_selection_and_text;
    std::function<void()> write_footer_grid;
    std::function<void()> render_footer_rows;
    std::function<void(std::size_t)> clear_visual;
    std::function<void(std::size_t)> refresh_number_or_grid;
    std::function<void(std::size_t)> clear_status;
};

// ROM 0x19CB2: service the signed menu selection, then enter the shared
// reverse scan used to clear slot code 4. `refresh_value` is the D1 result
// published by the existing selection-index boundary before it is scaled by
// ten for the slot offset.
bool run_menu_selection_code_four(
    MenuSelectionCodeFourState& state,
    bool source_gate_is_nonzero,
    std::int16_t refresh_value,
    const MenuSelectionCodeFourCallbacks& callbacks);

struct MenuCountdownState {
    std::int8_t countdown = 0;
};

// ROM 0x1A12E: decrement the nonzero menu countdown and call the expiry
// boundary when the counter reaches zero.
bool advance_menu_countdown(MenuCountdownState& state,
                            const std::function<void()>& on_expiry);

void clear_selected_menu_record(std::vector<std::uint8_t>& records,
                                std::size_t selected_index);

struct EffectDefaultsState {
    bool active = false;
    std::uint16_t origin_x = 0;
    std::uint16_t origin_y = 0;
    std::int16_t horizontal_step = 0;
    std::int16_t vertical_step = 0;
    std::int16_t lifetime = 0;
    std::int16_t palette = 0;
};

// ROM 0x1A54E: publish the default world-effect placement and timing values.
void initialize_effect_defaults(EffectDefaultsState& state);

struct EffectEntityResetState {
    bool active = false;
    std::uint16_t accumulator = 0;
    std::uint16_t entity_value = 0;
};

// ROM 0x1A8F4: reset the effect accumulator and the entity's auxiliary word.
void reset_effect_entity(EffectEntityResetState& state);

bool arm_effect_after_threshold(EffectDefaultsState& state,
                                std::uint16_t source_value);

struct WordStreamResetState {
    std::vector<std::uint16_t> words;
    std::uint8_t dirty_flags = 0;
};

// ROM 0x1B5A6: clear the 0x1C0-word source work area, then submit its stream
// and mark both source upload classes dirty.
void reset_word_stream_area(WordStreamResetState& state,
                            const std::function<void()>& write_word_stream);

struct BehaviorRecordState {
    std::uint8_t width = 0;
    std::uint8_t height = 0;
    std::uint8_t origin_x = 0;
    std::uint8_t origin_y = 0;
    std::uint32_t update_descriptor = 0;
    std::uint32_t draw_descriptor = 0;
};

struct BehaviorRecordCallbacks {
    std::function<void()> service_audio;
    std::function<void()> initialize_record;
};

// ROM 0x1B7D0: initialize the common four-byte behavior dimensions.
void initialize_behavior_record(BehaviorRecordState& state);

// ROM 0x1B5B8 / 0x1B612 / 0x1B628 / 0x1B654 / 0x1B680: initialize a behavior
// record and attach one of the source-derived behavior descriptor pairs.
void attach_behavior_descriptor_a(BehaviorRecordState& state,
                                  const BehaviorRecordCallbacks& callbacks);
void attach_behavior_descriptor_b(BehaviorRecordState& state,
                                  const BehaviorRecordCallbacks& callbacks);
void attach_behavior_descriptor_c(BehaviorRecordState& state,
                                  const BehaviorRecordCallbacks& callbacks);
void attach_behavior_descriptor_d(BehaviorRecordState& state,
                                  const BehaviorRecordCallbacks& callbacks);
void attach_behavior_descriptor_e(BehaviorRecordState& state,
                                  const BehaviorRecordCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
