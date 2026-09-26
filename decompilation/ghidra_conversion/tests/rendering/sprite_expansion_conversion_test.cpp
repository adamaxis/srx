#include "shadowrun_ghidra/rendering/sprite_expansion_conversion.hpp"

#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    const EntitySpriteProjection entity{
        100, 200, 0x00U, 0x0040U, 0x0020U};
    const std::vector<EntitySpriteDescriptor> ordinary{
        {10, -4, 0x12U, 0x0100U},
        {-2, 6, 0x34U, 0x0200U},
        {static_cast<std::int16_t>(0x8000), 0, 0, 0}};
    std::uint8_t next_sequence = 2U;
    std::vector<ExpandedSpriteRecord> output;
    const auto ordinary_status = expand_entity_sprite_descriptors(
        entity, ordinary, next_sequence, output);
    if (!expect(ordinary_status.terminated &&
                !ordinary_status.alternate_tile_group &&
                next_sequence == 4U && output.size() == 2U &&
                output[0].screen_x == 110 && output[0].screen_y == 196 &&
                output[0].shape == 0x12U && output[0].sequence == 2U &&
                output[0].tile == 0x0140U &&
                output[1].screen_x == 98 && output[1].screen_y == 206 &&
                output[1].shape == 0x34U && output[1].sequence == 3U &&
                output[1].tile == 0x0240U)) {
        return 1;
    }

    const EntitySpriteProjection alternate_entity{
        -100, -200, 0x80U, 0x00F0U, 0x0020U};
    const std::vector<EntitySpriteDescriptor> alternate{
        {static_cast<std::int16_t>(0x8002), 0, 0, 0},
        {8, 9, 0x56U, 0x1000U},
        {static_cast<std::int16_t>(0x8000), 0, 0, 0}};
    const auto alternate_status = expand_entity_sprite_descriptors(
        alternate_entity, alternate, next_sequence, output);
    if (!expect(alternate_status.terminated &&
                alternate_status.alternate_tile_group &&
                next_sequence == 5U && output.size() == 3U &&
                output[2].screen_x == -92 && output[2].screen_y == -191 &&
                output[2].shape == 0x56U && output[2].sequence == 4U &&
                output[2].tile == 0x1020U)) {
        return 2;
    }

    const std::vector<EntitySpriteDescriptor> incomplete{{1, 1, 0, 0x20U}};
    const auto incomplete_status = expand_entity_sprite_descriptors(
        entity, incomplete, next_sequence, output);
    if (!expect(!incomplete_status.terminated &&
                output.size() == 4U && next_sequence == 6U)) {
        return 3;
    }

    return 0;
}
