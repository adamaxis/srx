#include "shadowrun_ghidra/field_coordinate_conversion.hpp"

#include <cstdint>

namespace {

bool expect(bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    if (!expect(fold_field_coordinate(0x0070U) == 7U &&
                fold_field_coordinate(0x0080U) == 0U &&
                fold_field_coordinate(0x00F0U) == 7U)) {
        return 1;
    }
    if (!expect(offset_field_coordinate(0x0070U) == -1 &&
                offset_field_coordinate(0x0080U) == 0 &&
                offset_field_coordinate(0x00F0U) == 7)) {
        return 1;
    }
    if (!expect(quantize_field_delta(0x1000, 0U) == 64 &&
                quantize_field_delta(0, 0x1000U) == -64 &&
                quantize_field_delta(0x007FU, 0U) == 0)) {
        return 1;
    }
    return 0;
}
