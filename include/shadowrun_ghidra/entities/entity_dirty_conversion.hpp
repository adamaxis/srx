#pragma once

#include <array>
#include <cstdint>

namespace shadowrun::ghidra_conversion {

struct EntityRenderState {
    std::uint8_t flags = 0;
};

// ROM 0x5B7E: set the render-update bit on all sixteen entity slots.
void mark_entity_slots_for_render(
    std::array<EntityRenderState, 16>& entities);

} // namespace shadowrun::ghidra_conversion
