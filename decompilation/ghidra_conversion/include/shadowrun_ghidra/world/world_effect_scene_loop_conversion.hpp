#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

// Host-facing state for ROM 0x1ADE6.  The source routine is the title/world
// scene driver, not a hardware loop: it initializes the scene, samples input,
// advances the active scene services in source order, and hands control to a
// mode loop or field service when one of the source exit conditions fires.
struct WorldEffectSceneLoopState {
    std::uint16_t work_x = 0x0120U; // FF1596
    std::uint16_t work_y = 0x00D0U; // FF1598

    bool world_sequence_active = false; // FFDF8C
    bool exit_requested = false;        // FFE080
    bool scene_complete = false;        // FFE084
    std::uint16_t scene_completion_aux = 0; // FFE086
    bool title_phase_active = false;    // FFE0F6
    bool alternating_profile_active = false; // FFE0AA

    std::uint8_t source_audio_track = 0; // FFF0A0, input to initialization
    std::uint8_t audio_track = 0;        // FFE189
    std::uint8_t audio_countdown = 0;    // FFE16D
    bool alternate_audio = false;        // FFE18E != 0
    bool runner_selection_mode = false;  // FFF0A9 != 0

    std::uint8_t title_service_flags = 0; // FF1592
    std::uint16_t world_effect_mode = 0;   // FFE076
    bool burst_active = false;             // FFE0AC
    bool noise_effect_active = false;      // FFF0A1 bit 0
    bool profile_transition_active = false; // FF1592 bit 7
    bool profile_motion_active = false;    // FFE058
    bool title_effect_motion_active = false; // FFE05E
    bool world_effect_motion_active = false; // FFE06E
    bool matrix_transition_active = false; // FFE0DE
    bool title_animation_active = false;   // FFE0D4

    std::uint16_t controller_events = 0; // FFDD22
    bool footer_blocked = false;          // FFFBCA != 0
    bool current_runner_footer_visible = false; // *(FFFC04+0x4C) != 0
    std::int8_t hud_status = 0;           // FFF0B1; signed branch source
    bool frame_presented = false;         // FFDD02 / presentation boundary
    std::uint8_t dirty_flags = 0;         // FFEF14 bit 1 from this driver
};

struct WorldEffectSceneLoopCallbacks {
    // Scene initialization: 0x1B5A6, 0x26958, 0x1A908, 0x1B680.
    std::function<void()> reset_word_stream_area;
    std::function<void()> upload_decoded_source_stream;
    std::function<void()> initialize_world_effect_mode;
    std::function<void()> attach_behavior_descriptor_e;

    // 0x00BD2, 0x00BF2, 0x195C, and 0x17FDA.
    std::function<void()> wait_for_vblank;
    std::function<void()> latch_controller_events;
    std::function<std::uint16_t()> sample_controller_input;
    std::function<void()> advance_menu_random_glitch;

    // World-sequence handoff at 0x1D280/0x1D2E6 and the mode entry calls.
    std::function<void()> initialize_world_sequence;
    // True means the source 0x1D2E6 call returned with carry: the sequence
    // has completed and control moves to a gameplay/runner mode.
    std::function<bool()> advance_world_sequence;
    std::function<void()> prepare_mode_entry_service;
    std::function<void(std::uint16_t command)> service_audio_command_frame;
    std::function<void()> run_runner_selection_screen;
    // 0x54662: enter the Matrix node-function action loop after the world
    // transition completes.
    std::function<void()> run_matrix_node_action_loop;

    // Scene-completion path at 0x1B302/0x18776/0x26A00.
    // Return false when the 0x3C-frame jitter has cleared FFE084.
    std::function<bool()> advance_title_jitter;
    std::function<void()> advance_hud_fade_service;
    std::function<void()> finish_title_fade;

    // Main active-scene services through 0x1A370.
    std::function<void()> advance_matrix_program_loading; // 0x19788
    std::function<void()> advance_title_world_interaction_service; // 0x1C754
    std::function<void()> handle_menu_directional_input; // 0x19432
    std::function<void()> advance_world_effect_controller; // 0x1A370
    std::function<void(std::uint16_t command)> service_audio_if_enabled; // 0x530DE
    std::function<void()> advance_alternating_profile; // 0x1C0C2
    std::function<void()> advance_profile_copy_timer; // 0x1BF7A

    // Title/world sprite services after the common scene services.
    std::function<void()> advance_title_figure_animation; // 0x1B364
    std::function<void()> append_title_vertical_sprite; // 0x1A7BE
    std::function<void()> advance_title_phase_cycle; // 0x1B218
    std::function<void()> advance_title_sprite_resource_service; // 0x26972
    std::function<void()> advance_title_sprite_layers; // 0x1C3FA
    std::function<void()> advance_title_sweep_animation; // 0x1C61C
    std::function<void()> advance_audio_menu_countdown; // 0x1C1F6
    std::function<void()> advance_title_noise_stream; // 0x1B45C
    std::function<void()> advance_title_reveal_transfer_service; // 0x1D1F4
    std::function<void()> advance_title_secondary_effect_service; // 0x1CD5A
    std::function<void()> dispatch_title_world_phase_service; // 0x1B87A
    std::function<void()> advance_title_burst_animation; // 0x1BA6E
    std::function<void()> advance_title_noise_effect; // 0x1BB6A
    std::function<void()> advance_title_profile_transition; // 0x1BEBA
    std::function<void()> advance_title_profile_motion; // 0x1BCE8
    std::function<void()> advance_title_effect_motion; // 0x1C866
    std::function<void()> advance_world_effect_motion; // 0x1CA12
    std::function<void()> advance_world_effect_mode_animation; // 0x1A5A0
    std::function<void()> advance_title_animation_cycle; // 0x1A48A
    std::function<void()> advance_matrix_program_transition; // 0x1B282
    std::function<void()> update_persistent_title_effect; // 0x1CCDC
    std::function<void()> advance_persistent_world_effect; // 0x1CC24
    std::function<void()> tick_world_effect_animation; // 0x1AA40
    std::function<void()> append_world_sprite_records; // 0x268E6

    // Optional field refresh branch entered when controller bit 15 is set.
    std::function<bool()> refresh_field_resource_streams; // 0x176BA
    std::function<void()> run_matrix_loop; // 0x1822C
    std::function<void()> refresh_field_hud_assets; // 0x186FA
    std::function<void()> prepare_menu_hud_sequence; // 0x1789C
    std::function<void(std::uint16_t value)> set_display_control;
};

enum class WorldEffectSceneExitReason {
    frame_budget_exhausted,
    handed_to_mode,
    title_fade_complete,
    hud_abort,
    resource_refresh_stopped,
};

struct WorldEffectSceneLoopResult {
    WorldEffectSceneExitReason reason =
        WorldEffectSceneExitReason::frame_budget_exhausted;
    std::size_t frames_executed = 0;
};

// ROM 0x1ADE6.  max_frames is a host-side inspection bound; zero means run
// until the source routine reaches one of its return paths.  No ROM address,
// VDP register, or emulated CPU state is required by this conversion.
WorldEffectSceneLoopResult run_world_effect_scene_loop(
    WorldEffectSceneLoopState& state,
    const WorldEffectSceneLoopCallbacks& callbacks,
    std::size_t max_frames = 0);

} // namespace shadowrun::ghidra_conversion
