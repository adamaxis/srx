#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

// These are source-analysis types. This tree is deliberately not included by
// the native game or by the GenesisBus emulator.
struct ControllerSample {
    std::uint8_t held = 0;
    std::uint8_t newly_pressed = 0;
};

// ROM 0x195C: FFDD22 keeps the current controller byte in its low half and
// the newly asserted bits in its high half.
ControllerSample derive_controller_sample(std::uint8_t current,
                                           std::uint8_t previous);

struct ControllerEventLatchState {
    std::int16_t activity_timer = 0;
    bool start_latched = false;
};

struct ControllerEventLatchResult {
    std::uint16_t event_bits = 0;
    bool activity_latched = false;
    bool start_latched = false;
};

// ROM 0x0BF2: combine the sampled high-byte event words and latch activity
// and Start state. The source timer writes are represented as typed state.
ControllerEventLatchResult latch_controller_events(
    ControllerEventLatchState& state, std::uint16_t pending_events,
    std::uint16_t sampled_word);

// ROM 0x0D44: MULU #$20E5, ADDQ.W #1, MOVE.W into FFF9F8. Host state may be
// wider; only the low word is the seed.
std::uint32_t advance_source_prng(std::uint32_t& state);

// ROM 0x0D3A: tick 0x0D44, MULU bound, SWAP, EXT.L.
std::int16_t sample_source_random_bound(std::uint32_t& state,
                                        std::uint16_t bound);

constexpr std::size_t kPaletteEntryCount = 64;
constexpr std::size_t kPaletteTransitionPasses = 17;
using Palette = std::array<std::uint16_t, kPaletteEntryCount>;

// ROM 0x1368 copies 32 longwords, i.e. the complete 64-word palette.
void copy_target_palette(Palette& target, const Palette& source);

// ROM 0x1088 and 0x10D6 each perform one component step across all 64 words.
std::uint16_t palette_toward_black(std::uint16_t color);
std::uint16_t palette_toward_target(std::uint16_t color,
                                    std::uint16_t target);

enum class PaletteTransitionMode {
    TowardBlack,
    TowardTarget,
};

// ROM 0x131C/0x12F6: the 17 color passes are kept separate from the source's
// vblank wait routine at 0x1342.
void run_palette_transition(Palette& current, const Palette& target,
                             PaletteTransitionMode mode);

// Typed state for the source title reveal branch. The caller owns the outer
// 0x1A9E loop, input sampling, prompt drawing, and hardware presentation.
struct TitleRevealState {
    std::int16_t reveal_delay = 0x5A;
    std::int16_t column_cadence = 0;
    std::uint16_t revealed_columns = 0;
    std::int16_t sprite_cadence = 0;
    std::uint16_t sprite_phase = 0;
    std::int16_t sprite_x = 0x98;
    std::uint16_t completion_state = 0;
    std::int16_t text_timer = 0x5A;
};

// Source calls represented: 0x1C1E, 0x1C6A, and 0x1E4E. Returns true when a
// new eight-pixel logo column was revealed during this frame.
bool advance_title_reveal(TitleRevealState& state);

constexpr std::size_t kTitleInputHistoryCapacity = 8;
constexpr std::uint8_t kTitleExitPalettePasses = 17;
constexpr std::uint8_t kTitleExitVblanksPerPalettePass = 2;
constexpr std::uint8_t kTitleModeEntryAudioCommand = 0x14U;
constexpr std::uint8_t kTitleModeEntryAudioTransactions = 6U;
constexpr std::uint8_t kTitleModeEntryVblanksPerTransaction = 3U;

struct TitleInputHistory {
    std::array<std::uint8_t, kTitleInputHistoryCapacity> events{};
    std::size_t count = 0;
    bool sequence_complete = false;
};

// ROM 0x1BEA records the non-Start edge byte from FFDD22's high half until
// the eight-entry source buffer is full. Returns true when an entry is added.
bool record_title_input_edge(TitleInputHistory& history,
                             std::uint16_t sampled_word);

enum class TitleInputSequenceResult {
    NotMatched,
    Matched,
};

// ROM 0x1BAC: validate the seven-byte source sequence. A mismatch clears the
// source history; a match marks completion instead of using FF as a sentinel.
TitleInputSequenceResult finalize_title_input_sequence(
    TitleInputHistory& history);

