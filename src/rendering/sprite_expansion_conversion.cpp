#include "shadowrun_ghidra/rendering/sprite_expansion_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

std::int16_t add_word(const std::int16_t base,
                      const std::int16_t offset) {
    const auto sum = static_cast<std::int32_t>(base) +
                     static_cast<std::int32_t>(offset);
    return static_cast<std::int16_t>(static_cast<std::uint16_t>(sum));
}

} // namespace

SpriteExpansionStatus expand_entity_sprite_descriptors(
    const EntitySpriteProjection& entity,
    const std::vector<EntitySpriteDescriptor>& descriptors,
    std::uint8_t& next_sequence,
    std::vector<ExpandedSpriteRecord>& output) {
    SpriteExpansionStatus status{};
    std::size_t descriptor_index = 0;
    if (!descriptors.empty() &&
        descriptors.front().x_offset == static_cast<std::int16_t>(0x8002)) {
        status.alternate_tile_group = true;
        descriptor_index = 1;
    }

    for (; descriptor_index < descriptors.size(); ++descriptor_index) {
        const auto& descriptor = descriptors[descriptor_index];
        if (descriptor.x_offset == static_cast<std::int16_t>(0x8000)) {
            status.terminated = true;
            break;
        }

        auto tile = descriptor.tile;
        if ((entity.entity_flags & 0x80U) == 0U) {
            tile = static_cast<std::uint16_t>(tile | entity.tile_flags);
        }
        if (status.alternate_tile_group) {
            tile = static_cast<std::uint16_t>(
                tile + entity.alternate_tile_add);
        }

        output.push_back(ExpandedSpriteRecord{
            add_word(entity.screen_x, descriptor.x_offset),
            add_word(entity.screen_y, descriptor.y_offset),
            descriptor.shape,
            next_sequence,
            tile});
        next_sequence = static_cast<std::uint8_t>(next_sequence + 1U);
    }
    return status;
}

namespace {

void enqueue_sprite_record(
    SpriteRecordQueue& queue,
    const std::uint32_t source,
    const std::uint16_t length,
    const std::uint16_t shifted_word) {
    queue.records.push_back(QueuedSpriteRecord{
        source, length,
        static_cast<std::uint16_t>(shifted_word << 5U)});
}

} // namespace

void append_entity_sprite_record(
    SpriteRecordQueue& queue,
    const std::uint32_t source,
    const std::uint16_t length,
    const std::uint16_t record_word_2a) {
    enqueue_sprite_record(queue, source, length, record_word_2a);
}

void append_sprite_record(
    SpriteRecordQueue& queue,
    const std::uint32_t source,
    const std::uint16_t length,
    const std::uint16_t tile_index) {
    enqueue_sprite_record(queue, source, length, tile_index);
}

} // namespace shadowrun::ghidra_conversion
