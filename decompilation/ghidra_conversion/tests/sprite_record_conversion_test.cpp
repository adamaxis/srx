#include "shadowrun_ghidra/sprite_record_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<SpriteRecord> records;
    append_entity_sprite_record(records, 0x12345678U, 0x9ABCU, 3);
    append_sprite_record(records, 0xCAFEBABEU, 0x1357U, -1);
    if (!expect(records.size() == 2U &&
                records[0].source_data == 0x12345678U &&
                records[0].source_attribute == 0x9ABCU &&
                records[0].tile_offset == 0x0060U &&
                records[1].source_data == 0xCAFEBABEU &&
                records[1].source_attribute == 0x1357U &&
                records[1].tile_offset == 0xFFE0U)) {
        return 1;
    }
    return 0;
}
