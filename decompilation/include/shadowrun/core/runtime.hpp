#pragma once

#include "shadowrun/core/genesis_bus.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace shadowrun {

namespace ram {
constexpr GenesisAddress kFrameCounter = 0xFFDF88;
constexpr GenesisAddress kVblankTick = 0xFFF9EC;
constexpr GenesisAddress kFrameSubcounter = 0xFFF9F0;
constexpr GenesisAddress kFrameObserved = 0xFFF9F6;
constexpr GenesisAddress kFrameTarget = 0xFFF9EE;
constexpr GenesisAddress kInputPort0 = 0xFFDD22;
constexpr GenesisAddress kInputPort1 = 0xFFDD26;
constexpr GenesisAddress kControllerState0 = 0xFFDD20;
constexpr GenesisAddress kControllerState1 = 0xFFDD24;
constexpr GenesisAddress kControllerDecode0 = 0xFFDD04;
constexpr GenesisAddress kControllerDecode1 = 0xFFDD06;
constexpr GenesisAddress kInputStatus = 0xFFDDC0;
constexpr GenesisAddress kInputStatus2 = 0xFFDDC2;
constexpr GenesisAddress kControllerSignature = 0xFFEF0E;
constexpr GenesisAddress kObjectUploadStatus = 0xFFDBF9;
constexpr GenesisAddress kVdpUploadBuffer = 0xFFD87E;
constexpr GenesisAddress kDisplayBufferEven = 0xFFD8DC;
constexpr GenesisAddress kDisplayBufferOdd = 0xFFD8BC;
constexpr GenesisAddress kVdpDataWord = 0xFFDD08;
constexpr GenesisAddress kVdpDataWord2 = 0xFFDD0C;
constexpr GenesisAddress kVdpRegisterWord0A = 0xFFDD0A;
constexpr GenesisAddress kVdpRegisterWord0E = 0xFFDD0E;
constexpr GenesisAddress kVdpAddressWord = 0xFFDD12;
constexpr GenesisAddress kVdpClearCommandA = 0xFFDD16;
constexpr GenesisAddress kVdpClearCommandB = 0xFFDD14;
constexpr GenesisAddress kVblankFlags = 0xFFEF14;
constexpr GenesisAddress kVblankIdleGate = 0xFFDFE8;
constexpr GenesisAddress kVblankAnimationState = 0xFFDDC4;
constexpr GenesisAddress kVblankAnimationMode = 0xFFDF40;
constexpr GenesisAddress kVblankAnimationTimer = 0xFFDF90;
constexpr GenesisAddress kVblankAnimationIndex = 0xFFDF92;
constexpr GenesisAddress kVblankAnimationLock = 0xFFE18A;
constexpr GenesisAddress kVblankQueueA = 0xFFE100;
constexpr GenesisAddress kVblankQueueB = 0xFFE102;
constexpr GenesisAddress kTilePlaneSourceA = 0xFFDDDE;
constexpr GenesisAddress kTilePlaneSourceB = 0xFFDDE6;
constexpr GenesisAddress kTilePlaneWidth = 0xFFDDEA;
constexpr GenesisAddress kTilePlaneMask = 0xFFDF58;
constexpr GenesisAddress kTileMapIndexTable = 0xFF2700;
constexpr GenesisAddress kTilePixelBufferPointer = 0xFFDDFE;
constexpr GenesisAddress kBackgroundTileTable = 0xFF2710;
constexpr GenesisAddress kResourcePointer = 0xFFDD2C;
constexpr GenesisAddress kBackgroundQueueBusy = 0xFFE100;
constexpr GenesisAddress kBackgroundQueuePointer = 0xFFE104;
constexpr GenesisAddress kBackgroundQueueTimer = 0xFFE11C;
constexpr GenesisAddress kCameraX = 0xFFDDC6;
constexpr GenesisAddress kCameraY = 0xFFDDC8;
constexpr GenesisAddress kCameraXLowerLimit = 0xFFDDCA;
constexpr GenesisAddress kCameraYLowerLimit = 0xFFDDCC;
constexpr GenesisAddress kCameraXUpperLimit = 0xFFDDCE;
constexpr GenesisAddress kCameraYUpperLimit = 0xFFDDD0;
constexpr GenesisAddress kVblankWorkA = 0xFFDF2A;
constexpr GenesisAddress kVblankWorkAPointer = 0xFFDF2C;
constexpr GenesisAddress kVblankWorkB = 0xFFDF30;
constexpr GenesisAddress kVblankWorkBPointer = 0xFFDF32;
constexpr GenesisAddress kRandomState = 0xFFF9F8;
constexpr GenesisAddress kEntityStateRandomBucket = 0xFFF0DD;
constexpr GenesisAddress kEntityStateFrameTimer = 0xFFF0CE;
constexpr GenesisAddress kEntityStateFrameCounter0 = 0xFF018D;
constexpr GenesisAddress kEntityStateFrameCounter1 = 0xFF028D;
constexpr GenesisAddress kEntityStateFrameCounter2 = 0xFF038D;
constexpr GenesisAddress kEntityStateGlobalPhase = 0xFFE186;
constexpr GenesisAddress kEntityAnimationWakeTimer = 0xFFE166;
constexpr GenesisAddress kEntityStateScalingMode = 0xFFF0B7;
constexpr GenesisAddress kEntityStateScalingBase = 0xFFFBBe;
constexpr GenesisAddress kBootstrapMagic = 0xFFF9E8;
constexpr GenesisAddress kRamStart = 0xFF0000;
}

