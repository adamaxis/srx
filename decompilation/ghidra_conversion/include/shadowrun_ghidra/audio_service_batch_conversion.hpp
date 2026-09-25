#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct MaskedTransferCallbacks {
    std::function<void()> begin_transfer;
    std::function<void(std::uint16_t)> write_word;
    std::function<void(std::uint16_t)> set_control;
};

std::size_t write_masked_transfer(
    std::uint16_t mask,
    const std::vector<std::uint16_t>& source,
    std::size_t count,
    const MaskedTransferCallbacks& callbacks,
    bool blank_display_during_transfer);

struct AudioInitializationCallbacks {
    std::function<void()> initialize_ram;
    std::function<void()> reset_processor;
    std::function<void(std::int32_t)> append_one;
    std::function<void(std::uint32_t)> append_three;
};

void initialize_audio_sequence(
    const std::array<std::uint32_t, 4>& parameters,
    const AudioInitializationCallbacks& callbacks);

struct AudioBusCallbacks {
    std::function<void()> begin_bus;
    std::function<void()> flush_command;
    std::function<void()> append_command;
    std::function<void()> end_bus;
};

void run_audio_bus_transaction(const AudioBusCallbacks& callbacks);
void run_audio_transaction_wrapper(const AudioBusCallbacks& callbacks);

void run_fixed_audio_pair(const std::function<void(std::uint16_t, std::uint16_t)>& pair);
void run_controller_access(const std::function<void(std::uint16_t)>& access);

void initialize_title_audio_assets(
    const std::array<std::uint32_t, 4>& assets,
    const AudioInitializationCallbacks& callbacks,
    const std::function<void()>& flush_audio);

void copy_audio_state(const std::vector<std::uint32_t>& source,
                     std::vector<std::uint32_t>& destination,
                     std::size_t count);

void audio_noop_hook();

struct AudioTitlePreparationState {
    std::uint8_t scratch = 0;
    std::uint32_t dirty_flags = 0;
};

void prepare_audio_title(AudioTitlePreparationState& state,
                         const std::function<void()>& prepare_sprites);

void build_title_sprite_once(const std::function<void()>& build_sprites);

void reset_audio_menu_arrays(std::vector<std::int32_t>& first,
                             std::vector<std::int32_t>& second);
void fill_audio_menu_sentinel(std::vector<std::int32_t>& values);

void begin_title_audio_sequence(AudioTitlePreparationState& state,
                                const std::function<void()>& prepare_sprites,
                                const std::function<void()>& build_sprites);

std::size_t run_indexed_title_services(
    std::vector<std::function<bool(std::size_t)>>& handlers,
    const std::function<void(std::size_t)>& publish_index);

void write_menu_repeated_grid(const std::function<void()>& write_grid);

std::uint8_t select_display_status(std::int8_t display_mode);

} // namespace shadowrun::ghidra_conversion
