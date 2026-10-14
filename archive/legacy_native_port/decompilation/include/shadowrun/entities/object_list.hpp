#pragma once

#include "shadowrun/core/genesis_bus.hpp"

#include <cstddef>
#include <cstdint>

namespace shadowrun {

namespace ram {
constexpr GenesisAddress kObjectListCount = 0xFFDD02;
constexpr GenesisAddress kObjectListOutputPointer = 0xFFDD18;
constexpr GenesisAddress kObjectListCountMirror = 0xFFDD1C;
constexpr GenesisAddress kObjectListVdpCommand = 0xFFDD10;
constexpr GenesisAddress kObjectListInitialOutput = 0xFFD986;
constexpr GenesisAddress kObjectListScratch = 0xFFD97E;
constexpr GenesisAddress kObjectGenerationReset = 0xFFDFEA;
constexpr GenesisAddress kObjectGenerationProgress = 0xFFDFEC;
constexpr GenesisAddress kObjectGenerationBaseX = 0xFFDFEE;
constexpr GenesisAddress kObjectGenerationBaseY = 0xFFDFF0;
constexpr GenesisAddress kEntityTimedGate = 0xFFDFF2;
constexpr GenesisAddress kEntityTimedGateDelay = 0xFFDFF4;
constexpr GenesisAddress kEntityRandomGate = 0xFFDFF6;
constexpr GenesisAddress kEntityRandomGateDelay = 0xFFDFF8;
constexpr GenesisAddress kEntityRandomAux = 0xFFDFFA;
constexpr GenesisAddress kEntityRandomXOffset = 0xFFDFFE;
constexpr GenesisAddress kEntityRandomYOffset = 0xFFE000;
constexpr GenesisAddress kEntityRandomValue = 0xFFE002;
constexpr GenesisAddress kEntityRandomPhaseGate = 0xFFE004;
constexpr GenesisAddress kEntityObjectControlPointer = 0xFFFC04;
constexpr GenesisAddress kObjectOutputCount = 0xFFDF30;
constexpr GenesisAddress kObjectOutputPointer = 0xFFDF32;
constexpr GenesisAddress kObjectOutputCursorByte = 0xFFDF35;
}

// Four-word records consumed by ROM 0x1378 and 0x13BE. The first word uses
// $8000 as the list terminator; the remaining names stay deliberately neutral
// until the surrounding map/entity tables are typed.
struct RawObjectRecord {
    std::uint16_t word0;
    std::uint16_t word1;
    std::uint16_t word2;
    std::uint16_t word3;
};

// Eight-byte records emitted to the runtime object list.
struct BuiltObjectRecord {
    std::int16_t word0;
    std::uint8_t byte2;
    std::uint8_t ordinal;
    std::uint16_t word4;
    std::int16_t word6;
};

enum class ObjectListVariant {
    Base = 0x1378,
    WithWord3Offset = 0x13BE
};

// RAM bookkeeping performed by both ROM builders. next_output_index models
// the post-increment A3 pointer relative to the output array supplied by the
// caller; object_count is the word stored at FFDD02 and FFDD1C.
struct ObjectListBuildState {
    std::uint16_t object_count;
    std::size_t next_output_index;
};

struct ObjectListBusResult {
    std::size_t built;
    GenesisAddress next_output_address;
    std::uint16_t object_count;
    std::uint8_t next_ordinal;
};

struct EntityObjectOutputResult {
    std::size_t emitted;
    GenesisAddress next_output_address;
    std::uint16_t next_count;
    std::uint16_t next_ordinal;
};

struct GeneratedObjectListResult {
    bool generated;
    std::size_t lists_processed;
    std::size_t objects_emitted;
    std::uint16_t progress;
    std::uint16_t next_count;
    std::uint8_t next_ordinal;
};

struct AnimatedEntityOutputResult {
    bool emitted;
    std::size_t passes;
    std::size_t objects_emitted;
    GenesisAddress next_output_address;
    std::uint16_t next_count;
    std::uint8_t next_ordinal;
};

struct EntityVariantOutputResult {
    bool emitted;
    std::uint8_t selector;
    GenesisAddress table_address;
    std::size_t lists_processed;
    std::size_t objects_emitted;
    GenesisAddress next_output_address;
    std::uint16_t next_count;
    std::uint8_t next_ordinal;
};

struct EntityCycleOutputResult {
    bool emitted;
    std::size_t descriptor_records_emitted;
    std::size_t list_records_emitted;
    GenesisAddress next_output_address;
    std::uint16_t next_count;
    std::uint8_t next_ordinal;
};

struct EntitySlotObjectOutputResult {
    std::size_t slots_examined;
    std::size_t active_slots;
    std::size_t projected_slots;
    std::size_t objects_emitted;
    GenesisAddress next_output_address;
    std::uint16_t next_count;
    std::uint8_t next_ordinal;
};

struct EntityPhaseObjectOutputResult {
    std::size_t lists_processed;
    std::size_t objects_emitted;
    GenesisAddress next_output_address;
    std::uint16_t next_count;
    std::uint8_t next_ordinal;
};

struct EntityDispatchOutputResult {
    bool handled;
    GenesisAddress target_address;
    std::size_t lists_processed;
    std::size_t objects_emitted;
    GenesisAddress next_output_address;
    std::uint16_t next_count;
    std::uint8_t next_ordinal;
};

struct EntityRandomScanResult {
    bool triggered;
    bool scan_entered;
    bool unresolved_0xAFF2_call;
    std::size_t candidate_records_examined;
    std::size_t child_records_initialised;
    std::size_t objects_emitted;
    GenesisAddress next_output_address;
    std::uint16_t next_count;
    std::uint8_t next_ordinal;
};

struct EntityStateScanResult {
    std::size_t records_examined;
    std::size_t records_initialised;
};

struct EntityStateContinuationResult {
    std::size_t records_examined;
    std::size_t records_initialised;
    bool unresolved_0xB330_path;
    bool z80_command_dispatched;
};

// Direct translation of ROM 0x61A0. descriptor_address points at the selected
// descriptor record from the 0x4ACC table; source_record_address supplies the
// entity word read at offset +0x2A. The routine appends one eight-byte record
// through the Genesis RAM pointer at FFDF32 and updates its byte cursor/count.
void append_descriptor_object(GenesisBus& bus,
                              GenesisAddress descriptor_address,
                              GenesisAddress source_record_address);

// Direct translation of ROM 0x622C/0x624C. The state list is addressed by the
// entity longword at +0x12. A3, D5, and the object-list count are explicit
// arguments/state so this remains usable from the later 0x4658 reconstruction.
// max_records is only a host safety bound; valid lists terminate at 0x8000.
EntityObjectOutputResult append_entity_state_objects(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint16_t ordinal,
    std::size_t max_records);

// Direct translation of ROM 0xAEBA. It advances the signed interpolation
// progress, selects one of the four ROM lists at 0xAF3E, computes the 32
// fixed-point offsets from 0x15E16, and invokes the 0x1378 object builder for
// each row. The list/record bound is host-side protection for malformed data.
GeneratedObjectListResult generate_object_list_frame(
    GenesisBus& bus,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list);

// Direct translation of ROM 0x5B98. It emits sixteen copies of the selected
// object list using the interleaved signed factor words at 0x5BEE and the
// four-entry pointer table at 0x5C2E. max_records_per_list bounds malformed
// data without changing the sentinel-driven valid path.
AnimatedEntityOutputResult emit_animated_entity_objects(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list);

// Direct translation of ROM 0x5C7E/0x5D0A. It selects the table family from
// the entity's +0x4C/+0x9C byte and the control block at FFFC04, preserves the
// +0xA6 state gate, and emits the selected one- or four-list combination.
EntityVariantOutputResult emit_entity_variant_objects(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list);

// Direct translation of ROM 0x4E86. It preserves the byte timer/reset path,
// the 0x4F42 descriptor selection, the 0x61A0 queue append, and the trailing
// 0x13BE list emission using the signed words at 0x4F22.
EntityCycleOutputResult emit_entity_cycle_objects(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list);

// Direct translation of ROM 0x481E. It projects the ten compact slots at
// FF164C with 0x5A62, selects the type-specific list source, and emits each
// visible slot through the reviewed 0x1378 builder.
EntitySlotObjectOutputResult emit_entity_slot_objects(
    GenesisBus& bus,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list);

// Direct translations of the two phase-list emitters called by 0x4658.
// 0x5936 emits its ordinary one-list form unless the +0x0E/sprite phase
// selects the paired offset form; 0x59EA directly selects that paired form.
EntityPhaseObjectOutputResult emit_entity_phase_objects_0x5936(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list);
EntityPhaseObjectOutputResult emit_entity_phase_objects_0x59EA(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list);

// Direct translation of ROM 0x5662, plus the 0x557E countdown gate that calls
// it. The selected descriptor is appended through 0x61A0 when the timer wraps
// and its trailing list is emitted through 0x13BE.
EntityPhaseObjectOutputResult emit_entity_timed_objects_0x5662(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list);

// Direct translation of the normal object-emission path at ROM 0x56DE. It
// shares the 0x5756 descriptor table with 0x5662, but retains its distinct
// phase limit of sixteen. The phase-cutoff branch clears +0xB2 and the
// indexed-object byte, then enters the directly typed 0x55C2C auxiliary
// state-grid update.
EntityPhaseObjectOutputResult emit_entity_state_objects_0x56DE(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list);

// Direct translation of the normal path in ROM 0x5322, reached from the
// 0x52BE state handler. It uses the separate 0x5876 descriptor table and its
// phase-nine cutoff; the wrapper invokes the separate 0x52CC scan afterward.
EntityPhaseObjectOutputResult emit_entity_state_objects_0x5322(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list);

// The four additional 0x490A handlers share the same verified normal
// timer/descriptor/list contract but use their own ROM table, flags, timer
// reset, and phase cutoff. The 0x4CDE/0x4D54 cutoff continuations are exposed
// separately below.
EntityPhaseObjectOutputResult emit_entity_state_objects_0x521E(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list);
EntityPhaseObjectOutputResult emit_entity_state_objects_0x508A(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list);
EntityPhaseObjectOutputResult emit_entity_state_objects_0x4FE2(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list);
EntityPhaseObjectOutputResult emit_entity_state_objects_0x5152(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list);

// Resolve the word-sized state byte through ROM 0x490A. Only handlers whose
// output contracts are already typed are executed here; an untyped target is
// returned with handled=false so the enclosing 0x4658 pass can retain it as
// an explicit reconstruction boundary.
EntityDispatchOutputResult dispatch_entity_state_objects_0x490A(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list);

// Direct translation of ROM 0x5468. It copies the reviewed child-state
// fields from source_record_address to target_record_address and sets the
// target's random-dispatch phase. This is used by the 0x53C4 scan and its
// auxiliary-record path.
void initialise_entity_random_child_0x5468(
    GenesisBus& bus,
    GenesisAddress source_record_address,
    GenesisAddress target_record_address);

// Direct translation of ROM 0x5622. It clears the ten +0xDE bytes in the
// phase records and applies the caller's byte mask to +0xDF.
std::size_t clear_entity_generation_records_0x5622(
    GenesisBus& bus,
    std::uint8_t mask = 0xFDU);

// Direct translation of the gate, nine-record scan, auxiliary setup, and the
// ordinary matching-record 0xAFF2 initializer in ROM 0x53C4. Only the
// 0xAFF2 -> 0xB330 fallback is retained as an explicit result boundary.
EntityRandomScanResult run_entity_random_scan_0x53C4(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list);

// Direct translation of the ten-record cutoff continuation at ROM 0x52CC.
// It scans FF0100 through FF0A00 and copies the source entity into eligible
// records without entering the separate animation subsystem.
EntityStateScanResult scan_entity_state_records_0x52CC(
    GenesisBus& bus,
    GenesisAddress source_record_address);

// Direct scans for the post-cutoff continuations at ROM 0x4CEC and 0x4D62.
// Their ordinary record setup and the 0x530DE -> 0x52F3C Z80 command path are
// bus-backed; only the nested 0xAFF2 -> 0xB330 animation path remains a
// reported boundary.
EntityStateContinuationResult scan_entity_state_records_0x4CEC(
    GenesisBus& bus,
    GenesisAddress source_record_address);
EntityStateContinuationResult scan_entity_state_records_0x4D62(
    GenesisBus& bus,
    GenesisAddress source_record_address);

EntityPhaseObjectOutputResult emit_entity_timed_objects_0x557E(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list);

// Direct translation of the self-contained object-emission body at ROM
// 0x5494. The later 0x53C4 scan/dispatch that calls it remains a separate
// boundary because it walks additional state records after this return.
EntityPhaseObjectOutputResult emit_entity_random_objects_0x5494(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress output_address,
    std::uint8_t first_ordinal,
    std::size_t max_records_per_list);

// ROM 0x0E34: establish the initial object-output stream and clear its VDP
// staging words. The command is derived from FFDD10 through ROM 0x0E9C.
void initialise_object_output(GenesisBus& bus);

std::size_t build_object_list(const RawObjectRecord* input,
                              std::size_t input_capacity,
                              BuiltObjectRecord* output,
                              std::size_t output_capacity,
                              std::int16_t word0_base,
                              std::int16_t word6_base,
                              std::uint16_t word4_flags,
                              std::int16_t word3_offset,
                              std::uint8_t first_ordinal,
                              ObjectListBuildState& state,
                              ObjectListVariant variant);

// Direct translation of the record loop shared by ROM 0x1378 and 0x13BE.
// Arithmetic is explicitly narrowed to preserve 68000 word wraparound.
std::size_t build_object_list(const RawObjectRecord* input,
                              std::size_t input_capacity,
                              BuiltObjectRecord* output,
                              std::size_t output_capacity,
                              std::int16_t word0_base,
                              std::int16_t word6_base,
                              std::uint16_t word4_flags,
                              std::int16_t word3_offset,
                              std::uint8_t first_ordinal,
                              std::uint16_t& object_count,
                              ObjectListVariant variant);

// Bus-backed translation of the same 0x1378/0x13BE loops. input_address and
// output_address are Genesis addresses held in A2/A3 by the original code;
// max_records is a host-side safety bound and does not alter valid ROM data.
ObjectListBusResult build_object_list_from_bus(
    GenesisBus& bus,
    GenesisAddress input_address,
    GenesisAddress output_address,
    std::size_t max_records,
    std::int16_t word0_base,
    std::int16_t word6_base,
    std::uint16_t word4_flags,
    std::int16_t word3_offset,
    std::uint8_t first_ordinal,
    ObjectListVariant variant);

} // namespace shadowrun