using VBlankRoutine = std::function<void(GenesisBus&)>;
using FrameWaitRoutine = std::function<void()>;

// These callbacks correspond one-for-one with calls made by ROM 0x1872. All
// reviewed bodies have direct defaults; hooks remain available as observation
// or replacement boundaries for frontend-specific integration.
struct VBlankRoutineHooks {
    VBlankRoutine routine_0x0EE6;
    VBlankRoutine routine_0x5672C;
    VBlankRoutine routine_0x2C92;
    VBlankRoutine routine_0x2CB2;
    VBlankRoutine routine_0x6148;
    VBlankRoutine routine_0x61F4;
};

struct VBlankServiceResult {
    std::uint16_t tick;
    std::uint8_t flags_seen;
};

struct PaletteTransitionResult {
    std::size_t palette_steps;
    std::size_t frame_waits;
};

struct DeltaVdpStreamResult {
    std::size_t rows_completed;
    std::size_t source_bytes_consumed;
    bool terminated_by_zero;
};

struct PrimaryVdpStreamResult {
    std::size_t segments;
    std::size_t longwords_emitted;
    std::size_t words_emitted;
    std::size_t source_bytes_consumed;
    bool split_at_64k_boundary;
};

struct CompressedVdpStreamResult {
    std::size_t output_bytes;
    std::size_t source_bytes_consumed;
    std::uint16_t initial_counter;
    std::uint16_t returned_word;
};

struct FrameTilePlaneUpdateResult {
    std::size_t draw_calls;
    std::size_t words_emitted;
    bool early_return;
};

// Small, typed translations of the low-level routines reached from the
// startup/game-loop cluster. Calls which are still unreviewed remain outside
// this class instead of being invented as placeholder game logic.
class ShadowrunRuntime {
public:
    explicit ShadowrunRuntime(GenesisBus& bus) : bus_(bus) {}

    // ROM 0x0BD2: increment the frame counter and wait for the interrupt-side
    // target to differ from the last observed value.
    void wait_for_frame() const;

    // ROM 0x0BF2: return the high-byte input flags and latch the status words.
    std::uint16_t poll_input_flags() const;

    // ROM 0x19F2: perform the two-phase controller port handshake and store
    // the inverted signature byte at FFEF0E.
    void initialise_controller_ports() const;

    // ROM 0x195C: sample both probed ports, retain the current low bytes,
    // expose newly asserted bits in the high bytes, and decode each nibble.
    void sample_controller_input() const;

    // ROM 0x0EC0: upload 64 words from FFD87E, preserving the per-word VDP
    // command/data write sequence.
    void upload_vdp_word_buffer() const;

    // ROM 0x1910: transfer two words from the alternating display buffer.
    void service_display_interrupt() const;

    // ROM 0x0F1C and 0x0F3C: the small VDP word-write paths reached by the
    // interrupt flags.
    // ROM 0x0E9C: emit the two control words derived from a D0 word.
    void write_vdp_command_sequence(std::uint16_t value) const;
    void write_vdp_addressed_word() const;
    void write_vdp_register_pair() const;

    // ROM 0x1232: stream a rectangular word block from A1 to VDP data. The
    // two DBF counters and the D2 word addend remain explicit; each row's
    // command is advanced by 0x80 in its low word.
    void stream_vdp_grid(GenesisAddress source,
                         std::uint32_t command,
                         std::uint16_t inner_dbf_counter,
                         std::uint16_t outer_dbf_counter,
                         std::uint16_t word_addend) const;

