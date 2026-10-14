#pragma once

#include "shadowrun_ghidra/field/field_map_event_system_conversion.hpp"

#include <array>
#include <filesystem>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace shadowrun::ghidra_conversion::scripting {

using StoryFacts = std::unordered_map<std::string, bool>;
using StoryFactReader =
    std::function<std::optional<bool>(std::string_view)>;
using StoryFactWriter =
    std::function<bool(std::string_view, bool)>;

struct ScriptMenuChoice {
    std::string id;
    std::string label_key;
};

enum class ScriptMenuInputMode : std::uint8_t {
    button_choice,
    cursor,
};

struct ScriptMenuDefinition {
    std::string id;
    std::string text_profile;
    std::string speaker_id;
    std::string title_key;
    // Dynamic menu prompts stay in the same menu panel as their choices.
    // Empty selects the catalog-backed title_key path.
    std::string inline_text;
    bool inline_prompt = false;
    std::vector<ScriptMenuChoice> choices;
    ScriptMenuInputMode input_mode = ScriptMenuInputMode::button_choice;
    std::size_t selected_index = 0U;
};

using ScriptStringMap = std::unordered_map<std::string, std::string>;

// Speaker-facing presentation properties are content metadata. The native
// panel consumes this value but does not maintain a parallel speaker table.
struct ScriptSpeakerProfile {
    std::string id;
    std::string name;
    std::string text_profile;
    std::string portrait_alias;
};

struct ScriptDialogueTextKeys {
    std::string title_key;
    std::string continuation_key;
};

struct ScriptShopOffer {
    std::uint8_t type = 0;
    std::uint8_t value = 0;
    std::uint8_t quantity = 0;
};

struct ScriptShopOfferGroup {
    std::string id;
    std::array<std::string, 2> title_keys{};
    std::vector<ScriptShopOffer> offers;
};

struct ScriptShopTextKeys {
    std::string universal_clips_title;
    std::string universal_clips_description;
    std::string transaction_prompt;
    std::string no_inventory_slot;
    std::string insufficient_funds;
    std::string special_action_limit;
    std::string protected_sale;
    std::string sale_quote_heading;
    std::string purchase_price_heading;
};

struct ScriptShopDefinition {
    std::string id;
    std::string title_key;
    ScriptShopTextKeys text_keys;
    std::vector<ScriptShopOfferGroup> groups;
};

struct ScriptWeaponTemplate {
    struct SourceAction {
        std::uint8_t effect_table_index = 0;
        std::uint8_t effect_value = 0;
        std::uint8_t difficulty = 0;
    };

    std::string id;
    std::uint8_t item_code = 0;
    std::uint8_t magazine_capacity = 0;
    std::uint8_t damage_bar_index = 0;
    std::string power_profile;
    std::string power_label;
    std::string fire_mode;
    std::string trigger_behavior;
    std::uint8_t rounds_per_action = 0;
    std::string attack_cadence;
    std::string attack_delay_scale;
    std::string reload_delay_scale;
    // D4540 +0x25 indexes 166AD4 (NEG.B), +0x26 is the initial
    // 0x55948 hit-resolution difficulty.
    SourceAction source_action{};
};

struct ScriptArmorTemplate {
    std::string item_id;
    std::uint8_t item_code = 0;
    std::uint8_t armor_index = 0;
    std::uint8_t primary_modifier = 0;
    std::uint8_t secondary_modifier = 0;
};

struct ScriptItemDefinition {
    struct Pricing {
        std::uint16_t purchase_base = 0;
        std::uint16_t resale_base = 0;
    };

    struct Presentation {
        std::string icon_resource_key;
        std::string detail_resource_key;
    };

    struct Detail {
        std::string kind;
        std::uint8_t value = 0;
    };

    std::uint8_t code = 0;
    std::uint8_t action_motion_distance_profile = 0;
    std::string id;
    std::string kind;
    std::string name_key;
    std::string category_key;
    Pricing pricing{};
    Presentation presentation{};
    bool has_detail_art = false;
    std::optional<Detail> detail;
    std::optional<ScriptWeaponTemplate> weapon;
    std::optional<ScriptArmorTemplate> armor;
};

