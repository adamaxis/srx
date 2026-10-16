#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

constexpr std::size_t kEntityTableSlotCount = 10;

// ROM 0x164F2: D1 starts at FFDF6E, A0 at FFD740, D7=#9. TST.W (A0) BEQ
// found (ANDI #0x1E,CCR). Else ADDA #0x1E, ADDQ D1, DBF. Full table ORI #1,CCR.
std::optional<std::uint16_t> find_free_entity_table_slot(
    std::uint16_t start_cursor,
    const std::array<std::uint16_t, kEntityTableSlotCount>& occupancy_words);

bool entity_table_has_free_slot(
    const std::array<std::uint16_t, kEntityTableSlotCount>& occupancy_words);

struct EntityReadinessState {
    bool active = false;
    bool ready = false;
};

// ROM 0x2FF0: find the first of thirteen entity records whose active bit and
// readiness bit are both set. The source stops when none exists; the host
// reports that condition as an empty optional.
std::optional<std::size_t> find_ready_entity_slot(
    const std::array<EntityReadinessState, 13>& entities);

struct EffectSlotTileScriptState {
    std::int16_t timer = 0;        // A0+2
    std::uint16_t frame_index = 0; // A0+0xE
};

struct EffectSlotTileScript {
    std::uint16_t frame_count = 0;
    std::uint16_t timer_reload = 0;
    std::vector<std::uint16_t> frame_offsets;
    std::vector<std::int8_t> remap_bytes;
};

enum class EffectSlotTileScriptResult {
    waiting,
    copied,
    empty,
};

struct EffectSlotTileCopy {
    std::uint16_t pattern_offset = 0;
    std::uint16_t vram_destination = 0;
};

using EffectSlotTileCopyCallback =
    std::function<void(const EffectSlotTileCopy&)>;

// ROM 0x1641A: SUBQ (A0+2); BMI walks *(A0+0x10) count/timer/offsets then
// copies 0x20-byte tiles through 0xF5C. *(A0+4) is the tile list; *FFDDDE
// rows are tile*32; *FFDDFE supplies ((word&0x7FF)-0x201)<<5.
EffectSlotTileScriptResult advance_effect_slot_tile_script(
    EffectSlotTileScriptState& slot,
    const EffectSlotTileScript& script,
    const std::vector<std::uint16_t>& tile_list,
    const std::vector<std::uint16_t>& nametable_words,
    const EffectSlotTileCopyCallback& copy_tile);

} // namespace shadowrun::ghidra_conversion
