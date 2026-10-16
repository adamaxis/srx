#include "shadowrun_ghidra/entities/entity_projection_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

std::int32_t signed_word(const std::uint16_t value) {
    if (value < 0x8000U) {
        return static_cast<std::int32_t>(value);
    }
    return static_cast<std::int32_t>(value) - 0x10000;
}

std::uint16_t project_coordinate(const std::uint16_t world,
                                 const std::uint16_t camera) {
    const auto raw_delta = static_cast<std::uint16_t>(world - camera);
    const auto delta = signed_word(raw_delta);

    // This is the source's arithmetic word shift followed by its bit-3
    // rounding rule. Keep the intermediate wider than a signed 16-bit word;
    // only the final screen coordinate wraps back to a source word.
    auto cell_delta = delta >> 4;
    if (((raw_delta >> 3U) & 1U) != 0U) {
        ++cell_delta;
    }
    return static_cast<std::uint16_t>(cell_delta + 0x80);
}

bool inside_standard_window(const std::uint16_t x,
                            const std::uint16_t y) {
    const auto signed_x = signed_word(x);
    const auto signed_y = signed_word(y);
    return signed_x > 0x6F && signed_x < 0x1AE &&
           signed_y > 0x62 && signed_y < 0x17A;
}

} // namespace

EntityProjectionResult project_entity_screen_position(
    EntityProjectionState& state,
    const std::uint16_t camera_x,
    const std::uint16_t camera_y,
    const EntityProjectionMode mode) {
    const auto screen_x = project_coordinate(state.world_x, camera_x);
    const auto screen_y = static_cast<std::uint16_t>(
        state.sprite_offset_y + project_coordinate(state.world_y, camera_y));

    EntityProjectionResult result{};
    if (mode == EntityProjectionMode::StandardField) {
        state.screen_x = screen_x;
        state.screen_y = screen_y;
        result.wrote_screen_x = true;
        result.wrote_screen_y = true;
        result.visible = inside_standard_window(screen_x, screen_y);
        if (!result.visible) {
            state.screen_y = 0;
            result.source_result = screen_x;
            return result;
        }
        result.source_result = 1;
        return result;
    }

    const auto signed_x = signed_word(screen_x);
    if (signed_x >= 0 && signed_x < 0x200) {
        state.screen_x = screen_x;
        result.wrote_screen_x = true;
        const auto signed_y = signed_word(screen_y);
        if (signed_y >= 0 && signed_y < 0x1A0) {
            state.screen_y = screen_y;
            result.wrote_screen_y = true;
            result.visible = true;
        }
    }
    if (!result.visible) {
        state.screen_y = 0;
    }
    result.source_result = screen_x;
    return result;
}

EntityProjectionResult project_entity_screen_position_standard(
    EntityProjectionState& state,
    const std::uint16_t camera_x,
    const std::uint16_t camera_y) {
    return project_entity_screen_position(
        state, camera_x, camera_y, EntityProjectionMode::StandardField);
}

} // namespace shadowrun::ghidra_conversion
