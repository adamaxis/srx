#pragma once

#include "shadowrun/genesis_bus.hpp"
#include "shadowrun/runtime.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>

namespace shadowrun {

namespace session_ram {
constexpr GenesisAddress kBootstrapMagic = 0xFFF9E8;
constexpr GenesisAddress kRandomState = 0xFFF9F8;
constexpr GenesisAddress kRandomAddend = 0xFFF9EC;
constexpr GenesisAddress kReadyFlag0 = 0xFFFC08;
constexpr GenesisAddress kReadyFlag1 = 0xFFFC0A;
constexpr GenesisAddress kMainLoopControl = 0xFFDFE8;
constexpr GenesisAddress kFrameTransition = 0xFFDFE6;
constexpr GenesisAddress kModeBlock = 0xFFFBFE;
constexpr GenesisAddress kModeByte = 0xFFFBF8;
constexpr GenesisAddress kSessionMode = 0xFFDDC4;

constexpr GenesisAddress kWorkLong12 = 0xFFFC12;
constexpr GenesisAddress kWorkLong16 = 0xFFFC16;
constexpr GenesisAddress kWorkLong1A = 0xFFFC1A;
constexpr GenesisAddress kWorkLong1E = 0xFFFC1E;
constexpr GenesisAddress kWorkLong22 = 0xFFFC22;
constexpr GenesisAddress kWorkWord26 = 0xFFFC26;
constexpr GenesisAddress kWorkWord27 = 0xFFFC27;
constexpr GenesisAddress kWorkLong28 = 0xFFFC28;
constexpr GenesisAddress kWorkLong2A = 0xFFFC2A;
constexpr GenesisAddress kWorkLong2C = 0xFFFC2C;
constexpr GenesisAddress kWorkLong30 = 0xFFFC30;
constexpr GenesisAddress kWorkLong34 = 0xFFFC34;
constexpr GenesisAddress kWorkLong38 = 0xFFFC38;

constexpr GenesisAddress kFrameInput0 = 0xFFDD22;
constexpr GenesisAddress kFrameInput1 = 0xFFDD26;
constexpr GenesisAddress kFrameInputLatch0 = 0xFFDD04;
constexpr GenesisAddress kFrameInputLatch1 = 0xFFDD06;
constexpr GenesisAddress kFrameActive = 0xFFE18A;
constexpr GenesisAddress kEntityAnimationUpdateGate = 0xFFE18B;
constexpr GenesisAddress kEntityAnimationUpdateTimer = 0xFFE18C;
constexpr GenesisAddress kEntityPointerTable = 0xFFDE8A;
constexpr GenesisAddress kFrameListBuildMode = 0xFFE175;
constexpr GenesisAddress kFrameListPrimaryCount = 0xFFDF94;
constexpr GenesisAddress kFrameListPrimary = 0xFFDF96;
constexpr GenesisAddress kFrameListSecondaryCount = 0xFFDFC8;
constexpr GenesisAddress kFrameListSecondary = 0xFFDFCA;
constexpr GenesisAddress kFrameAnimationMode = 0xFFDFE0;
constexpr GenesisAddress kFrameClipHorizontalMode = 0xFFE173;
constexpr GenesisAddress kFrameClipVerticalMode = 0xFFE174;
constexpr GenesisAddress kFrameTransformProgress = 0xFFDF5C;
constexpr GenesisAddress kFrameTransformState = 0xFFDF5E;
constexpr GenesisAddress kFrameMovementCacheVertical = 0xFFDF36;
constexpr GenesisAddress kFrameMovementCacheHorizontal = 0xFFDF38;
constexpr GenesisAddress kFrameSpatialStatus = 0xFFDF5A;
constexpr GenesisAddress kFrameEntityCollisionState = 0xFFDF64;
constexpr GenesisAddress kFrameEntityGlobalGate = 0xFFDF84;
constexpr GenesisAddress kEntityAnimationResourceMode = 0xFFDF40;
constexpr GenesisAddress kEntityAnimationResourceReference = 0xFF13AE;
constexpr GenesisAddress kEntityAnimationResourceScratch = 0xFF1380;
constexpr GenesisAddress kEntityAnimationResourceSource = 0xFFDD3C;
constexpr GenesisAddress kEntityAnimationResourceProbeX = 0xFFDD60;
constexpr GenesisAddress kEntityAnimationResourceProbeY = 0xFFDD62;
constexpr GenesisAddress kEntityAnimationResourceGateState = 0xFFDFD8;
constexpr GenesisAddress kEntityAnimationMode2Descriptor = 0xFFDE1E;
constexpr GenesisAddress kEntityAnimationMode2Selection = 0xFFDD40;
constexpr GenesisAddress kEntityAnimationMode2Offset = 0xFFDD42;
constexpr GenesisAddress kEntityAnimationMode2Timer = 0xFFE11E;
constexpr GenesisAddress kEntityAnimationResourceEntityBase = 0xFF0100;
constexpr std::size_t kEntityAnimationResourceEntityCount = 10;
constexpr GenesisAddress kEntityAnimationMode2RecordBase = 0xFF0400;
constexpr std::size_t kEntityAnimationMode2RecordCount = 7;
constexpr std::size_t kEntityAnimationMode2RecordStride = 0x100;
constexpr GenesisAddress kEntityAnimationDispatchTimer = 0xFFE188;
constexpr GenesisAddress kFrameEntityReactionOffsets = 0xFFDF44;
constexpr GenesisAddress kFrameEntityReactionCameraX = 0xFFDF50;
constexpr GenesisAddress kFrameEntityReactionCameraY = 0xFFDF52;
constexpr GenesisAddress kFrameEntityGateTimer = 0xFFDF86;
constexpr GenesisAddress kFrameEntityGateSource = 0xFFE162;
constexpr GenesisAddress kEntityReactionTileCounterBase = 0xFFE179;
constexpr GenesisAddress kEntityReactionTileCounterIndex = 0xFFF0C8;
constexpr GenesisAddress kEntityReactionInputFlags = 0xFFEF10;
constexpr GenesisAddress kEntityReactionSpecialGate = 0xFFE16F;
constexpr GenesisAddress kEntityReactionStateByte = 0xFFE178;
constexpr GenesisAddress kEntityReactionSelection = 0xFFE16E;
constexpr GenesisAddress kEntityReactionSubmode = 0xFFE17F;
constexpr GenesisAddress kEntityReactionTimer = 0xFFE11A;
constexpr GenesisAddress kEntityReactionModeGate = 0xFFFC10;
constexpr GenesisAddress kEntityReactionResetInput = 0xFFFC1C;
constexpr GenesisAddress kEntityReactionTransitionState = 0xFFDF3A;
constexpr GenesisAddress kEntityReactionTransitionStep = 0xFFDF3C;
constexpr GenesisAddress kEntityReactionAuxiliaryOffset = 0xFFDFE4;
constexpr GenesisAddress kEntityReactionOutputTable = 0xFFD740;
constexpr GenesisAddress kEntityReactionAlternateTileMap = 0xFF4700;
constexpr GenesisAddress kEntityTerrainBoundsXLower = 0xFFDDD2;
constexpr GenesisAddress kEntityTerrainBoundsYLower = 0xFFDDD4;
constexpr GenesisAddress kEntityTerrainBoundsXUpper = 0xFFDDD6;
constexpr GenesisAddress kEntityTerrainBoundsYUpper = 0xFFDDD8;
constexpr GenesisAddress kEntityTerrainCollisionMarker = 0xFFDD28;
constexpr GenesisAddress kEntityAnimationEventSpecialArgument = 0xFFDD66;
constexpr GenesisAddress kEntityAnimationEventTable = 0xFFF9FA;
constexpr GenesisAddress kEntityAnimationTargetX = 0xFFDD2A;
constexpr GenesisAddress kEntityAnimationTargetY = 0xFFDD2C;
constexpr GenesisAddress kEntityAnimationSavedDirection = 0xFFDD48;
constexpr GenesisAddress kEntityTerrainSelectedOffset = 0xFFDFE4;
constexpr GenesisAddress kEntityTerrainOutOfBounds = 0xFFDF68;
constexpr GenesisAddress kEntityTerrainCollisionGate = 0xFFDFE2;
constexpr GenesisAddress kEntityTerrainMapRecords = 0xFFDDEC;
constexpr GenesisAddress kEntitySpawnFlags = 0xFFFC40;
constexpr GenesisAddress kEntitySpawnGate = 0xFFFC0E;
constexpr GenesisAddress kFrameEntityTransitionCounter = 0xFFE177;
constexpr GenesisAddress kFrameEntityGlobalPointer = 0xFFE12E;
constexpr GenesisAddress kFrameEntityGlobalReset = 0xFFFC3C;
constexpr GenesisAddress kFrameEntityResourceControl = 0xFFE110;
constexpr GenesisAddress kFrameEntityResourceSelection = 0xFFE128;
constexpr GenesisAddress kFrameEntityResourceGate = 0xFFE176;
constexpr GenesisAddress kFrameEntityResourceStateA = 0xFFE180;
constexpr GenesisAddress kFrameEntityResourceStateB = 0xFFE182;
constexpr GenesisAddress kFrameEntityResourceMode = 0xFFF0AE;
constexpr GenesisAddress kFrameEntityResourceSubmode = 0xFFFBF9;
constexpr GenesisAddress kEntityAnimationState = 0xFFF0CC;
constexpr GenesisAddress kEntityAnimationGate = 0xFFF09D;
constexpr GenesisAddress kEntityAnimationResetWord = 0xFFDFDA;
constexpr GenesisAddress kEntityAnimationResetCounter = 0xFFDFDC;
constexpr GenesisAddress kEntityPhaseBase = 0xFF0100;
constexpr GenesisAddress kEntityPhaseState = 0xFFDE88;
constexpr GenesisAddress kEntityTimingWord = 0xFFDD3E;
constexpr GenesisAddress kEntityTimingAccumulator = 0xFFDD3C;
constexpr GenesisAddress kSpriteSlotFlags = 0xFFD86C;
constexpr GenesisAddress kPostObjectRecordBase = 0xFF0B00;
constexpr GenesisAddress kPostObjectState = 0xFFF11E;
constexpr GenesisAddress kPostObjectScroll = 0xFFF122;
constexpr GenesisAddress kPostObjectSecondaryCounter = 0xFFF140;
constexpr GenesisAddress kPostObjectCountdown = 0xFFF142;
constexpr std::size_t kEntityPhaseCount = 16;
constexpr std::size_t kEntityPhaseStride = 0x100;
constexpr std::size_t kSpriteSlotCount = 0x12;
constexpr std::size_t kPostObjectRecordCount = 6;
}

using FrameWindowRoutine = std::function<void(GenesisBus&)>;
using FrameEntityRoutine = std::function<void(GenesisBus&, GenesisAddress)>;
using EntityAnimationContinuationRoutine = std::function<void(
    GenesisBus&, GenesisAddress, GenesisAddress)>;

struct EntityAnimationAcaeCallContext {
    GenesisAddress target;
    std::uint16_t d0;
    GenesisAddress a1;
    GenesisAddress a0;
    bool carry;
};

using EntityAnimationAcaeContinuationRoutine = std::function<void(
    GenesisBus&, GenesisAddress, EntityAnimationAcaeCallContext&)>;

struct EntityAnimationResourceCallContext {
    GenesisAddress target;
    GenesisAddress a1;
    bool carry;
};

using EntityAnimationResourceContinuationRoutine = std::function<void(
    GenesisBus&, GenesisAddress, EntityAnimationResourceCallContext&)>;

// The entity reaction call inherits D4/D5/D6 and A2 from the preceding
// indirect display routine. A provider makes that live-register boundary
// explicit when the direct 0xB7B8 body is used from the frame pass.
struct EntityReactionRegisterContext {
    std::uint16_t d0;
    std::uint16_t d4;
    std::uint16_t d5;
    std::uint16_t d6;
    GenesisAddress auxiliary_base;
};

using EntityReactionRegisterProvider = std::function<
    EntityReactionRegisterContext(GenesisBus&, GenesisAddress)>;

struct ObjectOutputPassHooks {
    FrameEntityRoutine routine_0x557E;
    FrameEntityRoutine routine_0x53C4;
    FrameEntityRoutine routine_0x5936;
    FrameEntityRoutine routine_0x59EA;
    FrameEntityRoutine routine_0x490A;
    FrameEntityRoutine routine_0x4E86;
    FrameWindowRoutine routine_0x481E;
    FrameWindowRoutine routine_0x5D29C;
};

// The per-record body of ROM 0xA8EA contains several direct subroutine calls
// and one handler call through record+0x3A. The typed body supplies the
// reviewed state transitions and leaves still-indirect routines injectable.
struct FrameEntityUpdateHooks {
    FrameEntityRoutine routine_0x0391A;
    FrameEntityRoutine routine_0xAB02;
    FrameEntityRoutine routine_0xACAE;
    FrameEntityRoutine routine_0xAC84;
    FrameEntityRoutine routine_0xB7B8;
    EntityReactionRegisterProvider entity_reaction_registers;
    FrameEntityRoutine routine_0xEB92;
    FrameEntityRoutine routine_0x1A304;
    FrameWindowRoutine routine_0x1E2E0;
    FrameWindowRoutine routine_0x1E2EC;
    FrameEntityRoutine routine_0x1DC4C;
    FrameEntityRoutine routine_0x03404;
    FrameWindowRoutine routine_0x03548;
    FrameWindowRoutine routine_0x038FA;
    FrameEntityRoutine routine_0xA8EA_handler;
    // The 0x138B8/0x13904 entry points overlap the larger animation state
    // machine. This callback preserves the exact ROM target when that next
    // state-machine body has not yet been translated.
    EntityAnimationContinuationRoutine entity_animation_continuation;
    // The direct 0xACAE body supplies D0/A1/carry as an explicit call
    // context for its still-untranslated state/resource callees.
    EntityAnimationAcaeContinuationRoutine entity_animation_acae_continuation;
    // The 0x13F76 resource gate exposes 0x14D8E's carry result and the
    // 0x1E2E0 queue call without collapsing them into a bool-only hook.
    EntityAnimationResourceContinuationRoutine
        entity_animation_resource_continuation;
};

// These hooks correspond to the game-specific calls inside ROM 0x08C0. The
// surrounding frame sequencing is translated directly. The 0x4618 phase and
// the reviewed object-output portion of 0x4658 have direct defaults; hooks
// preserve the remaining subsystem boundaries at their original arguments.
struct FrameWindowHooks {
    FrameWindowRoutine routine_0xA8EA;
    FrameEntityUpdateHooks entity_update;
    FrameWindowRoutine routine_0x3A1C;
    FrameWindowRoutine routine_0x4618;
    FrameEntityRoutine routine_0x600E;
    FrameEntityRoutine routine_0x60FA;
    FrameWindowRoutine routine_0x4658;
    FrameWindowRoutine routine_0x12F6;
    ObjectOutputPassHooks object_output;
    std::function<void()> frame_wait;
};

struct FrameWindowResult {
    std::size_t iterations;
};

struct EntityPointerSortResult {
    std::size_t passes;
    std::size_t comparisons;
    std::size_t swaps;
    bool sorted;
};

struct FrameEntityListResult {
    std::size_t primary_records;
    std::size_t secondary_records;
};

struct FrameEntityUpdateResult {
    std::size_t records_examined;
    std::size_t active_records;
    std::size_t timer_decrements;
    std::size_t animation_fallbacks;
    std::size_t handler_calls;
    std::size_t unresolved_calls;
};

struct EntityCollisionPassResult {
    std::size_t candidates_examined;
    std::size_t collisions;
    std::size_t transforms;
    std::size_t unresolved_calls;
};

struct EntityAnimationPassResult {
    std::size_t records_examined;
    bool command_dispatched;
    bool selected_record_changed;
    bool reset_written;
    std::size_t unresolved_calls;
};

struct EntityAnimationEntryResult {
    bool timer_expired;
    bool suppressed_by_global_gate;
    bool timer_rearmed;
    std::size_t continuation_calls;
    std::size_t unresolved_calls;
};

struct EntityAnimationAcaeResult {
    bool entered;
    std::size_t continuation_calls;
    std::size_t unresolved_calls;
};

struct EntityAnimationCleanupResult {
    bool global_reset_cleared;
    bool active_flag_cleared;
    bool indexed_state_cleared;
};

struct EntityAnimationFallbackResult {
    bool delay_updated;
    bool handler_installed;
    bool frame_selected;
    std::uint16_t selected_frame;
};

struct EntityAnimationDelayResult {
    std::uint16_t sampled_base;
    std::uint16_t state_adjustment;
    std::uint16_t delay_written;
};

struct EntityAnimationResourceGateResult {
    bool blocked_by_global_gate;
    bool resource_queued;
    std::size_t continuation_calls;
    std::size_t unresolved_calls;
};

struct EntityAnimationResourceDecisionResult {
    bool carry;
    bool terrain_nonzero;
    bool terrain_collision_gate_active;
    bool occupancy_scan_stopped;
    std::uint16_t occupancy_checks;
    GenesisAddress scratch_record;
};

struct EntityAnimationResourceSpawnResult {
    bool carry;
    bool animation_slot_gate_passed;
    bool first_record_allocated;
    bool second_record_allocated;
    bool command_dispatched;
    bool mode2_transition_unresolved;
    GenesisAddress first_record;
    GenesisAddress second_record;
    std::uint16_t first_record_index;
    std::uint16_t second_record_index;
    std::size_t unresolved_calls;
};

struct EntityAnimationMode2ResourceResult {
    bool mode14_gate_armed;
    bool mode2_path_entered;
    bool special_gate_preexisting;
    bool command_dispatched;
    bool mode2_flags_cleared;
    std::size_t records_created;
    std::size_t sprite_slots_allocated;
    std::size_t unresolved_calls;
};

struct EntityAnimationMode2FlagResult {
    std::size_t records_examined;
    std::size_t records_marked;
};

struct EntityAnimationRouteResourceResult {
    bool carry;
    bool record_allocated;
    bool sprite_allocated;
    bool linked_record_present;
    bool linked_adjustment_applied;
    GenesisAddress record_address;
    std::uint16_t record_index;
    std::uint16_t sprite_slot_index;
    std::uint16_t transformed_distance;
};

struct EntityAnimationPhaseSelectionResult {
    std::uint8_t selected_phase;
    bool selected;
};

struct EntityAnimationEventParseResult {
    std::uint16_t accepted_events;
    std::uint16_t special_events;
    GenesisAddress output_end;
};

// The 0x13A92/0x13B12 path intentionally inherits a few 68000 data
// registers from its caller. Keeping them explicit avoids quietly inventing
// a C++ local value for the 0x125FA conversion when this branch is invoked
// from a larger CPU-context reconstruction.
struct EntityAnimationRegisterContext {
    std::uint16_t d4 = 0;
    std::uint16_t d5 = 0;
    std::uint16_t d7 = 0;
};

// ROM 0x1E0DC reaches 0x044BC with D1 and A6 inherited from its caller. They
// are separate from the event-transition register context above because the
// two call sites preserve different live registers.
struct EntityAnimationMode2RegisterContext {
    std::uint16_t d1 = 0;
    GenesisAddress a6 = 0;
};

struct EntityAnimationRateDecisionResult {
    bool carry;
    std::uint16_t residual_d1;
};

struct EntityAnimationPhaseTransitionResult {
    bool carry;
    bool phase_advanced;
    std::uint8_t previous_phase;
    std::uint8_t new_phase;
    std::uint16_t accepted_events_considered;
};

struct EntityAnimationBranchResult {
    bool entered;
    bool phase_transition_called;
    bool phase_advanced;
    bool display_updated;
    std::uint8_t previous_phase;
    std::uint8_t new_phase;
};

struct EntityAnimationPositionGateResult {
    bool carry;
    std::uint16_t distance;
    std::uint16_t rounded_minor_axis;
};

struct EntityAnimation142AeResult {
    bool entered;
    bool position_gate_called;
    bool position_gate_carry;
    bool linked_record_path;
    bool phase_position_updated;
    GenesisAddress continuation_target;
    std::uint16_t position_distance;
    std::uint16_t indexed_minor_axis;
    std::size_t continuation_calls;
    std::size_t unresolved_calls;
};

struct EntityAnimation1450EResult {
    bool entered;
    bool first_probe_attempted;
    bool reverse_probe_attempted;
    bool target_position_written;
    bool position_gate_called;
    bool position_gate_carry;
    bool routed_to_0x145A2;
    GenesisAddress source_record;
    std::uint16_t first_probe_status;
    std::uint16_t reverse_probe_status;
    std::size_t continuation_calls;
    std::size_t unresolved_calls;
};

struct EntityAnimation13D0CResult {
    bool entered;
    bool routed_to_0x13E40;
    bool blocked_by_animation_mode;
    bool linked_record_present;
    bool linked_position_copied;
    bool distance_table_used;
    bool target_position_copied;
    bool target_position_built;
    bool position_gate_called;
    bool position_gate_carry;
    bool routed_to_0x145A2;
    bool handler_installed;
    GenesisAddress linked_record;
    GenesisAddress timing_row;
    std::uint16_t direction;
    std::uint16_t distance;
    std::uint8_t distance_table_index;
    std::size_t continuation_calls;
    std::size_t unresolved_calls;
};

struct EntityAnimation14C7CResult {
    bool entered;
    bool carry;
    bool linked_record_present;
    bool state_0x0C_path;
    bool state_0x0D_or_0x0E_path;
    bool link_predicate_accepted;
    bool sprite_slot_allocated;
    bool movement_timer_written;
    bool direction_written;
    GenesisAddress linked_record;
    std::uint16_t direction;
    std::uint16_t distance;
};

struct EntityAnimationListSelectionResult {
    bool primary_list_path;
    bool list_empty;
    bool global_relationships_cleared;
    bool current_link_preserved;
    bool selected_link_changed;
    GenesisAddress previous_link;
    GenesisAddress selected_link;
    std::size_t candidates_examined;
};

struct EntityAnimationDirectionUpdateResult {
    bool requested_0x14682;
    std::uint8_t direction_flags;
    std::size_t continuation_calls;
    std::size_t unresolved_calls;
};

struct EntityAnimation14682Result {
    bool entered;
    bool route_already_active;
    bool map_blocked;
    bool path_written;
    bool route_escalated;
    std::uint16_t probe_index;
    std::uint16_t primary_direction;
    std::uint16_t secondary_direction;
    std::uint16_t target_grid;
    std::uint16_t path_value;
    GenesisAddress continuation_target;
    std::size_t continuation_calls;
    std::size_t unresolved_calls;
};

struct EntityAnimation145A2Result {
    bool entered;
    bool stopped_at_frame_cutoff;
    bool routed_to_0x149C0;
    bool direction_flags_updated;
    bool transformed_position;
    std::uint16_t transformed_direction;
    std::uint16_t transformed_distance;
    GenesisAddress continuation_target;
    std::size_t continuation_calls;
    std::size_t unresolved_calls;
};

struct EntityAnimationPathCheckResult {
    bool unobstructed;
    std::uint16_t target_grid;
    std::uint16_t current_grid;
};

struct EntityAnimationGridTargetResult {
    std::uint16_t target_x;
    std::uint16_t target_y;
    std::uint16_t direction;
};

struct EntityAnimation149C0Result {
    bool entered;
    bool unobstructed_path_cleared;
    bool path_entry_advanced;
    bool reached_end_of_path;
    bool alternate_route;
    bool transformed_position;
    std::uint16_t current_grid;
    std::uint16_t selected_grid;
    std::uint16_t transformed_direction;
    std::uint16_t transformed_distance;
    GenesisAddress continuation_target;
    std::size_t continuation_calls;
    std::size_t unresolved_calls;
};

struct EntityAnimationAlternateRouteResult {
    bool entered;
    bool primary_candidate_clear;
    bool fallback_used;
    bool path_written;
    bool route_cleared;
    std::uint16_t current_grid;
    std::uint16_t path_value;
};

struct EntityRelationshipPassResult {
    std::size_t candidates_examined;
    bool special_record_path;
    bool link_cleared;
    bool relation_established;
    GenesisAddress selected_record;
};

struct FrameVectorTransformResult {
    std::uint16_t d2;
    std::uint16_t d3;
};

// ROM 0x15CA4 reduces two signed coordinate differences to the larger-axis
// magnitude plus a rounded half of the smaller one. The routine exchanges
// full D0/D1 longwords before its word arithmetic, so the high word left in
// D0 is part of the result contract.
std::uint32_t calculate_frame_distance_0x15CA4(
    std::uint32_t d0,
    std::uint32_t d1,
    std::uint16_t d2,
    std::uint16_t d3);

struct FrameDeltaTransformResult {
    std::uint16_t d2;
    std::uint16_t d3;
};

struct EntityAnimationLinkCheckResult {
    bool carry;
    bool linked_record_present;
    bool linked_record_eligible;
    GenesisAddress linked_record;
    std::uint16_t direction;
    std::uint16_t distance;
};

struct EntityAnimationB1dcResult {
    bool initial_command_dispatched;
    bool linked_record_transformed;
    bool linked_gate_carry;
    bool linked_command_dispatched;
    bool reaction_update_called;
    bool reaction_position_updated;
    bool reaction_animation_updated;
    std::uint8_t reaction_random_bucket;
    EntityAnimationLinkCheckResult link_check;
    std::size_t unresolved_calls;
};

struct FrameSpatialPreludeResult {
    bool phase_three_path;
    std::uint16_t d2_after;
    std::uint16_t d3_after;
    std::uint8_t horizontal_status;
    std::uint8_t vertical_status;
};

struct ObjectOutputPassResult {
    std::size_t pointers_examined;
    std::size_t active_records;
    std::size_t objects_emitted;
    std::size_t unresolved_calls;
    bool unresolved_0x481E_call;
    bool unresolved_0x5D29C_call;
    GenesisAddress next_output_address;
    std::uint8_t next_ordinal;
};

struct EntityPhaseResult {
    std::size_t slots_examined;
    std::size_t active_slots;
    std::size_t projected_slots;
    std::size_t unresolved_0x600E_calls;
    std::size_t unresolved_0x60FA_calls;
    bool unresolved_0x4658_call;
};

struct SpriteSlotAllocationResult {
    bool allocated;
    std::uint16_t slot_index;
    std::uint16_t d1_value;
    std::uint16_t a2_value;
};

struct PostObjectRecordAllocationResult {
    bool allocated;
    GenesisAddress record_address;
    std::uint16_t record_index;
};

struct PostObjectSpawnResult {
    bool record_allocated;
    bool sprite_allocated;
    GenesisAddress record_address;
    std::uint16_t record_index;
    std::uint16_t sprite_slot_index;
    std::uint16_t sprite_d1_value;
    std::uint16_t sprite_a2_value;
};

struct EntitySlotInitialisationResult {
    bool sprite_allocated;
    bool unresolved_handler;
};

struct EntitySpawnSlotResult {
    bool allocated;
    bool animation_selected;
    GenesisAddress slot_address;
    std::uint16_t selected_frame;
};

using EntityReactionMapRoutine = std::function<bool(
    GenesisBus&, GenesisAddress, const EntityReactionRegisterContext&)>;

struct EntityReactionHooks {
    FrameWindowRoutine routine_0x53126;
    FrameWaitRoutine frame_wait;
    EntityReactionMapRoutine routine_0x1651E;
};

struct EntityReactionPassResult {
    bool entered;
    bool returned_with_gate;
    std::size_t resource_loads;
    std::size_t unresolved_calls;
};

using EntityTerrainCollisionRoutine = std::function<bool(
    GenesisBus&, GenesisAddress, std::uint16_t, std::uint16_t)>;

struct EntityTerrainHooks {
    EntityTerrainCollisionRoutine routine_0xEA36;
};

struct EntityTerrainSampleResult {
    std::uint16_t status;
    bool out_of_bounds;
};

using EntityMovementCollisionRoutine = std::function<std::uint16_t(
    GenesisBus&, GenesisAddress, std::uint16_t, std::uint16_t)>;
using EntityMovementReactionRoutine = std::function<
    EntityReactionPassResult(GenesisBus&, GenesisAddress,
                             const EntityReactionRegisterContext&)>;

struct EntityMovementHooks {
    EntityMovementCollisionRoutine routine_0xE7C8;
    EntityMovementReactionRoutine routine_0xB7B8;
};

struct EntityMovementPassResult {
    bool entered;
    bool completed;
    bool entity_deactivated;
    bool direction_failed;
    bool reaction_called;
    std::uint16_t initial_collision_state;
    std::uint16_t final_collision_state;
    std::size_t unresolved_calls;
};

// ROM 0x1A2CC scans the eighteen byte flags at FFD86C. The returned D0/D1/A2
// words are retained because callers use all three after the scan.
SpriteSlotAllocationResult allocate_sprite_slot_0x1A2CC(GenesisBus& bus);

// ROM 0x15A8C selects one of six 0x100-byte records at FF0B00 by +0x10 bit 0.
PostObjectRecordAllocationResult allocate_post_object_record_0x15A8C(
    GenesisBus& bus);

// ROM 0x15930 initializes a supplied entity record from the sprite-slot
// allocator and dispatches its fixed command/handler table entry.
EntitySlotInitialisationResult initialize_entity_slot_0x15930(
    GenesisBus& bus, GenesisAddress record_address);

// ROM 0x16B98/0x16E08: select a free FF164C slot, initialize its position,
// and choose the weighted animation/frame entry for the supplied entity.
EntitySpawnSlotResult update_entity_spawn_slot_0x16B98(
    GenesisBus& bus, GenesisAddress entity_record_address);

// ROM 0x1DBF0: clear the ten active relationship records and the global gate.
void clear_entity_relationships_0x1DBF0(GenesisBus& bus);

// ROM 0x1DBAA: arm the global entity gate unless the mode-0x36 transition is
// already idle, selecting the long or short gate timeout from FFE162.
void arm_entity_global_gate_0x1DBAA(GenesisBus& bus);

// ROM 0xBC36: build three signed camera-relative offset pairs in FFDF44 and
// snapshot the camera words used by the reaction renderer.
bool prepare_entity_reaction_offsets_0xBC36(
    GenesisBus& bus, GenesisAddress entity_record_address);

// ROM 0x423C: compose the 12-bit screen/map position from D0/D1 and the
// current VDP tile phase words. The default D1 preserves the earlier one-
// argument convenience call used by the focused leaf test.
std::int16_t calculate_entity_reaction_band_0x423C(
    GenesisBus& bus, std::uint16_t d0, std::uint16_t d1 = 0);

// ROM 0x1669C/0x166D0: calculate the two directional tile-map offsets used
// by reaction/resource updates. The original routines return a wrapping word.
std::uint16_t calculate_entity_reaction_tile_index_0x1669C(
    GenesisBus& bus, GenesisAddress entity_record_address);
std::uint16_t calculate_entity_reaction_tile_index_0x166D0(
    GenesisBus& bus, GenesisAddress entity_record_address);

// ROM 0x16760: mark the selected map tile as 0x000B and increment the
// direction-specific reaction counter unless it is already marked.
bool mark_entity_reaction_tile_0x16760(
    GenesisBus& bus, GenesisAddress entity_record_address);

// ROM 0x1651E: update the reaction tile planes or allocate the small output
// record used by the mode-zero path. The bool is the original carry result.
bool run_entity_reaction_map_0x1651E(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityReactionRegisterContext& registers);

// ROM 0xB7B8: perform the collision-state reaction/resource pass. The
// 0x53126 Z80 synchronization has a direct default and can be supplied with a
// frame-wait boundary or replaced as a whole; 0x1651E also remains hookable.
EntityReactionPassResult run_entity_reaction_0xB7B8(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityReactionRegisterContext& registers,
    const EntityReactionHooks& hooks = {});

// ROM 0xE7C8, including its FF1700 terrain samples and direct rectangle /
// directional collision helpers. The EA36 hook can observe or replace the
// final directional test while the shipped implementation is direct by
// default.
EntityTerrainSampleResult sample_entity_terrain_0xE7C8(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    std::uint16_t d5,
    std::uint16_t d6,
    const EntityTerrainHooks& hooks = {});

// ROM 0xBABC, including the fixed C002 directional table, B77E/B732 probes,
// the BB86 pointer-table fallback, and BBE4/B442 record transitions. The
// terrain/collision leaf 0xE7C8 has a direct default and remains overrideable
// through the movement hook.
EntityMovementPassResult run_entity_movement_0xBABC(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityReactionRegisterContext& registers,
    const EntityMovementHooks& hooks = {});

// ROM 0x5D6BC copies the post-object entity template, initializes its fixed
// fields, and allocates the sprite slot used by that new record.
PostObjectSpawnResult spawn_post_object_entity_0x5D6BC(GenesisBus& bus);

// ROM 0x13874 selects and builds the two per-frame entity pointer lists used
// by the surrounding update routine. The record filters and DBF extents are
// kept at their original byte/word widths.
FrameEntityListResult build_frame_entity_lists_0x13874(GenesisBus& bus);

// ROM 0xA8EA: build the frame lists, then run the sixteen-record update pass.
// The handler pointer at record+0x3A is dispatched only through the supplied
// hook unless it is the known 0x49A0 no-op entry.
FrameEntityUpdateResult update_frame_entity_records_0xA8EA(
    GenesisBus& bus,
    const FrameEntityUpdateHooks& hooks = {});

// ROM 0xEB92: test the current entity against subsequent 0x100-byte records,
// transform colliding positions, and apply the 0xECCC resource-side effects.
EntityCollisionPassResult run_entity_collision_pass_0xEB92(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const FrameEntityUpdateHooks& hooks = {});

// ROM 0x03404: refresh the three entity animation records after expiry. The
// fixed ROM tables, state-selected secondary stream, and 0x03548 tail are
// emitted through GenesisBus.
void refresh_entity_animation_0x03404(GenesisBus& bus);

// ROM 0x391A: select the next eligible entity record, render the two fixed
// strips, perform the known transition reset, and invoke 0x1DC4C with the
// caller-established selector seven. The relationship call remains hookable
// for observation/replacement; the direct default is now available.
EntityAnimationPassResult run_entity_animation_pass_0x391A(
    GenesisBus& bus,
    const FrameEntityUpdateHooks& hooks = {});

// ROM 0x138B8 and 0x13904 are overlapping entry points into the same
// animation state machine. These translations cover their direct timer and
// flag gates, including the short 0x13A4C branch, and report each later ROM
// continuation through the supplied typed callback.
EntityAnimationEntryResult run_entity_animation_entry_0x138B8(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityAnimationContinuationRoutine& continuation = {},
    const EntityAnimationResourceContinuationRoutine& resource_continuation = {});
EntityAnimationEntryResult run_entity_animation_entry_0x13904(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityAnimationContinuationRoutine& continuation = {},
    const EntityAnimationResourceContinuationRoutine& resource_continuation = {});

// ROM 0xAC84: choose the 0x138B8/0x13904 entry by record bit +0x10 bit 3.
// The caller-owned AE5E post-call effects remain in the surrounding A8EA
// routine, just as they do in the ROM.
EntityAnimationEntryResult run_entity_animation_dispatch_0xAC84(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityAnimationContinuationRoutine& continuation = {},
    const EntityAnimationResourceContinuationRoutine& resource_continuation = {});

// ROM 0xACAE: direct animation/reaction state machine with typed contexts for
// the remaining carry- and register-dependent callees.
EntityAnimationAcaeResult run_entity_animation_update_0xACAE(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityAnimationAcaeContinuationRoutine& continuation = {});

// ROM 0xB1DC: perform the direct state/linked-record setup, enter the linked
// animation gate, and run the already-translated 0x55C76 entity reaction.
EntityAnimationB1dcResult run_entity_animation_update_0xB1DC(
    GenesisBus& bus, GenesisAddress entity_record_address);

// ROM 0xABFC, including its 0xD7EA/0xD81C state-bit selection. The original
// incoming D1 is discarded by 0xD7EA; d0 is the mode word supplied by ABFC.
EntityAnimationDelayResult update_entity_animation_delay_0xABFC(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    std::uint16_t d0);

// ROM 0x1E514 clears the transient entity state and, when the record is not
// retained, clears the indexed object byte selected by +0x30.
EntityAnimationCleanupResult run_entity_animation_cleanup_0x1E514(
    GenesisBus& bus, GenesisAddress entity_record_address);

// ROM 0xAF7C/0xAFAE share the ABFC delay and AF6C state tail. Their only
// difference is the continuation handler installed at +0x3A.
EntityAnimationFallbackResult run_entity_animation_fallback_0xAF7C(
    GenesisBus& bus, GenesisAddress entity_record_address);
EntityAnimationFallbackResult run_entity_animation_fallback_0xAFAE(
    GenesisBus& bus, GenesisAddress entity_record_address);

// ROM 0x14D8E: initialize the FF1380 scratch entity, test its terrain sample,
// and scan the ten FF0100 entity records through 0x15ABA. The source A1
// pointer is explicit because the ROM stores it in FFDD3C for that scan.
EntityAnimationResourceDecisionResult
run_entity_animation_resource_decision_0x14D8E(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress source_record_address);

// ROM 0x1514E: allocate and initialize the two post-object animation records
// used by a resource frame, preserving the original carry/record-index
// boundaries. Its mode-2 0x1E04A side branch is direct as well.
EntityAnimationResourceSpawnResult
run_entity_animation_resource_spawn_0x1514E(
    GenesisBus& bus, GenesisAddress entity_record_address);

// ROM 0x1E04A: enter the mode-0x14 global gate or the mode-2 resource-list
// allocator. D1/A6 are live registers at the 0x1E0DC/0x044BC boundary in the
// original caller, so they remain explicit inputs rather than being inferred.
EntityAnimationMode2ResourceResult run_entity_animation_transition_0x1E04A(
    GenesisBus& bus,
    const EntityAnimationMode2RegisterContext& context = {});

// ROM 0x1E0B8: promote the mode-2 record flag from bit 1 to bit 2 across the
// seven FF0400 records.
EntityAnimationMode2FlagResult mark_entity_animation_mode2_records_0x1E0B8(
    GenesisBus& bus);

// ROM 0x15646: build the secondary animation resource record, allocate its
// sprite slot, and derive either the unlinked interpolation offsets or the
// linked-record random displacement through 0x55848.
EntityAnimationRouteResourceResult run_entity_animation_route_resource_0x15646(
    GenesisBus& bus, GenesisAddress entity_record_address);

// ROM 0x13F76: perform the animation resource/visibility gates. Its default
// 0x14D8E call is direct; the typed continuation still replaces the call when
// a caller needs to observe or supply a larger CPU-context boundary.
EntityAnimationResourceGateResult run_entity_animation_resource_gate_0x13F76(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityAnimationResourceContinuationRoutine& continuation = {});

// ROM 0x13E40: install the B45C continuation when the animation frame is at
// or below the 0x3C cutoff. The bool reports whether the write occurred.
bool finish_entity_animation_0x13E40(
    GenesisBus& bus,
    GenesisAddress entity_record_address);

// ROM 0x13A40 copies the source record's coordinate pair into the target's
// animation position fields.
void copy_entity_animation_position_0x13A40(
    GenesisBus& bus,
    GenesisAddress target_record_address,
    GenesisAddress source_record_address);

// ROM 0x13AF2 scans the eight phase bytes at record+0x66. It returns the first
// byte below thirteen, or zero if all eight entries meet the cutoff.
std::uint16_t scan_entity_animation_phase_0x13AF2(
    GenesisBus& bus,
    GenesisAddress entity_record_address);

// ROM 0x13C20 selects the first phase record whose +0x4C byte is below four,
// with the two signed FFFC0C/FFFC0D gate bytes preserving their early exits.
EntityAnimationPhaseSelectionResult select_entity_animation_phase_0x13C20(
    GenesisBus& bus);

// ROM 0x13C74 parses the counted event records at the supplied A2 pointer,
// copies the accepted type/argument pairs to FFDD2A, and stores type-B's
// argument at FFDD66.
EntityAnimationEventParseResult parse_entity_animation_events_0x13C74(
    GenesisBus& bus,
    GenesisAddress event_table);

// ROM 0x13BBA tests the current animation phase against the parsed event
// table and returns its decision in CCR carry.
bool check_entity_animation_transition_0x13BBA(
    GenesisBus& bus,
    GenesisAddress entity_record_address);

// ROM 0x13CDE/0x13CD4. event_type is the byte placed in D7 before the
// shared comparison/rate routine. The returned residual is the low word of
// D1 left by 0x55F20 and is retained for callers that continue the same
// register flow.
EntityAnimationRateDecisionResult evaluate_entity_animation_rate_0x13CDE(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    std::uint8_t event_type);

// ROM 0x13B12. Parse the current animation event table, test the special or
// selected event, and perform the phase advance plus 0xABFC/0x03404 tail.
EntityAnimationPhaseTransitionResult run_entity_animation_transition_0x13B12(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityAnimationRegisterContext& registers = {});

// ROM 0x13A92, including its 0x13B12/0x13CD4 branches and 0x13AE6 display
// update. The register context is explicit because the original routine has
// no prologue and consumes inherited D4/D5/D7 values.
EntityAnimationBranchResult run_entity_animation_branch_0x13A92(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityAnimationRegisterContext& registers = {});

// ROM 0x144E8/0x1460E. The position gate feeds 0x15CA4 with the current
// record's coordinate pair and its animation-position pair, then preserves
// the carry-producing close-distance side effects.
EntityAnimationPositionGateResult evaluate_entity_animation_position_gate_0x144E8(
    GenesisBus& bus,
    GenesisAddress entity_record_address);

// ROM 0x142AE. The short branch is direct through its position gate and
// indexed animation-position update. Tail targets retain their original
// addresses through the existing typed continuation callback.
EntityAnimation142AeResult run_entity_animation_continuation_0x142AE(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityAnimationContinuationRoutine& continuation = {});

// ROM 0x1450E. Build the forward/reverse terrain probes from the source
// record, write the accepted animation-position pair, and enter 0x144E8.
// A clear position-gate carry continues directly through 0x145A2.
EntityAnimation1450EResult run_entity_animation_target_update_0x1450E(
    GenesisBus& bus, GenesisAddress entity_record_address,
    GenesisAddress source_record_address = 0);

// ROM 0x14508 is the shared target-probe body entered with A1 loaded from
// FFFC04 rather than from the record's +0xA2 link.
EntityAnimation1450EResult run_entity_animation_update_0x14508(
    GenesisBus& bus, GenesisAddress entity_record_address);

// ROM 0x14492 selects a linked record from the already-built primary or
// secondary pointer list, preserving an existing member and avoiding a
// self-selection update to +0xB5.
EntityAnimationListSelectionResult select_entity_animation_link_0x14492(
    GenesisBus& bus, GenesisAddress entity_record_address);

// ROM 0x13D0C coordinates linked-record movement, its distance thresholds,
// the 0x144E8 position gate, and the direct 0x145A2 tail. The larger callers
// that establish the surrounding state-machine register context remain
// separate from this typed body.
EntityAnimation13D0CResult run_entity_animation_update_0x13D0C(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityAnimationContinuationRoutine& continuation = {});

// ROM 0x14C7C selects the close-linked-record path for animation states
// 0x0C through 0x0E and reuses the direct 0x15C84 predicate elsewhere.
EntityAnimation14C7CResult evaluate_entity_animation_link_gate_0x14C7C(
    GenesisBus& bus, GenesisAddress entity_record_address);

// ROM 0x14BA6, including its direct direction-sign flag derivation and the
// 0x14682 movement/collision body.
EntityAnimationDirectionUpdateResult update_entity_animation_direction_0x14BA6(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityAnimationContinuationRoutine& continuation = {});

// ROM 0x14682, including the 0x148D2 direction chooser, the 0x148AA/
// 0x14854 collision probes, and the 0x14AF4 route fallback.
EntityAnimation14682Result run_entity_animation_route_0x14682(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityAnimationContinuationRoutine& continuation = {});

// ROM 0x145A2. The 0x14C2E flag update, 0x14BA6/0x14682 route, 0x149C0
// route, and 0x15D3C transform are direct.
EntityAnimation145A2Result run_entity_animation_update_0x145A2(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    const EntityAnimationContinuationRoutine& continuation = {});

// ROM 0x14902 and 0x14B4E. These retain the word-only grid conversion and
// the direction/target writes to FFDD2A/FFDD2C.
std::uint16_t calculate_entity_animation_grid_0x14902(
    std::uint16_t x,
    std::uint16_t y);
EntityAnimationGridTargetResult select_entity_animation_grid_target_0x14B4E(
    GenesisBus&, std::uint16_t selected_grid, std::uint16_t current_grid);

// ROM 0x14914/0x1494C. The map test reads the indexed FF1700 byte table
// through the bus and returns the original carry meaning as `unobstructed`.
EntityAnimationPathCheckResult check_entity_animation_path_0x14914(
    GenesisBus&,
    std::uint16_t target_grid,
    std::uint16_t current_grid);

// ROM 0x149C0, including the direct no-obstacle/path-list/common-transform
// paths and its direct 0x14A54 alternate-route tail.
EntityAnimation149C0Result run_entity_animation_route_0x149C0(
    GenesisBus&,
    GenesisAddress,
    const EntityAnimationContinuationRoutine& continuation = {});

// ROM 0x14A54, the alternate route search entered after an exhausted path.
EntityAnimationAlternateRouteResult run_entity_animation_alternate_route_0x14A54(
    GenesisBus& bus,
    GenesisAddress entity_record_address);

// ROM 0x1DC4C. selector is the inherited D1 longword; it is explicit here
// because the two callers do not establish it inside 0x1DC4C itself.
EntityRelationshipPassResult update_entity_relationship_0x1DC4C(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress selector,
    std::uint16_t inherited_scan_counter = 7);

// ROM 0x3A90/0x3B1E return the word status selected by the frame-boundary
// arithmetic used by 0x3A1C. They retain the original D3/D4 and D6/D7 word
// inputs and read the two mode bytes directly from Genesis RAM.
std::uint16_t calculate_frame_horizontal_status_0x3A90(
    GenesisBus& bus, std::uint16_t d3, std::uint16_t d6);
std::uint16_t calculate_frame_vertical_status_0x3B1E(
    GenesisBus& bus, std::uint16_t d4, std::uint16_t d7);

// ROM 0x15DCC transforms a word direction/magnitude pair through the two
// signed lanes of the ROM interpolation table and returns the high words of
// the two signed products.
FrameVectorTransformResult transform_frame_vector_0x15DCC(
    GenesisBus& bus, std::uint16_t d2, std::uint16_t d3);

// ROM 0x15D3C/0x15D4E subtracts the reference pair, reduces the absolute
// major/minor axes, and selects a direction byte from two ROM tables.
FrameDeltaTransformResult transform_frame_delta_0x15D3C(
    GenesisBus& bus,
    std::uint16_t d2,
    std::uint16_t d3,
    std::uint16_t d4,
    std::uint16_t d5);

// ROM 0x15C84/0x15C58 checks the current record's linked animation record,
// derives its direction/distance through 0x15D3C, and stores the direction in
// +0x34 when the link is active and closer than 0x200.
EntityAnimationLinkCheckResult check_entity_animation_link_0x15C84(
    GenesisBus& bus, GenesisAddress entity_record_address);

// The reviewed prelude of ROM 0x3A1C: phase-3 reset writes, the 0x15DCC
// vector transform, signed clamps, camera offsets, and mode-byte stores.
FrameSpatialPreludeResult prepare_frame_spatial_state_0x3A1C(
    GenesisBus& bus);

struct FrameSpatialTailResult {
    bool early_return;
    bool horizontal_updated;
    bool vertical_updated;
    std::uint16_t d2_after;
    std::uint16_t d3_after;
    FrameTilePlaneUpdateResult horizontal_stream;
    FrameTilePlaneUpdateResult vertical_stream;
};

// ROM 0x3BAC: apply the fixed-point camera delta, update the two cached
// movement words, and refresh the affected tile planes.
FrameSpatialTailResult update_frame_spatial_tail_0x3BAC(GenesisBus& bus);

// ROM 0x5B1E repeatedly compares the sixteen pointers at FFDE8A, treating
// records with +0x10 bit 4 or 6 set as value zero, and swaps adjacent entries
// until the signed word at record+4 is in descending order.
EntityPointerSortResult sort_entity_pointer_table(
    GenesisBus& bus,
    GenesisAddress pointer_table = session_ram::kEntityPointerTable);

// ROM 0x4E38 and 0x4E64 are the two timer-byte helpers used by 0x4658. They
// return true only when their byte decrement underflows and sets the
// corresponding entity flag; all byte wrap and frame-counter tests are kept
// at the original widths.
bool update_entity_timer_flag_0x4E38(
    GenesisBus& bus, GenesisAddress entity_record_address);
bool update_entity_timer_flag_0x4E64(
    GenesisBus& bus, GenesisAddress entity_record_address);

// ROM 0x075E contains these two contiguous setup blocks around the calls to
// the still-untranslated game subsystems. The names describe the observed
// RAM role, not an inferred high-level game meaning.
class ShadowrunMainState {
public:
    explicit ShadowrunMainState(GenesisBus& bus) : bus_(bus) {}

