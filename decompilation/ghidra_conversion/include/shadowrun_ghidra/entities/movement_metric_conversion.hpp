#pragma once

#include <cstdint>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct RelativeHeadingMeasurement {
    std::uint16_t heading = 0;
    // The source publishes this as D3.w. The host keeps a wider comparison
    // type for callers, but the conversion wraps the computed word first.
    std::uint32_t distance = 0;
};

// ROM 0x15CA4 and the record-based variant at 0x15CC0: measure two-axis
// movement with an orthogonal step cost of one and a diagonal contribution of
// one half, rounded up. The host inputs are normalized to the source .w
// subtraction/NEG.W/ASR.W/ADD.W behavior before returning the result.
std::uint32_t compute_diagonal_step_cost(
    std::int32_t delta_x,
    std::int32_t delta_y);

// ROM 0x15CC0: D2=+4(A0)-+4(A1), D3=(A0)-(A1), then the 0x15CA4 abs/ASR/ADD.
std::uint32_t compute_record_diagonal_step_cost(
    std::uint16_t origin_x,
    std::uint16_t origin_y,
    std::uint16_t target_x,
    std::uint16_t target_y);

// ROM 0x15D4E: derive the clockwise fixed-point heading and the source's
// half-major-axis distance from a signed delta pair. The original's
// 1024-entry arctangent correction table is represented as a typed host
// constant rather than as a ROM lookup.
RelativeHeadingMeasurement measure_heading_and_distance(
    std::int32_t delta_x,
    std::int32_t delta_y);

// ROM 0x15D3C: subtract two world positions, then apply 0x15D4E.
RelativeHeadingMeasurement measure_relative_heading_and_distance(
    std::int32_t origin_x,
    std::int32_t origin_y,
    std::int32_t target_x,
    std::int32_t target_y);

// ROM 0x14D58: load two entity records' world positions, preserve the
// caller's unrelated registers, and return their relative heading/distance.
RelativeHeadingMeasurement measure_entity_target_heading_and_distance(
    std::uint16_t entity_x,
    std::uint16_t entity_y,
    std::uint16_t target_x,
    std::uint16_t target_y);

// ROM 0x15C58: when the linked target has the source's active flag without
// its excluded bit, report whether its measured distance is below 0x200.
bool is_entity_target_within_range(
    std::uint8_t target_flags,
    std::int32_t entity_x,
    std::int32_t entity_y,
    std::int32_t target_x,
    std::int32_t target_y);

// ROM 0x15C84: if a linked target exists and passes 0x15C58, return the
// caller-supplied heading that the source writes to the entity record.
std::optional<std::uint16_t> update_entity_heading_from_linked_target(
    bool has_linked_target,
    std::uint8_t target_flags,
    std::uint16_t measured_heading,
    std::int32_t entity_x,
    std::int32_t entity_y,
    std::int32_t target_x,
    std::int32_t target_y);

} // namespace shadowrun::ghidra_conversion
