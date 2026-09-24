#include "shadowrun_ghidra/entity_sort_conversion.hpp"

#include <array>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::array<EntityDepthEntry, 16> entities{};
    entities[0] = EntityDepthEntry{10, 0, 0};
    entities[1] = EntityDepthEntry{30, 0, 1};
    entities[2] = EntityDepthEntry{20, 0, 2};
    entities[3] = EntityDepthEntry{90, 0x10U, 3};
    entities[4] = EntityDepthEntry{-5, 0, 4};
    entities[5] = EntityDepthEntry{25, 0x40U, 5};
    for (std::uint16_t id = 6; id < entities.size(); ++id) {
        entities[id] = EntityDepthEntry{0, 0, id};
    }

    sort_entity_slots_by_depth(entities);
    if (!expect(entities[0].id == 1U && entities[1].id == 2U &&
                entities[2].id == 0U && entities[3].id == 3U &&
                entities[4].id == 5U && entities[5].id == 6U &&
                entities[6].id == 7U && entities[14].id == 15U &&
                entities[15].id == 4U)) {
        return 1;
    }

    // Equal effective depths retain source slot order.
    if (!expect(entities[3].id == 3U && entities[4].id == 5U &&
                entities[5].id == 6U)) {
        return 2;
    }
    return 0;
}