struct ScriptItemCatalog {
    std::vector<ScriptItemDefinition> items;
    std::vector<std::string> detail_text_keys;
    std::array<std::string, 3> detail_label_keys{};
};

struct ScriptDialogueRequest {
    std::string speaker_id;
    std::string text_profile;
    std::string event_id;
    std::string inline_text;
    std::string continuation_key;
};

struct ScriptFieldModeContext {
    std::uint8_t mode = 0;
    std::uint8_t event_code = 0;
    std::int8_t alternate_result = 0;
};

enum class ScriptRunFieldEventAction : std::uint8_t {
    no_action,
    objective_advanced,
    completed,
};

struct ScriptRunFieldEventDecision {
    ScriptRunFieldEventAction action =
        ScriptRunFieldEventAction::no_action;
    std::string run_id;
    std::uint32_t reward_nuyen = 0;
    std::uint8_t runner_progress_delta = 0;
};

enum class ScriptLocationEntryAction {
    no_action,
    open_elevator_selector,
};

struct ScriptElevatorFloorGroup {
    std::vector<std::uint16_t> scene_ids;
    std::uint8_t entry_floor = 0;
};

struct ScriptLocationEntryDecision {
    ScriptLocationEntryAction action = ScriptLocationEntryAction::no_action;
    ScriptElevatorFloorGroup elevator_group;
};

enum class ScriptActorActionKind : std::uint8_t {
    none,
    item,
    spell,
};

enum class ScriptFieldEncounterOutcomeKind : std::uint8_t {
    dialogue_event,
    dynamic_world_effect_sequence,
};

enum class ScriptFieldEncounterTriggerAction : std::uint8_t {
    blocked,
    not_due,
    sample_probability,
    random_miss,
    forward_active_sequence,
    select_scene_outcome,
};

struct ScriptFieldEncounterTriggerInput {
    std::uint16_t scene_id = 0;
    std::uint16_t frame_counter = 0;
    bool trigger_delay_active = false;
    bool current_entity_protected = false;
    bool activity_lock = false;
    bool sequence_mode = false;
    bool world_effects_active = false;
    bool placement_pending = false;
    // Absent for the first Lua call. The host draws only after Lua reports
    // sample_probability, preserving the ROM's conditional PRNG consumption.
    std::optional<std::uint16_t> probability_roll;
    std::uint16_t probability_threshold = 0;
};

// Source-derived fields consumed by the ROM close-target reaction path
// (0x55C76). Scripts receive named combat facts, never a byte-addressed
// Genesis record.
struct ScriptCombatantFacts {
    std::uint16_t record_type = 0;
    std::uint8_t source_flags = 0; // live +0x10
    std::uint8_t interaction_flags = 0; // live +0x11
    std::uint16_t cyberware_flags = 0;
    std::uint8_t physical_meter = 0;
    std::uint8_t mental_meter = 0;
    std::uint8_t posture = 0;
    std::uint8_t command_attribute_77 = 0;
    std::uint8_t command_attribute_78 = 0;
    std::uint8_t command_attribute_79 = 0;
    std::uint8_t command_attribute_7b = 0;
    std::uint8_t command_attribute_7c = 0;
    std::uint8_t command_attribute_85 = 0;
    std::uint8_t item18_base = 0;
    // ROM 0x1033E adds live +0x9D to the initial action trial count.
    std::uint8_t auxiliary_roll_bonus = 0;
    std::uint8_t action_result = 0; // live +0x9F
    std::array<std::uint8_t, 4> action_skills{}; // live +0x81..+0x84
    std::uint8_t armor_index = 0;
    std::int8_t race_or_state = 0;
    std::uint8_t class_code = 0;
    std::uint8_t equipped_action_code = 0;
    std::uint8_t action_profile_modifier_flags = 0; // live +0xDF
    // 0xFFFF means the source +0xD8 linked-record relationship has not been
    // supplied; never substitute an unrelated runner menu-table index.
    std::uint16_t linked_action_profile_index = 0xFFFFU;
    std::array<std::uint8_t, 8> item_codes{};
    std::array<std::uint8_t, 8> item_phases{};
    std::array<std::uint8_t, 10> action_phase_extension{}; // live +0x76..+0x7F
};

