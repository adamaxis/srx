#include "shadowrun_ghidra/rendering/sprite_record_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

void append_record(std::vector<SpriteRecord>& records,
                   const std::uint32_t source_data,
                   const std::uint16_t source_attribute,
                   const std::int16_t tile_index) {
    records.push_back(SpriteRecord{
        source_data,
        source_attribute,
        static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(tile_index) << 5U)});
}

} // namespace

void append_entity_sprite_record(std::vector<SpriteRecord>& records,
                                 const std::uint32_t source_data,
                                 const std::uint16_t source_attribute,
                                 const std::int16_t entity_frame_index) {
    append_record(records, source_data, source_attribute, entity_frame_index);
}

void append_sprite_record(std::vector<SpriteRecord>& records,
                          const std::uint32_t source_data,
                          const std::uint16_t source_attribute,
                          const std::int16_t tile_index) {
    append_record(records, source_data, source_attribute, tile_index);
}

} // namespace shadowrun::ghidra_conversion
