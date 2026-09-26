#include "shadowrun_ghidra/title_intro/boot_sequence_conversion.hpp"

#include "shadowrun_ghidra/title_intro/title_loop_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {
namespace {

void enter_stage(BootStartupState& state,
                 const BootStartupCallbacks& callbacks,
                 const BootStartupStage stage) {
    state.stage = stage;
    if (callbacks.enter_stage) {
        callbacks.enter_stage(stage);
    }
}

bool has_vdp_ready_callbacks(const VdpReadyCallbacks& callbacks) {
    return static_cast<bool>(callbacks.warmup_delay) &&
           static_cast<bool>(callbacks.wait_until_ready) &&
           static_cast<bool>(callbacks.settle_delay);
}

bool has_pre_title_callbacks(const PreTitleSequenceCallbacks& callbacks) {
    const auto& resources = callbacks.resources;
    const auto& animation = callbacks.animation;
    return static_cast<bool>(resources.clear_plane_a) &&
           static_cast<bool>(resources.clear_plane_b) &&
           static_cast<bool>(resources.prepare_title_resources) &&
           static_cast<bool>(resources.write_resource_stream) &&
           static_cast<bool>(resources.refresh_field_resource_streams) &&
           static_cast<bool>(resources.decode_and_upload_source_stream) &&
           static_cast<bool>(resources.advance_title_palette_fade) &&
           static_cast<bool>(resources.write_vdp_grid) &&
           static_cast<bool>(resources.reset_vdp_upload_state) &&
           static_cast<bool>(callbacks.wait_for_vblank) &&
           static_cast<bool>(animation.build_title_sprite_records) &&
           static_cast<bool>(animation.publish_logo_strobe_phase) &&
           static_cast<bool>(animation.step_entity_color_words) &&
           static_cast<bool>(callbacks.write_horizontal_scroll_stream) &&
           static_cast<bool>(callbacks.service_audio_variant) &&
           static_cast<bool>(callbacks.service_audio) &&
           static_cast<bool>(callbacks.sample_controller) &&
           static_cast<bool>(callbacks.step_palette_transition);
}

} // namespace

std::vector<std::string> required_boot_asset_aliases() {
    std::vector<std::string> aliases{
        "state",
        "boot.snow",
        "boot.sega.background",
        "boot.guy",
        "boot.fasa",
        "title.background",
        "title.logo",
        "title.text",
        "menu.startup.background",
        "menu.startup.font",
        "menu.startup.cursor",
    };
    for (std::size_t index = 0; index < 6U; ++index) {
        aliases.push_back("boot.sega.lightning." + std::to_string(index));
    }
    for (std::size_t index = 0; index < 8U; ++index) {
        aliases.push_back("boot.sega.strobe." + std::to_string(index));
    }
    for (std::size_t index = 0; index < 7U; ++index) {
        aliases.push_back("title.circuit.background." +
                          std::to_string(index));
    }
    for (std::size_t index = 0; index < 6U; ++index) {
        aliases.push_back("title.sprite." + std::to_string(index));
    }
    return aliases;
}

void advance_pre_title_logo_strobe(
    PreTitleSequenceState& state,
    const PreTitleSequenceCallbacks& callbacks) {
    --state.strobe_countdown;
    if (state.strobe_countdown < 0) {
        state.strobe_countdown = 4;
        state.strobe_phase = static_cast<std::uint8_t>(state.strobe_phase + 1U);
        if (state.strobe_phase >= 8U) {
            state.strobe_phase = 0U;
        }
        callbacks.animation.publish_logo_strobe_phase(state.strobe_phase);
        state.dirty_flags = static_cast<std::uint8_t>(state.dirty_flags | 0x01U);
    } else if (state.strobe_countdown == 2) {
        callbacks.animation.step_entity_color_words();
    }
}

void advance_pre_title_animation(
    PreTitleSequenceState& state,
    const PreTitleSequenceCallbacks& callbacks) {
    // 0x5D938.
    state.sprite_update_marker = 1U;
    callbacks.animation.build_title_sprite_records();
    callbacks.animation.build_title_sprite_records();

    // 0x5D996. FFDD30 is represented by sega_phase_delay and FFDD3C by
    // logo_strobe_active; FFE066/FFE064 remain typed caller-owned phase state.
    if (state.sega_phase_delay < 0 && !state.logo_strobe_active) {
        --state.logo_phase_countdown;
        if (state.logo_phase_countdown < 0) {
            state.logo_phase_countdown = 3;
            state.logo_phase = static_cast<std::uint8_t>(state.logo_phase + 1U);
            if (state.logo_phase > 5U) {
                state.logo_phase = 0U;
            }
        }
        callbacks.animation.build_title_sprite_records();
        callbacks.animation.build_title_sprite_records();
    }
    state.sprite_projection_marker = 1U;
}