    // ROM 0x1728: decode the packed delta stream used by the second resource
    // list and emit its filtered words to VDP data.
    DeltaVdpStreamResult stream_delta_vdp(
        GenesisAddress source,
        std::uint32_t command,
        std::uint16_t d7_word_count,
        std::uint16_t d5_word_count,
        std::uint16_t d1_row_count,
        std::uint16_t d2_word_mask,
        std::uint16_t d3_output_skip) const;

    // ROM 0x0F5C and its 0x0F98 worker: copy a packed-size byte stream to VDP
    // data, splitting at a 64 KiB source boundary when the original routine
    // detects one. The command advances by the same byte offset.
    PrimaryVdpStreamResult stream_primary_vdp(
        GenesisAddress source,
        std::uint32_t command,
        std::uint32_t byte_count) const;

    // ROM 0x1530: decode the primary resource stream into the 2 KiB staging
    // ring and emit each decoded byte as a VDP word selected by the stream's
    // lane bit.
    CompressedVdpStreamResult decode_compressed_vdp(
        GenesisAddress source,
        GenesisAddress staging,
        std::uint16_t command) const;

    // ROM 0x6148 and 0x61F4: drain the two VBlank upload queues through the
    // reviewed 0x0F5C/0x0F98 copier and clear each queue's count.
    void service_vblank_upload_queue() const;
    void service_vblank_descriptor_queue() const;
    void service_object_list_upload() const;
    void stream_tile_plane(GenesisAddress source,
                           GenesisAddress tile_table,
                           std::uint16_t vdp_command) const;
    // ROM 0x4018 and 0x40E2: camera-edge tile refreshes. They preserve the
    // original 3D4A/3DDE span emitters and the 4216/41F0 origin adjustments;
    // the returned counts describe recorded VDP traffic, not a renderer.
    FrameTilePlaneUpdateResult stream_frame_tile_plane_vertical_0x4018(
        std::uint16_t initial_d0) const;
    FrameTilePlaneUpdateResult stream_frame_tile_plane_horizontal_0x40E2(
        std::uint16_t initial_d0) const;
    void service_vblank_tile_plane_a() const;
    void service_vblank_tile_plane_b() const;
    void service_vblank_state_animation() const;

    // ROM 0x1056, 0x0FFE, and 0x102A: direct VDP clear streams used by the
    // resource/setup path. The interrupt-mask manipulation around the two
    // 1024-longword streams belongs to the CPU boundary, not this bus model.
    void clear_vdp_word_block() const;
    void clear_vdp_block_a() const;
    void clear_vdp_block_b() const;

    // ROM 0x1872: service the VBlank-side scheduler. Directly reviewed queue
    // callees run by default; the remaining game-specific calls stay behind
    // injectable hooks.
    VBlankServiceResult service_vblank(
        const VBlankRoutineHooks& hooks = {}) const;

    // ROM 0x1342: execute a DBF-counted sequence of 0x0BD2 frame waits and
    // 0x0BF2 input polls. Passing one reproduces the two passes used by the
    // palette transitions.
    std::size_t service_frame_input(
        std::uint16_t dbf_counter,
        const FrameWaitRoutine& frame_wait = {}) const;

    // ROM 0x12F6 and 0x131C: perform seventeen palette steps, with the
    // two-pass 0x1342 wait/input service between each step. A supplied wait
    // routine replaces only 0x0BD2, leaving the 0x0BF2 input poll intact for
    // deterministic headless tests or a frontend scheduler.
    PaletteTransitionResult fade_palette_to_target(
        const FrameWaitRoutine& frame_wait = {}) const;
    PaletteTransitionResult fade_palette_to_black(
        const FrameWaitRoutine& frame_wait = {}) const;

    // ROM 0x0D44 and 0x0D3A.
    std::uint16_t advance_random() const;
    std::uint16_t random_below(std::uint16_t upper_bound) const;
    std::int16_t scaled_random(std::uint16_t multiplier) const;

    // ROM 0x0CEC: four words are copied to RAM and the remaining fifteen are
    // streamed to the VDP control port.
    void apply_vdp_configuration(
        const std::array<std::uint16_t, 19>& configuration) const;

    // The A0-backed form of ROM 0x0CEC. The source is a Genesis address to
    // nineteen big-endian words; the first four are stored in the VDP setup
    // RAM block and the remaining fifteen are streamed to VDP control.
    void apply_vdp_configuration_from_bus(GenesisAddress source) const;

    // The RAM-clear/checksum portion of ROM 0x0C2C. The later initialization
    // calls are intentionally not folded into this method until each target
    // has its own reviewed C++ translation.
    void clear_or_checksum_ram(std::uint32_t checksum_seed = 0) const;

private:
    GenesisBus& bus_;
};

} // namespace shadowrun