struct ScriptActionProfileModifierPair {
    std::uint8_t primary_roll_count = 0;
    std::uint8_t primary_difficulty = 0;
    std::uint8_t secondary_roll_count = 0;
    std::uint8_t secondary_difficulty = 0;
};

struct ScriptCombatMeterUpdate {
    std::string target_id;
    std::uint8_t source_record_slot = 0;
    std::uint16_t record_type = 0;
    std::uint8_t physical_meter = 0;
    std::uint8_t mental_meter = 0;
    bool physical = false;
    bool changed = false;
};

struct ScriptActorSpellActionResolution {
    std::uint8_t action_code = 0;
    std::string target_id;
    std::uint8_t target_record_slot = 0;
    std::uint16_t target_record_type = 0;
    std::uint8_t action_result = 0;
    bool roll_attempted = false;
    std::optional<std::uint8_t> initial_trial_count;
    std::optional<std::uint8_t> difficulty;
    bool effect_started = false;
    std::optional<std::uint8_t> spell_effect_slot;
    std::optional<ScriptActorActionKind> next_action_kind;
    std::optional<std::uint8_t> next_action_code;
    std::optional<std::string> next_profile_id;
    std::string outcome;
    bool effect_requested = false;
    std::int16_t residual_effect = 0;
    std::uint8_t behavior_meter_delta = 0;
    bool inventory_changed = false;
    std::array<std::uint8_t, 8> item_codes{};
    std::array<std::uint8_t, 8> item_phases{};
    std::uint8_t mental_meter = 0;
    bool special_result_published = false;
    std::uint8_t special_result_successes = 0;
    std::uint8_t special_result_bonus = 0;
};

struct ScriptActorSpellEffectResolution {
    std::uint8_t action_code = 0;
    std::string target_id;
    std::uint8_t target_record_slot = 0;
    std::uint16_t target_record_type = 0;
    std::uint8_t target_trial_count = 0;
    std::uint8_t difficulty = 0;
    std::uint8_t target_roll_successes = 0;
    std::int16_t effect_value = 0;
    std::string outcome;
    bool physical = true;
    std::uint8_t physical_meter = 0;
    std::uint8_t mental_meter = 0;
    bool changed = false;
};

struct ScriptProjectileCombatTarget {
    std::string id;
    std::uint8_t source_record_slot = 0;
    ScriptCombatantFacts combatant;
};

struct ScriptFieldEncounterActor {
    std::uint16_t source_order = 0;
    std::uint32_t selection_cell_offset = 0;
    std::uint32_t sequence_list_offset = 0;
    std::string actor_name;
    std::string profile_id;
    std::uint16_t entity_kind = 0;
    ScriptActorActionKind action_kind = ScriptActorActionKind::none;
    std::uint8_t action_code = 0;
    std::uint8_t palette_line = 0;
    std::string sprite_stem;
    std::uint32_t tagged_template = 0;
    std::uint32_t template_offset = 0;
    std::uint8_t source_interaction_flags = 0;
    std::uint8_t physical_meter = 0;
    std::uint8_t delay_primary_attribute = 0;
    std::uint8_t delay_secondary_attribute = 0;
    std::uint16_t delay_capability_flags = 0;
    std::string ordered_overrides;
    ScriptCombatantFacts combatant;
};

// The selector result is data, not an event-specific C++ switch. Dialogue
// handler continuations remain source-owned until their respective branches
// have been traced and ported; dynamic actor lists are fully typed here.
struct ScriptFieldEncounterOutcome {
    ScriptFieldEncounterOutcomeKind kind =
        ScriptFieldEncounterOutcomeKind::dialogue_event;
    std::uint16_t scene_id = 0;
    std::uint8_t context_group = 0;
    std::uint8_t random_sum = 0;
    std::string event_name;
    std::uint32_t handler_offset = 0;
    std::string opening_text;
    std::array<std::string, 3> choices{};
    std::array<std::optional<std::uint8_t>, 3> choice_commands{};
    std::array<std::optional<std::uint32_t>, 3> choice_dispatch_targets{};
    std::vector<ScriptFieldEncounterActor> actors;
};

