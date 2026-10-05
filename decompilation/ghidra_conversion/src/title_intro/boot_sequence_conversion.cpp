#include "shadowrun_ghidra/title_intro/boot_sequence_conversion.hpp"

#include "shadowrun_ghidra/title_intro/title_loop_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {
namespace {

// ROM 0x5DCE6 contains fifteen words. 0x5D8BA selects an eight-word window
// by advancing the source pointer one word for each of the eight phases.
constexpr std::array<std::uint16_t, 15> kPreTitleLogoStrobeColorTable{
    0x0080U, 0x0060U, 0x0040U, 0x0020U, 0x0000U,
    0x0000U, 0x0000U, 0x0000U, 0x0080U, 0x0060U,
    0x0040U, 0x0020U, 0x0000U, 0x0000U, 0x0000U,
};

const std::vector<SpriteSourceRecord> kPreTitleSegaGuyPrimarySource{
    {-15, -63, 6, 0x811BU},
    {-23, -39, 11, 0x8163U},
    {-15, -7, 4, 0x8161U},
    {-54, -40, 13, 0x81C5U},
    {-67, -66, 11, 0x81CDU},
    {static_cast<std::int16_t>(-0x8000), 0, 0, 0},
};

const std::vector<SpriteSourceRecord> kPreTitleSegaGuySecondarySource{
    {-2, -63, 6, 0x891BU},
    {-2, -39, 11, 0x8963U},
    {-2, -7, 4, 0x8961U},
    {21, -40, 13, 0x89C5U},
    {42, -66, 11, 0x89CDU},
    {static_cast<std::int16_t>(-0x8000), 0, 0, 0},
};

const std::array<std::vector<SpriteSourceRecord>, 6>
    kPreTitleSegaLightningPrimarySources{
        std::vector<SpriteSourceRecord>{
            {-23, -55, 15, 0x0051U},
            {-10, -23, 14, 0x0061U},
            {9, -48, 11, 0x006DU},
            {static_cast<std::int16_t>(-0x8000), 0, 0, 0},
        },
        std::vector<SpriteSourceRecord>{
            {-31, -55, 15, 0x0079U},
            {-20, -23, 14, 0x0089U},
            {1, -34, 0, 0x0095U},
            {static_cast<std::int16_t>(-0x8000), 0, 0, 0},
        },
        std::vector<SpriteSourceRecord>{
            {-31, -54, 15, 0x0096U},
            {-23, -22, 14, 0x00A6U},
            {-20, -58, 14, 0x00B2U},
            {static_cast<std::int16_t>(-0x8000), 0, 0, 0},
        },
        std::vector<SpriteSourceRecord>{
            {-20, -42, 15, 0x00BEU},
            {-17, -58, 15, 0x00CEU},
            {-12, -18, 14, 0x00DEU},
            {15, -52, 11, 0x00EAU},
            {20, -20, 4, 0x00F6U},
            {static_cast<std::int16_t>(-0x8000), 0, 0, 0},
        },
        std::vector<SpriteSourceRecord>{
            {-31, -54, 15, 0x0096U},
            {-23, -22, 14, 0x00A6U},
            {-20, -58, 14, 0x00B2U},
            {static_cast<std::int16_t>(-0x8000), 0, 0, 0},
        },
        std::vector<SpriteSourceRecord>{
            {-31, -55, 15, 0x0079U},
            {-20, -23, 14, 0x0089U},
            {1, -34, 0, 0x0095U},
            {static_cast<std::int16_t>(-0x8000), 0, 0, 0},
        },
    };

const std::array<std::vector<SpriteSourceRecord>, 6>
    kPreTitleSegaLightningSecondarySources{
        std::vector<SpriteSourceRecord>{
            {-9, -55, 15, 0x0851U},
            {-22, -23, 14, 0x0861U},
            {-33, -48, 11, 0x086DU},
            {static_cast<std::int16_t>(-0x8000), 0, 0, 0},
        },
        std::vector<SpriteSourceRecord>{
            {-1, -55, 15, 0x0879U},
            {-12, -23, 14, 0x0889U},
            {-9, -34, 0, 0x0895U},
            {static_cast<std::int16_t>(-0x8000), 0, 0, 0},
        },
        std::vector<SpriteSourceRecord>{
            {-1, -54, 15, 0x0896U},
            {-9, -22, 14, 0x08A6U},
            {-12, -58, 14, 0x08B2U},
            {static_cast<std::int16_t>(-0x8000), 0, 0, 0},
        },
        std::vector<SpriteSourceRecord>{
            {-12, -42, 15, 0x08BEU},
            {-15, -58, 15, 0x08CEU},
            {-20, -18, 14, 0x08DEU},
            {-39, -52, 11, 0x08EAU},
            {-36, -20, 4, 0x08F6U},
            {static_cast<std::int16_t>(-0x8000), 0, 0, 0},
        },
        std::vector<SpriteSourceRecord>{
            {-1, -54, 15, 0x0896U},
            {-9, -22, 14, 0x08A6U},
            {-12, -58, 14, 0x08B2U},
            {static_cast<std::int16_t>(-0x8000), 0, 0, 0},
        },
        std::vector<SpriteSourceRecord>{
            {-1, -55, 15, 0x0879U},
            {-12, -23, 14, 0x0889U},
            {-9, -34, 0, 0x0895U},
            {static_cast<std::int16_t>(-0x8000), 0, 0, 0},
        },
    };

PreTitleSpriteBatch make_pre_title_sprite_batch(
    const PreTitleSpriteBatchSource source,
    const std::uint8_t phase,
    const std::int16_t origin_x,
    const std::int16_t origin_y,
    const std::uint8_t first_sequence,
    const std::uint16_t attribute_flags,
    const std::vector<SpriteSourceRecord>& source_records) {
    PreTitleSpriteBatch batch{};
    batch.source = source;
    batch.phase = phase;
    batch.origin_x = origin_x;
    batch.origin_y = origin_y;
    batch.first_sequence = first_sequence;
    batch.attribute_flags = attribute_flags;
    batch.records = build_title_sprite_records(
        source_records, origin_y, origin_x, first_sequence, attribute_flags);
    return batch;
}

void publish_pre_title_sprite_batch(
    const PreTitleSpriteBatchSource source,
    const std::uint8_t phase,
    const std::int16_t origin_x,
    const std::int16_t origin_y,
    const std::uint16_t attribute_flags,
    const std::vector<SpriteSourceRecord>& source_records,
    const PreTitleAnimationCallbacks& callbacks,
    std::uint8_t& next_sequence) {
    auto batch = make_pre_title_sprite_batch(
        source, phase, origin_x, origin_y, next_sequence, attribute_flags,
        source_records);
    next_sequence = static_cast<std::uint8_t>(
        next_sequence + static_cast<std::uint8_t>(batch.records.size()));
    callbacks.publish_sprite_batch(batch);
}

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
           static_cast<bool>(resources.write_pre_title_grid) &&
           static_cast<bool>(resources.reset_vdp_upload_state) &&
           static_cast<bool>(callbacks.wait_for_vblank) &&
           static_cast<bool>(animation.publish_sprite_batch) &&
           static_cast<bool>(animation.clear_last_sprite_sequence) &&
           static_cast<bool>(animation.publish_logo_strobe_palette) &&
           static_cast<bool>(animation.step_entity_color_words) &&
           static_cast<bool>(callbacks.write_horizontal_scroll_stream) &&
           static_cast<bool>(callbacks.publish_audio_request) &&
           static_cast<bool>(callbacks.sample_controller) &&
           static_cast<bool>(callbacks.latch_controller_events) &&
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
        PreTitleLogoStrobePalette palette{};
        palette.phase = state.strobe_phase;
        std::copy_n(kPreTitleLogoStrobeColorTable.begin() +
                        static_cast<std::size_t>(state.strobe_phase),
                    palette.color_words.size(), palette.color_words.begin());
        callbacks.animation.publish_logo_strobe_palette(palette);
        state.dirty_flags = static_cast<std::uint8_t>(state.dirty_flags | 0x01U);
    } else if (state.strobe_countdown == 2) {
        callbacks.animation.step_entity_color_words();
        // 0x5B2E2 visits the fifteen figure colors and subtracts one 3-bit
        // CRAM level from each nonzero component. Seven calls fully clear a
        // Genesis component; keep those source palette steps as typed state.
        if (state.figure_palette_fade_steps <
            kPreTitleFigurePaletteLevelsToBlack) {
            ++state.figure_palette_fade_steps;
        }
    }
}

