#include "shadowrun_ghidra/entity_timing_conversion.hpp"

#include <cstdint>

namespace {

bool expect(bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    EntityTimingState primary{0xC0U, 0, 0};
    if (!expect(advance_entity_primary_timer(primary, 1U, 7U) == 7U &&
                primary.flags == 0xC0U && primary.primary_timer == 0)) {
        return 1;
    }
    primary.flags = 0x80U;
    if (!expect(advance_entity_primary_timer(primary, 0U, 7U) == 0U &&
                primary.flags == 0U && primary.primary_timer == -1)) {
        return 1;
    }

    EntityTimingState secondary{0x20U, 0, 1};
    if (!expect(advance_entity_secondary_timer(secondary, 0U) == 0U &&
                secondary.flags == 0x20U && secondary.secondary_timer == 0)) {
        return 1;
    }
    if (!expect(advance_entity_secondary_timer(secondary, 0U) == 0U &&
                secondary.flags == 0U && secondary.secondary_timer == -1)) {
        return 1;
    }
    if (!expect(advance_entity_secondary_timer(secondary, 3U) == 3U)) {
        return 1;
    }
    return 0;
}
