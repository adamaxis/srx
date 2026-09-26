#include "shadowrun_ghidra/field/field_menu_state_batch_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {

namespace {

void call_once(const std::function<void()>& callback) {
    if (callback) {
        callback();
    }
}

} // namespace

std::size_t render_wrapped_text(
    const std::vector<std::uint8_t>& encoded,
    const std::size_t line_width,
    const WrappedTextCallbacks& callbacks) {
    if (line_width == 0U || encoded.empty()) {
        return 0U;
    }

    std::size_t written = 0U;
    std::size_t source_offset = 0U;
    std::size_t line = 0U;
    while (source_offset < encoded.size()) {
        call_once(callbacks.begin_transfer);

        // ROM 0xC168 scans through the next source space and remembers the
        // last one that still fits. A high-bit marker ends the scan. The host
        // guard for an overlong word keeps malformed extracted input from
        // looping forever; normal source strings have a fitting space or
        // terminator and follow the exact selected-count path.
        std::size_t scan_offset = source_offset;
        std::size_t scanned = 0U;
        std::size_t last_fitting_space = 0U;
        while (scan_offset < encoded.size()) {
            const auto value = encoded[scan_offset++];
            ++scanned;
            if ((value & 0x80U) != 0U) {
                break;
            }
            if (value == 0x20U) {
                if (scanned < line_width) {
                    last_fitting_space = scanned;
                    continue;
                }
                break;
            }
        }

        std::size_t line_length = scanned;
        if (scanned > line_width && last_fitting_space != 0U) {
            line_length = last_fitting_space;
        } else if (scanned > line_width && last_fitting_space == 0U) {
            // The ROM data keeps words within the width. If an external host
            // asset violates that contract, consume one bounded line instead
            // of reproducing a 68000 DBF underflow.
            line_length = line_width;
        }
        if (line_length == 0U) {
            break;
        }

        for (std::size_t column = 0U; column < line_length &&
                                      source_offset < encoded.size();
             ++column, ++source_offset) {
            const auto value = encoded[source_offset];
            if ((value & 0x80U) != 0U) {
                // 0x80 is a source line break; 0xFF is the final marker.
                // Both occupy the scanned source span but neither is a glyph.
                break;
            }
            const auto glyph = callbacks.glyph_for_byte
                                   ? callbacks.glyph_for_byte(value)
                                   : static_cast<std::uint16_t>(value);
            if (callbacks.write_glyph_at) {
                callbacks.write_glyph_at(line, column, glyph);
            } else if (callbacks.write_glyph) {
                callbacks.write_glyph(glyph);
            }
            ++written;
        }

        ++line;
        if (source_offset >= encoded.size()) {
            break;
        }

        // If the selected span ended before the marker, the next iteration
        // starts at the first unconsumed source byte. Otherwise consume the
        // marker now and either continue at the next line (0x80) or finish
        // (0xFF). The source distinguishes these by testing marker << 1.
        const auto marker = encoded[source_offset];
        if ((marker & 0x80U) != 0U) {
            ++source_offset;
            if ((marker & 0x40U) != 0U) {
                break;
            }
        }
    }
    return written;
}

bool dispatch_menu_handler(
    const std::size_t selected,
    const std::vector<std::function<void()>>& handlers) {
    if (selected >= handlers.size() || !handlers[selected]) {
        return false;
    }
    handlers[selected]();
    return true;
}

bool pass_entity_action_threshold_variant(
    const std::uint8_t entity_phase,
    const std::int16_t requested_threshold,
    const std::uint8_t current_threshold,
    const std::function<void()>& sample_threshold) {
    if (entity_phase == 1U) {
        return true;
    }
    call_once(sample_threshold);
    return requested_threshold >= 0 &&
           static_cast<std::uint8_t>(requested_threshold) >= current_threshold;
}