enum class ScriptActorHeartbeatPhase : std::uint8_t {
    behavior,
    render,
};

enum class ScriptActorAnimationMode : std::uint8_t {
    idle,
    walking,
    attack,
};

struct ScriptMovementOffset {
    std::int16_t x = 0;
    std::int16_t y = 0;
};

struct ScriptMovementStepState {
    std::uint16_t position_x = 0;
    std::uint16_t position_y = 0;
    std::uint16_t heading = 0;
    std::uint16_t animation_heading = 0;
    std::uint16_t collision_context = 0;
    std::uint16_t movement_substate = 0;
    std::int16_t route_status = 0;
    std::uint8_t route_flags = 0;
    std::uint8_t movement_flags = 0;
    std::uint8_t collision_flags = 0;
    std::uint8_t event_flags = 0;
    std::int16_t collision_extent = 0;
    std::uint16_t terrain_collision_code = 0;
};

struct ScriptMovementRouteFacts {
    std::int8_t primary_marker = 0;
    std::int8_t secondary_marker = 0;
    std::uint16_t primary_fallback_slot = 0;
    std::uint16_t secondary_fallback_slot = 0;
};

struct ScriptMovementCollisionResult {
    std::int16_t collision_code = 0;
    std::uint16_t terrain_collision_code = 0;
};

struct ScriptMovementCollisionRecoveryResult {
    bool movement_recovered = false;
    std::uint8_t route_flags = 0;
};

using ScriptMovementCollisionQuery =
    std::function<ScriptMovementCollisionResult(std::uint16_t, std::uint16_t)>;
using ScriptMovementRouteLookup =
    std::function<ScriptMovementRouteFacts(std::uint16_t)>;
using ScriptMovementCollisionRecovery =
    std::function<ScriptMovementCollisionRecoveryResult(
        std::uint16_t, std::uint16_t, std::int16_t, std::uint8_t)>;

struct ScriptActorProjectileAnimation {
    std::uint16_t selector = 0;
    std::uint16_t frame_count = 0;
    std::uint16_t frame = 0;
    std::int16_t accumulator = 0;
    std::int16_t increment = 0;
    std::int16_t period = 0;
    bool finite = false;
};

// Projectile behavior state is supplied by the native field scene, while
// Lua owns the source update decisions. The callbacks below only sample the
// host world and return the collision routine's source result.
enum class ScriptActorProjectileRole : std::uint8_t {
    primary,
    secondary,
};

enum class ScriptActorProjectileOutcome : std::uint8_t {
    none,
    stopped,
    retired,
    hit_resolution_pending,
    linked_followup_pending,
};

struct ScriptActorProjectileMovement {
    ScriptActorProjectileRole role = ScriptActorProjectileRole::primary;
    bool active = false;
    std::uint8_t flags = 0;
    std::uint8_t record_flags = 0;
    std::int32_t world_x = 0;
    std::int32_t world_y = 0;
    std::int16_t tile_offset_x = 0;
    std::int16_t tile_offset_y = 0;
    std::uint8_t retry_counter = 0;
    std::int8_t linked_action_timer = 0;
    std::int8_t action_result = 0; // live +0x9F, shared by the projectile pair
    std::uint16_t animation_selector = 0;
    std::string hit_target_id;
};

struct ScriptProjectileTerrainSample {
    std::uint16_t terrain_result = 0;
    bool terrain_boundary_active = false;
    bool outside_bounds_marker = false;
};

struct ScriptProjectileCollisionSearchResult {
    bool source_carry = false;
    bool source_position_updated = false;
    std::uint16_t source_x = 0;
    std::uint16_t source_y = 0;
    std::optional<std::string> hit_target_id;
};

struct ScriptActorProjectileMovementDecision {
    ScriptActorProjectileMovement state{};
    ScriptActorProjectileOutcome outcome =
        ScriptActorProjectileOutcome::none;
};

using ScriptProjectileTerrainProbe =
    std::function<ScriptProjectileTerrainSample(std::uint16_t x,
                                                std::uint16_t y)>;