void setup_pre_title_resources(
    PreTitleSequenceState& state,
    const PreTitleResourceCallbacks& callbacks) {
    // 0x5DA50, preserving the recovered call order.
    callbacks.clear_plane_a();
    callbacks.clear_plane_b();
    callbacks.prepare_title_resources();

    // 0x5DAE8 clears 0x1C0 words before 0x5DAFC writes the stream.
    std::fill(state.resource_work_words.begin(),
              state.resource_work_words.end(), 0U);
    callbacks.write_resource_stream(state.resource_work_words);
    callbacks.refresh_field_resource_streams();
    callbacks.decode_and_upload_source_stream();
    state.dirty_flags = static_cast<std::uint8_t>(state.dirty_flags | 0x0DU);
    callbacks.advance_title_palette_fade();
    callbacks.write_vdp_grid();
}

void finalize_pre_title_display(
    PreTitleSequenceState& state,
    const PreTitleResourceCallbacks& callbacks) {
    // 0x5D886, preserving the recovered call order.
    callbacks.clear_plane_a();
    callbacks.clear_plane_b();
    callbacks.reset_vdp_upload_state();
    callbacks.write_vdp_grid();
    callbacks.advance_title_palette_fade();
}

void fill_pre_title_horizontal_scroll(
    PreTitleSequenceState& state,
    const PreTitleSequenceCallbacks& callbacks) {
    for (auto& line : state.horizontal_scroll) {
        line = static_cast<std::uint16_t>(
            (advance_source_prng(state.random_state) >> 5U) & 0x1FU);
    }
    callbacks.write_horizontal_scroll_stream(state.horizontal_scroll);
}

PreTitleSequenceResult run_pre_title_sequence(
    PreTitleSequenceState& state,
    const PreTitleSequenceCallbacks& callbacks) {
    if (!has_pre_title_callbacks(callbacks)) {
        return PreTitleSequenceResult::callback_unresolved;
    }

    // These are the direct source writes at 0x5D746. The strobe cadence and
    // PRNG state remain caller-owned because their setup lives in other source
    // entries.
    state.startup_marker = 1U;
    setup_pre_title_resources(state, callbacks.resources);
    state.resource_marker = 1U;
    state.figure_x = 0x0119;
    state.figure_y = 0x01A8;
    state.logo_effect_counter = 0x0041;
    state.logo_strobe_active = false;
    state.frame_counter = 0x0150;
    state.strobe_phase = 9U;

    while (true) {
        callbacks.wait_for_vblank();
        if (state.logo_strobe_active) {
            advance_pre_title_logo_strobe(state, callbacks);
        }
        fill_pre_title_horizontal_scroll(state, callbacks);
        advance_pre_title_animation(state, callbacks);

        if (state.figure_y == 0x0160) {
            --state.sega_phase_delay;
            if (state.sega_phase_delay < 0) {
                ++state.sega_delay_cadence;
                if ((state.sega_delay_cadence & 1) == 0) {
                    if (state.logo_effect_counter < 6) {
                        callbacks.service_audio_variant();
                    }
                    --state.logo_effect_counter;
                    if (state.logo_effect_counter >= 0) {
                        if (state.logo_effect_counter == 0) {
                            callbacks.service_audio_variant();
                            state.logo_strobe_active = true;
                        }
                        --state.secondary_effect_counter;
                        if (state.logo_effect_counter > 4) {
                            callbacks.service_audio();
                        }
                    }
                }
            }
        } else {
            --state.figure_y;
            if (state.figure_y == 0x0160) {
                state.sega_phase_delay = 0x0014;
            }
        }

        state.dirty_flags = static_cast<std::uint8_t>(
            state.dirty_flags | 0x0EU);
        --state.frame_counter;
        if (state.frame_counter != -1) {
            continue;
        }

        callbacks.sample_controller();
        callbacks.step_palette_transition();
        finalize_pre_title_display(state, callbacks.resources);
        state.frame_counter = 0x0078;
        do {
            callbacks.wait_for_vblank();
            --state.frame_counter;
        } while (state.frame_counter != -1);
        callbacks.sample_controller();
        callbacks.step_palette_transition();
        return PreTitleSequenceResult::source_complete;
    }
}

BootStartupResult run_boot_startup(
    const AssetSetReport& assets,
    BootStartupState& state,
    const BootStartupCallbacks& callbacks) {
    if (!assets.complete()) {
        enter_stage(state, callbacks, BootStartupStage::failed);
        return BootStartupResult::assets_missing;
    }
    if (!callbacks.enter_stage || !callbacks.initialize_display ||
        !has_vdp_ready_callbacks(callbacks.vdp_ready) ||
        !callbacks.run_pre_title_sequence || !callbacks.setup_title_state) {
        enter_stage(state, callbacks, BootStartupStage::failed);
        return BootStartupResult::callback_unresolved;
    }

    enter_stage(state, callbacks, BootStartupStage::reset);
    callbacks.initialize_display();
    enter_stage(state, callbacks, BootStartupStage::vdp_ready);
    wait_for_vdp_ready(callbacks.vdp_ready);
    state.display_ready = true;

    enter_stage(state, callbacks, BootStartupStage::pre_title);
    callbacks.run_pre_title_sequence();
    enter_stage(state, callbacks, BootStartupStage::title_setup);
    callbacks.setup_title_state();
    state.title_ready = true;
    enter_stage(state, callbacks, BootStartupStage::title_ready);
    return BootStartupResult::title_ready;
}

} // namespace shadowrun::ghidra_conversion
