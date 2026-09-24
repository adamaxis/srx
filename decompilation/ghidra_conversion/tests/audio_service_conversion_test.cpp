#include "shadowrun_ghidra/audio_service_conversion.hpp"

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

    return 0;
}