void advance_pre_title_animation(
    PreTitleSequenceState& state,
    const PreTitleSequenceCallbacks& callbacks) {
    // 0x5D938.
    state.sprite_update_marker = 1U;
    std::uint8_t next_sequence = 2U;
    publish_pre_title_sprite_batch(
        PreTitleSpriteBatchSource::sega_guy_primary, 0U, state.figure_x,
        state.figure_y, 0x6000U, kPreTitleSegaGuyPrimarySource,
        callbacks.animation, next_sequence);
    publish_pre_title_sprite_batch(
        PreTitleSpriteBatchSource::sega_guy_secondary, 0U, state.figure_x,
        state.figure_y, 0x6000U, kPreTitleSegaGuySecondarySource,
        callbacks.animation, next_sequence);

    // 0x5D996. FFDD30 is represented by sega_phase_delay and FFDD3C by
    // logo_strobe_active; FFE066/FFE064 remain typed caller-owned phase state.
    state.sega_lightning_batches_active =
        state.sega_phase_delay < 0 && !state.logo_strobe_active;
    if (state.sega_lightning_batches_active) {
        --state.logo_phase_countdown;
        if (state.logo_phase_countdown < 0) {
            state.logo_phase_countdown = 3;
            state.logo_phase = static_cast<std::uint8_t>(state.logo_phase + 1U);
            if (state.logo_phase > 5U) {
                state.logo_phase = 0U;
            }
        }
        const auto phase = static_cast<std::uint8_t>(state.logo_phase % 6U);
        publish_pre_title_sprite_batch(
            PreTitleSpriteBatchSource::sega_lightning_primary, phase, 0x0158,
            0x0120, 0x2000U,
            kPreTitleSegaLightningPrimarySources[phase], callbacks.animation,
            next_sequence);
        publish_pre_title_sprite_batch(
            PreTitleSpriteBatchSource::sega_lightning_secondary, phase, 0x00D8,
            0x0120, 0x2000U,
            kPreTitleSegaLightningSecondarySources[phase],
            callbacks.animation, next_sequence);
    }
    callbacks.animation.clear_last_sprite_sequence();
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
    callbacks.write_pre_title_grid(
        {PreTitleGridPurpose::resource_setup, 3U, 12U, 0U});
}

