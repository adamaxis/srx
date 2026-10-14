#pragma once

#include <cstdint>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct EntitySpriteDescriptor {
    std::int16_t x_offset = 0;
    std::int16_t y_offset = 0;
    std::uint8_t shape = 0;
    std::uint16_t tile = 0;
};

struct EntitySpriteProjection {
    std::int16_t screen_x = 0;
    std::int16_t screen_y = 0;
    std::uint8_t entity_flags = 0;
    std::uint16_t tile_flags = 0;
    std::uint16_t alternate_tile_add = 0;
};

struct ExpandedSpriteRecord {
    std::int16_t screen_x = 0;
    std::int16_t screen_y = 0;
    std::uint8_t shape = 0;
    std::uint8_t sequence = 0;
    std::uint16_t tile = 0;
};

struct SpriteExpansionStatus {
    bool terminated = false;
    bool alternate_tile_group = false;
};

// ROM 0x622C/0x624C: expand one entity's four-word sprite descriptors into
// ordered host records. A leading 0x8002 descriptor is a group marker (the
// signed source spelling is -0x7FFE): it is
// skipped and enables the source's alternate tile addition. The source's
// 0x8000 descriptor terminator is required for a complete bounded expansion.
SpriteExpansionStatus expand_entity_sprite_descriptors(
    const EntitySpriteProjection& entity,
    const std::vector<EntitySpriteDescriptor>& descriptors,
    std::uint8_t& next_sequence,
    std::vector<ExpandedSpriteRecord>& output);

struct QueuedSpriteRecord {
    std::uint32_t source = 0; // A2+2
    std::uint16_t length = 0; // A2+6
    std::uint16_t tile_base = 0; // word << 5
};

struct SpriteRecordQueue {
    std::vector<QueuedSpriteRecord> records;
};

// ROM 0x61A0: enqueue A2+2/+6 and (record+0x2A)<<5, then FFDF32 += 8.
void append_entity_sprite_record(
    SpriteRecordQueue& queue,
    std::uint32_t source,
    std::uint16_t length,
    std::uint16_t record_word_2a);
// ROM 0x61CC: same queue, tile base from D0<<5.
void append_sprite_record(
    SpriteRecordQueue& queue,
    std::uint32_t source,
    std::uint16_t length,
    std::uint16_t tile_index);

} // namespace shadowrun::ghidra_conversion
