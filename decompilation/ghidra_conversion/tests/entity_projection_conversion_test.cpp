#include "shadowrun_ghidra/entity_projection_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    EntityProjectionState ordinary{
        0x0100U, 0x0100U, 0x0010U, 0xFFFFU, 0xFFFFU};
    const auto visible = project_entity_screen_position_standard(
        ordinary, 0x0080U, 0x0080U);
    if (!expect(visible.visible && visible.source_result == 1U &&
                visible.wrote_screen_x && visible.wrote_screen_y &&
                ordinary.screen_x == 0x0088U &&
                ordinary.screen_y == 0x0098U)) {
        return 1;
    }

    ordinary.world_x = 0xFF00U;
    const auto hidden = project_entity_screen_position_standard(
        ordinary, 0x0080U, 0x0080U);
    if (!expect(!hidden.visible && hidden.source_result == 0x0068U &&
                ordinary.screen_x == 0x0068U && ordinary.screen_y == 0U)) {
        return 2;
    }

    EntityProjectionState full_screen{
        0x0080U, 0x0080U, 0x0010U, 0xFFFFU, 0xFFFFU};
    const auto full_visible = project_entity_screen_position(
        full_screen, 0x0080U, 0x0080U, EntityProjectionMode::FullScreen);
    if (!expect(full_visible.visible &&
                full_visible.source_result == 0x0080U &&
                full_screen.screen_x == 0x0080U &&
                full_screen.screen_y == 0x0090U)) {
        return 3;
    }

    full_screen.world_x = 0xF000U;
    full_screen.screen_x = 0x1234U;
    full_screen.screen_y = 0x5678U;
    const auto full_hidden = project_entity_screen_position(
        full_screen, 0x0080U, 0x0080U, EntityProjectionMode::FullScreen);
    if (!expect(!full_hidden.visible && !full_hidden.wrote_screen_x &&
                !full_hidden.wrote_screen_y &&
                full_hidden.source_result == 0xFF78U &&
                full_screen.screen_x == 0x1234U &&
                full_screen.screen_y == 0U)) {
        return 4;
    }

    return 0;
}
