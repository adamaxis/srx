#include "shadowrun_ghidra/entity_tile_mapping_conversion.hpp"

namespace shadowrun::ghidra_conversion {

std::optional<EntityTilePair> map_world_delta_to_tile_pair(
    const std::uint16_t world_delta,
    const std::uint16_t map_stride) {
    const auto half_stride = static_cast<std::uint16_t>(map_stride >> 1U);
    if (half_stride == 0U) {
        return std::nullopt;
    }

    const auto half_delta = static_cast<std::uint16_t>(world_delta >> 1U);
    const auto major = static_cast<std::uint16_t>(half_delta / half_stride);
    const auto remainder = static_cast<std::uint16_t>(half_delta % half_stride);
    return EntityTilePair{
        static_cast<std::uint16_t>(major * 0x0200U + 0x0100U), major,
        static_cast<std::uint16_t>(remainder * 0x0200U + 0x0100U)};
}

} // namespace shadowrun::ghidra_conversion
