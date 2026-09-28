#include "shadowrun_ghidra/menus/magic_menu_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {
namespace {

void call_once(const std::function<void()>& callback) {
    if (callback) {
        callback();
    }
}

std::int16_t maximum_magic_column(const std::uint8_t row,
                                  const std::uint16_t first_entry_count) {
    auto maximum = static_cast<std::int16_t>(first_entry_count);
    if (row == 0U) {
        if (maximum >= 7) {
            maximum = 6;
        }
    } else {
        maximum = static_cast<std::int16_t>(maximum - 7);
    }
    return maximum;
}

} // namespace

MagicMenuPath select_magic_menu_path(const RunnerRecordFields& record) {
    if (record.archetype_index < 2U && record.auxiliary[0] != 0U) {
        return MagicMenuPath::spell_panel;
    }
    return MagicMenuPath::fallback_menu;
}

MagicMenuSelection move_magic_selection_column(
    MagicMenuSelection selection, const std::int8_t delta,
    const std::uint16_t first_entry_count) {
    const auto maximum = maximum_magic_column(selection.row,
                                              first_entry_count);
    const auto next = static_cast<std::int16_t>(selection.column) + delta;
    if (next < 0) {
        selection.column = static_cast<std::uint8_t>(
            std::max<std::int16_t>(0, maximum));
    } else if (next <= maximum) {
        selection.column = static_cast<std::uint8_t>(next);
    } else {
        selection.column = 0U;
    }
    return selection;
}

MagicMenuSelection move_magic_selection_row(
    MagicMenuSelection selection, const std::int8_t delta,
    const std::uint16_t first_entry_count) {
    if (static_cast<std::int16_t>(first_entry_count) - 7 < 0) {
        return selection;
    }

    const auto next = static_cast<std::int16_t>(selection.row) + delta;
    if (next < 0) {
        selection.row = 1U;
    } else if (next == 2) {
        selection.row = 0U;
    } else {
        selection.row = static_cast<std::uint8_t>(next);
    }

    if (maximum_magic_column(selection.row, first_entry_count) <
        static_cast<std::int16_t>(selection.column)) {
        selection.row = 0U;
    }
    return selection;
}

std::optional<std::int8_t> read_magic_spell_availability(
    const std::vector<std::uint8_t>& spell_records,
    const std::uint8_t spell_code) {
    const auto record_offset = static_cast<std::size_t>(spell_code) *
                               kMagicSpellRecordSize;
    constexpr std::size_t kAvailabilityOffset = 0x25U;
    if (record_offset + kAvailabilityOffset >= spell_records.size()) {
        return std::nullopt;
    }
    return static_cast<std::int8_t>(spell_records[record_offset +
                                                  kAvailabilityOffset]);
}

std::optional<MagicSpellSelection> select_magic_spell(
    const std::vector<std::uint8_t>& spell_table,
    const MagicMenuSelection selection,
    const std::vector<std::uint8_t>& spell_records) {
    if (spell_table.size() < 2U || selection.row > 1U ||
        selection.column >= 7U) {
        return std::nullopt;
    }
    const auto count = static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(spell_table[0]) << 8U) |
        spell_table[1]);
    const auto entry_index = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(selection.row) * 7U + selection.column);
    if (entry_index >= count) {
        return std::nullopt;
    }
    const auto entry_offset = static_cast<std::size_t>(2U) +
                              static_cast<std::size_t>(entry_index) * 2U;
    if (entry_offset + 1U >= spell_table.size()) {
        return std::nullopt;
    }

    const auto availability =
        read_magic_spell_availability(spell_records, spell_table[entry_offset]);
    if (!availability.has_value()) {
        return std::nullopt;
    }
    MagicSpellSelection result{};
    result.table_entry_index = entry_index;
    result.spell_code = spell_table[entry_offset];
    result.table_parameter = spell_table[entry_offset + 1U];
    result.availability_byte = *availability;
    result.availability = *availability >= 0
                              ? MagicSpellAvailability::available
                              : MagicSpellAvailability::unavailable;
    return result;
}

std::uint8_t clamp_magic_force(const std::uint8_t current,
                               const std::int8_t delta,
                               const std::uint8_t table_parameter) {
    auto next = static_cast<std::int16_t>(current) + delta;
    if (next > static_cast<std::int16_t>(table_parameter)) {
        next = table_parameter;
    }
    if (next < 1) {
        next = 1;
    }
    return static_cast<std::uint8_t>(next);
}

MagicActionPreparation prepare_magic_action(
    RunnerRecordFields& record, const MagicSpellSelection& selection,
    const std::uint8_t force) {
    MagicActionPreparation result{};
    if (selection.availability != MagicSpellAvailability::available) {
        return result;
    }

    result.result = MagicActionResult::prepared;
    result.spell_item_code = static_cast<std::uint8_t>(
        static_cast<unsigned>(selection.spell_code) + 1U);
    result.force = clamp_magic_force(force, 0, selection.table_parameter);
    result.clear_active_item_marker =
        record.equipped_inventory_slot >= 0 &&
        record.equipped_item_code != 0U;
    if (result.clear_active_item_marker) {
        record.ammo_rounds = result.force;
    }
    record.equipped_item_code = result.spell_item_code;
    record.magic_force = result.force;
    return result;
}

