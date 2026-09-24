#include "shadowrun_ghidra/menu_slot_batch_conversion.hpp"

#include <cstdint>
#include <vector>

using namespace shadowrun::ghidra_conversion;

int main() {
    MenuSlotCommandState command{1, {0, 3, 5, 6, 7}, 0x27, 0xFF, 0};
    std::vector<int> events;
    apply_menu_slot_command(
        command,
        {[&] { events.push_back(1); }, [&] { events.push_back(2); },
         [&] { events.push_back(3); }});
    if (command.slot_codes[1] != 0xFF || command.slot_timer != 6 ||
        (command.mode_flags & 0x20U) != 0 || command.prompt_state != 0 ||
        events != std::vector<int>{1, 2, 3}) {
        return 1;
    }
    command.slot_timer = 0;
    if (clear_ready_menu_slot_timer(command, 0, [&] { events.push_back(4); }) ||
        command.slot_timer != 0) {
        return 2;
    }
    command.slot_timer = 6;
    if (!clear_ready_menu_slot_timer(command, -1, [&] { events.push_back(5); }) ||
        command.slot_timer != 0) {
        return 2;
    }

    clear_menu_status_at([&] { events.push_back(6); },
                         [&] { events.push_back(7); });
    publish_menu_selection_then_text([&] { events.push_back(8); },
                                     [&] { events.push_back(9); });
    refresh_mode_metadata_leaf([&] { events.push_back(10); });
    run_menu_service_then_prepare([&] { events.push_back(11); },
                                  [&] { events.push_back(12); });
    if (events.back() != 12) {
        return 3;
    }

    command.slot_codes = {0, 6, 0, 6, 0};
    int finalized = 0;
    if (!finalize_menu_slot_code(
            command, 6, 2,
            {[&] { ++finalized; }, [&] { ++finalized; }, [&] { ++finalized; }}) ||
        command.slot_codes[3] != 0 || command.slot_timer != 20 ||
        finalized != 3) {
        return 4;
    }
    MenuCountdownState countdown{2};
    int expiries = 0;
    if (advance_menu_countdown(countdown, [&] { ++expiries; }) ||
        !advance_menu_countdown(countdown, [&] { ++expiries; }) || expiries != 1) {
        return 5;
    }

    std::vector<std::uint8_t> records(0x28U, 1U);
    clear_selected_menu_record(records, 1);
    if (records[0x14] != 0 || records[0] != 1) {
        return 6;
    }

    EffectDefaultsState defaults{};
    initialize_effect_defaults(defaults);
    if (!defaults.active || defaults.origin_x != 0x1200 ||
        defaults.origin_y != 0x0D00 || defaults.lifetime != -1) {
        return 7;
    }
    EffectEntityResetState reset{false, 4, 8};
    reset_effect_entity(reset);
    if (!reset.active || reset.accumulator != 0 || reset.entity_value != 0) {
        return 8;
    }
    if (arm_effect_after_threshold(defaults, 0x713F) ||
        !arm_effect_after_threshold(defaults, 0x7140)) {
        return 9;
    }

    WordStreamResetState stream{{1, 2}, 0};
    int stream_writes = 0;
    reset_word_stream_area(stream, [&] { ++stream_writes; });
    if (stream.words.size() != 0x1C0U || stream.words.front() != 0 ||
        stream_writes != 1 || stream.dirty_flags != 0x0C) {
        return 10;
    }

    BehaviorRecordState behavior{};
    int audio = 0;
    int initialized = 0;
    attach_behavior_descriptor_a(behavior, {[&] { ++audio; }, [&] { ++initialized; }});
    if (audio != 1 || initialized != 1 || behavior.width != 4 ||
        behavior.update_descriptor != 0x1C33A ||
        behavior.draw_descriptor != 0x1C330) {
        return 11;
    }
    attach_behavior_descriptor_b(behavior, {});
    if (behavior.update_descriptor != 0x1C29E) {
        return 12;
    }
    attach_behavior_descriptor_c(behavior, {});
    attach_behavior_descriptor_d(behavior, {});
    attach_behavior_descriptor_e(behavior, {});
    if (behavior.update_descriptor != 0x1C34A ||
        behavior.draw_descriptor != 0x1C344) {
        return 13;
    }
    return 0;
}
