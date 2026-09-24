#include "shadowrun_ghidra/movement_metric_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    if (!expect(compute_diagonal_step_cost(0, 0) == 0U)) {
        return 1;
    }
    if (!expect(compute_diagonal_step_cost(10, 4) == 12U)) {
        return 2;
    }
    if (!expect(compute_diagonal_step_cost(4, 10) == 12U)) {
        return 3;
    }
    if (!expect(compute_diagonal_step_cost(-10, 3) == 12U)) {
        return 4;
    }
    if (!expect(compute_diagonal_step_cost(-5, -5) == 8U)) {
        return 5;
    }
    if (!expect(compute_diagonal_step_cost(7, 2) == 8U)) {
        return 6;
    }

    return 0;
}
