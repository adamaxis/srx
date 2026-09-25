#include "shadowrun_ghidra/audio_service_batch_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {

namespace {

void call_once(const std::function<void()>& callback) {
    if (callback) {
        callback();
    }
}

} // namespace

std::size_t write_masked_transfer(
    const std::uint16_t mask,
    const std::vector<std::uint16_t>& source,
    const std::size_t count,
    const MaskedTransferCallbacks& callbacks,
    const bool blank_display_during_transfer) {
    if (blank_display_during_transfer && callbacks.set_control) {
        callbacks.set_control(0x8F80U);
    }
    call_once(callbacks.begin_transfer);
    const auto limit = std::min(count, source.size());
    for (std::size_t index = 0; index < limit; ++index) {
        if (callbacks.write_word) {
            callbacks.write_word(static_cast<std::uint16_t>(mask | source[index]));
        }
    }
    if (blank_display_during_transfer && callbacks.set_control) {
        callbacks.set_control(0x8F02U);
    }
    return limit;
}

void initialize_audio_sequence(
    const std::array<std::uint32_t, 4>& parameters,
    const AudioInitializationCallbacks& callbacks) {
    call_once(callbacks.initialize_ram);
    call_once(callbacks.reset_processor);
    if (callbacks.append_one) {
        callbacks.append_one(-1);
        callbacks.append_one(0x0B);
    }
    if (callbacks.append_three) {
        for (const auto parameter : parameters) {
            callbacks.append_three(parameter);
        }
    }
}

void run_audio_bus_transaction(const AudioBusCallbacks& callbacks) {
    call_once(callbacks.begin_bus);
    call_once(callbacks.flush_command);
    call_once(callbacks.append_command);
    call_once(callbacks.end_bus);
}

void run_audio_transaction_wrapper(const AudioBusCallbacks& callbacks) {
    run_audio_bus_transaction(callbacks);
}

void run_fixed_audio_pair(const std::function<void(std::uint16_t, std::uint16_t)>& pair) {
    if (pair) {
        pair(1, 0);
    }
}

void run_controller_access(const std::function<void(std::uint16_t)>& access) {
    if (access) {
        access(1);
    }
}

void initialize_title_audio_assets(
    const std::array<std::uint32_t, 4>& assets,
    const AudioInitializationCallbacks& callbacks,
    const std::function<void()>& flush_audio) {
    initialize_audio_sequence(assets, callbacks);
    call_once(flush_audio);
}

void copy_audio_state(const std::vector<std::uint32_t>& source,
                     std::vector<std::uint32_t>& destination,
                     const std::size_t count) {
    destination.assign(count, 0U);
    std::copy_n(source.begin(), std::min(count, source.size()), destination.begin());
}

void audio_noop_hook() {}

void prepare_audio_title(AudioTitlePreparationState& state,
                         const std::function<void()>& prepare_sprites) {
    state.scratch = 0;
    state.dirty_flags |= 2U;
    call_once(prepare_sprites);
}

void build_title_sprite_once(const std::function<void()>& build_sprites) {
    call_once(build_sprites);
}

void reset_audio_menu_arrays(std::vector<std::int32_t>& first,
                             std::vector<std::int32_t>& second) {
    first.assign(35U, 0);
    second.assign(35U, 0);
}

void fill_audio_menu_sentinel(std::vector<std::int32_t>& values) {
    values.assign(35U, -1);
}

void begin_title_audio_sequence(AudioTitlePreparationState& state,
                                const std::function<void()>& prepare_sprites,
                                const std::function<void()>& build_sprites) {
    prepare_audio_title(state, prepare_sprites);
    call_once(build_sprites);
    state.scratch = 0;
    state.dirty_flags |= 2U;
}

std::size_t run_indexed_title_services(
    std::vector<std::function<bool(std::size_t)>>& handlers,
    const std::function<void(std::size_t)>& publish_index) {
    const auto limit = std::min<std::size_t>(23U, handlers.size());
    std::size_t ran = 0U;
    for (std::size_t index = 0; index < limit; ++index) {
        if (publish_index) {
            publish_index(index);
        }
        if (handlers[index] && handlers[index](index)) {
            ++ran;
        }
    }
    return ran;
}

void write_menu_repeated_grid(const std::function<void()>& write_grid) {
    call_once(write_grid);
}

std::uint8_t select_display_status(const std::int8_t display_mode) {
    if (display_mode == 0) {
        return 0xCCU;
    }
    if (display_mode < 0) {
        return 0xC0U;
    }
    return 0x0CU;
}

} // namespace shadowrun::ghidra_conversion
