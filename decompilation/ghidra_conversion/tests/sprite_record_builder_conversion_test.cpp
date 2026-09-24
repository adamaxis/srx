#include "shadowrun_ghidra/sprite_record_builder_conversion.hpp"

#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    const std::vector<SpriteSourceRecord> source{
        {10, -4, 3, 0x20U},
        {-2, 8, 5, 0x40U},
        {static_cast<std::int16_t>(-0x8000), 99, 7, 0x80U},
        {30, 30, 9, 0x100U},
    };
    const auto records = build_title_sprite_records(source, 100, 200, 7U,
                                                     0x4000U);
    if (records.size() != 2U || records[0].y != 110 || records[0].x != 196 ||
        records[0].size != 3U || records[0].sequence != 7U ||
        records[0].attributes != 0x4020U || records[1].y != 98 ||
        records[1].x != 208 || records[1].sequence != 8U) {
        return 1;
    }

    const auto offset_records =
        build_title_sprite_records_with_attribute_offset(
            source, 0, 0, 0xFEU, 0x1000U, -0x10);
    if (offset_records.size() != 2U || offset_records[0].sequence != 0xFEU ||
        offset_records[0].attributes != 0x1010U ||
        offset_records[1].sequence != 0xFFU ||
        offset_records[1].attributes != 0x1030U) {
        return 2;
    }

    return 0;
}
