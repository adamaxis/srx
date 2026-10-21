#include "shadowrun_ghidra/world/world_entry_sequence_conversion.hpp"

#include <vector>

namespace {

int expect(const bool condition) {
    return condition ? 0 : 1;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    const auto camera_step = [](const std::int32_t x,
                                const std::int32_t y) {
        return WorldEntryStep{WorldEntryStepKind::camera_adjustment, x, y};
    };
    const auto delay_step = [](const std::uint16_t additional_waits) {
        WorldEntryStep step{};
        step.kind = WorldEntryStepKind::delay;
        step.additional_wait_count = additional_waits;
        return step;
    };
    const auto render_step = [](const std::uint16_t x,
                                const std::uint16_t y) {
        WorldEntryStep step{};
        step.kind = WorldEntryStepKind::render_frame;
        step.sample_count_x = x;
        step.sample_count_y = y;
        return step;
    };

    WorldEntrySequenceState state{};
    state.world_origin_x = 100;
    state.world_origin_y = 200;
    std::vector<int> order;
    std::size_t selected_streams = 0U;
    const WorldEntrySequenceCallbacks callbacks{
        [&](const bool compact_layout) {
            ++selected_streams;
            if (compact_layout) {
                return WorldEntrySequencePlan{1U, {render_step(1U, 2U)}};
            }
            return WorldEntrySequencePlan{
                1U, {camera_step(7, 1), render_step(1U, 2U)}};
        },
        [&] { order.push_back(1); },
        [&] { order.push_back(2); },
        [&] { order.push_back(3); },
        [&] { order.push_back(4); },
        [&] { order.push_back(5); },
        [&] { order.push_back(6); },
        [&] { order.push_back(7); },
        [&] { order.push_back(8); }};

    if (expect(run_world_entry_sequence(state, false, callbacks) ==
                   WorldEntrySequenceAction::completed &&
               selected_streams == 1U && state.animation_marker == -1 &&
               state.world_origin_x == 93 && state.world_origin_y == 199 &&
               state.accumulated_x == 1 && state.accumulated_y == 2 &&
               state.display_transfer_pending && state.sprite_records_pending &&
               order.size() == 20U && order[0] == 1 && order[1] == 2 &&
               order[2] == 3 && order[3] == 4 && order[11] == 4 &&
               order[12] == 1 && order[15] == 4 && order[16] == 5 &&
               order[17] == 6 && order[18] == 7 && order[19] == 8) != 0) {
        return 1;
    }

    std::vector<int> delayed_order;
    const WorldEntrySequenceCallbacks consecutive_delay_callbacks{
        [&](bool) {
            return WorldEntrySequencePlan{
                1U, {camera_step(7, 1), delay_step(1U),
                     render_step(1U, 2U)}};
        },
        [&] { delayed_order.push_back(1); },
        [&] { delayed_order.push_back(2); },
        [&] { delayed_order.push_back(3); },
        [&] { delayed_order.push_back(4); },
        [&] { delayed_order.push_back(5); },
        [&] { delayed_order.push_back(6); },
        [&] { delayed_order.push_back(7); },
        [&] { delayed_order.push_back(8); },
    };
    if (expect(run_world_entry_sequence(state, false,
                                        consecutive_delay_callbacks) ==
                   WorldEntrySequenceAction::completed &&
               delayed_order.size() == 28U && delayed_order[0] == 1 &&
               delayed_order[11] == 4 && delayed_order[12] == 1 &&
               delayed_order[15] == 4 && delayed_order[16] == 1 &&
               delayed_order[23] == 4 && delayed_order[24] == 5 &&
               delayed_order[27] == 8) != 0) {
        return 4;
    }

    state = {};
    if (expect(run_world_entry_sequence(state, true, callbacks) ==
                   WorldEntrySequenceAction::completed &&
               state.accumulated_x == 1 && state.accumulated_y == 2) != 0) {
        return 2;
    }

    const auto make_endpoint_plan = [&](const std::vector<WorldEntryStep>&
                                            prelude,
                                        const std::uint16_t sample_x_total,
                                        const std::uint16_t sample_y_total) {
        WorldEntrySequencePlan plan{};
        plan.frame_count = 56U;
        plan.steps = prelude;
        const auto frame_count = plan.frame_count;
        const auto base_x = static_cast<std::uint16_t>(sample_x_total /
                                                        frame_count);
        const auto extra_x = static_cast<std::uint16_t>(sample_x_total %
                                                         frame_count);
        const auto base_y = static_cast<std::uint16_t>(sample_y_total /
                                                        frame_count);
        const auto extra_y = static_cast<std::uint16_t>(sample_y_total %
                                                         frame_count);
        for (std::uint16_t i = 0; i < frame_count; ++i) {
            plan.steps.push_back(render_step(
                static_cast<std::uint16_t>(base_x + (i < extra_x ? 1U : 0U)),
                static_cast<std::uint16_t>(base_y + (i < extra_y ? 1U : 0U))));
        }
        return plan;
    };

    const auto run_endpoint_case = [&](const bool compact_layout,
                                       const std::vector<WorldEntryStep>&
                                           prelude,
                                       const std::uint16_t sample_x_total,
                                       const std::uint16_t sample_y_total,
                                       const std::uint16_t expected_vblanks,
                                       const std::int32_t expected_origin_x,
                                       const std::int32_t expected_origin_y) {
        WorldEntrySequenceState endpoint_state{};
        endpoint_state.world_origin_x = 0x11E;
        endpoint_state.world_origin_y = 0xE4;
        std::uint16_t vblanks = 0U;
        std::uint16_t frame_dispatches = 0U;
        WorldEntrySequenceCallbacks endpoint_callbacks{};
        endpoint_callbacks.select_sequence_plan =
            [&](const bool) {
                return make_endpoint_plan(prelude,
                                          sample_x_total, sample_y_total);
            };
        endpoint_callbacks.wait_for_vblank = [&] { ++vblanks; };
        endpoint_callbacks.dispatch_frame_buffer = [&] { ++frame_dispatches; };
        endpoint_callbacks.write_frame_transfer = [&] { ++frame_dispatches; };
        endpoint_callbacks.advance_title_sprite_layers =
            [&] { ++frame_dispatches; };
        endpoint_callbacks.append_world_sprite_records =
            [&] { ++frame_dispatches; };
        const auto result = run_world_entry_sequence(
            endpoint_state, compact_layout, endpoint_callbacks);
        return result == WorldEntrySequenceAction::completed &&
               endpoint_state.world_origin_x == expected_origin_x &&
               endpoint_state.world_origin_y == expected_origin_y &&
               endpoint_state.accumulated_x == sample_x_total &&
               endpoint_state.accumulated_y == sample_y_total &&
               vblanks == expected_vblanks && frame_dispatches == 224U;
    };

    // ROM 0x2679E reads the packed command stream after its 0x37 count byte.
    // The listed semantic steps are independently decoded source facts; in
    // particular, a high bit in a vertical sample byte does not make it a
    // camera command. Zero vertical samples consume the A4 delay stream.
    std::vector<WorldEntryStep> primary_prelude;
    for (std::uint16_t i = 0U; i < 11U; ++i) {
        primary_prelude.push_back(camera_step(1, i == 3U ? 0 : 1));
    }
    std::vector<WorldEntryStep> compact_prelude{
        camera_step(1, 1), camera_step(1, 1), camera_step(1, 1),
        camera_step(1, 2), camera_step(1, 1), camera_step(1, 2),
        camera_step(1, 1), camera_step(1, 2), camera_step(2, 1),
        camera_step(1, 2), camera_step(7, 7),
    };
    for (const auto waits : {14U, 12U, 10U, 8U, 6U, 4U}) {
        compact_prelude.push_back(delay_step(
            static_cast<std::uint16_t>(waits)));
    }

    if (!run_endpoint_case(false, primary_prelude, 109U, 386U, 179U,
                           0x113, 0xDA) ||
        !run_endpoint_case(true, compact_prelude, 84U, 410U, 239U,
                           0x10C, 0xCF)) {
        return 5;
    }

    std::uint16_t invalid_plan_callbacks = 0U;
    const WorldEntrySequenceCallbacks invalid_callbacks{
        [](bool) { return WorldEntrySequencePlan{1U, {}}; },
        [&] { ++invalid_plan_callbacks; }};
    state.animation_marker = 6;
    if (expect(run_world_entry_sequence(state, false, invalid_callbacks) ==
                   WorldEntrySequenceAction::invalid_plan &&
               invalid_plan_callbacks == 0U && state.animation_marker == 6) != 0) {
        return 3;
    }
    return 0;
}
