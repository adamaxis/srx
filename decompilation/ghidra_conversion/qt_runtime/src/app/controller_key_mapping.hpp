#pragma once

#include "shadowrun_ghidra/menus/startup_menu_conversion.hpp"

#include <cstdint>
#include <QtCore/Qt>

namespace shadowrun::ghidra_conversion::qt_runtime {

inline constexpr std::uint8_t kTitleStartButton = 0x80U;

inline std::uint8_t sampled_menu_byte(const int key) noexcept {
    switch (key) {
    case Qt::Key_Up:
        return kStartupMenuUp;
    case Qt::Key_Down:
        return kStartupMenuDown;
    case Qt::Key_A:
    case Qt::Key_Z:
        return kStartupMenuA;
    case Qt::Key_X:
        return kStartupMenuB;
    case Qt::Key_C:
        return kStartupMenuC;
    case Qt::Key_Space:
    case Qt::Key_Return:
    case Qt::Key_Enter:
        return kStartupMenuA;
    default:
        return 0U;
    }
}

inline std::uint8_t sampled_start_options_byte(const int key) noexcept {
    if (key == Qt::Key_Left) return kStartupMenuLeft;
    if (key == Qt::Key_Right) return kStartupMenuRight;
    return sampled_menu_byte(key);
}

// The title loop breaks on Genesis Start; other menu keys remain A/B/C.
inline std::uint8_t sampled_title_byte(const int key) noexcept {
    switch (key) {
    case Qt::Key_Space:
    case Qt::Key_Return:
    case Qt::Key_Enter:
        return kTitleStartButton;
    default:
        return sampled_menu_byte(key);
    }
}

inline std::uint8_t sampled_field_direction_bit(const int key) noexcept {
    switch (key) {
    case Qt::Key_Up:
        return 0x01U;
    case Qt::Key_Down:
        return 0x02U;
    case Qt::Key_Left:
        return 0x04U;
    case Qt::Key_Right:
        return 0x08U;
    default:
        return 0U;
    }
}

// In field mode Space/Enter map to Genesis Start, distinct from A.
inline std::uint8_t sampled_field_button_byte(const int key) noexcept {
    switch (key) {
    case Qt::Key_Space:
    case Qt::Key_Return:
    case Qt::Key_Enter:
        return 0x80U;
    default:
        return sampled_menu_byte(key);
    }
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