    // The first state writes after ROM 0x0C2C and 0x5D746 return.
    void initialise_session() const;

    // The 0x07F0 block, reached when the input/status word is nonzero.
    void reset_session_work_fields() const;

    // The inner 0x08C0 frame-loop prologue.
    void begin_frame_window() const;

    // The reviewed 31-pass body beginning at ROM 0x08C0, including the
    // interrupt wait and the two controller samples that follow it.
    FrameWindowResult run_frame_window(
        const FrameWindowHooks& hooks = {}) const;

    // ROM 0x4618: walk the sixteen 0x100-byte records at FF0100, project
    // active records through 0x5A56, and apply the reviewed 0x5FDC/0x600E
    // state updates. The nested 0x60FA frame update has a direct default;
    // the 0x4658 post-pass also has a reviewed direct default.
    EntityPhaseResult run_entity_phase(
        const FrameWindowHooks& hooks = {}) const;

    // ROM 0x4658: sort FFDE8A, emit each active entity's object records, and
    // run the shared post-loop object/list calls. The optional bound protects
    // host execution if a synthetic list omits its ROM sentinel.
    ObjectOutputPassResult run_object_output_pass(
        const ObjectOutputPassHooks& hooks = {},
        std::size_t max_records_per_list = 256) const;

private:
    GenesisBus& bus_;
};

} // namespace shadowrun
