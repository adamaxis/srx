#include "vdp_model.hpp"

#include <QColor>
#include <QPainter>

#include <iostream>

namespace {

bool expect(const bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
        return false;
    }
    return true;
}

std::uint16_t cram(const std::uint16_t red, const std::uint16_t green,
                   const std::uint16_t blue) {
    return static_cast<std::uint16_t>((red << 1U) | (green << 5U) |
                                      (blue << 9U));
}

shadowrun::ghidra_conversion::qt_runtime::VdpModel::Pattern solid(
    const std::uint8_t color) {
    shadowrun::ghidra_conversion::qt_runtime::VdpModel::Pattern pattern{};
    pattern.fill(color);
    return pattern;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion::qt_runtime;

    const VdpNameTableEntry source_entry{true, 2U, true, true, 0x05ABU};
    const auto decoded = VdpNameTableEntry::decode(source_entry.encode());
    if (!expect(decoded.priority && decoded.palette == 2U &&
                    decoded.vertical_flip && decoded.horizontal_flip &&
                    decoded.tile_index == 0x05ABU,
                "Genesis name-word fields did not round-trip")) {
        return 1;
    }
    const auto maximum_entry = VdpNameTableEntry::decode(0xFFFFU);
    if (!expect(maximum_entry.priority && maximum_entry.palette == 3U &&
                    maximum_entry.vertical_flip &&
                    maximum_entry.horizontal_flip &&
                    maximum_entry.tile_index == 0x07FFU,
                "name-word tile index must be the low eleven bits")) {
        return 1;
    }

    VdpModel model;
    model.set_cram_word(0U, 1U, cram(7U, 0U, 0U)); // red
    model.set_cram_word(0U, 2U, cram(0U, 7U, 0U)); // green
    model.set_cram_word(0U, 3U, cram(0U, 0U, 7U)); // blue
    model.set_cram_word(0U, 4U, cram(7U, 7U, 0U)); // yellow
    model.set_cram_word(0U, 5U, cram(7U, 0U, 7U)); // magenta
    model.set_cram_word(0U, 6U, cram(0U, 7U, 7U)); // cyan
    model.set_cram_word(0U, 7U, cram(7U, 7U, 7U)); // white
    model.set_pattern(1U, solid(1U));
    model.set_pattern(2U, solid(2U));
    model.set_pattern(3U, solid(3U));
    model.set_pattern(4U, solid(4U));
    model.set_pattern(5U, solid(5U));
    model.set_pattern(6U, solid(6U));
    model.set_pattern(7U, solid(7U));

    model.set_name_word(VdpPlane::plane_b, 0U, 0U,
                        VdpNameTableEntry{false, 0U, false, false, 1U}.encode());
    model.set_name_word(VdpPlane::plane_a, 1U, 0U,
                        VdpNameTableEntry{false, 0U, false, false, 2U}.encode());
    model.set_name_word(VdpPlane::plane_b, 1U, 0U,
                        VdpNameTableEntry{true, 0U, false, false, 4U}.encode());
    model.set_name_word(VdpPlane::plane_b, 2U, 0U,
                        VdpNameTableEntry{true, 0U, false, false, 4U}.encode());
    model.set_name_word(VdpPlane::plane_a, 2U, 0U,
                        VdpNameTableEntry{true, 0U, false, false, 5U}.encode());
    model.set_name_word(VdpPlane::plane_a, 3U, 0U,
                        VdpNameTableEntry{true, 0U, false, false, 5U}.encode());
    model.set_name_word(VdpPlane::window, 3U, 0U,
                        VdpNameTableEntry{false, 0U, false, false, 6U}.encode());
    model.set_name_word(VdpPlane::window, 4U, 0U,
                        VdpNameTableEntry{false, 0U, false, false, 6U}.encode());
    model.set_name_word(VdpPlane::plane_b, 5U, 0U,
                        VdpNameTableEntry{false, 0U, false, false, 1U}.encode());
    model.set_name_word(VdpPlane::plane_a, 5U, 0U,
                        VdpNameTableEntry{false, 0U, false, false, 2U}.encode());

    QImage composed;
    model.compose(
        composed,
        [](QPainter& painter) {
            painter.fillRect(0, 0, 8, 8, QColor(0, 0, 255));
        },
        [](QPainter& painter) {
            painter.fillRect(32, 0, 8, 8, QColor(255, 255, 255));
        });
    if (!expect(composed.size() == QSize(320, 224),
                "compose must produce the H40 320x224 field surface") ||
        !expect(composed.pixelColor(1, 1) == QColor(0, 0, 255),
                "low sprites must cover low-priority B and A") ||
        !expect(composed.pixelColor(9, 1) == QColor(255, 255, 0),
                "high-priority B must cover low-priority A") ||
        !expect(composed.pixelColor(17, 1) == QColor(255, 0, 255),
                "high-priority A must cover high-priority B") ||
        !expect(composed.pixelColor(25, 1) == QColor(0, 255, 255),
                "window plane must follow both high-priority map passes") ||
        !expect(composed.pixelColor(33, 1) == QColor(255, 255, 255),
                "high sprites must follow the window plane") ||
        !expect(composed.pixelColor(41, 1) == QColor(0, 255, 0),
                "equal-priority A must follow B") ||
        !expect(composed.pixelColor(49, 1) == QColor(0, 0, 0),
                "transparent color zero must reveal the backdrop")) {
        return 1;
    }

    VdpModel scroll_model;
    scroll_model.set_cram_word(0U, 1U, cram(7U, 0U, 0U));
    scroll_model.set_cram_word(0U, 2U, cram(0U, 7U, 0U));
    scroll_model.set_cram_word(0U, 3U, cram(0U, 0U, 7U));
    scroll_model.set_pattern(1U, solid(1U));
    scroll_model.set_pattern(2U, solid(2U));
    scroll_model.set_pattern(3U, solid(3U));
    scroll_model.set_name_word(VdpPlane::plane_b, 0U, 0U, 1U);
    scroll_model.set_name_word(VdpPlane::plane_b, 0U, 1U, 2U);
    scroll_model.set_name_word(VdpPlane::plane_a, 1U, 0U, 3U);
    scroll_model.set_plane_b_vertical_scroll_offset(8U);
    if (!expect(scroll_model.scroll().plane_a_vertical == 0 &&
                    scroll_model.scroll().plane_b_vertical == 8,
                "FFDD0E must update plane B vertical scroll independently")) {
        return 1;
    }
    QImage scrolled;
    scroll_model.compose(scrolled);
    if (!expect(scrolled.pixelColor(0, 0) == QColor(0, 255, 0),
                "plane B must use the direct FFDD0E vertical scroll input") ||
        !expect(scrolled.pixelColor(8, 0) == QColor(0, 0, 255),
                "plane A must keep its independent zero vertical scroll")) {
        return 1;
    }

    VdpSatScrollFrame latched_frame{};
    latched_frame.scroll.plane_b_vertical = 8;
    scroll_model.set_scroll({});
    QImage latched;
    scroll_model.compose(latched, {}, {}, &latched_frame);
    if (!expect(latched.pixelColor(0, 0) == QColor(0, 255, 0),
                "compose must use the supplied VBlank-latched scroll frame")) {
        return 1;
    }

    return 0;
}