void finalize_pre_title_display(
    PreTitleSequenceState& state,
    const PreTitleResourceCallbacks& callbacks) {
    // 0x5D886, preserving the recovered call order.
    callbacks.clear_plane_a();
    callbacks.clear_plane_b();
    callbacks.reset_vdp_upload_state();
    callbacks.write_pre_title_grid(
        {PreTitleGridPurpose::final_display, 4U, 16U, 0x4000U});
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

    if (begin_pre_title_sequence(state, callbacks) !=
        PreTitleSequenceResult::source_complete) {
        return PreTitleSequenceResult::callback_unresolved;
    }

    while (state.presentation_stage !=
           PreTitleSequenceState::PresentationStage::complete) {
        if (advance_pre_title_sequence_frame(state, callbacks) ==
            PreTitleSequenceResult::callback_unresolved) {
            return PreTitleSequenceResult::callback_unresolved;
        }
    }
    return PreTitleSequenceResult::source_complete;
}

PreTitleSequenceResult begin_pre_title_sequence(
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
    state.sega_lightning_batches_active = false;
    state.frame_counter = 0x0150;
    state.strobe_phase = 9U;
    state.presentation_stage =
        PreTitleSequenceState::PresentationStage::sega_entry;
    state.sega_lightning_phase = 0;
    state.sega_shock_frames = 0;
    state.fasa_hold_frames = 0;
    state.figure_palette_fade_steps = 0U;
    return PreTitleSequenceResult::source_complete;
}

