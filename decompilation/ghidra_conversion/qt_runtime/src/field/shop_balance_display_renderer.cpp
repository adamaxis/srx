#include "shop_balance_display_renderer.hpp"

#include "numeric_visual_renderer.hpp"

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

constexpr int kBalanceRightEdge = 184;
constexpr int kBalanceY = 200;

} // namespace

void draw_field_shop_balance(QPainter& painter, const QImage& font,
                             const std::uint32_t balance,
                             const std::optional<QColor> tint) {
    // ROM 0x378/0x384 sets Plane A to $C000 with 64 columns. Shop 0xC406
    // starts the 0xC216 decimal stream at $CCAC (row 25, column 22); 0xC216
    // writes the remaining digits leftward from that rightmost cell.
    draw_numeric_visual(painter, font, kBalanceRightEdge, kBalanceY, balance,
                        NumericVisualAlignment::right, true, tint);
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