enum class TitlePromptVariant {
    VariantA,
    VariantB,
};

struct TitlePromptSchedule {
    static constexpr std::uint16_t kInitialFrames = 0x0E10;
    static constexpr std::uint16_t kPromptStart = 0x0C6C;
    static constexpr std::uint16_t kBlinkPeriod = 0x0040;
    static constexpr std::uint16_t kVariantBFrames = 10;

    std::uint16_t frames_remaining = kInitialFrames;
    bool finished = false;
};

// ROM 0x1A9E: after the initial countdown, choose the title prompt record
// from the low six bits of the frame counter. The 0x1DCA/0x1DE0 text writers
// remain presentation boundaries outside this source-analysis tree.
std::optional<TitlePromptVariant> advance_title_prompt_schedule(
    TitlePromptSchedule& schedule);

constexpr std::uint16_t kTitleLogoColumnLimit = 0x0026;
constexpr std::uint16_t kTitleCompletionColumn = 0x0025;
constexpr std::uint16_t kTitleCompletionPaletteWordCount = 16;

struct TitleLogoRevealState {
    std::uint16_t columns_revealed = 0;
    std::uint16_t source_descriptor_offset = 0;
};

struct TitleLogoRevealResult {
    bool column_render_requested = false;
    bool completed = false;
    bool completion_palette_copy_requested = false;
    std::uint16_t column_index = 0;
};

// ROM 0x1E4E: consume one two-byte logo descriptor and request its render.
// The source blitters and completion palette copy are represented as events;
// no ROM pointer or VDP write crosses into this conversion tree.
TitleLogoRevealResult advance_title_logo_column(TitleLogoRevealState& state);

enum class TitleTextAction {
    None,
    StepPaletteTowardTarget,
    ShowCopyright,
};

struct TitleTextState {
    std::uint16_t completion_state = 0;
    std::int16_t countdown = 0x5A;
    std::uint8_t palette_steps = 0;
};

// ROM 0x1C1E: count down the completed-logo text delay. The returned action
// represents the source palette step or the unresolved copyright/text setup.
TitleTextAction advance_title_text_state(TitleTextState& state);

struct TitleSpriteState {
    std::int16_t cadence = 0;
    std::uint16_t phase = 0;
    std::int16_t x = 0x98;
    bool projection_visible = false;
};

struct TitleSpriteAdvanceResult {
    bool phase_changed = false;
    bool projection_rebuild_requested = false;
};

// ROM 0x1C6A: advance the six-phase sprite cadence and request the source
// sprite-table projection. Projection remains a presentation boundary.
TitleSpriteAdvanceResult advance_title_sprite_state(TitleSpriteState& state);

struct TitleAnimationTimer {
    std::int16_t countdown = 0;
    std::uint16_t animation_offset = 0;
};

// ROM 0x26D0: five idle frames per animation-offset advance. Returns true on
// the wrap that changes the offset and would set the source update flag.
bool advance_title_animation_timer(TitleAnimationTimer& timer);

struct TitleCircuitLightState {
    std::int16_t countdown;
    std::uint8_t phase;
};

// ROM 0x2634: only the completed-title state cycles the seven light phases.
// The initial countdown/phase are caller-provided because setup occurs in a
// separate source path; no invented initial register values are used here.
std::optional<std::uint8_t> advance_title_circuit_lights(
    TitleCircuitLightState& state, std::uint16_t completion_state);

enum class TitleLoopFrameResult {
    running,
    start_requested,
    source_complete,
    palette_transitioning,
    palette_transition_complete,
};

// ROM 0x131C performs 17 palette steps. Each pass calls 0x1342 with D5=1,
// which waits for two VBlanks and latches the already sampled controller
// state each time. The host owns one VBlank per title-loop call while this
// typed fade state remains active.
struct TitleExitPaletteFadeState {
    std::uint8_t passes_completed = 0;
    std::uint8_t vblanks_in_pass = 0;
    bool active = false;
    bool complete = false;
};

// ROM 0x075E runs another 17-pass palette transition on the no-input branch
// after the 18-VBlank mode-entry audio service, then re-enters run_game_loop.
struct TitleIdleRestartFadeState {
    std::uint8_t passes_completed = 0;
    std::uint8_t vblanks_in_pass = 0;
    bool active = false;
    bool complete = false;
};

