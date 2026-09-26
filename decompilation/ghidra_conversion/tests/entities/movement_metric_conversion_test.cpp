#include "shadowrun_ghidra/entities/movement_metric_conversion.hpp"

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
    if (!expect(compute_diagonal_step_cost(0x1000AU, 4) == 12U &&
                compute_diagonal_step_cost(-65526, 4) == 12U)) {
        return 18;
    }
    // NEG.W cannot represent the positive magnitude of 0x8000.  The source
    // therefore ASR.Ws the wrapped word and ADD.Ws the result back into D0.
    if (!expect(compute_diagonal_step_cost(-32768, -32768) == 0x4000U)) {
        return 19;
    }

    const auto negative_vertical = measure_heading_and_distance(0, -100);
    if (!expect(negative_vertical.heading == 0x4000U &&
                negative_vertical.distance == 100U)) {
        return 7;
    }
    const auto positive_horizontal = measure_heading_and_distance(100, 0);
    if (!expect(positive_horizontal.heading == 0x8000U &&
                positive_horizontal.distance == 100U)) {
        return 8;
    }
    const auto southwest = measure_heading_and_distance(-100, 100);
    if (!expect(southwest.heading == 0xA000U && southwest.distance == 150U)) {
        return 9;
    }
    const auto zero = measure_heading_and_distance(0, 0);
    if (!expect(zero.heading == 0U && zero.distance == 0U)) {
        return 10;
    }
    const auto relative = measure_relative_heading_and_distance(10, 20, 10, 120);
    if (!expect(relative.heading == 0xC000U && relative.distance == 100U)) {
        return 11;
    }
    const auto entity_target = measure_entity_target_heading_and_distance(
        0x0100U, 0x0200U, 0x0100U, 0x0264U);
    if (!expect(entity_target.heading == 0xC000U &&
                entity_target.distance == 100U)) {
        return 17;
    }
    const auto quantized = measure_heading_and_distance(-64, -63);
    if (!expect(quantized.heading == 0xE100U && quantized.distance == 95U)) {
        return 12;
    }

    if (!expect(is_entity_target_within_range(
                    0x01U, 0, 0, 100, 0))) {
        return 13;
    }
    if (!expect(!is_entity_target_within_range(
                    0x41U, 0, 0, 100, 0) &&
                !is_entity_target_within_range(
                    0x00U, 0, 0, 100, 0) &&
                !is_entity_target_within_range(
                    0x01U, 0, 0, 0x400, 0))) {
        return 14;
    }
    const auto updated_heading = update_entity_heading_from_linked_target(
        true, 0x01U, 0xA000U, 0, 0, 100, 0);
    if (!expect(updated_heading.has_value() &&
                updated_heading.value() == 0xA000U)) {
        return 15;
    }
    if (!expect(!update_entity_heading_from_linked_target(
                     false, 0x01U, 0xA000U, 0, 0, 100, 0)
                     .has_value())) {
        return 16;
    }

    return 0;
}