using ScriptProjectileCollisionSearch =
    std::function<ScriptProjectileCollisionSearchResult(
        std::uint16_t x, std::uint16_t y, std::uint8_t source_flags)>;

struct ScriptActorMovementStep {
    enum class Outcome : std::uint8_t {
        accepted,
        reset,
    };

    Outcome outcome = Outcome::accepted;
    ScriptMovementStepState state{};
    std::uint16_t direction_animation = 0;
};

struct ScriptActorTargetCandidate {
    std::string id;
    std::uint16_t world_x = 0;
    std::uint16_t world_y = 0;
    std::uint16_t heading = 0;
    std::uint16_t distance = 0;
    std::uint8_t service_flags = 0; // source entity +0x10
    std::uint8_t interaction_flags = 0; // source entity +0x11
    std::uint8_t behavior_flags = 0; // source entity +0xB0
    std::uint8_t spawn_state_flags = 0; // source entity +0x9E
    bool has_action = false; // source entity +0x0C != 0
    bool world_activity_actor = false;
    // 0x13D0C routes either toward the linked record or toward a point
    // projected perpendicular to it. These are the source 0x15CA4 metrics
    // from the acting record to each candidate endpoint; Lua selects the
    // endpoint and applies the 0x1E0 close-range rule.
    std::uint16_t direct_route_step_cost = 0;
    std::uint16_t perpendicular_route_step_cost = 0;
    // ROM 0x145A2 stores the chosen destination's measured distance at +0xC8;
    // 0xB574 uses it to select the actor's per-pass displacement table.
    std::uint16_t direct_route_distance = 0;
    std::uint16_t perpendicular_route_distance = 0;
    std::optional<std::uint8_t> source_record_slot;
    std::uint8_t descriptor_class = 0;
    std::optional<ScriptCombatantFacts> combatant;
};

using ScriptActorRandomBound =
    std::function<std::optional<std::uint16_t>(std::uint16_t)>;
// True when 0x14D8E leaves carry clear for these linked world coordinates.
using ScriptActorActionLineClear =
    std::function<bool(std::uint16_t world_x, std::uint16_t world_y)>;