// The state owned by ROM 0x1A9E. Each child state remains typed and
// independently testable; this coordinator preserves the source call order
// without carrying Genesis addresses or VDP state into the host runtime.
struct TitleLoopState {
    // FFDD2C / FFDD40: initial logo reveal delay and two-column cadence.
    std::int16_t logo_reveal_delay = 0x5A;
    std::int16_t logo_column_cadence = 0;

    TitleTextState text{};
    TitleCircuitLightState circuit_lights{0, 0};
    TitleAnimationTimer animation_timer{};
    TitleInputHistory input_history{};
    ControllerEventLatchState controller_events{};
    TitleExitPaletteFadeState exit_palette_fade{};
    TitleLogoRevealState logo{};
    TitleSpriteState sprite{};
    TitlePromptSchedule prompt{};

    std::uint8_t display_dirty_flags = 0;
    ControllerEventLatchResult last_controller_events{};
    bool initial_controller_sampled = false;
};

struct TitleLoopCallbacks {
    // ROM 0x0BD2 / 0x195C: host frame and controller sampling boundaries.
    std::function<void()> wait_for_vblank;
    std::function<std::uint16_t()> sample_controller;

    // ROM 0x1C1E, 0x2634, and 0x26D0: presentation updates resulting from
    // the already-converted state transitions.
    std::function<void(TitleTextAction)> apply_text_action;
    std::function<void(std::uint8_t)> publish_circuit_light_phase;
    std::function<void(std::uint16_t)> publish_animation_offset;

    // ROM 0x1DCA / 0x1DE0, 0x1E4E, and 0x1C6A: direct host presentation
    // boundaries. The conversion owns when these requests occur.
    std::function<void(TitlePromptVariant)> draw_prompt_variant;
    std::function<void(const TitleLogoRevealResult&)>
        render_logo_column;
    std::function<void()> copy_completion_palette;
    std::function<void(const TitleSpriteAdvanceResult&)>
        rebuild_sprite_projection;
    std::function<void(const ControllerEventLatchResult&)>
        publish_controller_events;
    std::function<void()> reset_vdp_upload_state;

    // ROM 0x1A9E's post-loop sample/palette/input finalization. The palette
    // operation is a host boundary; the input sequence itself is converted.
    std::function<void()> latch_palette_transition_events;
    std::function<void()> step_palette_transition;
};

void begin_title_idle_restart_fade(TitleIdleRestartFadeState& state);

// Advances one of the caller's 34 VBlank waits. Returns true on the final
// palette pass so the host can restart the ordinary boot/title path.
bool advance_title_idle_restart_fade(
    TitleIdleRestartFadeState& state,
    const TitleLoopCallbacks& callbacks);

// ROM 0x1A9E: execute one source title-loop frame in source order. This is
// the conversion-side coordinator the Qt frontend consumes; it is not a
// Genesis emulator loop. Missing presentation callbacks leave the typed
// source state advancing while keeping that host boundary explicit.
TitleLoopFrameResult advance_title_loop_frame(
    TitleLoopState& state, const TitleLoopCallbacks& callbacks);

// ROM 0x075E calls 0x53126 after the title loop and before 0x59092. Its six
// command-0x14 transactions each precede three VBlank waits. The command's
// actual audio resource remains an explicit host boundary.
struct TitleModeEntryServiceState {
    std::uint8_t transactions_started = 0;
    std::uint8_t vblanks_in_transaction = 0;
    bool active = false;
    bool complete = false;
};

struct TitleModeEntryServiceCallbacks {
    std::function<void()> wait_for_vblank;
    std::function<void(std::uint8_t)> run_audio_transaction;
    std::function<void()> flush_audio_bus;
    std::function<void()> service_audio_tick;
};

enum class TitleModeEntryServiceResult {
    waiting,
    complete,
};

void begin_title_mode_entry_service(
    TitleModeEntryServiceState& state,
    const TitleModeEntryServiceCallbacks& callbacks);
TitleModeEntryServiceResult advance_title_mode_entry_service_vblank(
    TitleModeEntryServiceState& state,
    const TitleModeEntryServiceCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
