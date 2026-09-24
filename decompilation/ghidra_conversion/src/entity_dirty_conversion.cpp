#include "shadowrun_ghidra/entity_dirty_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void mark_entity_slots_for_render(
    std::array<EntityRenderState, 16>& entities) {
    for (auto& entity : entities) {
        entity.flags = static_cast<std::uint8_t>(entity.flags | 1U);
    }
}

} // namespace shadowrun::ghidra_conversion