// Source-derived identity and spawn facts passed to an actor's Lua heartbeat.
// Lua owns per-instance behavior state; the host retains world facts and
// consumes the returned presentation/action snapshot.
struct ScriptActorBehaviorContext {
    std::string_view profile_id;
    std::uint16_t entity_kind = 0;
    ScriptActorActionKind action_kind = ScriptActorActionKind::none;
    std::uint8_t action_code = 0;
    std::uint8_t entry_delay = 0;
    bool entry_started = false;
    bool entry_complete = false;
    std::string_view instance_id;
    std::uint16_t initial_heading = 0;
    bool spawn = false;
    bool movement_allowed = false;
    // Field-player action inputs and source-derived readiness facts. Lua
    // decides whether this heartbeat should service the equipped action;
    // the native host validates and performs the resulting ROM operation.
    bool combat_confirm_held = false;
    bool combat_action_ready = false;
    bool combat_action_available = false;
    bool combat_input_blocked = false;
    std::uint8_t controller_nibble = 0;
    std::array<std::uint16_t, 16> controller_heading_table{};
    std::optional<std::uint16_t> movement_heading_override;
    ScriptActorHeartbeatPhase phase = ScriptActorHeartbeatPhase::behavior;
    ScriptActorAnimationMode animation_mode =
        ScriptActorAnimationMode::idle;
    std::optional<std::uint8_t> animation_direction_override;
    std::optional<std::int16_t> animation_accumulator_override;
    std::optional<ScriptActorProjectileAnimation> projectile_animation;
    std::optional<ScriptActorProjectileMovement> projectile_movement;
    // The first primary-record behavior pass resolves ROM 0x55640 once;
    // subsequent movement and the cloned secondary record retain its result.
    bool projectile_attack_seed_pending = false;
    std::vector<ScriptProjectileCombatTarget> projectile_combat_targets;
    ScriptProjectileTerrainProbe projectile_terrain_probe;
    ScriptProjectileCollisionSearch projectile_collision_search;
    bool movement_services_available = false;
    std::array<ScriptMovementOffset, 8> movement_offsets{};
    std::array<std::uint16_t, 8> movement_idle_selectors{};
    ScriptMovementStepState movement_state{};
    std::uint8_t movement_entity_flags = 0;
    bool movement_transition_active = false;
    std::uint16_t movement_scene_id = 0;
    std::uint8_t world_scene_state = 0; // FFE186; delayed spell callback gate
    std::uint16_t movement_collision_mode = 0;
    ScriptMovementCollisionQuery movement_collision;
    ScriptMovementRouteLookup movement_route_lookup;
    ScriptMovementCollisionRecovery movement_collision_recovery;
    // Dynamic field encounters carry their source entity coordinates and the
    // measured target facts from the native geometry query. Lua chooses the
    // behavior; the host does not infer a target from screen proximity.
    bool dynamic_world_encounter_actor = false;
    std::optional<std::uint8_t> source_record_slot;
    std::uint8_t source_record_flags = 0;
    std::uint8_t source_interaction_flags = 0;
    std::uint8_t source_behavior_flags = 0;
    std::uint8_t source_spawn_state_flags = 0;
    // ROM 0x13D84 indexes the world-actor route-distance table with live
    // record +0x4D (the typed combatant posture fact).
    std::optional<std::uint8_t> source_actor_posture;
    bool world_activity_active = false;
    // ROM 0x14C7C's kind-0xD branch checks animation frame +0x1E here.
    std::int16_t ai_animation_frame = 0;
    // Positive C062 loop length supplied from extracted source art.
    std::uint8_t walk_animation_frame_count = 0;
    // Directional C0A2 target-action facts extracted to a named actor profile.
    // Lua owns the per-instance frame accumulator and finite-list completion.
    std::uint8_t action_animation_frame_count = 0;
    std::uint16_t action_animation_increment = 0;
    std::uint16_t action_animation_threshold = 0;
    std::optional<std::uint16_t> action_animation_audio_event;
    // Directional C072/C0B2 hit-reaction facts extracted from each actor
    // template. Lua owns the reaction timer and finite animation state.
    std::uint8_t hurt_animation_frame_count = 0;
    std::uint16_t hurt_animation_increment = 0;
    std::uint16_t hurt_animation_threshold = 0;
    std::optional<std::uint16_t> hurt_audio_event;
    std::optional<std::uint16_t> down_audio_event;
    // ROM FFF9F8 bit 7 selects 0x4D vs 0x4E after the target heading is
    // accepted. The source condition is a fact; Lua chooses the cue.
    bool target_heading_audio_alternate = false;
    // 0x44BC fills +0x78/+0x7B/+0x90 from the selected ROM template. Lua
    // uses these typed facts when 0xB11C resets the signed +0xDC delay.
    std::int16_t initial_action_delay = 0;
    std::uint8_t delay_primary_attribute = 0;
    std::uint8_t delay_secondary_attribute = 0;
    std::uint16_t delay_capability_flags = 0;
    std::array<bool, 18> world_effect_slot_occupancy{};
    // ROM +0x11.4 target-selection latch, forwarded as a fact; Lua owns the
    // visible 0x5C7E marker cadence and returns its presentation state.
    bool target_selection_latched = false;
    bool target_selection_requested = false;
    bool target_selection_advances_cursor = false;
    bool target_selection_special_marker = false;
    std::uint8_t target_selection_mode = 0;
    std::optional<std::uint8_t> target_selection_current_slot;
    std::optional<std::uint8_t> target_selection_linked_slot;
    std::optional<std::uint8_t> target_selection_owner_slot;
    std::vector<ScriptActorTargetCandidate> ai_target_candidates;
    std::vector<ScriptActorTargetCandidate> world_target_selection_candidates;
    ScriptActorRandomBound ai_random_bound;
    std::uint8_t entry_animation_frame_count = 2;
    std::optional<ScriptCombatantFacts> combatant;
    std::array<ScriptActionProfileModifierPair, 5>
        action_profile_modifiers{};
    // D4EA0 action rows 5 and 12 select +0x77 or +0x7C for the
    // corresponding +0xDF dynamic-profile branch.
    std::array<std::int8_t, 2> action_attribute_selectors{};
    bool combat_meter_service_blocked = false;
    // The source weapon row copied into projectile +0x57; this is separate
    // from the actor profile's item/spell action classification above.
    std::uint8_t projectile_action_code = 0;
    // ROM 0x5A94 writes the caster's projected Y to +0x0C and 0x5ABE clears
    // it outside the field window. Unset means the host did not measure it.
    std::optional<bool> source_projection_active;
    // Closed over the caster. Unset means the host cannot run 0x14D8E
    // honestly, and Lua then refuses the cast.
    ScriptActorActionLineClear action_line_clear;
};