MagicForceLoopResult apply_magic_force_input(
    MagicForceSelection& state, const std::uint16_t input,
    const std::uint8_t table_parameter) {
    constexpr std::uint16_t kRaiseForce = 0x0800U;
    constexpr std::uint16_t kLowerForce = 0x0400U;
    // ROM 0x10016 exits on bits 9, E, or C. Bit D is the explicit loop-back
    // edge at 0x10092 and must not be folded into the finish mask.
    constexpr std::uint16_t kFinishInput = 0x0200U | 0x4000U | 0x1000U;
    if ((input & kRaiseForce) != 0U) {
        state.force = clamp_magic_force(state.force, 1, table_parameter);
        return MagicForceLoopResult::adjusted;
    }
    if ((input & kLowerForce) != 0U) {
        state.force = clamp_magic_force(state.force, -1, table_parameter);
        return MagicForceLoopResult::adjusted;
    }
    if ((input & kFinishInput) != 0U) {
        return MagicForceLoopResult::finished;
    }
    return MagicForceLoopResult::waiting;
}

void run_magic_force_selection_loop(
    const MagicForceSelectionLoopCallbacks& callbacks) {
    // ROM 0x10016 enters with the C85A8 force-display variant, then redraws
    // with C85AC only after bit 9, E, or C returns from the frame loop.
    call_once(callbacks.refresh_initial_display);
    for (;;) {
        call_once(callbacks.advance_menu_sprite_overlay);
        call_once(callbacks.wait_for_vblank);
        call_once(callbacks.latch_controller_events);
        call_once(callbacks.sample_controller_input);

        const auto input = callbacks.read_sampled_input
                               ? callbacks.read_sampled_input()
                               : 0U;
        if ((input & 0x0800U) != 0U) {
            // Source tests B before A when both edges are present.
            if (callbacks.adjust_force) {
                callbacks.adjust_force(1);
            }
            continue;
        }
        if ((input & 0x0400U) != 0U) {
            if (callbacks.adjust_force) {
                callbacks.adjust_force(-1);
            }
            continue;
        }
        if ((input & (0x0200U | 0x4000U | 0x1000U)) != 0U) {
            call_once(callbacks.refresh_final_display);
            return;
        }
        // Bit D and an empty input word both follow the source loop-back edge.
    }
}

void refresh_magic_force_display(
    const std::uint16_t current_force, const std::uint16_t maximum_force,
    const MagicForceDisplayCallbacks& callbacks) {
    // ROM 0x10172 calls 0x10156 before the conditional force marker.
    call_once(callbacks.write_repeated_grid);
    if (current_force >= 2U) {
        call_once(callbacks.write_repeated_grid);
    }
    call_once(callbacks.write_grid);
    if (maximum_force != current_force) {
        call_once(callbacks.write_repeated_grid);
    }
    call_once(callbacks.write_repeated_grid);
}

void adjust_magic_force(
    RunnerRecordFields& record, MagicForceState& state,
    const std::int16_t delta, const MagicForceAdjustmentCallbacks& callbacks) {
    // ROM 0x100A8 performs a word add, clamps to the selected maximum, and
    // then replaces zero with one before publishing the byte field.
    auto next = static_cast<std::int32_t>(state.current_force) + delta;
    next = std::min(next, static_cast<std::int32_t>(state.maximum_force));
    next = std::max(next, 1);
    state.current_force = static_cast<std::uint16_t>(next);
    record.magic_force = static_cast<std::uint8_t>(state.current_force);

    call_once(callbacks.build_grouped_decimal_glyphs);
    refresh_magic_force_display(state.current_force, state.maximum_force,
                                callbacks.display);
    call_once(callbacks.write_repeated_grid);

    // ROM 0x100A8 saves only FFF096 around 0x1055E.  The equipment refresh
    // may replace the selected maximum at FFF098; that replacement is part
    // of the source state and must not be silently rolled back here.
    const auto saved_current = state.current_force;
    call_once(callbacks.refresh_equipment_combat_values);
    state.current_force = saved_current;

    call_once(callbacks.write_repeated_menu_grid);
    call_once(callbacks.refresh_equipment_action_metric);
}

void prepare_magic_action_details(
    const MagicActionDetailCallbacks& callbacks) {
    // ROM 0x102D4 preserves three distinct text-render calls between the
    // grid preparation and posture/availability services.
    call_once(callbacks.prepare_menu_grid_then_service);
    call_once(callbacks.render_spell_text);
    call_once(callbacks.render_spell_text);
    call_once(callbacks.render_spell_text);
    call_once(callbacks.adjust_runner_posture);
    call_once(callbacks.render_spell_availability);
    call_once(callbacks.build_grouped_decimal_glyphs);
}

void render_magic_spell_availability(
    const std::int8_t availability_byte,
    const MagicSpellAvailabilityCallbacks& callbacks) {
    // ROM 0x1088C branches on the signed +0x25 spell-record byte.
    if (availability_byte < 0) {
        call_once(callbacks.write_repeated_grid);
        return;
    }
    call_once(callbacks.render_spell_text);
    call_once(callbacks.write_grid);
}

void clear_magic_menu_grids(
    const bool include_extra_grid,
    const MagicGridControlCallbacks& callbacks) {
    // ROM 0x1098E writes nine grids unconditionally and a tenth only when
    // FFF0AF is nonzero.
    for (int index = 0; index < 9; ++index) {
        call_once(callbacks.write_grid);
    }
    if (include_extra_grid) {
        call_once(callbacks.write_grid);
    }
    if (callbacks.set_display_command) {
        callbacks.set_display_command(0x8174U);
    }
}

void refresh_menu_grid_quadruple(
    const MagicGridControlCallbacks& callbacks) {
    // ROM 0x10A82 brackets four grid writes with the two source display
    // commands; these are typed presentation callbacks on the host.
    if (callbacks.set_display_command) {
        callbacks.set_display_command(0x8134U);
    }
    for (int index = 0; index < 4; ++index) {
        call_once(callbacks.write_grid);
    }
    if (callbacks.set_display_command) {
        callbacks.set_display_command(0x8174U);
    }
}

} // namespace shadowrun::ghidra_conversion