std::size_t service_active_effect_slots(
    const EffectServiceState& state,
    const std::function<void(std::size_t)>& service_slot) {
    if (state.mode == 2U && state.effect_active) {
        return 0U;
    }
    std::size_t serviced = 0U;
    const auto limit = std::min<std::size_t>(10U, state.active_slots.size());
    for (std::size_t index = 0; index < limit; ++index) {
        if (state.active_slots[index] && service_slot) {
            service_slot(index);
            ++serviced;
        }
    }
    return serviced;
}

std::size_t populate_entity_table(
    const std::vector<EntityDescriptorPair>& descriptors,
    std::vector<EntityTableRecord>& records,
    const std::function<std::optional<std::size_t>()>& allocate_slot) {
    std::size_t populated = 0U;
    for (const auto& descriptor : descriptors) {
        if (descriptor.id == 0) {
            break;
        }
        if (!allocate_slot) {
            break;
        }
        const auto slot = allocate_slot();
        if (!slot.has_value()) {
            break;
        }
        if (*slot >= records.size()) {
            records.resize(*slot + 1U);
        }
        auto& record = records[*slot];
        record.status = -1;
        record.id = descriptor.id;
        record.value = descriptor.value;
        record.behavior_token = 0x0001641AU;
        record.field_a = 0U;
        record.field_b = 0U;
        ++populated;
    }
    return populated;
}

bool mark_projected_terrain_cell(
    std::vector<std::uint16_t>& terrain_words,
    const std::function<std::optional<std::size_t>()>& project_cell,
    std::uint8_t& marked_count,
    bool& render_dirty) {
    if (!project_cell) {
        return false;
    }
    const auto cell = project_cell();
    if (!cell.has_value() || *cell >= terrain_words.size() ||
        terrain_words[*cell] == 0x000BU) {
        return false;
    }
    ++marked_count;
    terrain_words[*cell] = 0x000BU;
    render_dirty = true;
    return true;
}

std::size_t publish_world_activity_descriptors(
    std::vector<std::uint32_t>& source_ids,
    std::vector<WorldActivityDescriptor>& published,
    bool& active,
    const std::function<void()>& schedule_timeout) {
    active = true;
    published.clear();
    for (const auto id : source_ids) {
        if (id == 0U) {
            break;
        }
        published.push_back(
            {id, 0x00016A6AU + static_cast<std::uint32_t>(published.size() * 6U)});
    }
    call_once(schedule_timeout);
    source_ids.clear();
    return published.size();
}

bool process_terrain_work_batch(
    const std::vector<TerrainWorkEntry>& entries,
    const std::vector<bool>& markers,
    const TerrainWorkCallbacks& callbacks,
    bool& render_dirty) {
    render_dirty = false;
    bool processed = false;
    for (std::size_t index = 0; index < entries.size(); ++index) {
        const auto& entry = entries[index];
        if (entry.id == 0) {
            return processed;
        }
        const bool marked = index < markers.size() && markers[index];
        if (!marked) {
            continue;
        }
        if (callbacks.mark_boundary) {
            callbacks.mark_boundary(entry);
        }
        processed = true;
        if (entry.alternate_region && callbacks.copy_region_rows &&
            callbacks.copy_region_rows()) {
            render_dirty = true;
        }
    }
    return processed;
}

void refresh_field_hud(const std::function<void()>& advance_footer,
                       const std::function<void()>& write_grid) {
    call_once(advance_footer);
    for (int index = 0; index < 5; ++index) {
        call_once(write_grid);
    }
}

void refresh_mode_glyph(
    std::vector<std::uint8_t>& mode_values,
    const std::size_t mode_index,
    const std::vector<std::uint8_t>& remap_table,
    const std::function<void()>& write_grid) {
    if (mode_index >= mode_values.size()) {
        return;
    }
    const auto value = mode_values[mode_index];
    if (value < 0x11U) {
        return;
    }
    const auto table_index = static_cast<std::size_t>(value - 0x11U);
    if (table_index < remap_table.size()) {
        mode_values[mode_index] = remap_table[table_index];
    }
    call_once(write_grid);
}

