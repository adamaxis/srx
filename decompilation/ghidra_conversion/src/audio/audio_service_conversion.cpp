#include "shadowrun_ghidra/audio/audio_service_conversion.hpp"

namespace shadowrun::ghidra_conversion {

bool advance_audio_service_tick(
    AudioServiceTickState& state,
    const bool audio_enabled,
    const AudioServiceCallbacks& callbacks) {
    state.phase = static_cast<std::uint8_t>(state.phase + 1U);
    return service_audio_if_enabled(audio_enabled, callbacks);
}

bool service_audio_command_queue(const AudioServiceCallbacks& callbacks) {
    if (!callbacks.begin_bus_access || !callbacks.flush_audio_command ||
        !callbacks.append_audio_command || !callbacks.end_bus_access) {
        return false;
    }

    callbacks.begin_bus_access();
    callbacks.flush_audio_command();
    callbacks.append_audio_command();
    callbacks.end_bus_access();
    return true;
}

bool service_audio_if_enabled(
    const bool audio_enabled,
    const AudioServiceCallbacks& callbacks) {
    return audio_enabled && service_audio_command_queue(callbacks);
}

void activate_entity_audio_state(
    EntityAudioState& state,
    const std::function<void()>& service_audio) {
    state.flags = static_cast<std::uint8_t>(state.flags | 0x80U);
    state.behavior_timer =
        state.behavior_variant == 0x0EU ? 0x0010U : 0x000CU;
    state.update_marker = 0U;
    state.destination_x = state.current_x;
    state.destination_y = state.current_y;
    state.effect_word = 0U;
    if (service_audio) {
        service_audio();
    }
}

namespace {

std::uint16_t signed_jitter(const std::uint16_t magnitude) {
    auto value = static_cast<std::int16_t>(magnitude & 0x7FU);
    if ((value & 1) != 0) {
        value = static_cast<std::int16_t>(-value);
    }
    return static_cast<std::uint16_t>(value);
}

} // namespace

EntityAudioEffectResult advance_entity_audio_effect(
    EntityAudioEffectRecord& state,
    const std::uint16_t random_word,
    const std::function<void()>& expire,
    const std::function<void()>& retrigger,
    const std::function<void()>& motion) {
    if ((state.flags & 0x80U) == 0U) {
        --state.lifetime;
        if (state.lifetime >= 0) {
            return EntityAudioEffectResult::waiting;
        }
        --state.bounce;
        if (state.bounce >= 0) {
            if (motion) {
                motion();
            }
            return EntityAudioEffectResult::motion;
        }
        if (retrigger) {
            retrigger();
        }
        return EntityAudioEffectResult::retriggered;
    }

    ++state.jitter_count;
    if (state.timer >= 0) {
        return EntityAudioEffectResult::waiting;
    }
    --state.phase;
    if (static_cast<std::int8_t>(state.phase) < 0) {
        state.flags = static_cast<std::uint8_t>(state.flags & ~0x01U);
        if (expire) {
            expire();
        }
        return EntityAudioEffectResult::expired;
    }

    state.x = static_cast<std::uint16_t>(
        state.dest_x + signed_jitter(random_word));
    state.y = static_cast<std::uint16_t>(
        state.dest_y + signed_jitter(static_cast<std::uint16_t>(random_word >> 4U)));
    state.aux = 0U;
    state.timer = static_cast<std::int16_t>(
        static_cast<std::uint16_t>(state.timer) & 0x7FFFU);
    state.rate = 0U;
    state.flags2 = static_cast<std::uint8_t>(state.flags2 | 0x01U);
    return EntityAudioEffectResult::jittered;
}

void apply_spawned_effect_bounce(EntityAudioEffectRecord& state) {
    state.lift = static_cast<std::int16_t>(state.lift + 0x20);
    state.lift_sum = static_cast<std::int16_t>(state.lift_sum + state.lift);
    state.height = static_cast<std::int16_t>(state.lift_sum >> 8);
    if (state.height < 0) {
        return;
    }
    if (state.lift < 0x40) {
        state.timer = 4;
        state.lift_sum = 0;
        state.lift = 0;
        state.heading_x = 0;
        state.heading_y = 0;
        return;
    }
    state.lift = static_cast<std::int16_t>(-(state.lift >> 1));
    state.heading_x = static_cast<std::int16_t>(state.heading_x >> 1);
    state.heading_y = static_cast<std::int16_t>(state.heading_y >> 1);
    state.lift_sum = 0;
}

} // namespace shadowrun::ghidra_conversion
