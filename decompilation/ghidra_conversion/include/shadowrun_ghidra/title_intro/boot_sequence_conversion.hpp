#pragma once

#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"
#include "shadowrun_ghidra/core/timing_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

constexpr std::size_t kPreTitleHorizontalScrollLines = 224U;
constexpr std::size_t kPreTitleResourceWorkWordCount = 0x1C0U;

// The aliases are the direct host resources required before the title can
// hand off to the startup menu. Full-screen title animation samples are not
// part of this set; the title is composed from its background, logo, text,
// circuit layers, and independently named sprites.
std::vector<std::string> required_boot_asset_aliases();

struct PreTitleSequenceState {
    // Confirmed writes in 0x5D746. The names remain descriptive where the
    // source global's complete game meaning is not proven yet.
    std::uint8_t startup_marker = 0;
    std::uint8_t resource_marker = 0;
    std::int16_t figure_x = 0;
    std::int16_t figure_y = 0;
    std::int16_t logo_effect_counter = 0;
    std::int16_t sega_phase_delay = 0;
    std::int16_t sega_delay_cadence = 0;
    std::int16_t secondary_effect_counter = 0;
    bool logo_strobe_active = false;

    // 0x5D996 owns these title-phase words. Their initial values are set by
    // surrounding title setup, not invented by this leaf conversion.
    std::int16_t logo_phase_countdown = 0;
    std::uint8_t logo_phase = 0;
    std::uint8_t sprite_update_marker = 0;
    std::uint8_t sprite_projection_marker = 0;

    // 0x5D8BA's source cadence/phase pair. The initial values are caller
    // state because setup occurs outside that leaf.
    std::int16_t strobe_countdown = 0;
    std::uint8_t strobe_phase = 0;

    std::uint32_t random_state = 0;
    std::int16_t frame_counter = 0;
    std::uint8_t dirty_flags = 0;
    std::array<std::uint16_t, kPreTitleResourceWorkWordCount>
        resource_work_words{};
    std::array<std::uint16_t, kPreTitleHorizontalScrollLines> horizontal_scroll{};
};

// Direct child-call boundaries recovered from 0x5DA50. The zeroing is
// performed by setup_pre_title_resources; the callback represents the source
// write-word-stream operation without exposing a VDP or ROM address.
struct PreTitleResourceCallbacks {
    std::function<void()> clear_plane_a;
    std::function<void()> clear_plane_b;
    std::function<void()> prepare_title_resources;
    std::function<void(const std::array<
                       std::uint16_t, kPreTitleResourceWorkWordCount>&)>
        write_resource_stream;
    std::function<void()> refresh_field_resource_streams;
    std::function<void()> decode_and_upload_source_stream;
    std::function<void()> advance_title_palette_fade;
    std::function<void()> write_vdp_grid;
    std::function<void()> reset_vdp_upload_state;
};

// Direct child-call boundaries recovered from 0x5D938, 0x5D996, and 0x5D8BA.
struct PreTitleAnimationCallbacks {
    std::function<void()> build_title_sprite_records;
    std::function<void(std::uint8_t phase)> publish_logo_strobe_phase;
    std::function<void()> step_entity_color_words;
};

struct PreTitleSequenceCallbacks {
    PreTitleResourceCallbacks resources;
    PreTitleAnimationCallbacks animation;
    std::function<void()> wait_for_vblank;
    std::function<void(const std::array<std::uint16_t,
                                        kPreTitleHorizontalScrollLines>&)>
        write_horizontal_scroll_stream;
    std::function<void()> service_audio_variant;
    std::function<void()> service_audio;
    std::function<void()> sample_controller;
    std::function<void()> step_palette_transition;
};

enum class PreTitleSequenceResult {
    source_complete,
    callback_unresolved,
};

// ROM 0x5D8BA: update the five-tick logo strobe cadence and publish its
// eight-phase palette selection. The actual palette table remains an asset /
// presentation boundary.
void advance_pre_title_logo_strobe(
    PreTitleSequenceState& state,
    const PreTitleSequenceCallbacks& callbacks);

// ROM 0x5D938 -> 0x5D996: set the sprite-update markers, build the two
// ordinary title records, advance the six-phase logo projection when its
// delay gate is open, and build that projection twice as the source does.
void advance_pre_title_animation(
    PreTitleSequenceState& state,
    const PreTitleSequenceCallbacks& callbacks);

// ROM 0x5DA50 -> 0x5DAE8 -> 0x5DAFC: perform the pre-title resource child
// calls in source order and zero the 0x1C0-word work area before its stream
// writer is invoked.
void setup_pre_title_resources(
    PreTitleSequenceState& state,
    const PreTitleResourceCallbacks& callbacks);

// ROM 0x5D886: clear the title planes, reset upload state, publish the grid,
// and perform the final palette-fade step before the settling wait.
void finalize_pre_title_display(
    PreTitleSequenceState& state,
    const PreTitleResourceCallbacks& callbacks);

// ROM 0x5DAC4: fill all 224 horizontal-scroll entries from the converted
// source PRNG and publish the host-side line table.
void fill_pre_title_horizontal_scroll(
    PreTitleSequenceState& state,
    const PreTitleSequenceCallbacks& callbacks);

// ROM 0x5D746: initialize the pre-title source fields, run the source frame
// loop, perform the final 0x78 DBF wait, and return at the title handoff.
// Hardware writes and unresolved child routines stay explicit callbacks.
PreTitleSequenceResult run_pre_title_sequence(
    PreTitleSequenceState& state,
    const PreTitleSequenceCallbacks& callbacks);

enum class BootStartupStage {
    reset,
    vdp_ready,
    pre_title,
    title_setup,
    title_ready,
    failed,
};

struct BootStartupState {
    BootStartupStage stage = BootStartupStage::reset;
    std::uint32_t vblank_count = 0;
    bool display_ready = false;
    bool title_ready = false;
};

struct BootStartupCallbacks {
    std::function<void(BootStartupStage)> enter_stage;
    std::function<void()> initialize_display;
    VdpReadyCallbacks vdp_ready;
    std::function<void()> run_pre_title_sequence;
    std::function<void()> setup_title_state;
};

enum class BootStartupResult {
    assets_missing,
    callback_unresolved,
    title_ready,
};

// ROM 0x0236 -> 0x073A -> 0x075E -> 0x5D746: bind the extracted boot/title
// asset set to the converted startup ordering. This is a conversion-side
// startup harness, not the native game loop or a Genesis hardware emulator.
BootStartupResult run_boot_startup(
    const AssetSetReport& assets,
    BootStartupState& state,
    const BootStartupCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
