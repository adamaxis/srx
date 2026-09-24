#include "shadowrun_ghidra/entity_dirty_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::array<EntityRenderState, 16> entities{};
    for (std::size_t index = 0; index < entities.size(); ++index) {
        entities[index].flags = static_cast<std::uint8_t>(index << 1U);
    }
    mark_entity_slots_for_render(entities);
    for (std::size_t index = 0; index < entities.size(); ++index) {
        if (!expect(entities[index].flags ==
                    static_cast<std::uint8_t>((index << 1U) | 1U))) {
            return 1;
        }
    }
    return 0;
}