struct ScriptActorBehaviorDecision {
    enum class Action {
        hold,
        play_entry_animation,
    };

    std::string package_id;
    Action action = Action::hold;
    std::string animation_id = "idle";
    std::uint8_t direction = 0;
    std::uint8_t animation_frame = 0;
    std::uint8_t source_record_flags = 0;
    std::optional<std::uint16_t> animation_selector;
    bool combat_action_requested = false;
    bool transition_started = false;
    bool transition_complete = false;
    std::optional<std::uint16_t> audio_event;
    std::optional<std::uint16_t> target_heading_audio_event;
    std::optional<std::uint8_t> weapon_item_id;
    std::optional<std::uint16_t> movement_heading;
    std::optional<std::uint16_t> movement_current_heading;
    std::optional<std::uint16_t> movement_route_distance;
    std::optional<std::string> ai_target_id;
    std::optional<std::uint16_t> ai_target_heading;
    std::optional<std::uint16_t> ai_target_distance;
    bool ai_target_engaged = false;
    std::uint8_t target_motion_delay = 0;
    std::int16_t action_delay = 0;
    std::optional<std::uint8_t> target_motion_effect_slot_request;
    bool target_motion_effect_visible = false;
    std::uint8_t target_motion_effect_direction = 0;
    std::uint8_t target_motion_effect_phase = 0;
    bool target_motion_effect_slot_released = false;
    bool spell_effect_visible = false;
    std::uint8_t spell_effect_phase = 0;
    std::uint8_t spell_effect_action_code = 0;
    bool spell_effect_completed = false;
    std::optional<std::uint8_t> spell_effect_slot_released;
    bool target_marker_visible = false;
    bool target_marker_animating = false;
    std::uint8_t target_marker_style = 0;
    std::uint8_t target_marker_radius = 0;
    bool target_selection_processed = false;
    std::optional<std::string> selected_world_actor_id;
    std::optional<ScriptActorMovementStep> movement_step;
    std::optional<ScriptActorProjectileMovementDecision>
        projectile_movement;
    std::optional<std::int16_t> animation_increment;
    std::int16_t animation_accumulator = 0;
    std::optional<ScriptCombatMeterUpdate> combat_meter_update;
    std::optional<ScriptActorSpellActionResolution>
        spell_action_resolution;
    std::optional<ScriptActorSpellEffectResolution>
        spell_effect_resolution;
    // One 0x13FFE spell result for the 21-pass refresh. Nil on other passes.
    std::optional<std::string> source_action_dispatch;
};

struct ScriptNativeOperation {
    enum class Type {
        actor_walk_heading,
        ui_say,
        ui_menu,
        service_call,
    };

    Type type = Type::actor_walk_heading;
    std::string actor_id;
    std::uint8_t heading = 0;
    std::string service_id;
    std::optional<ScriptDialogueRequest> dialogue;
    std::optional<ScriptMenuDefinition> menu;
};

struct ScriptTaskStep {
    enum class Status {
        completed,
        awaiting_native_operation,
        failed,
    };

    Status status = Status::failed;
    std::optional<ScriptNativeOperation> operation;
    std::optional<std::string> result_id;
};

// Lua modules use the global game facade. game.state reads and writes the
// native story-fact store through a constrained boolean-property proxy;
// game.location reads native context through a read-only proxy. Lua never
// receives pointers to mutable engine, save, or Qt objects.
class GameScriptRuntime final {
public:
    GameScriptRuntime();
    ~GameScriptRuntime();