PreTitleSequenceResult advance_pre_title_sequence_frame(
    PreTitleSequenceState& state,
    const PreTitleSequenceCallbacks& callbacks) {
    if (!has_pre_title_callbacks(callbacks)) {
        return PreTitleSequenceResult::callback_unresolved;
    }
    if (state.presentation_stage ==
        PreTitleSequenceState::PresentationStage::uninitialized) {
        return PreTitleSequenceResult::callback_unresolved;
    }
    if (state.presentation_stage ==
        PreTitleSequenceState::PresentationStage::complete) {
        return PreTitleSequenceResult::source_complete;
    }

    using PresentationStage = PreTitleSequenceState::PresentationStage;
    if (state.presentation_stage == PresentationStage::sega_fade_out ||
        state.presentation_stage == PresentationStage::fasa_fade_in ||
        state.presentation_stage == PresentationStage::fasa_fade_out) {
        // ROM 0x131C / 0x12F6 each execute 17 palette passes. The shared
        // 0x1342 wait runs two VBlanks and latches controller events before
        // each one-step CRAM update. Preserve those frames as native state
        // rather than completing a whole blocking fade in one host update.
        callbacks.wait_for_vblank();
        callbacks.latch_controller_events();
        ++state.palette_transition_vblank_phase;
        if (state.palette_transition_vblank_phase >=
            kPreTitleVBlanksPerPalettePass) {
            state.palette_transition_vblank_phase = 0U;
            ++state.palette_transition_passes_completed;
        }
        if (state.palette_transition_passes_completed <
            kPreTitlePaletteTransitionPasses) {
            return PreTitleSequenceResult::source_complete;
        }

        state.palette_transition_passes_completed =
            kPreTitlePaletteTransitionPasses;
        if (state.presentation_stage == PresentationStage::sega_fade_out) {
            finalize_pre_title_display(state, callbacks.resources);
            state.palette_transition_passes_completed = 0U;
            state.palette_transition_vblank_phase = 0U;
            state.presentation_stage = PresentationStage::fasa_fade_in;
        } else if (state.presentation_stage ==
                   PresentationStage::fasa_fade_in) {
            state.fasa_hold_frames = 0x0079U;
            state.presentation_stage = PresentationStage::fasa_hold;
        } else {
            state.presentation_stage = PresentationStage::complete;
        }
        return PreTitleSequenceResult::source_complete;
    }

    if (state.presentation_stage == PresentationStage::fasa_hold) {
        if (state.fasa_hold_frames != 0U) {
            callbacks.wait_for_vblank();
            --state.fasa_hold_frames;
            if (state.fasa_hold_frames == 0U) {
                // 0x5D878 samples input immediately after the 0x78 DBF
                // wait, then enters 0x131C to fade FASA to black.
                callbacks.sample_controller();
                callbacks.step_palette_transition();
                state.palette_transition_passes_completed = 0U;
                state.palette_transition_vblank_phase = 0U;
                state.presentation_stage = PresentationStage::fasa_fade_out;
            }
            return PreTitleSequenceResult::source_complete;
        }
    }

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
                    callbacks.publish_audio_request(
                        {PreTitleAudioServicePath::enabled_variant_transaction,
                         PreTitleAudioTrigger::logo_effect_counter_below_six,
                         0x0016U});
                }
                --state.logo_effect_counter;
                if (state.logo_effect_counter >= 0) {
                    if (state.logo_effect_counter == 0) {
                        callbacks.publish_audio_request(
                            {PreTitleAudioServicePath::
                                 enabled_variant_transaction,
                             PreTitleAudioTrigger::
                                 logo_effect_counter_reached_zero,
                             0x0016U});
                        state.logo_strobe_active = true;
                    }
                    --state.secondary_effect_counter;
                    if (state.logo_effect_counter > 4) {
                        callbacks.publish_audio_request(
                            {PreTitleAudioServicePath::
                                 enabled_service_fc08_select,
                             PreTitleAudioTrigger::
                                 logo_effect_counter_above_four,
                             0x005CU});
                    }
                }
            }
        }
    } else {
        --state.figure_y;
        if (state.figure_y == 0x0160) {
            state.sega_phase_delay = 0x0014;
            state.sega_shock_frames = 0x0078;
        }
    }

    if (state.logo_strobe_active) {
        state.presentation_stage =
            PreTitleSequenceState::PresentationStage::sega_strobe;
    } else if (state.sega_phase_delay < 0 && state.figure_y == 0x0160) {
        state.presentation_stage =
            PreTitleSequenceState::PresentationStage::sega_shock;
        state.sega_lightning_phase = static_cast<std::uint16_t>(
            state.logo_phase % 6U);
    } else {
        state.presentation_stage =
            PreTitleSequenceState::PresentationStage::sega_entry;
    }

    state.dirty_flags = static_cast<std::uint8_t>(
        state.dirty_flags | 0x0EU);
    --state.frame_counter;
    if (state.frame_counter != -1) {
        return PreTitleSequenceResult::source_complete;
    }

    callbacks.sample_controller();
    callbacks.step_palette_transition();
    state.palette_transition_passes_completed = 0U;
    state.palette_transition_vblank_phase = 0U;
    state.presentation_stage = PresentationStage::sega_fade_out;
    return PreTitleSequenceResult::source_complete;
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
