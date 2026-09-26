#include "shadowrun_ghidra/core/counter_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    if (!expect(apply_byte_counter_delta_with_negative_floor(12, -2) == 10)) {
        return 1;
    }
    if (!expect(apply_byte_counter_delta_with_negative_floor(1, -2) == 0xFF)) {
        return 2;
    }
    if (!expect(apply_byte_counter_delta_with_negative_floor(0, -2) == 0xFF)) {
        return 3;
    }
    if (!expect(apply_byte_counter_delta_with_negative_floor(0xFF, -2) == 0xFF)) {
        return 4;
    }
    if (!expect(apply_byte_counter_delta_with_negative_floor(0xFE, 4) == 2)) {
        return 5;
    }
    if (!expect(apply_byte_counter_delta_with_negative_floor(0x7F, 1) == 0xFF)) {
        return 6;
    }

    return 0;
}
