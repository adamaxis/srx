#include "shadowrun_ghidra/audio/audio_service_conversion.hpp"

#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    AudioServiceTickState tick{0xFFU};
    std::vector<int> tick_order;
    const AudioServiceCallbacks tick_callbacks{
        [&tick_order] { tick_order.push_back(1); },
        [&tick_order] { tick_order.push_back(2); },
        [&tick_order] { tick_order.push_back(3); },
        [&tick_order] { tick_order.push_back(4); },
    };
    if (!advance_audio_service_tick(tick, true, tick_callbacks) ||
        tick.phase != 0U || tick_order != std::vector<int>{1, 2, 3, 4}) {
        return 1;
    }

    std::vector<int> order;
    const AudioServiceCallbacks callbacks{
        [&order] { order.push_back(1); },
        [&order] { order.push_back(2); },
        [&order] { order.push_back(3); },
        [&order] { order.push_back(4); },
    };
    if (!expect(!service_audio_if_enabled(false, callbacks) &&
                order.empty() && service_audio_if_enabled(true, callbacks) &&
                order == std::vector<int>{1, 2, 3, 4})) {
        return 2;
    }

    if (!expect(!service_audio_command_queue({}) &&
                !service_audio_if_enabled(true, {}))) {
        return 3;
    }

    EntityAudioState entity{0x01U, 0x0EU, 9U, 0U, 0x1234U, 0x5678U,
                            0U, 0U, 0xFFFFU};
    bool serviced = false;
    activate_entity_audio_state(entity,
                                [&serviced] { serviced = true; });
    if (entity.flags != 0x81U || entity.behavior_timer != 0x10U ||
        entity.update_marker != 0U || entity.destination_x != 0x1234U ||
        entity.destination_y != 0x5678U || entity.effect_word != 0U ||
        !serviced) {
        return 4;
    }

    EntityAudioEffectRecord jitter{0x80U, 0U, -1, 0x1111U, 0x2222U,
                                   0U, 0U, 0x0100U, 0x0200U, 2U, 0U, 0, 0};
    int expire_count = 0;
    int retrigger_count = 0;
    int motion_count = 0;
    const auto jittered = advance_entity_audio_effect(
        jitter, 0x0011U,
        [&expire_count] { ++expire_count; },
        [&retrigger_count] { ++retrigger_count; },
        [&motion_count] { ++motion_count; });
    // 0x11 bit 0 set → NEG 0x11 = -0x11. LSR#4 of 0x11 is 1, bit 0 set → -1.
    if (jittered != EntityAudioEffectResult::jittered ||
        jitter.x != static_cast<std::uint16_t>(0x0100U - 0x11U) ||
        jitter.y != static_cast<std::uint16_t>(0x0200U - 1U) ||
        jitter.phase != 1U || jitter.aux != 0U || jitter.rate != 0U ||
        (jitter.flags2 & 0x01U) == 0U || jitter.timer != 0x7FFFU ||
        expire_count != 0 || retrigger_count != 0 || motion_count != 0) {
        return 5;
    }

    EntityAudioEffectRecord dying{0x81U, 0U, -1, 0U, 0U, 0U, 0U, 0U, 0U,
                                  0U, 0U, 0, 0};
    if (advance_entity_audio_effect(
            dying, 0U, [&expire_count] { ++expire_count; }, {}, {}) !=
            EntityAudioEffectResult::expired ||
        (dying.flags & 0x01U) != 0U || expire_count != 1) {
        return 6;
    }

    EntityAudioEffectRecord hold{0U, 0U, 0, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U,
                                 1, 0};
    if (advance_entity_audio_effect(hold, 0U, {}, {}, {}) !=
            EntityAudioEffectResult::waiting ||
        hold.lifetime != 0) {
        return 7;
    }
    EntityAudioEffectRecord bounce{0U, 0U, 0, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U,
                                   0, 1};
    if (advance_entity_audio_effect(
            bounce, 0U, {}, {}, [&motion_count] { ++motion_count; }) !=
            EntityAudioEffectResult::motion ||
        motion_count != 1) {
        return 8;
    }
    EntityAudioEffectRecord retrigger{0U, 0U, 0, 0U, 0U, 0U, 0U, 0U, 0U, 0U,
                                      0U, 0, 0};
    if (advance_entity_audio_effect(
            retrigger, 0U, {},
            [&retrigger_count] { ++retrigger_count; }, {}) !=
            EntityAudioEffectResult::retriggered ||
        retrigger_count != 1) {
        return 9;
    }

    return 0;
}
