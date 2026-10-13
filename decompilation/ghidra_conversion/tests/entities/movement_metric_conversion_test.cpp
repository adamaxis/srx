#include "shadowrun_ghidra/entities/movement_metric_conversion.hpp"

#include <array>

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
    if (!expect(compute_record_diagonal_step_cost(10U, 4U, 0U, 0U) == 12U &&
                compute_record_diagonal_step_cost(0U, 0U, 10U, 4U) == 12U)) {
        return 20;
    }

    const auto negative_vertical = measure_heading_and_distance(0, -100);
    if (!expect(negative_vertical.heading == 0x0000U &&
                negative_vertical.distance == 100U)) {
        return 7;
    }
    const auto positive_horizontal = measure_heading_and_distance(100, 0);
    if (!expect(positive_horizontal.heading == 0x4000U &&
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
    if (!expect(relative.heading == 0x8000U && relative.distance == 100U)) {
        return 11;
    }
    const auto entity_target = measure_entity_target_heading_and_distance(
        0x0100U, 0x0200U, 0x0100U, 0x0264U);
    if (!expect(entity_target.heading == 0x8000U &&
                entity_target.distance == 100U)) {
        return 17;
    }
    const auto quantized = measure_heading_and_distance(-64, -63);
    if (!expect(quantized.heading == 0xDF00U && quantized.distance == 95U)) {
        return 12;
    }

    // Cardinal outputs must agree with the ROM's eight movement ordinals:
    // up, northeast, right, southeast, down, southwest, left, northwest.
    const std::array<std::array<std::int32_t, 3>, 8> cardinal_headings{{
        {{0, -100, 0x0000}}, {{100, -100, 0x2000}},
        {{100, 0, 0x4000}}, {{100, 100, 0x6000}},
        {{0, 100, 0x8000}}, {{-100, 100, 0xA000}},
        {{-100, 0, 0xC000}}, {{-100, -100, 0xE000}},
    }};
    for (const auto& sample : cardinal_headings) {
        if (!expect(measure_heading_and_distance(sample[0], sample[1]).heading ==
                    static_cast<std::uint16_t>(sample[2]))) {
            return 22;
        }
    }

    // D0 is a word result in 0x15D6C, so the host representation must wrap
    // the distance before exposing it to callers.
    if (!expect(measure_heading_and_distance(0x7FFF, 0x7FFF).distance ==
                0xBFFEU)) {
        return 21;
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
