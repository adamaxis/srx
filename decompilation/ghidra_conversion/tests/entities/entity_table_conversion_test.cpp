#include "shadowrun_ghidra/entities/entity_table_conversion.hpp"

#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::array<std::uint16_t, 10> occupancy{};
    occupancy.fill(1U);
    if (entity_table_has_free_slot(occupancy) ||
        find_free_entity_table_slot(0x10U, occupancy).has_value()) {
        return 1;
    }
    occupancy[7] = 0U;
    const auto found = find_free_entity_table_slot(0x10U, occupancy);
    if (!entity_table_has_free_slot(occupancy) || !found.has_value() ||
        *found != 0x17U) {
        return 2;
    }
    std::array<EntityReadinessState, 13> entities{};
    entities[9] = {true, true};
    const auto ready = find_ready_entity_slot(entities);
    if (!ready.has_value() || *ready != 9U) {
        return 3;
    }
    entities[2] = {true, false};
    const auto still_ready = find_ready_entity_slot(entities);
    if (!still_ready.has_value() || *still_ready != 9U) {
        return 4;
    }

    EffectSlotTileScriptState slot{};
    const EffectSlotTileScript script{
        2U, 3U, {0U, 1U}, {0, 1, -1}};
    const std::vector<std::uint16_t> tile_list{2U, 3U};
    std::vector<std::uint16_t> nametable(64U, 0U);
    nametable[32] = 0x0010U;
    nametable[33] = 0x0011U;
    nametable[48] = 0x0221U;
    nametable[49] = 0x0222U;
    std::vector<EffectSlotTileCopy> copies;
    const auto copy = [&](const EffectSlotTileCopy& issued) {
        copies.push_back(issued);
    };

    slot.timer = 1;
    if (advance_effect_slot_tile_script(
            slot, script, tile_list, nametable, copy) !=
            EffectSlotTileScriptResult::waiting ||
        slot.timer != 0 || slot.frame_index != 0U || !copies.empty()) {
        return 5;
    }

    if (advance_effect_slot_tile_script(
            slot, script, tile_list, nametable, copy) !=
            EffectSlotTileScriptResult::copied ||
        slot.timer != 3 || slot.frame_index != 1U || copies.size() != 2U ||
        copies[0].pattern_offset != 0x0400U ||
        copies[0].vram_destination != 0x0200U ||
        copies[1].pattern_offset != 0x0420U ||
        copies[1].vram_destination != 0x0220U) {
        return 6;
    }

    copies.clear();
    slot.timer = 0;
    if (advance_effect_slot_tile_script(
            slot, script, tile_list, nametable, copy) !=
            EffectSlotTileScriptResult::copied ||
        slot.frame_index != 0U || copies.size() != 2U ||
        copies[0].pattern_offset != 0xC1E0U ||
        copies[0].vram_destination != 0x0200U) {
        return 7;
    }

    copies.clear();
    EffectSlotTileScriptState halted{};
    const EffectSlotTileScript stop_script{1U, 1U, {0U}, {-1}};
    if (advance_effect_slot_tile_script(
            halted, stop_script, tile_list, nametable, copy) !=
            EffectSlotTileScriptResult::empty ||
        halted.frame_index != 0U || !copies.empty()) {
        return 8;
    }
    return 0;
}
