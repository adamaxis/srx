#pragma once

#include <cstdint>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct SpriteRecord {
    std::uint32_t source_data = 0;
    std::uint16_t source_attribute = 0;
    std::uint16_t tile_offset = 0;
};

// ROM 0x61A0: append a sprite record whose tile offset is the entity frame
// index shifted into the source's tile-word position.
void append_entity_sprite_record(std::vector<SpriteRecord>& records,
                                 std::uint32_t source_data,
                                 std::uint16_t source_attribute,
                                 std::int16_t entity_frame_index);

// ROM 0x61CC: append a sprite record whose tile offset comes from the caller's
// supplied word, shifted into the source's tile-word position.
void append_sprite_record(std::vector<SpriteRecord>& records,
                          std::uint32_t source_data,
                          std::uint16_t source_attribute,
                          std::int16_t tile_index);

} // namespace shadowrun::ghidra_conversion
