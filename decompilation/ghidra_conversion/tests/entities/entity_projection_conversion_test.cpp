#include "shadowrun_ghidra/entities/entity_projection_conversion.hpp"

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

    const auto check_standard_projection = [](
        const std::uint16_t world_x,
        const std::uint16_t world_y,
        const std::uint16_t sprite_offset_y,
        const std::uint16_t expected_screen_x,
        const std::uint16_t expected_screen_y,
        const std::uint16_t expected_source_result,
        const bool expected_visible) {
        EntityProjectionState state{
            world_x, world_y, sprite_offset_y, 0xFFFFU, 0xFFFFU};
        const auto result = project_entity_screen_position_standard(
            state, 0U, 0U);
        return result.visible == expected_visible &&
               result.source_result == expected_source_result &&
               result.wrote_screen_x && result.wrote_screen_y &&
               state.screen_x == expected_screen_x &&
               state.screen_y == (expected_visible ? expected_screen_y : 0U);
    };

    // ROM 0x5A98 / 0x5AA0: standard-field anchor x is >= 0x70 and <= 0x1AD.
    // Hidden projections clear +0x0C at 0x5ABE and return sx in D0.
    if (!expect(check_standard_projection(
            0xFEF0U, 0U, 0U, 0x006FU, 0x0080U, 0x006FU, false))) {
        return 5;
    }
    if (!expect(check_standard_projection(
            0xFF00U, 0U, 0U, 0x0070U, 0x0080U, 1U, true))) {
        return 6;
    }
    if (!expect(check_standard_projection(
            0x12D0U, 0U, 0U, 0x01ADU, 0x0080U, 1U, true))) {
        return 7;
    }
    if (!expect(check_standard_projection(
            0x12E0U, 0U, 0U, 0x01AEU, 0x0080U, 0x01AEU, false))) {
        return 8;
    }

    // ROM 0x5AA8 / 0x5AB0: standard-field anchor y is >= 0x63 and < 0x17A.
    if (!expect(check_standard_projection(
            0U, 0xFE20U, 0U, 0x0080U, 0x0062U, 0x0080U, false))) {
        return 9;
    }
    if (!expect(check_standard_projection(
            0U, 0xFE30U, 0U, 0x0080U, 0x0063U, 1U, true))) {
        return 10;
    }
    if (!expect(check_standard_projection(
            0U, 0x0F90U, 0U, 0x0080U, 0x0179U, 1U, true))) {
        return 11;
    }
    if (!expect(check_standard_projection(
            0U, 0x0FA0U, 0U, 0x0080U, 0x017AU, 0x0080U, false))) {
        return 12;
    }

    // ROM 0xB594 supplies -0x14: it moves this anchor from sy=0x70 to 0x5C.
    EntityProjectionState offset_case{
        0U, 0xFF00U, 0U, 0xFFFFU, 0xFFFFU};
    const auto unoffset = project_entity_screen_position_standard(
        offset_case, 0U, 0U);
    if (!expect(unoffset.visible && offset_case.screen_y == 0x0070U)) {
        return 13;
    }
    offset_case.sprite_offset_y = 0xFFECU;
    const auto offset_hidden = project_entity_screen_position_standard(
        offset_case, 0U, 0U);
    if (!expect(!offset_hidden.visible &&
                offset_hidden.source_result == 0x0080U &&
                offset_case.screen_x == 0x0080U &&
                offset_case.screen_y == 0U)) {
        return 14;
    }

    return 0;
}
