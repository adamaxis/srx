#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
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

enum class TitleTextAction {
    None,
    StepPaletteTowardTarget,
    ShowCopyright,
};

struct TitleTextState {
    std::uint16_t completion_state = 0;
    std::int16_t countdown = 0x5A;
};

// ROM 0x1C1E: count down the completed-logo text delay. The returned action
// represents the source palette step or the unresolved copyright/text setup.
TitleTextAction advance_title_text_state(TitleTextState& state);

struct TitleSpriteState {
    std::int16_t cadence = 0;
    std::uint16_t phase = 0;
    std::int16_t x = 0x98;
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

} // namespace shadowrun::ghidra_conversion
