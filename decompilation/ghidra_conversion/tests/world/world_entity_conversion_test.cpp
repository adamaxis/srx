#include "shadowrun_ghidra/world/world_entity_conversion.hpp"

#include <array>
#include <vector>

using namespace shadowrun::ghidra_conversion;

int main() {
    WorldFrameState world{true, true, true, true, false, true, true, 2};
    std::vector<int> events;
    const WorldFrameCallbacks callbacks{
        [&] { events.push_back(1); }, [&] { events.push_back(2); },
        [&] { events.push_back(3); }, [&] { events.push_back(4); },
        [&] { events.push_back(5); }, [&] { events.push_back(6); },
        [&] { events.push_back(7); }, [&] { events.push_back(8); },
        [&] { events.push_back(9); }, [&] { events.push_back(10); },
        [&] { events.push_back(11); }, [&] { events.push_back(12); },
        [&] { events.push_back(13); }, [&] { events.push_back(14); },
        [&] { events.push_back(15); }, [&] { events.push_back(16); },
        [&] { events.push_back(17); }};
    run_world_frame_services(world, callbacks);
    if (world.audio_pending || world.world_dirty || world.effect_transition_pending ||
        world.compressed_entity_data_pending || events.size() != 17 ||
        events[0] != 1 || events[1] != 2 || events.back() != 17) {
        return 1;
    }

    std::array<EntityFrameRecord, 16> entities{};
    entities[0] = {true, 1, 0xF0, 1, 2, 0, false};
    int expanded = 0;
    update_entity_frame(entities, {
        {}, {}, {}, [&](EntityFrameRecord&) {}, {}, {}, {}, {}, {},
        [&](EntityFrameRecord&) { ++expanded; }, {}, {}, {}});
    if (expanded != 1) {
        return 2;
    }

    EffectAnimationState animation{0, 0, 1, 0, true};
    int builds = 0;
    for (int i = 0; i < 8; ++i) {
        advance_effect_animation(animation, {}, {}, [&] { ++builds; });
    }
    if (animation.active || builds == 0) {
        return 3;
    }

    JitterEffectState jitter{};
    jitter.animation = {0, 1, 3, 0, true};
    jitter.remaining = 2;
    int jitter_builds = 0;
    advance_jitter_effect(jitter, [](std::uint32_t& state) {
        state = 0x12345678U;
        return state;
    }, {}, [&] { ++jitter_builds; });
    if (jitter_builds != 1) {
        return 4;
    }
    return 0;
}
