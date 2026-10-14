#include "shadowrun_ghidra/entities/entity_delay_conversion.hpp"

#include <cstdint>

namespace {

int expect(const bool condition) {
    return condition ? 0 : 1;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    if (expect(average_entity_delay_attributes(40, 80) == 60) != 0 ||
        // The source performs byte addition before the shift: 240 + 32 wraps
        // to 16, producing 8 rather than 136.
        expect(average_entity_delay_attributes(240, 32) == 8) != 0) {
        return 1;
    }

    const EntityDelayAttributes base{40, 80, 0};
    if (expect(compute_entity_delay_factor(base) == 59) != 0 ||
        expect(compute_entity_delay_factor({40, 80, 0x1000}) == 63) != 0 ||
        expect(compute_entity_delay_factor({40, 80, 0x2000}) == 67) != 0 ||
        expect(compute_entity_delay_factor({40, 80, 0x4000}) == 71) != 0 ||
        // Bit 12 wins when multiple source modifier bits are present.
        expect(compute_entity_delay_factor({40, 80, 0x5000}) == 63) != 0 ||
        expect(compute_entity_delay_factor({0, 0, 0}) == 0) != 0) {
        return 1;
    }

    EntityDelayState delay{};
    const EntityDelayAttributes zero_factor{0, 1, 0};
    update_entity_animation_delay(delay, zero_factor, EntityDelayScale::Normal);
    if (expect(delay.animation_delay == 106) != 0) {
        return 1;
    }

    update_entity_animation_delay(delay, zero_factor, EntityDelayScale::Double);
    if (expect(delay.animation_delay == 212) != 0) {
        return 1;
    }

    update_entity_animation_delay(delay, zero_factor, EntityDelayScale::Half);
    if (expect(delay.animation_delay == 53) != 0) {
        return 1;
    }

    if (expect(decode_entity_delay_scale(0) == EntityDelayScale::Normal) != 0 ||
        expect(decode_entity_delay_scale(2) == EntityDelayScale::Double) != 0 ||
        expect(decode_entity_delay_scale(1) == EntityDelayScale::Half) != 0 ||
        expect(decode_entity_delay_scale(3) == EntityDelayScale::Half) != 0) {
        return 1;
    }

    update_entity_animation_delay(
        delay, {200, 200, 0}, EntityDelayScale::Normal);
    if (expect(delay.animation_delay == 15) != 0) {
        return 1;
    }

    return 0;
}
