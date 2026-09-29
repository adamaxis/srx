#include "shadowrun_ghidra/entities/entity_table_conversion.hpp"

namespace shadowrun::ghidra_conversion {

std::optional<std::uint16_t> find_free_entity_table_slot(
    const std::uint16_t start_cursor,
    const std::array<std::uint16_t, kEntityTableSlotCount>& occupancy_words) {
    auto cursor = start_cursor;
    for (const auto occupancy : occupancy_words) {
        if (occupancy == 0U) {
            return cursor;
        }
        ++cursor;
    }
    return std::nullopt;
}

bool entity_table_has_free_slot(
    const std::array<std::uint16_t, kEntityTableSlotCount>& occupancy_words) {
    return find_free_entity_table_slot(0U, occupancy_words).has_value();
}

std::optional<std::size_t> find_ready_entity_slot(
    const std::array<EntityReadinessState, 13>& entities) {
    for (std::size_t index = 0; index < entities.size(); ++index) {
        if (entities[index].active && entities[index].ready) {
            return index;
        }
    }
    return std::nullopt;
}

EffectSlotTileScriptResult advance_effect_slot_tile_script(
    EffectSlotTileScriptState& slot,
    const EffectSlotTileScript& script,
    const std::vector<std::uint16_t>& tile_list,
    const std::vector<std::uint16_t>& nametable_words,
    const EffectSlotTileCopyCallback& copy_tile) {
    --slot.timer;
    if (slot.timer >= 0) {
        return EffectSlotTileScriptResult::waiting;
    }

    auto next = static_cast<std::uint16_t>(slot.frame_index + 1U);
    if (static_cast<std::int16_t>(next) >=
        static_cast<std::int16_t>(script.frame_count)) {
        next = 0U;
    }
    slot.frame_index = next;
    slot.timer = static_cast<std::int16_t>(script.timer_reload);

    if (tile_list.empty() || next >= script.frame_offsets.size()) {
        return EffectSlotTileScriptResult::empty;
    }
    const auto dest_entry = script.frame_offsets[next];
    if (dest_entry >= tile_list.size()) {
        return EffectSlotTileScriptResult::empty;
    }

    const auto row_words = [](const std::uint16_t tile) {
        return static_cast<std::size_t>(
            static_cast<std::uint16_t>(tile << 5U) >> 1U);
    };
    const auto dest_row = row_words(tile_list[dest_entry]);
    const auto source_row = row_words(tile_list.front());

    std::size_t copies = 0U;
    for (const auto remap : script.remap_bytes) {
        if (remap < 0) {
            break;
        }
        const auto word_index = static_cast<std::size_t>(
            static_cast<std::uint16_t>(static_cast<std::int16_t>(remap)));
        const auto dest_cell = dest_row + word_index;
        const auto source_cell = source_row + word_index;
        if (dest_cell >= nametable_words.size() ||
            source_cell >= nametable_words.size()) {
            continue;
        }
        const auto dest_word = nametable_words[dest_cell];
        const auto source_word = nametable_words[source_cell];
        const auto pattern = static_cast<std::uint16_t>(
            (dest_word & 0x07FFU) - 0x0201U);
        if (copy_tile) {
            copy_tile({static_cast<std::uint16_t>(pattern << 5U),
                       static_cast<std::uint16_t>(source_word << 5U)});
        }
        ++copies;
    }
    return copies == 0U ? EffectSlotTileScriptResult::empty
                        : EffectSlotTileScriptResult::copied;
}

} // namespace shadowrun::ghidra_conversion
