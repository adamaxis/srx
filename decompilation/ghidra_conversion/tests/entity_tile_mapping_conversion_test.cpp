#include "shadowrun_ghidra/entity_tile_mapping_conversion.hpp"

#include <cstdint>

int main() {
    using namespace shadowrun::ghidra_conversion;

    const auto mapped = map_world_delta_to_tile_pair(20U, 8U);
    if (!mapped || mapped->major_index != 2U ||
        mapped->primary_coordinate != 0x0500U ||
        mapped->secondary_coordinate != 0x0500U) {
        return 1;
    }
    if (map_world_delta_to_tile_pair(10U, 0U)) {
        return 2;
    }
    return 0;
}
