#include "shadowrun_ghidra/movement_direction_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    const auto horizontal = choose_dominant_axis_directions(10, 4, 0, 0);
    if (!expect(horizontal.primary == 6 && horizontal.secondary == 0)) {
        return 1;
    }

    const auto negative_horizontal =
        choose_dominant_axis_directions(-10, 4, 0, 0);
    if (!expect(negative_horizontal.primary == 2 &&
                negative_horizontal.secondary == 0)) {
        return 2;
    }

    const auto vertical = choose_dominant_axis_directions(3, 12, 0, 0);
    if (!expect(vertical.primary == 0 && vertical.secondary == 6)) {
        return 3;
    }

    const auto negative_vertical =
        choose_dominant_axis_directions(3, -12, 0, 0);
    if (!expect(negative_vertical.primary == 4 &&
                negative_vertical.secondary == 6)) {
        return 4;
    }

    const auto tie = choose_dominant_axis_directions(5, -5, 0, 0);
    if (!expect(tie.primary == 6 && tie.secondary == 4)) {
        return 5;
    }

    return 0;
}