    GameScriptRuntime(const GameScriptRuntime&) = delete;
    GameScriptRuntime& operator=(const GameScriptRuntime&) = delete;
    GameScriptRuntime(GameScriptRuntime&&) noexcept;
    GameScriptRuntime& operator=(GameScriptRuntime&&) noexcept;

    bool load_package(
        const std::filesystem::path& script_directory,
        const std::filesystem::path& dialogue_text_file = {});
    const std::string* dialogue_text(std::string_view key) const;
    const std::unordered_map<std::string, std::string>&
    dialogue_text_entries() const noexcept;
    std::optional<FieldScriptRoute> resolve_field_location_route(
        const FieldScriptRouteContext& context);
    std::optional<ScriptRunFieldEventDecision> resolve_active_run_field_event(
        std::uint8_t map_id, std::uint8_t event_id);
    std::optional<ScriptLocationEntryDecision> resolve_field_location_entry(
        std::uint16_t scene_id, FieldCollisionEventCode event_code,
        const std::vector<std::vector<std::uint16_t>>& source_elevator_groups);
    std::optional<std::uint16_t> resolve_elevator_destination(
        const ScriptElevatorFloorGroup& group,
        std::uint8_t floor_index);
    std::optional<FieldMapEventPolicyDecision> resolve_field_collision_event(
        const FieldMapCollisionEventInput& input);
    std::optional<ScriptFieldEncounterOutcome> resolve_field_encounter(
        std::uint16_t scene_id, std::uint8_t first_roll,
        std::uint8_t second_roll);
    std::optional<ScriptFieldEncounterTriggerAction>
    evaluate_field_encounter_trigger(
        const ScriptFieldEncounterTriggerInput& input);
    std::optional<std::vector<FieldMapEventRule>> map_event_rules();
    void set_story_state(const StoryFacts& facts);
    void set_story_fact_accessors(StoryFactReader read,
                                  StoryFactWriter write);
    StoryFacts story_state() const;
    void set_location_context(std::string_view type, std::string_view id,
                              std::string_view text_profile = {},
                              std::string_view speaker_id = {},
                              std::string_view event_id = {});
    void set_field_mode_context(ScriptFieldModeContext context) noexcept;
    std::optional<ScriptActorBehaviorDecision> actor_heartbeat(
        const ScriptActorBehaviorContext& context);
    // Kept for the existing script-runtime caller contract. New game-runtime
    // code should use actor_heartbeat() to make the per-tick boundary clear.
    std::optional<ScriptActorBehaviorDecision> actor_behavior_decision(
        const ScriptActorBehaviorContext& context);
    std::optional<std::string> call_string(
        std::string_view module, std::string_view function);
    // Read an ordered, unique ID array that has no display labels.
    std::optional<std::vector<std::string>> string_list(
        std::string_view module, std::string_view function);
    std::optional<ScriptStringMap> string_map(
        std::string_view module, std::string_view function);
    std::optional<ScriptSpeakerProfile> speaker_profile(
        std::string_view speaker_id);
    std::optional<ScriptDialogueTextKeys> dialogue_event_text_keys(
        std::string_view speaker_id, std::string_view text_profile,
        std::string_view event_id);
    std::optional<ScriptMenuDefinition> resolve_dialogue_menu(
        ScriptMenuDefinition menu);
    std::optional<ScriptShopDefinition> shop_definition(
        std::string_view shop_id);
    std::optional<ScriptItemCatalog> item_catalog();
    ScriptTaskStep start_task(
        std::string_view module, std::string_view function);
    ScriptTaskStep complete_native_operation(bool succeeded);
    ScriptTaskStep complete_native_menu_choice(std::string_view choice_id);
    ScriptTaskStep complete_native_menu_input(std::string_view input_id);
    ScriptTaskStep complete_native_service_result(std::string_view result_id);
    bool task_waiting_for_native_operation() const noexcept;
    void cancel_task() noexcept;
    std::optional<ScriptMenuDefinition> menu(
        std::string_view module, std::string_view function,
        std::string_view text_profile,
        std::string_view menu_id = "manager");

    const std::string& failure() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace shadowrun::ghidra_conversion::scripting