std::optional<std::uint8_t> select_hud_mode(
    const std::uint8_t display_mode,
    const std::uint8_t slot_value,
    const std::vector<std::uint8_t>& mode_table) {
    std::size_t index = static_cast<std::size_t>(slot_value) * 2U;
    if (display_mode >= 1U && slot_value >= 7U && slot_value < 0x11U) {
        ++index;
    }
    if (index >= mode_table.size()) {
        return std::nullopt;
    }
    return mode_table[index];
}

std::uint32_t pass_through_value(const std::uint32_t value) {
    return value;
}

void run_menu_slot_service(std::uint8_t& pending_flags,
                           const std::uint8_t pending_mask,
                           const MenuServiceCallbacks& callbacks) {
    call_once(callbacks.set_selection);
    call_once(callbacks.render_text);
    if ((pending_flags & pending_mask) != 0U) {
        call_once(callbacks.service_auxiliary);
        call_once(callbacks.refresh_grid);
    }
    call_once(callbacks.prepare_offset);
}

std::optional<MenuPendingFlagMergeResult> merge_menu_pending_flags(
    MenuPendingFlagState& state,
    const std::function<void(std::size_t metadata_offset)>& refresh_metadata) {
    if (state.selected_mode_index >= state.pending_flags.size()) {
        return std::nullopt;
    }
    const auto record_offset = state.selected_runner_index * 0x14U + 0x05U;
    if (record_offset >= state.runner_records.size()) {
        return std::nullopt;
    }

    const auto source_flags = state.runner_records[record_offset];
    state.pending_flags[state.selected_mode_index] = static_cast<std::uint8_t>(
        state.pending_flags[state.selected_mode_index] | source_flags);
    const auto metadata_offset = static_cast<std::size_t>(source_flags) * 0x20U;
    if (refresh_metadata) {
        refresh_metadata(metadata_offset);
    }
    return MenuPendingFlagMergeResult{source_flags, metadata_offset,
                                      state.pending_flags[
                                          state.selected_mode_index]};
}

void run_menu_metadata_service(const MenuServiceCallbacks& callbacks) {
    call_once(callbacks.service_auxiliary);
    call_once(callbacks.prepare_offset);
}

void run_menu_selection_service(const std::uint8_t display_ready,
                                const MenuServiceCallbacks& callbacks) {
    call_once(callbacks.set_selection);
    call_once(callbacks.render_text);
    if (display_ready != 0U) {
        call_once(callbacks.prepare_offset);
        call_once(callbacks.service_auxiliary);
        call_once(callbacks.refresh_grid);
    }
    call_once(callbacks.prepare_offset);
}

void run_menu_selection_variant(const std::uint8_t display_ready,
                                const MenuServiceCallbacks& callbacks) {
    call_once(callbacks.set_selection);
    call_once(callbacks.render_text);
    if (display_ready != 0U) {
        call_once(callbacks.service_secondary);
    }
    call_once(callbacks.prepare_offset);
}

void run_menu_progression_variant(const std::uint8_t display_ready,
                                  const MenuServiceCallbacks& callbacks) {
    call_once(callbacks.set_selection);
    call_once(callbacks.render_text);
    if (display_ready != 0U) {
        call_once(callbacks.service_auxiliary);
        call_once(callbacks.render_progression);
    }
    call_once(callbacks.prepare_offset);
}

std::optional<std::size_t> clear_first_ready_menu_slot(
    const std::vector<std::uint8_t>& statuses,
    const std::function<void(std::size_t)>& clear_visual,
    const std::function<void(std::size_t)>& refresh_number,
    const std::function<void(std::size_t)>& clear_status) {
    const auto limit = std::min<std::size_t>(5U, statuses.size());
    for (std::size_t slot = limit; slot-- > 0U;) {
        if (statuses[slot] != 7U) {
            continue;
        }
        if (clear_visual) {
            clear_visual(slot);
        }
        if (refresh_number) {
            refresh_number(slot);
        }
        if (clear_status) {
            clear_status(slot);
        }
        return slot;
    }
    return std::nullopt;
}

} // namespace shadowrun::ghidra_conversion
