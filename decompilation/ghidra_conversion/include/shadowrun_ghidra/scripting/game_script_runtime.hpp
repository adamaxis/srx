#pragma once

#include "shadowrun_ghidra/core/movement_direction.hpp"
#include "shadowrun_ghidra/field/field_map_event_system_conversion.hpp"
#include "shadowrun_ghidra/menus/command_section.hpp"
#include "shadowrun_ghidra/scripting/dialogue_outcome.hpp"
#include "shadowrun_ghidra/scripting/weapon_action_policy.hpp"

#include <array>
#include <cstddef>
#include <filesystem>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <utility>
#include <vector>

namespace shadowrun::ghidra_conversion::scripting {

// Stable gameplay identities for regions used by the taxi service. The
// numeric field-resource selector is translated at the native boundary;
// gameplay Lua only sees these names.
enum class ScriptFieldRegion : std::uint8_t {
    downtown_seattle,
    puyallup_barrens,
    redmond_barrens,
    penumbra_district,
    council_island,
    renraku_arcology,
    salish_shidhe,
};

enum class ScriptTaxiCaptionPresentation : std::uint8_t {
    white_on_black,
};

inline constexpr std::string_view script_field_region_id(
    const ScriptFieldRegion region) noexcept {
    switch (region) {
    case ScriptFieldRegion::downtown_seattle: return "downtown_seattle";
    case ScriptFieldRegion::puyallup_barrens: return "puyallup_barrens";
    case ScriptFieldRegion::redmond_barrens: return "redmond_barrens";
    case ScriptFieldRegion::penumbra_district: return "penumbra_district";
    case ScriptFieldRegion::council_island: return "council_island";
    case ScriptFieldRegion::renraku_arcology: return "renraku_arcology";
    case ScriptFieldRegion::salish_shidhe: return "salish_shidhe";
    }
    return {};
}

inline constexpr std::optional<ScriptFieldRegion> script_field_region_from_id(
    const std::string_view id) noexcept {
    if (id == "downtown_seattle") return ScriptFieldRegion::downtown_seattle;
    if (id == "puyallup_barrens") return ScriptFieldRegion::puyallup_barrens;
    if (id == "redmond_barrens") return ScriptFieldRegion::redmond_barrens;
    if (id == "penumbra_district") return ScriptFieldRegion::penumbra_district;
    if (id == "council_island") return ScriptFieldRegion::council_island;
    if (id == "renraku_arcology") return ScriptFieldRegion::renraku_arcology;
    if (id == "salish_shidhe") return ScriptFieldRegion::salish_shidhe;
    return std::nullopt;
}

// These conversions are confined to the host adapter. The source region
// value is FFFBF8 / selector state; it is not a Lua-side region identifier.
inline constexpr std::optional<ScriptFieldRegion>
script_field_region_from_source_value(const std::uint8_t value) noexcept {
    switch (value) {
    case 0U: return ScriptFieldRegion::downtown_seattle;
    case 1U: return ScriptFieldRegion::puyallup_barrens;
    case 2U: return ScriptFieldRegion::redmond_barrens;
    case 3U: return ScriptFieldRegion::penumbra_district;
    case 4U: return ScriptFieldRegion::council_island;
    case 5U: return ScriptFieldRegion::renraku_arcology;
    case 6U: return ScriptFieldRegion::salish_shidhe;
    default: return std::nullopt;
    }
}

inline constexpr std::uint8_t script_field_region_source_value(
    const ScriptFieldRegion region) noexcept {
    switch (region) {
    case ScriptFieldRegion::downtown_seattle: return 0U;
    case ScriptFieldRegion::puyallup_barrens: return 1U;
    case ScriptFieldRegion::redmond_barrens: return 2U;
    case ScriptFieldRegion::penumbra_district: return 3U;
    case ScriptFieldRegion::council_island: return 4U;
    case ScriptFieldRegion::renraku_arcology: return 5U;
    case ScriptFieldRegion::salish_shidhe: return 6U;
    }
    return 0U;
}

using StoryFacts = std::unordered_map<std::string, bool>;
using StoryFactReader =
    std::function<std::optional<bool>(std::string_view)>;
using StoryFactWriter =
    std::function<bool(std::string_view, bool)>;

// JSON-compatible Lua data used for durable game state. Functions, userdata,
// cyclic tables, and non-string/non-array keys are deliberately excluded.
struct ScriptPersistentValue {
    using Array = std::vector<ScriptPersistentValue>;
    using Object = std::map<std::string, ScriptPersistentValue>;
    using Storage = std::variant<std::monostate, bool, std::int64_t, double,
                                 std::string, Array, Object>;
    Storage value;
};

struct ScriptMenuChoice {
    std::string id;
    std::string label_key;
    // Optional per-task presentation text for content whose visible label
    // includes live gameplay values (for example hospital prices). Stable
    // choice IDs and selection policy remain Lua-owned.
    std::string inline_label;
};

enum class ScriptMenuInputMode : std::uint8_t {
    button_choice,
    cursor,
};

enum class ScriptMenuLayout : std::uint8_t {
    standard,
    compact_list,
};

// ROM 0x1F3C4 dialogue choices are the A/B/C buttons, so a button menu shows
// at most three rows [C]. The limit is data, not layout: a button menu with
// more choices is hosted as a cursor list instead of being truncated.
inline constexpr std::size_t kRomDialogueButtonChoices = 3U;

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
    ScriptMenuLayout layout = ScriptMenuLayout::standard;
    std::size_t selected_index = 0U;
    std::size_t button_choice_limit = kRomDialogueButtonChoices;
};

// The live field-menu route is stored in the same Lua state as gameplay
// scripts. These callbacks are narrow host services; command section,
// inventory selection, posture focus, and the active route stack remain
// Lua-owned.
struct ScriptMenuRouteArgs {
    std::optional<menus::CommandSection> command_section;
    std::optional<int> page;
    std::optional<int> row;
    std::optional<int> column;
};

enum class ScriptNotebookBoundary : std::uint8_t {
    event_gates_unresolved,
    tips_unresolved,
    current_run_unresolved,
    contacts_unresolved,
    shadowrunners_unresolved,
    mr_johnsons_unresolved,
};

struct ScriptMenuRouteHost {
    std::function<void(std::uint16_t)> play_sound;
    std::function<void(std::uint16_t)> stop_sound;
    std::function<void(std::uint32_t)> redraw;
    // mode (E.DisplayBlank), then for ON the measured black-frame count and
    // the raster line where the display re-enables on the reveal frame.
    std::function<void(std::uint16_t, std::uint16_t, std::uint16_t)> blank;
    // phase (E.MenuMask), staged frame index, then a black rect x,y,w,h.
    // LEAD frames show the last menu frame before the display goes off;
    // REVEAL frames start on the frame the display comes back on.
    std::function<void(std::uint16_t, std::uint16_t, std::int16_t,
                       std::int16_t, std::int16_t, std::int16_t)>
        mask;
    std::function<void(std::uint16_t)> set_window;
    std::function<void(std::uint16_t)> set_screen;
    std::function<void(bool, bool)> menu_closed;
    std::function<void(std::uint16_t, const ScriptMenuRouteArgs&)>
        legacy_open;
    std::function<std::pair<bool, bool>(std::uint16_t, std::uint16_t)>
        legacy_advance;
    std::function<void(ScriptNotebookBoundary)> notebook_boundary;
};

enum class ScriptFieldReturnTransitionPhase : std::uint8_t {
    none = 0U,
    menu_fade_out = 1U,
    black_hold = 2U,
    field_fade_in = 3U,
    complete = 4U,
};

struct ScriptRunnerAttributes {
    int body = 0;
    int quickness = 0;
    int strength = 0;
    int charisma = 0;
    int intelligence = 0;
    int willpower = 0;
};

enum class ScriptRunnerRace : std::uint8_t {
    human,
    elf,
    dwarf,
    orc,
    troll,
    unknown,
};

enum class ScriptRunnerClass : std::uint8_t {
    mage,
    shaman,
    samurai,
    decker,
    unknown,
};

// Source identity selectors are decoded here, at the native profile boundary.
// Race row order is ROM 0xC22A6; class 0/1 are the caster paths at 0xFBA6,
// with the selected/recruited profiles identifying the four class names.
constexpr ScriptRunnerRace runner_race_from_source_index(
    const std::uint8_t index) noexcept {
    switch (index) {
    case 0U: return ScriptRunnerRace::human;
    case 1U: return ScriptRunnerRace::elf;
    case 2U: return ScriptRunnerRace::dwarf;
    case 3U: return ScriptRunnerRace::orc;
    case 4U: return ScriptRunnerRace::troll;
    default: return ScriptRunnerRace::unknown;
    }
}

constexpr ScriptRunnerClass runner_class_from_source_index(
    const std::uint8_t index) noexcept {
    switch (index) {
    case 0U: return ScriptRunnerClass::mage;
    case 1U: return ScriptRunnerClass::shaman;
    case 2U: return ScriptRunnerClass::samurai;
    case 3U: return ScriptRunnerClass::decker;
    default: return ScriptRunnerClass::unknown;
    }
}

constexpr std::string_view runner_race_name(
    const ScriptRunnerRace race) noexcept {
    switch (race) {
    case ScriptRunnerRace::human: return "human";
    case ScriptRunnerRace::elf: return "elf";
    case ScriptRunnerRace::dwarf: return "dwarf";
    case ScriptRunnerRace::orc: return "orc";
    case ScriptRunnerRace::troll: return "troll";
    case ScriptRunnerRace::unknown: return "unknown";
    }
    return "unknown";
}

constexpr std::string_view runner_class_name(
    const ScriptRunnerClass character_class) noexcept {
    switch (character_class) {
    case ScriptRunnerClass::mage: return "mage";
    case ScriptRunnerClass::shaman: return "shaman";
    case ScriptRunnerClass::samurai: return "samurai";
    case ScriptRunnerClass::decker: return "decker";
    case ScriptRunnerClass::unknown: return "unknown";
    }
    return "unknown";
}

struct ScriptRunnerSkills {
    int sorcery = 0;
    int firearms = 0;
    int pistols = 0;
    int smgs = 0;
    int shotguns = 0;
    int melee_combat = 0;
    int throwing = 0;
    int computer = 0;
    int biotech = 0;
    int electronics = 0;
    int reputation = 0;
    int negotiation = 0;
};

struct ScriptMenuPartyMember {
    // Native adapter storage for source +0x10. The Lua menu API exposes its
    // lifecycle and clip-distribution meanings as named booleans instead.
    std::uint8_t status = 0U;
    std::uint8_t clips = 0U;
    std::uint8_t rounds = 0U;
    // Native adapter action fields. Lua receives item and spell IDs plus the
    // active runner's semantic spell list.
    int weapon_slot = -1;
    std::uint8_t weapon_code = 0U;
    std::uint8_t armor = 0U;
    std::uint8_t physical = 0U;
    std::uint8_t stun = 0U;
    ScriptRunnerRace race = ScriptRunnerRace::unknown;
    ScriptRunnerClass character_class = ScriptRunnerClass::unknown;
    // Typed runner fields used by the ROM Magic menu: +0x7F, +0x9B,
    // +0x9D, +0x4D, and +0xA0 respectively.
    std::uint8_t magic_resource = 0U;
    std::uint8_t karma = 0U;
    std::uint8_t magic_force = 0U;
    std::uint8_t posture = 0U;
    // Source table index and continuation are translated at the Lua boundary.
    std::uint16_t magic_table_index = 0U;
    std::uint32_t state_function = 0U;
    // Native source adapter storage only. Lua receives ordered equipment
    // records with item IDs and item-specific values through game.party().
    std::array<std::uint8_t, 8> inventory{};
    std::array<std::uint8_t, 8> phase{};
    ScriptRunnerAttributes attributes;
    ScriptRunnerSkills skills;
};

struct ScriptMenuSpellEntry {
    // The C++/Lua bridge carries stable domain IDs; the native host resolves
    // those IDs when it accesses the source spell table.
    std::string spell_id;
    std::uint8_t force_limit = 0U;
    std::int8_t availability = -1;
    std::uint8_t difficulty_modifier = 0U;
    std::int16_t base_effect = 0;
};

struct ScriptMenuSpellTable {
    std::uint8_t entry_count = 0U;
    std::array<ScriptMenuSpellEntry, 14> entries{};
};

struct ScriptNotebookFacts {
    bool current_run_available = false;
    std::optional<std::array<bool, 3>> event_gates;
    // Ordered menu catalog IDs. The host has already resolved persistent save
    // state before these facts cross into Lua.
    std::optional<std::vector<int>> tip_ids;
    std::optional<std::vector<int>> referral_ids;
    std::optional<std::vector<int>> contact_ids;
    std::optional<std::vector<int>> shadowrunner_ids;
};

struct ScriptCyberdeckFile {
    std::int8_t id = -1;
    std::uint16_t size = 0U;
    std::int8_t value = 0;
};

struct ScriptCyberdeckRecord {
    std::uint8_t mpcp = 0U;
    std::uint8_t hardening = 0U;
    std::uint16_t memory = 0U;
    std::uint16_t storage = 0U;
    std::uint8_t load_io = 0U;
    std::uint8_t response = 0U;
    std::array<std::uint8_t, 4> persona{};
    std::uint8_t persona_condition = 0U;
    std::int8_t model = -1;
    std::array<std::uint8_t, 12> ratings{};
    std::array<std::int8_t, 5> active_programs{{-1, -1, -1, -1, -1}};
    std::array<ScriptCyberdeckFile, 5> files{};
};

struct ScriptCyberdeckProgramSlot {
    // ROM 0x177B0 initializes FFF0ED to 6 and FFF0E8 to 1; the former is
    // the footer readiness icon, and the latter selects "Ready..." at E2FBE.
    std::uint8_t status = 6U;
    std::uint16_t timer = 1U;
    std::uint8_t phase = 1U;
};

struct ScriptMatrixNodeFacts {
    std::uint8_t ic_condition = 0U;
    std::uint8_t ic_code = 0U;
    std::uint8_t ic_rating = 0U;
    std::uint8_t ic_rating2 = 0U;
};

enum class ScriptMatrixRunStatus : std::uint8_t {
    pending,
    inactive,
    active,
    completed,
};

enum class ScriptMatrixRunNode : std::uint8_t {
    cpu,
    datastore,
};

enum class ScriptMatrixRunAction : std::uint8_t {
    crash_system,
    delete_file,
    upload_file,
    download_file,
};

struct ScriptMatrixRunObjective {
    ScriptMatrixRunNode node = ScriptMatrixRunNode::datastore;
    ScriptMatrixRunAction action = ScriptMatrixRunAction::download_file;
    std::string file_class;
    std::optional<int> file_id;
};

// Native Matrix objective facts use named operations and a typed lifecycle.
struct ScriptMatrixRunState {
    ScriptMatrixRunStatus status = ScriptMatrixRunStatus::pending;
    ScriptMatrixRunObjective objective;
    int file_size = 0;
    bool completion_marker_present = false;
    bool completion_recorded = false;
    // Stable Matrix system identity used by the selected objective.
    std::optional<int> target_system_id;
};

struct ScriptMatrixFieldState {
    std::optional<bool> alarm;
    std::optional<bool> security_off;
    std::optional<bool> maglocks_off;
};

struct ScriptActorRoleFacts {
    bool spotted = false;
    bool reinforcement = false;
    bool despawn_offscreen = false;
    bool quest = false;
    bool runner = false;
    bool pedestrian = false;
    bool hostile = false;
};

struct ScriptActorLifecycleFacts {
    bool active = false;
    bool thinking_enabled = false;
    bool ai_runner = false;
    bool downed = false;
    bool group_leader = false;
    bool dead = false;
};

struct ScriptActorInteractionFacts {
    bool projectile = false;
    bool proximity_interaction_pending = false; // [H] 0xAA88 consumes it.
    bool use_collision_heading = false; // [H] 0xB6BE selects +0x36.
    bool collision_recovery_active = false; // [H] 0x14E0A selects recovery.
    bool action_hook_pending = false; // [H] 0xA926 calls the actor hook.
    bool defense_difficulty_reduction = false;
    bool breathing_out = false;
    bool unseen = false;
    bool no_coverage_difficulty_bonus = false;
};

struct ScriptActorBehaviorFacts {
    bool route_clear = false; // [H] source bit value is known; use is not.
    bool path_active = false;
    bool restart_steering = false;
    bool wall_following = false;
    bool in_target_band = false;
    bool route_transition = false; // [H] source bit value is known; use is not.
    bool fleeing = false;
    bool escort = false;
};

// Gameplay actors exchange named facts with Lua. The field host translates
// these to or from legacy record flags at its adapter boundary.
struct ScriptActorFacts {
    ScriptActorLifecycleFacts lifecycle;
    ScriptActorInteractionFacts interaction;
    ScriptActorBehaviorFacts behavior;
    ScriptActorRoleFacts roles;
};

struct ScriptMatrixFieldActorState {
    std::string actor_id;
    ScriptActorRoleFacts roles;
};

// Semantic facts and outcomes for Matrix story files. The native adapter
// translates Notebook and save-record encodings before Lua sees this state.
struct ScriptMatrixDatastoreFacts {
    std::optional<std::uint32_t> nuyen;
    std::optional<bool> renraku_map_overlay_collected;
    std::optional<bool> johnson_contact_recorded;
    std::optional<bool> ucas_virology_research_complete;
    std::optional<bool> ucas_virology_report_collected;
    std::optional<bool> ito_project_file_collected;
    std::optional<bool> ito_confidential_file_collected;
    std::optional<bool> ito_financial_file_collected;
    std::optional<bool> ito_management_file_collected;
    std::optional<bool> aztechnology_confidential_file_collected;
    std::optional<bool> gates_financial_file_collected;
    std::optional<bool> gates_outdated_file_collected;
    std::optional<bool> hollywood_prisoner_file_collected;
    std::optional<bool> hollywood_outdated_file_collected;
    std::optional<bool> mitsuhama_confidential_file_collected;
    std::optional<bool> mitsuhama_security_file_collected;
    std::optional<bool> fuchi_management_file_collected;
    std::optional<bool> fuchi_security_file_collected;
    std::optional<bool> fuchi_security_followup_collected;
    std::optional<bool> renraku_confidential_file_collected;
    std::optional<bool> renraku_confidential_followup_collected;
    std::optional<bool> renraku_shaman_project_report_collected;
    std::optional<bool> renraku_mako_project_report_collected;
    std::optional<bool> renraku_michael_project_report_collected;
    std::optional<bool> renraku_map_project_report_collected;
    std::optional<bool> renraku_management_file_collected;
    std::optional<bool> lone_star_financial_file_collected;
    std::optional<bool> lone_star_prisoner_file_collected;
    std::optional<bool> ares_confidential_file_collected;
    std::optional<bool> ares_confidential_followup_collected;
};

enum class ScriptMatrixSecurityColor : std::uint8_t {
    unvisited,
    blue,
    green,
    orange,
    red,
};

// Host-side Matrix discovery state is expressed as facts about each map cell.
// The ROM stores these facts in FFFB30 and FFFAA4; neither packed bytes nor
// source-sized arrays cross the Lua boundary.
struct ScriptMatrixCellKnowledge {
    std::uint8_t analysis_level = 0U;
    ScriptMatrixSecurityColor security_color =
        ScriptMatrixSecurityColor::unvisited;
    bool revealed = false;
};

inline constexpr std::size_t kMatrixMapWidth = 14U;
inline constexpr std::size_t kMatrixMapHeight = 10U;

// ROM 0x53290 clears the 14x10 cell facts when the selected system changes.
struct ScriptMatrixMapKnowledge {
    std::uint8_t system_id = 0U;
    std::array<std::array<ScriptMatrixCellKnowledge, kMatrixMapWidth>,
               kMatrixMapHeight> rows{};
};

struct ScriptMatrixWorldState {
    std::optional<ScriptMatrixRunState> run;
    ScriptMatrixFieldState field;
    std::vector<ScriptMatrixFieldActorState> actors;
    std::optional<ScriptMatrixDatastoreFacts> datastore;
    // ROM FFF0CE is shared by Matrix node effects and field action services.
    // Signed-byte semantics determine when its karma rollover fires.
    std::optional<std::int8_t> karma_counter;
    // ROM FFF0A3 is reset by the Attack and Slow program handlers.
    std::optional<std::int8_t> passive_ic_counter;
    // Matrix discovery survives reopening the same system, resets when its
    // system changes, and is not part of the host save format.
    std::optional<ScriptMatrixMapKnowledge> map_knowledge;
};

struct ScriptMenuRouteGame {
    std::function<void()> reselect_leader;
    std::function<int()> leader_posture;
    std::function<void(int)> set_leader_posture;
    std::function<ScriptRunnerClass()> leader_class;
    std::function<std::array<ScriptMenuPartyMember, 3>()> party;
    std::function<std::optional<ScriptMenuSpellTable>()> spell_table;
    std::function<void(const ScriptMenuSpellTable&)> set_spell_table;
    std::function<int()> active_slot;
    std::function<int(int)> magazine_capacity;
    std::function<void(int, int, int)> set_runner_ammo;
    std::function<std::array<bool, 3>()> slot_active_set;
    std::function<void(const std::array<ScriptMenuPartyMember, 3>&)>
        commit_party;
    std::function<int(int, int)> roll_successes;
    std::function<bool()> pocket_extra_row;
    std::function<bool()> in_combat;
    // ROM 0xEFC4 allows Save/Load only when the player is outside [C].
    std::function<bool()> is_outside_area;
    std::function<bool(int)> save_game;
    std::function<bool(int)> load_game;
    std::function<int(int)> save_status;
    std::function<std::optional<std::uint32_t>()> nuyen;
    std::function<void(std::uint32_t)> set_nuyen;
    std::function<bool(ScriptFieldRegion)> begin_taxi_cabbie;
    std::function<void()> consume_debug_unlock;
    std::function<void()> seed_debug_contacts;
    std::function<void()> set_walk_fast;
    std::function<void()> install_test_deck;
    std::function<int(int)> source_random_bound;
    // Matrix 0x54A8E uses the same source RNG stream; this purpose-specific
    // binding keeps route QA able to script alert draws independently.
    std::function<int(int)> matrix_alert_random_bound;
    std::function<void(int)> dismiss_runner;
    std::function<std::optional<std::vector<int>>()> group_item_ids;
    std::function<std::optional<ScriptNotebookFacts>()> notebook_facts;
    std::function<std::optional<bool>()> cyberdeck_available;
    std::function<int()> deck_enter;
    std::function<void(int)> deck_leave;
    std::function<void(int)> set_deck_mode;
    std::function<bool()> matrix_online;
    std::function<std::optional<ScriptMatrixNodeFacts>()> matrix_node;
    std::function<int()> alert;
    std::function<bool()> world_effect_gate_negative;
    std::function<bool()> matrix_node_discovered;
    std::function<ScriptMatrixWorldState()> matrix_world_state;
    std::function<void(const ScriptMatrixWorldState&)>
        commit_matrix_world_state;
    std::function<ScriptCyberdeckRecord()> deck_record;
    std::function<std::array<ScriptCyberdeckProgramSlot, 5>()> deck_slots;
    std::function<void(const ScriptCyberdeckRecord&,
                       const std::array<ScriptCyberdeckProgramSlot, 5>&)>
        deck_commit;
    std::function<void(int)> program_unloaded;
};

struct ScriptMenuRouteBindings {
    ScriptMenuRouteHost host;
    ScriptMenuRouteGame game;
};

struct ScriptMenuRouteStartArguments {
    std::optional<std::uint8_t> matrix_system_id;
    // Optional source-node seed used by deterministic full-frame QA starts.
    std::optional<std::uint8_t> matrix_node_id;
    // Optional current IC condition for a progressed Matrix snapshot.
    std::optional<std::uint8_t> matrix_node_ic_condition;
    // Optional semantic analysis level for a progressed Matrix QA snapshot.
    std::optional<std::uint8_t> matrix_node_analysis_level;
    std::optional<ScriptFieldRegion> taxi_current_region;
    bool return_to_field = false;
};

struct ScriptMatrixMapCellVisual {
    std::array<std::uint16_t, 4> tile_words{};
};

enum class ScriptMatrixWorldModelId : std::uint8_t {
    system_access_network = 0U,
    central_processing_unit = 1U,
    sub_processor_unit = 2U,
    input_output_port = 3U,
    datastore = 4U,
    slave_module = 5U,
};

struct ScriptMatrixNodeStatusView {
    std::string file_class_label;
    std::string alert_band_label;
    int action_difficulty = 0;
    int background_profile = 0;
    int palette_theme = 1;
    ScriptMatrixWorldModelId world_model_id =
        ScriptMatrixWorldModelId::system_access_network;
};

struct ScriptMatrixTraversalView {
    ScriptMatrixWorldModelId world_model_id =
        ScriptMatrixWorldModelId::system_access_network;
    // ROM 0x19DF2 selects palette row 0 for an unknown node and rows 1..4
    // for a known node's security color.
    int palette_theme = 0;
    // 0x1A908 selects an IC mode 0..8, -1 selects the 0x1AA10 fallback,
    // and 0x45 suppresses this world-effect layer.
    int world_effect_mode = 0x45;
    int world_effect_phase = 0;
    // ROM 0x199D6 copies C/A/B confirmation into FFF0B7 (-1/0/1).
    int action_mode = 0;
      // Analyze's FFE00C countdown selects one of four paired ring layers;
      // the attached behavior-A runner changes pose after five service calls.
      int analyze_effect_phase = -1;
      int analyze_runner_phase = -1;
    // Lua owns the 61-VBlank setup and the selected 179/239-VBlank entry
    // profile. Qt only presents the resulting source-timed model canvas.
    int world_entry_phase = 2;
    int world_entry_setup_vblanks = 0;
    int world_entry_sequence_vblanks = 0;
    int world_entry_sequence_duration = 0;
    int world_entry_published_frame = 0;
    int model_canvas_x = 0;
    int model_canvas_y = 0;
    int model_canvas_width = 0;
    int model_canvas_height = 0;
    int analysis_level = 0;
    std::string function_label;
    std::string file_class_label;
    std::string alert_band_label;
    int action_difficulty = -1;
    std::string action_difficulty_label;
    std::string ice_label;
    std::string ice_rating_label;
    int ice_rating_column = 0;
    std::string second_ice_label;
    std::string second_ice_rating_label;
    int second_ice_rating_column = 0;
    std::string alert_label;
    std::string active_program_label;
    std::string active_program_status_label;
};

enum class ScriptMatrixRunInfoKind : std::uint8_t {
    no_run,
    complete,
    active,
};

struct ScriptMatrixRunInfoView {
    ScriptMatrixRunInfoKind kind = ScriptMatrixRunInfoKind::no_run;
    std::string message;
    std::string node_label;
    std::string node;
    std::string function_label;
    std::string node_function;
    std::string action_label;
    std::string action;
};

struct ScriptMatrixDeckStatsView {
    std::string manufacturer;
    std::string model;
    std::array<std::string, 10> labels{};
    std::array<unsigned int, 10> values{};
};

enum class ScriptMatrixLogoutVisualPhase {
    FADE,
    CROSS,
    DOT,
    SPARKLE,
    BLACK,
};

struct ScriptMatrixLogoutVisual {
    ScriptMatrixLogoutVisualPhase phase =
        ScriptMatrixLogoutVisualPhase::FADE;
    int horizontal_left = 8;
    int horizontal_right = 320;
    int vertical_top = 8;
    int vertical_bottom = 224;
};

struct ScriptNotebookCurrentRunView {
    std::string offer_variant_id;
    std::string contact_id;
    std::string run_id;
    int payment_amount = -1;
    std::string johnson_label;
    std::string johnson_name;
    std::string run_type_label;
    std::string run_type_name;
    std::string payment_label;
    std::string currency_symbol;
    std::vector<std::string> body_lines;
};

struct ScriptMenuRouteLayout {
    std::optional<int> title_x;
    std::optional<int> title_y;
    std::optional<int> separator_x;
    std::optional<int> separator_y;
    std::optional<int> label_x;
    std::optional<int> label_y;
    std::optional<int> cursor_x;
    std::optional<int> cursor_y;
    std::optional<int> row_spacing;
};

struct ScriptMenuRouteSnapshot {
    bool open = false;
    bool field_return_transition_active = false;
    std::uint16_t field_return_transition_elapsed_frames = 0U;
    std::uint8_t field_return_transition_palette_step = 0U;
    ScriptFieldReturnTransitionPhase field_return_transition_phase =
        ScriptFieldReturnTransitionPhase::none;
    std::uint16_t top_screen = 0U;
    std::string top_title;
    std::string top_text;
    std::string top_footer_text;
    std::vector<std::string> top_option_labels;
    std::optional<ScriptMenuRouteLayout> top_layout;
    std::optional<menus::CommandSection> command_section;
    std::optional<int> selected_inventory_row;
    // Other menu screens may have their own visible list index or row. These
    // are independent presentation coordinates, not command-page selectors.
    std::optional<int> page;
    std::optional<int> row;
    std::optional<bool> root_inventory_detail_pending;
    std::optional<bool> root_inventory_cursor_hidden;
    // Picture upload still in flight: only rows y>=57 are visible.
    std::optional<bool> root_inventory_picture_partial;
    std::optional<int> root_inventory_ammo_count;
    std::optional<int> top_column;
    std::optional<int> top_row;
    std::optional<int> top_selection;
    std::string top_selection_id;
    std::string top_action_id;
    std::optional<int> top_program_index;
    std::optional<int> top_program_slot;
    std::optional<int> top_program_choice;
    std::optional<bool> top_enter_callback_present;
    std::optional<bool> top_enter_called;
    std::optional<int> top_enter_transition;
    std::optional<bool> top_caption_visible;
    std::optional<ScriptTaxiCaptionPresentation>
        top_caption_presentation;
    std::optional<int> top_command;
    std::optional<int> top_quantity;
    std::optional<int> top_recipient;
    std::optional<int> top_source;
    std::optional<int> top_destination_slot;
    std::optional<int> top_message;
    std::optional<int> top_item_slot;
    std::optional<int> top_item_code;
    std::optional<int> top_item_phase;
    std::optional<int> top_target;
    std::optional<int> top_page;
    std::optional<int> top_detail_entry_id;
    std::optional<int> top_detail_row;
    std::optional<ScriptNotebookCurrentRunView> top_notebook_current_run;
    bool top_matrix_state_present = false;
    std::optional<int> top_matrix_system_id;
    std::optional<int> top_matrix_node;
    std::optional<int> top_matrix_previous_node;
    std::optional<int> top_matrix_alert;
    std::string top_matrix_phase;
    std::string top_matrix_pending_action;
    std::optional<bool> top_matrix_world_effect_gate_negative;
    std::optional<ScriptMatrixNodeStatusView> top_matrix_node_status;
    std::optional<ScriptMatrixTraversalView> top_matrix_traversal_view;
    std::optional<ScriptMatrixRunInfoView> top_matrix_run_info;
    std::optional<ScriptMatrixDeckStatsView> top_matrix_deck_stats;
    std::string top_matrix_deck_description;
    bool top_matrix_deck_detail_labels_present = false;
    std::array<std::string, 3> top_matrix_deck_detail_labels{};
    bool top_matrix_deck_program_detail_values_present = false;
    std::array<std::string, 3> top_matrix_deck_program_detail_values{};
    bool top_matrix_deck_runner_detail_labels_present = false;
    std::array<std::string, 2> top_matrix_deck_runner_detail_labels{};
    bool top_matrix_deck_runner_details_present = false;
    std::array<std::string, 2> top_matrix_deck_runner_details{};
    bool top_matrix_deck_program_ratings_present = false;
    std::array<std::uint8_t, 12> top_matrix_deck_program_ratings{};
    bool top_matrix_deck_active_programs_present = false;
    std::array<std::int8_t, 5> top_matrix_deck_active_programs{{
        -1, -1, -1, -1, -1}};
    bool top_matrix_deck_program_slot_status_present = false;
    std::array<std::uint8_t, 5> top_matrix_deck_program_slot_status{};
    std::optional<int> top_matrix_deck_palette_phase;
    std::optional<int> top_matrix_deck_parent_column;
    std::optional<int> top_matrix_deck_parent_row;
    bool top_matrix_deck_parent_program_name_present = false;
    std::string top_matrix_deck_parent_program_name;
    bool top_matrix_storage_panel_lines_present = false;
    std::array<std::string, 6> top_matrix_storage_panel_lines{};
    std::optional<int> top_matrix_storage_total;
    std::optional<int> top_matrix_storage_available;
    std::string top_matrix_storage_phase;
    std::string top_matrix_storage_separator;
    std::vector<std::string> top_matrix_storage_file_names;
    std::vector<int> top_matrix_storage_file_sizes;
    bool top_matrix_storage_erase_name_present = false;
    std::string top_matrix_storage_erase_name;
    std::optional<int> top_matrix_storage_erase_size;
    std::vector<std::string> top_matrix_storage_empty_message_lines;
    std::optional<int> top_matrix_countdown;
    std::optional<int> top_matrix_transition_fade_step;
    std::optional<ScriptMatrixLogoutVisual> top_matrix_logout_visual;
    bool top_matrix_map_cells_present = false;
    std::array<ScriptMatrixMapCellVisual, 140> top_matrix_map_cells{};
    bool top_matrix_map_labels_present = false;
    std::array<std::string, 6> top_matrix_map_labels{};
    std::vector<int> top_targets;
    std::optional<bool> top_recipient_full;
    std::optional<bool> top_yes;
    bool top_grid_cells_present = false;
    std::array<std::int8_t, 12> top_grid_cells{};
    std::optional<int> top_grid_effect_frames;
    std::vector<int> top_visible_item_ids;
    std::vector<int> top_visible_option_ids;
    std::vector<int> top_visible_entry_ids;
    struct MagicView {
        int row = 0;
        int column = 0;
        int table_entry_index = 0;
        std::string spell_id;
        int title_icon_x = 80;
        int title_icon_y = 24;
        int title_text_x = 128;
        int title_text_y = 32;
        int spell_grid_x = 32;
        int spell_grid_y = 168;
        int spell_grid_column_step = 32;
        int spell_grid_row_step = 24;
        int spell_grid_icon_width = 24;
        int spell_grid_icon_height = 16;
        int cursor_offset_x = 0;
        int cursor_offset_y = 0;
        int posture_bar_x = 160;
        int posture_bar_y = 136;
        int posture_text_x = 160;
        int posture_text_y = 144;
        std::vector<std::string> spell_ids;
        int force = 1;
        int maximum_force = 1;
        int success_value = 0;
        int success_row = -1;
        int effect_value = 0;
        int effect_row = -1;
        int availability = -1;
        int physical_defense_value = 0;
        int physical_defense_row = -1;
        int mana_defense_value = 0;
        int mana_defense_row = -1;
        int posture = 0;
        int cursor_phase = 0;
        // Phase whose SAT list is on screen: 0xFDFE builds FFD986 and only
        // requests the upload (FFD980), so the slot ring lags F0DB by one
        // frame [C e6 8958/8959]. -1 = follow cursor_phase.
        int cursor_sat_phase = -1;
        bool present = false;
    };
    std::optional<MagicView> magic_view;
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

struct ScriptShopProgressionOffer {
    std::uint8_t action_code = 0;
    std::uint8_t source_metadata = 0;
    std::uint16_t base_cost = 0;
};

struct ScriptShopEquipmentUpgradeOffer {
    std::uint8_t category = 0;
    std::uint32_t base_cost = 0;
    std::string label_key;
    std::string selected_name_key;
    std::string description_key;
};

struct ScriptShopOfferGroup {
    std::string id;
    std::array<std::string, 2> title_keys{};
    std::vector<ScriptShopOffer> offers;
    std::vector<ScriptShopProgressionOffer> progression_offers;
    std::vector<ScriptShopEquipmentUpgradeOffer> equipment_upgrade_offers;
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
    std::string progression_training_unavailable;
    std::string progression_maximum_level;
    std::string progression_cost_heading;
    std::string progression_level_label;
    std::string cyberware_already_owned;
    std::string cyberware_spurs_conflict;
    std::string cyberware_insufficient_essence;
    std::string cyberware_purchase_heading;
    std::string cyberware_essence_heading;
    std::string cyberware_essence_cost_heading;
};

struct ScriptShopDefinition {
    std::string id;
    std::string title_key;
    std::uint8_t menu_mode = 0; // ROM FFF0B6
    std::int8_t menu_result = 0; // ROM FFF0B7
    ScriptShopTextKeys text_keys;
    std::vector<std::uint8_t> equipment_upgrade_meter_thresholds;
    std::vector<ScriptShopOfferGroup> groups;
};

struct ScriptShopMenuSnapshot {
    bool open = false;
    std::string shop_id;
    std::uint8_t menu_mode = 0U;
    std::int8_t menu_result = 0;
    std::uint8_t action_cursor = 0U;
    std::uint8_t action_panel = 0U;
    std::uint8_t selected_action = 0U;
    std::string event;
};

struct ScriptWeaponTemplate {
    struct SourceAction {
        // Native selector reconstructed from the named weapon identity.
        std::uint8_t effect_table_index = 0;
        std::uint8_t effect_value = 0;
        std::uint8_t difficulty = 0;
    };

    std::string id;
    std::uint8_t item_code = 0;
    std::uint8_t magazine_capacity = 0;
    // Resolved gameplay value; damage_bar_index is a native presentation mapping.
    std::uint8_t damage_effect = 0;
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
    std::uint8_t primary_modifier = 0;
    std::uint8_t secondary_modifier = 0;
};

struct ScriptTargetMotionRange {
    std::uint16_t minimum_distance = 0;
    std::uint16_t maximum_distance = 0;
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
    ScriptTargetMotionRange target_motion_range{};
    std::string id;
    std::string kind;
    std::string name_key;
    std::string category_key;
    std::optional<Pricing> pricing;
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

// Dialogue content selects a semantic presentation contract. The game owns
// page flow; the active host decides how that view is drawn and timed.
enum class ScriptDialoguePresentation : std::uint8_t {
    field_npc,
    field_mode_text,
};

// Mirrors the public E.DialogueSource identities in Lua. Native field entry
// points choose a source; dialogue_framework.lua resolves its route policy.
enum class ScriptDialogueSource : std::uint8_t {
    building_entry,
    building_service,
    building_exit,
    field_npc_contact,
    field_event,
    field_activity,
    random_encounter,
    scripted_encounter,
    vid_phone,
    recruited_runner,
    run_target,
    story_activity,
    story_reward,
    party_recovery,
    field_menu,
    taxi,
    cinematic,
};

constexpr std::string_view script_dialogue_source_id(
    const ScriptDialogueSource source) noexcept {
    switch (source) {
    case ScriptDialogueSource::building_entry: return "building_entry";
    case ScriptDialogueSource::building_service: return "building_service";
    case ScriptDialogueSource::building_exit: return "building_exit";
    case ScriptDialogueSource::field_npc_contact: return "field_npc_contact";
    case ScriptDialogueSource::field_event: return "field_event";
    case ScriptDialogueSource::field_activity: return "field_activity";
    case ScriptDialogueSource::random_encounter: return "random_encounter";
    case ScriptDialogueSource::scripted_encounter: return "scripted_encounter";
    case ScriptDialogueSource::vid_phone: return "vid_phone";
    case ScriptDialogueSource::recruited_runner: return "recruited_runner";
    case ScriptDialogueSource::run_target: return "run_target";
    case ScriptDialogueSource::story_activity: return "story_activity";
    case ScriptDialogueSource::story_reward: return "story_reward";
    case ScriptDialogueSource::party_recovery: return "party_recovery";
    case ScriptDialogueSource::field_menu: return "field_menu";
    case ScriptDialogueSource::taxi: return "taxi";
    case ScriptDialogueSource::cinematic: return "cinematic";
    }
    return {};
}

struct ScriptDialogueOpenDescriptor {
    std::string source;
    std::string route_id;
    std::string scene_outcome;
    std::string exit_behavior;
    std::string transition;
    std::uint8_t transition_frames = 0U;
    std::string return_anchor;
    std::string return_facing;
    std::string music_return_policy;
    std::string input_lock;
    std::string scene_change_result_id;
    std::uint8_t input_lock_frames = 0U;
    std::uint8_t text_delay_frames = 0U;
};

// A resolved field route may refine a source default without changing the
// source identity. Empty fields inherit the E.DialogueSource defaults.
struct ScriptDialogueOpenRouteData {
    std::string scene_outcome;
    std::string exit_behavior;
    std::string transition;
    std::uint8_t transition_frames = 0U;
    std::string return_anchor;
    std::string return_facing;
    std::string music_return_policy;
    std::string input_lock;
    std::string scene_change_result_id;
    std::uint8_t input_lock_frames = 0U;
};

struct ScriptDialogueRequest {
    std::string speaker_id;
    std::string text_profile;
    std::string event_id;
    std::string inline_text;
    std::string continuation_key;
    ScriptDialoguePresentation presentation =
        ScriptDialoguePresentation::field_npc;
    std::uint8_t input_delay_frames = 0U;
};

enum class ScriptPackageRunStatus : std::uint8_t {
    none,
    handoff_due,
    payment_due,
};

enum class ScriptFieldModePromptId : std::uint8_t {
    none,
    little_chiba_store,
    ares_emporium,
};

// Translate the source-selected mode/event pair into the content identity
// consumed by Lua. ROM 0x1F830 selects Little Chiba's Scene-2/event-1 prompt;
// 0x1FA78 selects the Ares Scene-2/event-11 prompt [C]. The pair itself stays
// inside the adapter and is never exposed as Lua state.
constexpr ScriptFieldModePromptId
script_field_mode_prompt_id_from_source_context(
    const std::uint8_t source_mode,
    const std::uint8_t source_event_code) noexcept {
    if (source_mode == 2U && source_event_code == 1U) {
        return ScriptFieldModePromptId::little_chiba_store;
    }
    if (source_mode == 2U && source_event_code == 11U) {
        return ScriptFieldModePromptId::ares_emporium;
    }
    return ScriptFieldModePromptId::none;
}

constexpr std::string_view script_field_mode_prompt_id(
    const ScriptFieldModePromptId prompt_id) noexcept {
    switch (prompt_id) {
    case ScriptFieldModePromptId::little_chiba_store:
        return "little_chiba_store";
    case ScriptFieldModePromptId::ares_emporium:
        return "ares_emporium";
    case ScriptFieldModePromptId::none:
        return "none";
    }
    return "none";
}

// ROM 0x1F7E8 tests signed FFF0D5: zero selects the normal entry page,
// positive selects the package-payment page at 0xF4A98, and negative selects
// the package-handoff page at 0xF4A90 [C]. Lua receives this named status.
constexpr ScriptPackageRunStatus script_package_run_status_from_source_result(
    const std::int8_t source_result) noexcept {
    return source_result < 0 ? ScriptPackageRunStatus::handoff_due
         : source_result > 0 ? ScriptPackageRunStatus::payment_due
                             : ScriptPackageRunStatus::none;
}

constexpr std::string_view script_package_run_status_id(
    const ScriptPackageRunStatus status) noexcept {
    switch (status) {
    case ScriptPackageRunStatus::handoff_due:
        return "handoff_due";
    case ScriptPackageRunStatus::payment_due:
        return "payment_due";
    case ScriptPackageRunStatus::none:
        return "none";
    }
    return "none";
}

struct ScriptFieldModeContext {
    // Source-only route selectors. Lua receives a semantic prompt_id instead.
    std::uint8_t source_mode = 0;
    std::uint8_t source_event_code = 0;
    // Source-only signed result; Lua reads ScriptPackageRunStatus instead.
    std::int8_t source_signed_result = 0;
    // Source transition facts stay in this adapter; Lua receives named
    // profile-presence and serpent-lair transition facts.
    // Scene mode is FFFC0E; transition context is FFFC10.
    std::optional<std::uint16_t> source_alternate_profile;
    std::optional<std::uint16_t> source_transition_context;
    // Entity JSON chooses the named host service; Lua uses this ID to select
    // its matching field-mode narrative profile.
    std::string service_id;
    // A named modal variant lets a shared field-mode presenter resolve
    // service-specific notices without encoding content keys in C++.
    std::string prompt_variant;
    // Stable region identities passed to the Lua cabbie dialogue. The native
    // adapter maps source selector values through the table at 0xD445D [C].
    std::optional<ScriptFieldRegion> taxi_current_region;
    std::optional<ScriptFieldRegion> taxi_destination_region;
};

enum class ScriptRunFieldEventAction : std::uint8_t {
    no_action,
    objective_advanced,
    completed,
    entry_dialogue,
};

struct ScriptRunFieldEventDecision {
    ScriptRunFieldEventAction action =
        ScriptRunFieldEventAction::no_action;
    std::string run_id;
    std::uint32_t reward_nuyen = 0;
    std::uint8_t runner_progress_delta = 0;
    std::string dialogue_module;
    std::string dialogue_function;
    std::string actor_group_id;
};

struct ScriptActorDefeatFacts {
    bool is_ghoul_bounty_target = false;
};

enum class ScriptRunActorDefeatAction : std::uint8_t {
    no_action,
    objective_advanced,
};

struct ScriptRunActorDefeatDecision {
    ScriptRunActorDefeatAction action =
        ScriptRunActorDefeatAction::no_action;
    std::string run_id;
    std::uint8_t objective_progress = 0;
};

enum class ScriptRunJohnsonReturnAction : std::uint8_t {
    no_action,
    completed,
};

struct ScriptRunJohnsonReturnDecision {
    ScriptRunJohnsonReturnAction action =
        ScriptRunJohnsonReturnAction::no_action;
    std::string run_id;
    std::uint32_t reward_nuyen = 0;
    std::uint8_t runner_progress_delta = 0;
    std::uint8_t objective_progress = 0;
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

enum class ScriptPlayerDelayPurpose : std::uint8_t {
    // ROM 0xACAE enters the weapon firing path, which uses the weapon's
    // attack_delay_scale when resetting +0xDC. [C]
    weapon_attack,
    // ROM 0x14226 selects its helper scale from equipped +0x57. [C]
    reload_action,
    // ROM 0xACAE's shared tail replaces the helper delay with mode 0. [C]
    shared_action,
};

enum class ScriptFieldEncounterOutcomeKind : std::uint8_t {
    dialogue_event,
    dynamic_world_effect_sequence,
};

enum class ScriptFieldEncounterPlacementPolicy : std::uint8_t {
    explicit_actor_positions,
    screen_edge_candidates,
    map_derived_candidates,
};

enum class ScriptFieldEncounterTriggerAction : std::uint8_t {
    blocked,
    not_due,
    sample_probability,
    random_miss,
    select_alarm_sequence,
    select_scene_outcome,
};

enum class ScriptFieldSafeAction : std::uint8_t {
    none,
    renraku_map_overlay,
    fuchi_prototype_cyberheart,
};

struct ScriptFieldSafeDecision {
    ScriptFieldSafeAction action = ScriptFieldSafeAction::none;
    bool raise_alarm = false;
    bool reset_strike_team_stage = false;
    bool defer_alarm_transition = false;
};

struct ScriptFieldEncounterTriggerInput {
    std::uint16_t scene_id = 0;
    bool encounter_check_due = false;
    bool display_busy = false;
    bool leader_unseen = false;
    bool world_activity_locked = false;
    bool alarm_active = false;
    bool combat_active = false;
    bool encounter_in_progress = false;
    // Absent on the first Lua call. The host samples only after Lua requests
    // it, preserving conditional RNG consumption while exposing game-level
    // probabilities instead of the source's 16-bit operands.
    std::optional<double> probability_sample;
    double encounter_probability = 0.0;
};

struct ScriptFieldAlarmRegion {
    std::uint32_t min_tile_x = 0U;
    std::uint32_t max_tile_x = 0U;
    std::uint32_t min_tile_y = 0U;
    std::uint32_t max_tile_y = 0U;
};

struct ScriptFieldAlarmActorFacts {
    std::string actor_id;
    ScriptActorRoleFacts roles;
};

struct ScriptFieldAlarmProfile {
    std::vector<ScriptFieldAlarmRegion> alarm_regions;
    std::optional<std::string> reinforcement_activity_id;
};

struct ScriptFieldAlarmInput {
    bool alarm_check_due = false;
    bool building_scene = false;
    bool alarm_timeout_scene = false;
    std::uint32_t player_tile_x = 0U;
    std::uint32_t player_tile_y = 0U;
    bool security_disabled = false;
    bool alarm_active = false;
    std::int32_t alarm_time_remaining_frames = 0;
    std::optional<double> probability_sample;
    bool direct_raise = false;
    bool clear_alarm = false;
    bool disable_alarm = false;
    bool reset = false;
    bool scene_exit = false;
    std::vector<ScriptFieldAlarmRegion> alarm_regions;
    std::vector<ScriptFieldAlarmActorFacts> actors;
    std::optional<std::string> reinforcement_activity_id;
};

enum class ScriptFieldAlarmFlagAction : std::uint8_t {
    none,
    clear_spotted,
    promote_reinforcement,
};

struct ScriptFieldAlarmResult {
    bool alarm_active = false;
    bool security_disabled = false;
    std::int32_t alarm_time_remaining_frames = 0;
    bool request_probability_sample = false;
    bool schedule_timeout = false;
    std::optional<std::uint16_t> sound_start;
    std::optional<std::uint16_t> sound_stop;
    ScriptFieldAlarmFlagAction flag_action = ScriptFieldAlarmFlagAction::none;
    bool spawn_reinforcements = false;
    std::optional<std::string> reinforcement_activity_id;
    std::vector<ScriptFieldAlarmActorFacts> actors;
};

// Semantic combat inputs assembled from the live actor/profile record before
// Lua sees them. Core labels follow the ROM Attributes/Skills catalog
// (0xD1AA / 0xC1A2A); the source offsets stay at the import boundary.
struct ScriptCombatAttributes {
    std::uint8_t body = 0;
    std::uint8_t quickness = 0;
    std::uint8_t strength = 0;
    std::uint8_t intelligence = 0;
    std::uint8_t willpower = 0;
};

struct ScriptCombatSkills {
    std::uint8_t sorcery = 0;
    std::uint8_t firearms = 0;
    std::uint8_t pistols = 0;
    std::uint8_t smgs = 0;
    std::uint8_t shotguns = 0;
    std::uint8_t melee_combat = 0;
};

// Combat-facing abilities decoded from the runner/actor cyberware word.
// Source bit positions stay in the host adapter; gameplay scripts receive
// named capabilities and the bonuses those capabilities provide.
struct ScriptCombatCyberware {
    bool cybereyes = false;
    bool hand_razors = false;
    bool spurs = false;
    bool smartlink = false;
    std::uint8_t melee_strength_bonus = 0;
    std::uint8_t melee_defense_bonus = 0;
};

// Combat rules derived from source profile fields. Lua receives the behavior
// each rule needs, not race/state and class selector bytes.
struct ScriptCombatTraits {
    bool ignores_cover_condition_penalty = false;
    bool adds_two_condition_difficulty = false;
    bool is_special_spell_target = false;
    bool takes_fixed_meter_damage = false;
    // The host's zero-initialized source class selector is class 0, which
    // passes the signed class < 2 check in 0x1047E.
    bool qualifies_for_party_support_bonus = true;
    bool is_ghoul_bounty_target = false;
};

// Named combat profile choices decoded from the source actor record. Runtime
// scripts receive these choices rather than the packed +0xDF bit field.
struct ScriptCombatActionProfileOptions {
    bool primary = false;
    bool secondary = false;
};

// Source-derived facts consumed by the ROM close-target reaction path
// (0x55C76). Scripts receive semantic actor state, never a byte-addressed
// Genesis record.
struct ScriptCombatantFacts {
    // Host link classification from the live record. The Lua bridge converts
    // this into is_party_member; scripts never receive the numeric selector.
    std::uint16_t record_type = 0;
    // ROM 0x51C8 checks live sprite type +0x0E and skips the flame-bolt death
    // pose for Thon. The field adapter resolves it into this named rule [C].
    bool blocks_spell_death_pose = false;
    std::uint8_t source_flags = 0; // live +0x10
    std::uint8_t interaction_flags = 0; // live +0x11
    ScriptCombatCyberware cyberware;
    std::uint8_t physical_meter = 0;
    std::uint8_t mental_meter = 0;
    // Live actor +0x4D. The source value stays native; Lua receives the
    // posture adjustment percentage used by its combat rules.
    std::uint8_t source_posture = 0;
    ScriptCombatTraits traits;
    ScriptCombatAttributes attributes;
    ScriptCombatSkills skills;
    // Live +0x9D is the actor's force, used by spell and action rules.
    std::uint8_t force = 0;
    std::uint16_t last_action_successes = 0U;
    // Resolved item identity; the source +0x76 armor selector stays in the
    // native record adapter.
    std::string equipped_armor_id;
    std::uint8_t equipped_action_code = 0;
    ScriptCombatActionProfileOptions action_profile_options;
    // Host-side selector from the linked source record (+0xD8 -> +0xA0,
    // 0x55786). The Lua adapter resolves it to named profile values; 0xFFFF
    // means the relationship is unavailable.
    std::uint16_t linked_action_profile_index = 0xFFFFU;
    // Host-side source-record adapter only. Lua receives the ordered,
    // item-specific `inventory` view from combat_inventory.hpp.
    std::array<std::uint8_t, 8> item_codes{};
    std::array<std::uint8_t, 8> item_phases{};
    std::array<ScriptWeaponActionPolicy, 12> weapon_action_policies =
        default_script_weapon_action_policies();
    std::uint8_t spell_trial_cap = 0; // live +0x7F
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
    std::uint8_t source_record_flags = 0;
    bool source_record_flags_updated = false;
    bool physical = false;
    bool changed = false;
};

enum class ScriptPartyMemberRole : std::uint8_t {
    leader,
    companion_one,
    companion_two,
};

struct ScriptPartyMemberLifecycleFacts {
    bool active = false;
    bool downed = false;
    bool dead = false;
};

struct ScriptPartyRunnerDownContext {
    ScriptPartyMemberRole target = ScriptPartyMemberRole::leader;
    std::uint8_t physical_meter = 0; // live runner +0x4C
    std::uint8_t mental_meter = 0; // live runner +0x9C
    std::array<ScriptPartyMemberLifecycleFacts, 3> party{};
};

struct ScriptPartyRunnerDownDecision {
    bool downed = false;
    bool party_wipe_imminent = false;
};

enum class ScriptPartyWakeDestination : std::uint8_t {
    little_chiba,
    docwagon,
};

struct ScriptPartyMemberRecoveryFacts {
    ScriptPartyMemberLifecycleFacts lifecycle;
    std::uint8_t physical_meter = 0;
    std::uint8_t mental_meter = 0;
};

struct ScriptPartyWipeRecoveryContext {
    std::array<ScriptPartyMemberRecoveryFacts, 3> party{};
    std::uint32_t nuyen = 0;
    bool motel_bill_paid = false;
};

struct ScriptPartyWipeRecoveryDecision {
    bool leader_active = false;
    bool leader_downed = false;
    bool leader_dead = false;
    std::uint8_t leader_physical_meter = 0;
    std::uint8_t leader_mental_meter = 0;
    std::array<bool, 2> dismiss_companions{};
    std::array<std::uint8_t, 2> companion_physical_meters{};
    std::uint32_t remaining_nuyen = 0;
    ScriptPartyWakeDestination wake_destination =
        ScriptPartyWakeDestination::little_chiba;
};

struct ScriptPartyRunnerRefreshMemberFacts {
    ScriptPartyMemberLifecycleFacts lifecycle;
    bool selection_pending = false;
    bool ends_roster = false;
};

struct ScriptPartyRunnerRefreshContext {
    ScriptPartyMemberRole previous_role = ScriptPartyMemberRole::leader;
    bool suppression_override = false;
    std::array<ScriptPartyRunnerRefreshMemberFacts, 3> party{};
};

struct ScriptPartyRunnerRefreshDecision {
    ScriptPartyMemberRole active_role = ScriptPartyMemberRole::leader;
    std::array<bool, 3> selection_pending{};
    bool active_role_changed = false;
};

enum class ScriptPartyWipeObjectiveResult : std::uint8_t {
    unchanged,
    cleared,
    unavailable,
};

struct ScriptPartyWipeRunFailureContext {
    bool world_activity_scene_reset = false;
};

struct ScriptPartyWipeRunFailureDecision {
    bool page_pending = false;
    bool clear_high_effects = false;
    bool clear_world_activity_request = false;
    ScriptPartyWipeObjectiveResult objective_result =
        ScriptPartyWipeObjectiveResult::unchanged;
    bool clear_player_spawn_request = false;
};

enum class ScriptActorSpellFieldEffectKind : std::uint8_t {
    none,          // 0x00 / return at 0x1584C [C]
    field_barrier, // 0x01 / 0x15830 -> 0x49A2 [C]
};

struct ScriptActorSpellActionResolution {
    std::string spell_id;
    // Derived from spell_id at the host boundary for source-record checks.
    std::uint8_t source_action_code = 0;
    std::string target_id;
    // Untargeted 0xAE6E calls carry the caster's source slot/type here and an
    // empty target id [C]; the stale 68k A1 value is not a stable target [H].
    bool untargeted = false;
    // For untargeted casts these fields identify the caster record [C].
    std::uint8_t target_record_slot = 0;
    std::uint16_t target_record_type = 0;
    ScriptActorSpellFieldEffectKind field_effect_kind =
        ScriptActorSpellFieldEffectKind::none;
    std::uint16_t successes = 0U;
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
    // Host-side record update assembled from semantic Lua inventory items.
    std::array<std::uint8_t, 8> item_codes{};
    std::array<std::uint8_t, 8> item_phases{};
    std::uint8_t mental_meter = 0;
    bool special_result_published = false;
    std::uint8_t special_result_successes = 0;
    std::uint8_t special_result_bonus = 0;
};

struct ScriptActorSpellEffectResolution {
    std::string spell_id;
    // Derived from spell_id at the host boundary for source behavior checks.
    std::uint8_t source_action_code = 0;
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
    std::uint8_t source_record_flags = 0;
    bool source_record_flags_changed = false;
    bool changed = false;
};

struct ScriptProjectileCombatTarget {
    std::string id;
    // Native record link used to apply Lua's ID-keyed combat result.
    std::uint8_t source_record_slot = 0;
    ScriptCombatantFacts combatant;
};

struct ScriptFieldEncounterActor {
    std::uint16_t source_order = 0;
    std::string sequence_id;
    std::optional<std::uint16_t> run_target_spawn_x;
    std::optional<std::uint16_t> run_target_spawn_y;
    std::string actor_name;
    std::string profile_id;
    std::uint16_t entity_kind = 0;
    ScriptActorActionKind action_kind = ScriptActorActionKind::none;
    std::uint8_t action_code = 0;
    std::uint8_t palette_line = 0;
    std::string sprite_stem;
    bool activity_probe_enabled = false;
    std::uint8_t source_spawn_state_flags = 0; // +0x9E template byte [C]
    std::uint8_t source_interaction_flags = 0;
    std::uint8_t physical_meter = 0;
    std::uint8_t action_delay_modifier = 0;
    ScriptCombatantFacts combatant;
};

// Host-side spawn descriptors for conditional pedestrian variants. Lua gets
// semantic candidate identities and variant names; this catalog retains
// placement and presentation data for FieldSceneRuntime to publish an actor.
struct ScriptConditionalActorCandidate {
    std::uint16_t scene_id = 0;
    std::uint8_t choice = 0;
    std::string candidate_id;
    std::string actor_name;
    std::string profile_id;
    std::uint16_t entity_kind = 0;
    std::uint8_t palette_line = 0;
    std::uint16_t world_x = 0;
    std::uint16_t world_y = 0;
    std::uint16_t heading = 0;
    // Offline extraction resolves the ordered source writes to final state.
    std::uint8_t source_spawn_state_flags = 0;
    std::string selection_group;
};

// Semantic gameplay facts for field actor selection. ROM event bytes, story
// bitfields, compact party markers, and effect indices are normalized by the
// field host before Lua applies the selection policy.
enum class ScriptFieldSceneMode : std::uint8_t {
    none,
    street,
    building,
    cave,
    unknown,
};

// Actor behavior and movement tuning are expressed as game concepts. The
// host derives these facts from the source actor record before calling Lua.
enum class ScriptActorEngagementPolicy : std::uint8_t {
    patrol,
    watch_nearby_runner,
    engage_within_range,
    engage,
};

enum class ScriptActorApproachBand : std::uint8_t {
    far,
    medium,
    close,
};

struct ScriptActorBehaviorProfile {
    ScriptActorEngagementPolicy engagement_policy =
        ScriptActorEngagementPolicy::patrol;
    std::optional<ScriptActorApproachBand> approach_band;
};

// ROM 0x13EF4/0x13F36/0x13F52 derive the think policy from actor +0x4D;
// 0x13D84 uses that field to select a range pair at 0x13E58 [C].
ScriptActorBehaviorProfile
script_actor_behavior_profile_from_source_posture(
    std::uint8_t source_posture) noexcept;
ScriptCombatCyberware script_combat_cyberware_from_source_flags(
    std::uint16_t source_flags) noexcept;
// +0x9B race/state and class selectors are decoded at the host boundary.
// ROM 0x55740 uses exact +0x9B values for condition modifiers, 0x55B78 has
// the 0x17 spell-target branch, and 0x559BC consumes the ghoul selector [C].
ScriptCombatTraits script_combat_traits_from_source_values(
    std::int8_t race_or_state, std::uint8_t class_code) noexcept;
// ROM 0x51C8 compares live +0x0E with kind 0xF before calling 0xB0AC; 0x44BC
// copies the template's first word to +0x0E. Resolve that sprite type here [C].
bool script_sprite_type_blocks_spell_death_pose(
    std::uint16_t source_sprite_type) noexcept;
// Convert source sprite selectors to named content profiles before they reach
// gameplay Lua. Selector values remain an importer/runtime adapter detail.
std::optional<std::string_view>
script_actor_visual_profile_id_from_source_type(
    std::uint16_t source_sprite_type) noexcept;
std::optional<std::uint16_t>
script_source_type_from_actor_visual_profile_id(
    std::string_view visual_profile_id) noexcept;
// ROM +0xDF bits 0/1 choose the primary/secondary dynamic action profile [C].
ScriptCombatActionProfileOptions
script_combat_action_profile_options_from_source_flags(
    std::uint8_t source_flags) noexcept;
std::uint16_t script_posture_reduction_percent_from_source_value(
    std::uint8_t source_posture) noexcept;
std::optional<std::uint8_t>
script_source_posture_from_reduction_percent(
    std::uint16_t posture_reduction_percent) noexcept;

enum class ScriptSceneTenActivityVariant : std::uint8_t {
    standard,
    alternate,
};

ScriptFieldSceneMode script_field_scene_mode_from_source_word(
    std::uint16_t source_mode) noexcept;
ScriptSceneTenActivityVariant
script_scene_ten_activity_variant_from_source_event(
    std::uint8_t source_event) noexcept;
std::vector<std::string>
script_mode_one_activity_ids_present_from_source_markers(
    std::uint8_t primary_marker, std::uint8_t secondary_marker);

enum class ScriptSceneActivityState : std::uint8_t {
    idle,
    actors_being_installed,
    story_interaction_active,
    other_activity_active,
    queued_activity,
};

struct ScriptConditionalActorSelectionContext {
    ScriptSceneActivityState activity_state = ScriptSceneActivityState::idle;
    std::optional<std::string> active_offer_id;
    ScriptFieldSceneMode scene_mode = ScriptFieldSceneMode::none;
    ScriptSceneTenActivityVariant scene_ten_activity_variant =
        ScriptSceneTenActivityVariant::standard;
    std::vector<std::string> completed_story_facts;
    std::vector<std::string> party_present_activity_ids;
    bool recruitment_encounter_ready = false;
};

enum class ScriptConditionalActorSelectionKind : std::uint8_t {
    none,
    story_contact,
    pending_world_activity,
    random_pedestrian,
    mode_one_world_activity,
};

enum class ScriptConditionalActorVariant : std::uint8_t {
    none,
    first,
    second,
};

struct ScriptConditionalActorSelection {
    ScriptConditionalActorSelectionKind kind =
        ScriptConditionalActorSelectionKind::none;
    ScriptConditionalActorVariant selection_variant =
        ScriptConditionalActorVariant::none;
    bool choice_changed = false;
    std::string selection_id;
};

enum class ScriptFieldActorContactHandler : std::uint8_t {
    ordinary,
    world_effect,
    pending_world_effect,
};

enum class ScriptFieldActorContactAction : std::uint8_t {
    start_activity,
    aragorn_contact,
    ilene_activity,
    random_pedestrian_contact,
    generic_contact_dialogue,
};

enum class ScriptWorldActivityKind : std::uint8_t {
    scene_script,
    event_driven,
};

// The source selector is resolved to a stable content id at the import
// boundary. This binding stays in the native adapter and is never exposed to
// gameplay Lua.
struct ScriptActivitySelectorBinding {
    std::uint16_t scene_id = 0;
    std::uint16_t selector = 0;
    std::string activity_id;
};

// Import-only mapping for connecting extracted actor rows to stable content
// identities. Source offsets and extractor keys stay in the native adapter.
struct ScriptWorldActivityGroupBinding {
    std::uint16_t scene_id = 0;
    std::uint32_t source_group_offset = 0;
    std::string activity_id;
};

// A source-selected field script list is installed by the world-activity
// selector and consumed by the next actor-service pass. It remains separate
// from random-encounter actors because its placement and lifecycle come from
// the scene-script list at 0x167AE / 0x43B6.
struct ScriptWorldActivityActor {
    std::uint16_t scene_id = 0;
    std::string instance_id;
    std::string activity_id;
    ScriptWorldActivityKind activity_kind =
        ScriptWorldActivityKind::event_driven;
    std::uint16_t source_order = 0;
    bool activity_probe_enabled = false;
    std::string actor_name;
    std::string profile_id;
    std::uint16_t entity_kind = 0;
    std::string sprite_stem;
    ScriptActorActionKind action_kind = ScriptActorActionKind::none;
    std::uint8_t action_code = 0;
    std::uint8_t palette_line = 0;
    std::uint16_t world_x = 0;
    std::uint16_t world_y = 0;
    std::uint16_t heading = 0;
    std::uint8_t source_spawn_state_flags = 0;
    std::uint8_t source_interaction_flags = 0;
    std::uint8_t source_behavior_flags = 0;
    std::uint8_t physical_meter = 0;
    std::uint8_t action_delay_modifier = 0;
    ScriptCombatantFacts combatant{};
};

struct ScriptFieldEncounterChoice {
    std::string id;
    std::string label;
};

// Encounter content reaches the host as semantic IDs and typed actor records.
// Dialogue behavior remains an explicit boundary until its Lua events are ported.
struct ScriptFieldEncounterOutcome {
    ScriptFieldEncounterOutcomeKind kind =
        ScriptFieldEncounterOutcomeKind::dialogue_event;
    ScriptFieldEncounterPlacementPolicy placement_policy =
        ScriptFieldEncounterPlacementPolicy::screen_edge_candidates;
    std::uint16_t scene_id = 0;
    std::string outcome_id;
    std::string sequence_id;
    std::string event_id;
    std::string opening_text;
    std::vector<ScriptFieldEncounterChoice> choices;
    std::vector<ScriptFieldEncounterActor> actors;
};

enum class ScriptActorHeartbeatPhase : std::uint8_t {
    behavior,
    render,
};

enum class ScriptActorItemSpawnResult : std::uint8_t {
    spawned,
    blocked,
};

enum class ScriptActorAnimationMode : std::uint8_t {
    idle,
    walking,
    attack,
    strike, // ROM 0xB1DC C0A2 unarmed strike list
    cast,   // ROM 0xAF7C/0xAFAE C082 spell pose
    runner_down, // ROM 0xB330 C0B2 party runner down list
    runner_hurt, // ROM 0xAFF2 C072 party runner hurt list
};

enum class ScriptMovementSubstateUpdate : std::uint8_t {
    unchanged,
    direct_translation,
    cardinal_fallback,
};

enum class ScriptCardinalDirection : std::uint8_t {
    north,
    east,
    south,
    west,
};

struct ScriptMovementRouteUpdate {
    enum class Kind : std::uint8_t {
        unchanged,
        reset,
        blocked_by_direction,
    };

    Kind kind = Kind::unchanged;
    ScriptCardinalDirection direction = ScriptCardinalDirection::north;
};

// The Lua movement policy reports semantic directions and route outcomes.
// These adapters translate only when committing to ROM-shaped live state.
std::uint16_t source_movement_substate_for_cardinal_direction(
    ScriptCardinalDirection direction) noexcept;
std::int16_t source_route_status_after_script_update(
    std::int16_t current_status,
    const ScriptMovementRouteUpdate& update) noexcept;

struct ScriptMovementStepState {
    std::uint16_t position_x = 0;
    std::uint16_t position_y = 0;
    std::uint16_t heading = 0;
    std::uint16_t animation_heading = 0;
    ScriptMovementRouteUpdate route_update{};
    std::uint8_t route_flags = 0;
    ScriptMovementSubstateUpdate substate_update =
        ScriptMovementSubstateUpdate::unchanged;
    std::optional<ScriptCardinalDirection> cardinal_fallback_direction;
    // Actions replace the packed movement bytes previously exposed to Lua.
    // Their source encoding stays in the Qt/native adapter.
    bool mark_reset_after_block = false;
    bool mark_directional_fallback = false;
    bool clear_collision_recovery = false;
    bool clear_pending_event = false;
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
using ScriptMovementModeTransition =
    std::function<ScriptMovementCollisionRecoveryResult(
        std::uint16_t, std::uint16_t, std::int16_t, std::uint8_t)>;

// Clip length is read from the loaded host image sequence; the asset identity
// remains private to the field renderer and is never passed through Lua.
struct ScriptActorProjectileAnimation {
    std::size_t frame_count = 0U;
};

struct ScriptActorHitSparkSpawnRequest {
    std::string target_id;
    std::uint16_t world_x = 0U;
    std::uint16_t world_y = 0U;
};

// The field scene adapts projectile records to this game-level state at the
// host boundary. Lua owns movement and burst policy; callbacks expose map and
// collision facts without exposing the record representation.
enum class ScriptActorProjectileRole : std::uint8_t {
    primary,
    secondary,
};

enum class ScriptActorProjectilePhase : std::uint8_t {
    inactive,
    flying,
    awaiting_animation,
};

struct ScriptActorProjectileVector {
    double x = 0.0;
    double y = 0.0;
};

enum class ScriptActorProjectileOutcome : std::uint8_t {
    none,
    stopped,
    retired,
    hit_resolution_pending,
    linked_followup_pending,
    burst_followup_restarted,
};

enum class ScriptActorProjectileAnimationAction : std::uint8_t {
    unchanged,
    stop,
    restart,
};

struct ScriptActorProjectileMovement {
    ScriptActorProjectileRole role = ScriptActorProjectileRole::primary;
    ScriptActorProjectilePhase phase = ScriptActorProjectilePhase::inactive;
    bool impact_occurred = false;
    bool animation_finished = false;
    ScriptActorProjectileVector position{};
    ScriptActorProjectileVector velocity{};
    double remaining_flight_range = 0.0;
    std::uint32_t shots_remaining = 0U;
    double bearing_degrees = 0.0;
    ScriptActorProjectileVector burst_origin{};
    std::uint16_t attack_successes = 0U;
    ScriptActorProjectileAnimationAction animation_action =
        ScriptActorProjectileAnimationAction::unchanged;
    std::string hit_target_id;
};

struct ScriptProjectileTerrainSample {
    bool blocked = false;
    bool boundary_active = false;
    bool outside_map = false;
};

enum class ScriptProjectileContactKind : std::uint8_t {
    none,
    target_hit,
    linked_followup,
};

struct ScriptProjectileCollisionSearchResult {
    ScriptProjectileContactKind kind = ScriptProjectileContactKind::none;
    std::optional<ScriptActorProjectileVector> target_position;
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
        std::uint16_t x, std::uint16_t y)>;

struct ScriptActorMovementStep {
    enum class Outcome : std::uint8_t {
        accepted,
        reset,
    };

    // The parent behavior decision carries the semantic animation ID and
    // facing. The host resolves those to presentation assets after movement.
    Outcome outcome = Outcome::accepted;
    ScriptMovementStepState state{};
};

struct ScriptActorTargetListRebuildFacts {
    std::uint16_t frame_counter = 0U; // FFDF88, typed field-frame fact [C]
    std::uint16_t display_service_word = 0U; // FFDFE8 [C]
};

struct ScriptActorTargetListRebuildDecision {
    bool rebuild_world = false;
    bool rebuild_runner = false;
};

enum class ScriptFormationPosition : std::uint8_t {
    primary,
    secondary,
    other,
};

enum class ScriptTargetSelectionPolicy : std::uint8_t {
    standard,
    marker,
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
    bool projected_y_nonzero = false; // source entity +0x0C != 0 [C]
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
    // Native link used by the field adapter when Lua returns this candidate's
    // stable ID. The bridge does not serialize the source position.
    std::optional<std::uint8_t> source_record_slot;
    std::optional<ScriptCombatantFacts> combatant;
    // ROM 0x141E8 reads the linked record's +0x2E heading before jumping to
    // 0xB39A. `heading` above is the relative bearing used by route policy;
    // keep the linked record's facing separate for C042 pose selection.
    std::optional<std::uint16_t> source_record_heading;
    // Semantic role used by formation-follow policy. The native field adapter
    // derives this from the actor's live record identity before Lua sees it.
    std::optional<ScriptFormationPosition> formation_position;
    std::optional<ScriptPartyMemberRole> party_member_role;
};

using ScriptActorRandomBound =
    std::function<std::optional<std::uint16_t>(std::uint16_t)>;
// True when 0x14D8E leaves carry clear for these linked world coordinates.
using ScriptActorActionLineClear =
    std::function<bool(std::uint16_t world_x, std::uint16_t world_y)>;
struct ScriptWorldPosition {
    std::int32_t x = 0;
    std::int32_t y = 0;
};
struct ScriptActorDestinationMeasurement {
    std::uint16_t heading = 0;
    std::uint16_t distance = 0;
    std::uint16_t step_cost = 0;
};
using ScriptActorDestinationMeasure =
    std::function<std::optional<ScriptActorDestinationMeasurement>(
        std::uint16_t world_x, std::uint16_t world_y)>;

struct ScriptActorActionTiming {
    std::uint8_t quickness = 0;
    std::uint8_t intelligence = 0;
    std::uint8_t action_delay_modifier = 0;
};

// Source-derived identity and spawn facts passed to an actor's Lua heartbeat.
// Lua owns per-instance behavior state; the host retains world facts and
// consumes the returned presentation/action snapshot.
struct ScriptActorBehaviorContext {
    std::string_view profile_id;
    // Native-only source selector. actor_heartbeat translates it to a named
    // visual_profile_id and never serializes this integer to Lua.
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
    // The host's loaded magazine and weapon template show whether this held
    // action will spawn a shot (empty-magazine reloads do not start a pose).
    bool player_weapon_action_will_fire = false;
    bool combat_input_blocked = false;
    // ROM 0xAD86: a negative +0x56 sends the field player's held-A action
    // to 0xAE6E/0x1033E. The host offers the equipped D4EA0 row code
    // (+0x57, spell row + 1) and the B-selected +0xA2 target in
    // ai_target_candidates[1]; Lua owns the cast policy.
    bool player_spell_available = false;
    std::uint8_t player_spell_code = 0;
    // ROM 0xAD86: non-negative +0x56 with +0x57 == 0 enters the 0xB1DC
    // unarmed strike.
    bool player_melee_available = false;
    std::optional<MovementDirection> movement_input_direction;
    std::optional<MovementDirection> movement_direction_override;
    ScriptActorHeartbeatPhase phase = ScriptActorHeartbeatPhase::behavior;
    ScriptActorAnimationMode animation_mode =
        ScriptActorAnimationMode::idle;
    std::optional<std::uint8_t> animation_direction_override;
    bool animation_clock_reset_requested = false;
    std::optional<ScriptActorProjectileAnimation> projectile_animation;
    std::optional<ScriptActorProjectileMovement> projectile_movement;
    // The first primary-record behavior pass resolves ROM 0x55640 once;
    // subsequent movement and the cloned secondary record retain its result.
    bool projectile_attack_seed_pending = false;
    std::vector<ScriptProjectileCombatTarget> projectile_combat_targets;
    ScriptProjectileTerrainProbe projectile_terrain_probe;
    ScriptProjectileCollisionSearch projectile_collision_search;
    bool movement_services_available = false;
    // Gameplay movement uses a direction and distance. The native adapter
    // does not pass the ROM's eight-entry direction-offset table into Lua.
    // C002's cardinal vector magnitude is 40 map units per update [C].
    std::int16_t movement_default_step_distance = 40;
    ScriptMovementStepState movement_state{};
    std::uint8_t movement_entity_flags = 0;
    bool walk_fast_enabled = false;
    bool movement_transition_active = false;
    std::uint16_t movement_scene_id = 0;
    // Native-only FFE186 value for source-shaped entity services. The Lua
    // adapter exposes `field_sequence_active` and `final_battle_active`.
    std::uint8_t world_scene_state = 0;
    std::uint16_t movement_collision_mode = 0;
    ScriptMovementCollisionQuery movement_collision;
    ScriptMovementRouteLookup movement_route_lookup;
    ScriptMovementCollisionRecovery movement_collision_recovery;
    // The field-player B7B8 path dispatches mode/map events before BADE's
    // later B732 route-fallback policy. Generic actor probes need not provide
    // this field-state service.
    ScriptMovementModeTransition movement_mode_transition;
    // Dynamic combat profiles carry source entity coordinates and measured
    // target facts from the native geometry query. Lua chooses behavior; the
    // host does not infer a target from screen proximity. This includes
    // source-default actors that use an extracted dynamic combat profile.
    bool dynamic_world_encounter_actor = false;
    // Narrowly validates item-action results for routed default-list actors
    // that keep their static profile policy.
    bool source_default_item_action_profile = false;
    // Lua receives the resolved waypoint capability, not the template's
    // source sprite selector.
    bool source_actor_has_waypoint_data = false;
    // ROM world-activity script lists may create non-encounter actors in the
    // same runtime pool. Keep that provenance explicit for Lua profile rules.
    bool world_activity_script_actor = false;
    // Native record link for result validation and host-side updates only.
    std::optional<std::uint8_t> source_record_slot;
    // ROM actor +0x3E: 0x44BC copies FFDD40 for scene actors and 0x42CC
    // initializes runner records to 0..2 [C]. Retained for native validation;
    // Lua receives semantic actor facts instead of this raw value.
    std::optional<std::uint8_t> source_actor_slot;
    // Current position in the field world. Source-record decoding stays in
    // the native adapter; Lua receives one named position value.
    std::optional<ScriptWorldPosition> world_position;
    ScriptActorFacts actor_facts;
    // One field-frame token suppresses later due actor thoughts [C].
    std::optional<bool> source_think_suppressed;
    // Shared Thon action cooldown, measured in eligible behavior checks.
    std::optional<std::uint32_t> thon_action_cooldown_remaining;
    // FFE16F / FFDF40 actor-think inputs. Alarm remains optional until the
    // field security fact is known; scene mode and camera/profile bounds are
    // populated from the live field state [C].
    std::optional<bool> source_alarm_active;
    std::optional<ScriptFieldSceneMode> scene_mode;
    std::optional<std::uint16_t> source_camera_x;
    std::optional<std::uint16_t> source_camera_y;
    std::optional<std::uint16_t> source_camera_limit_x;
    std::optional<std::uint16_t> source_camera_limit_y;
    std::uint16_t ammunition_remaining = 0;
    bool hold_for_ending_activity = false;
    // Animation policy is exposed as game state, not as the source record's
    // packed pose selector. The host derives these facts from the actor's
    // current semantic animation state.
    bool animation_blocks_behavior = false;
    bool can_start_item_action = true;
    std::optional<ScriptActorItemSpawnResult> source_item_spawn_result;
    // ROM posture bytes become a behavior policy and named approach band at
    // the native boundary; Lua never interprets the source posture value.
    std::optional<ScriptActorBehaviorProfile> actor_behavior_profile;
    bool world_activity_active = false;
    // ROM 0x14C7C's kind-0xD branch checks animation frame +0x1E here.
    std::int16_t ai_animation_frame = 0;
    // ROM FFF9F8 bit 7 selects 0x4D vs 0x4E after the target heading is
    // accepted. The source condition is a fact; Lua chooses the cue.
    bool target_heading_audio_alternate = false;
    // The importer builds a semantic action-timing value from Quickness,
    // Intelligence, and the 0xD7EA capability result. [C]
    std::int16_t initial_action_delay = 0;
    ScriptActorActionTiming action_timing{};
    std::array<bool, 18> world_effect_slot_occupancy{};
    // ROM +0x11.4 target-selection latch, forwarded as a fact; Lua owns the
    // visible 0x5C7E marker cadence and returns its presentation state.
    bool target_selection_latched = false;
    // ROM 0x5C82..0x5CBA: the leader (FFFC04) record's +0x56/+0x57/+0x91
    // choose whether the 0x5C7E marker tiers the target's +0x9C stun meter
    // (unarmed with no hand cyberware, or spell 5) instead of +0x4C.
    bool target_marker_reads_stun_meter = false;
    bool target_selection_requested = false;
    ScriptTargetSelectionPolicy target_selection_policy =
        ScriptTargetSelectionPolicy::standard;
    // Candidates arrive in gameplay priority order from the field adapter.
    // Lua filters their semantic facts and chooses by stable actor ID.
    std::vector<ScriptActorTargetCandidate> ai_target_candidates;
    std::vector<ScriptActorTargetCandidate> world_target_selection_candidates;
    ScriptActorRandomBound ai_random_bound;
    std::optional<ScriptCombatantFacts> combatant;
    // Host-side source data. The Lua adapter sends only the selected profile
    // values and resolved attributes to each combatant.
    std::array<ScriptActionProfileModifierPair, 5>
        action_profile_modifiers{};
    // Resolved attribute names for the two +0xDF dynamic-profile branches.
    // The host interprets the source row's signed selector before Lua sees it.
    std::array<std::string_view, 2> action_profile_attributes{};
    bool combat_meter_service_blocked = false;
    // Semantic weapon identity copied into projectile +0x57 by the source.
    // Lua receives the item ID after the host resolves the source code.
    std::optional<ScriptWeaponActionId> projectile_action;
    // ROM 0x5A94 writes the caster's projected Y to +0x0C and 0x5ABE clears
    // it outside the field window. Unset means the host did not measure it.
    std::optional<bool> source_projection_active;
    // Closed over the caster. Unset means the host cannot run 0x14D8E
    // honestly, and Lua then refuses the cast.
    ScriptActorActionLineClear action_line_clear;
    // Source geometry fact service for a Lua-selected world destination.
    ScriptActorDestinationMeasure measure_destination;
    // Current physical condition for actors whose combat facts are not
    // otherwise available. The native adapter widens the source value here;
    // gameplay scripts consume a condition value, not a memory byte.
    std::optional<std::int32_t> physical_condition;
    // Identity of the party member currently controlled by the field player.
    // The host keeps roster placement; Lua uses this actor identity to locate
    // the controlled actor and key actor-owned state.
    std::string controlled_party_member_id;
    // True only for the controlled party actor's own heartbeat. World actors
    // also receive that party identity for targeting and must not consume its
    // actor-keyed hurt reaction.
    bool is_controlled_party_actor = false;
    // Named formation role for actor-follow policy. The host translates live
    // record identity here; Lua does not consume a source record slot for it.
    std::optional<ScriptFormationPosition> formation_position;
    // The actor has a live field-scene record and can use field movement.
    // Lua receives this fact instead of inferring record existence from a slot.
    bool live_field_actor = false;
    // This actor occupies an active gameplay record, including party members.
    // Lua uses the fact for shared actor services, not the record's index.
    bool has_live_actor_record = false;
};

enum class ScriptTarislarTombOutroStep {
    inactive,
    armed,
    progressed,
    actor_target_present,
    completion_ready,
    unresolved,
};

enum class ScriptFieldSpritePriority : std::uint8_t {
    low = 0,
    high = 1,
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
    bool animation_flip_x = false;
    ScriptFieldSpritePriority sprite_priority =
        ScriptFieldSpritePriority::low;
    std::uint8_t animation_frame = 0;
    ScriptActorFacts actor_facts;
    // A gameplay event; the field host starts the encounter activity delay.
    // ROM provenance: 0x139BE / 0x13F8A / 0xB258 call 0x1DBAA [C].
    bool combat_engagement_started = false;
    std::optional<std::uint32_t> field_event_request;
    bool think_token_claimed = false; // FFE175 was consumed in this heartbeat [C]
    // Lua returns the updated shared Thon action cooldown in behavior checks.
    std::optional<std::uint32_t> thon_action_cooldown_remaining;
    bool animation_complete = false;
    std::string animation_variant;
    bool combat_action_requested = false;
    bool transition_started = false;
    bool transition_complete = false;
    std::optional<std::uint16_t> audio_event;
    std::optional<std::uint16_t> target_heading_audio_event;
    // ROM 0xB212: a +0xA2-linked unarmed strike writes the 0x15D3C bearing
    // to +0x34 and +0x2E before selecting C0A2. Player behavior pass only.
    std::optional<std::uint16_t> strike_heading;
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
    std::string spell_effect_id;
    bool spell_effect_completed = false;
    std::optional<std::uint8_t> spell_effect_slot_released;
    std::optional<std::uint16_t> field_barrier_remaining;
    bool target_marker_visible = false;
    bool target_marker_animating = false;
    std::uint8_t target_marker_style = 0;
    std::uint8_t target_marker_radius = 0;
    bool target_selection_processed = false;
    std::optional<std::string> selected_world_actor_id;
    std::optional<ScriptActorMovementStep> movement_step;
    std::optional<ScriptActorProjectileMovementDecision>
        projectile_movement;
    std::optional<ScriptActorHitSparkSpawnRequest>
        hit_spark_spawn_request;
    bool hit_spark_retired = false;
    std::optional<ScriptCombatMeterUpdate> combat_meter_update;
    std::optional<ScriptActorSpellActionResolution>
        spell_action_resolution;
    std::optional<ScriptActorSpellEffectResolution>
        spell_effect_resolution;
    // One 0x13FFE spell result for the 21-pass refresh. Nil on other passes.
    std::optional<std::string> source_action_dispatch;
};

using ScriptActorHeartbeatObserver =
    std::function<void(std::string_view actor_id,
                       std::string_view profile_id,
                       ScriptActorHeartbeatPhase phase)>;

struct ScriptNativeOperation {
    enum class Type {
        actor_walk_direction,
        ui_say,
        ui_story_page,
        ui_credits_page,
        ui_menu,
        service_call,
    };

    Type type = Type::actor_walk_direction;
    std::string actor_id;
    MovementDirection direction = MovementDirection::north;
    std::string service_id;
    std::optional<ScriptDialogueRequest> dialogue;
    struct StoryPageRequest {
        std::string asset_alias;
        std::string text_key;
        std::uint16_t origin_x = 0U;
        std::uint16_t origin_y = 0U;
        std::uint8_t columns = 0U;
        std::uint8_t rows = 0U;
        std::array<std::uint8_t, 3> ink_rgb{};
    };
    std::optional<StoryPageRequest> story_page;
    struct CreditsPageRequest {
        std::string asset_alias;
        std::string page_id;
    };
    std::optional<CreditsPageRequest> credits_page;
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
    std::optional<DialogueOutcome> dialogue_outcome;
    std::optional<ScriptDialogueOpenDescriptor> dialogue_open;
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
        const std::filesystem::path& dialogue_text_file = {},
        const std::filesystem::path& json_content_directory = {});
    const std::string* dialogue_text(std::string_view key) const;
    const std::unordered_map<std::string, std::string>&
    dialogue_text_entries() const noexcept;
    std::optional<FieldScriptRoute> resolve_field_location_route(
        const FieldScriptRouteContext& context);
    std::optional<std::string> active_run_id();
    std::optional<std::string> active_run_offer_variant_id();
    std::optional<ScriptRunFieldEventDecision> resolve_active_run_field_event(
        std::string_view location_id);
    std::optional<ScriptRunActorDefeatDecision>
    resolve_active_run_actor_defeat(const ScriptActorDefeatFacts& facts);
    std::optional<ScriptRunJohnsonReturnDecision>
    resolve_active_run_johnson_return(std::string_view contact_id);
    std::optional<std::string> johnson_interaction_contact_id(
        std::string_view text_profile);
    std::optional<ScriptPersistentValue> save_run_registry_state();
    bool validate_run_registry_state(const ScriptPersistentValue& state);
    bool restore_run_registry_state(const ScriptPersistentValue& state);
    std::optional<ScriptLocationEntryDecision> resolve_field_location_entry(
        std::uint16_t scene_id, FieldCollisionEventCode event_code,
        const std::vector<std::vector<std::uint16_t>>& source_elevator_groups);
    std::optional<std::uint16_t> resolve_elevator_destination(
        const ScriptElevatorFloorGroup& group,
        std::uint8_t floor_index);
    std::optional<FieldMapEventPolicyDecision> resolve_field_collision_event(
        const FieldMapCollisionEventInput& input);
    std::optional<ScriptFieldSafeDecision> resolve_field_safe_action(
        std::uint16_t scene_id, bool projected_terrain_event_published,
        bool fuchi_cyberheart_safe_armed);
    // Normalized weighted-choice position in [0, 1); the host keeps source
    // random mechanics behind this content-facing boundary.
    std::optional<ScriptFieldEncounterOutcome> resolve_field_encounter(
        std::uint16_t scene_id, double weighted_selection);
    std::optional<ScriptFieldEncounterOutcome> resolve_field_actor_sequence(
        std::uint16_t scene_id, std::string_view sequence_id);
    std::optional<std::vector<ScriptWorldActivityActor>>
    world_activity_actors_for_script(std::uint16_t scene_id,
                                     std::uint16_t script_index);
    std::optional<std::vector<ScriptWorldActivityActor>>
    world_activity_actors_for_activity_id(std::uint16_t scene_id,
                                          std::string_view activity_id);
    std::optional<std::vector<ScriptWorldActivityActor>>
    world_activity_actors_for_activity_id(std::string_view activity_id);
    std::optional<std::string> activity_id_for_source_group(
        std::uint16_t scene_id, std::uint32_t source_group_offset) const;
    std::optional<ScriptFieldActorContactAction> field_actor_contact_task(
        std::uint16_t scene_id, std::string_view actor_instance_id,
        std::string_view activity_id,
        ScriptFieldActorContactHandler handler);
    std::optional<std::string> generic_field_contact_dialogue(
        std::uint16_t scene_id, std::string_view activity_id,
        std::uint8_t row_variant,
        std::optional<std::int16_t> signed_selector = std::nullopt);
    std::optional<std::vector<ScriptConditionalActorCandidate>>
    conditional_actor_candidates_for_scene(std::uint16_t scene_id);
    bool reset_conditional_actor_selection();
    // Consume Lua's typed first/second selection once before the host starts
    // the associated world-activity request.
    std::optional<ScriptConditionalActorVariant>
    consume_conditional_actor_selection();
    std::optional<ScriptConditionalActorSelection>
    resolve_conditional_actor_selection(
        std::uint16_t scene_id,
        const ScriptConditionalActorSelectionContext& context,
        const ScriptActorRandomBound& random_bound);
    std::optional<std::string>
    pending_world_activity_id_for_scene(
        std::uint16_t scene_id, bool world_effect_active,
        bool pending_world_effect);
    std::optional<ScriptFieldEncounterOutcome>
    resolve_run_target_actor_group(std::string_view actor_group_id);
    // audio.lua scene_music: the music id Lua selects on scene init from
    // scene profile word +0x40 (FFDE06). nullopt = no music / failure.
    std::optional<std::uint16_t> scene_music(std::uint16_t scene_id,
                                             std::uint16_t profile_music);
    std::optional<ScriptFieldEncounterTriggerAction>
    evaluate_field_encounter_trigger(
        const ScriptFieldEncounterTriggerInput& input);
    std::optional<ScriptFieldAlarmProfile> field_alarm_profile(
        std::uint16_t scene_id);
    std::optional<ScriptFieldAlarmResult> evaluate_field_alarm(
        const ScriptFieldAlarmInput& input);
    std::optional<std::vector<FieldMapEventRule>> map_event_rules();
    void set_story_state(const StoryFacts& facts);
    void set_story_fact_accessors(StoryFactReader read,
                                  StoryFactWriter write);
    StoryFacts story_state() const;
    void set_location_context(std::string_view type, std::string_view id,
                              std::string_view text_profile = {},
                              std::string_view speaker_id = {},
                              std::string_view event_id = {},
                              std::optional<std::uint8_t> scene_id =
                                  std::nullopt);
    void set_field_mode_context(ScriptFieldModeContext context) noexcept;
    // Lua resolves attack/reload mode from the weapon data and applies the
    // shared 0xABFC / 0xD7EA calculation to the marshalled runner facts. [C]
    std::optional<std::int16_t> player_weapon_action_delay(
        std::uint8_t primary_attribute,
        std::uint8_t secondary_attribute,
        std::uint16_t capability_flags, std::uint8_t weapon_code,
        ScriptPlayerDelayPurpose purpose);
    std::optional<ScriptPartyRunnerDownDecision> party_runner_down(
        const ScriptPartyRunnerDownContext& context);
    std::optional<ScriptPartyWipeRecoveryDecision> party_wipe_recovery(
        const ScriptPartyWipeRecoveryContext& context);
    std::optional<ScriptPartyRunnerRefreshDecision> party_runner_refresh(
        const ScriptPartyRunnerRefreshContext& context);
    std::optional<ScriptPartyWipeRunFailureDecision>
    prepare_party_wipe_run_failure(
        const ScriptPartyWipeRunFailureContext& context);
    std::optional<ScriptActorBehaviorDecision> actor_heartbeat(
        const ScriptActorBehaviorContext& context);
    std::optional<ScriptActorTargetListRebuildDecision>
    should_rebuild_actor_target_lists(
        const ScriptActorTargetListRebuildFacts& facts);
    // The host selects actors that can be affected by the current scene
    // effect. Lua checks their stable identities and live gameplay state.
    std::optional<bool> has_world_scene_effect_target(
        const std::vector<std::string>& actor_ids);
    // Lua owns outro progression and receives semantic activity and actor
    // facts. The host resolves scene/mode details and queues the continuation.
    std::optional<ScriptTarislarTombOutroStep> advance_tarislar_tomb_outro(
        bool active,
        const std::vector<std::string>& actor_ids, bool arm);
    void set_actor_heartbeat_observer(
        ScriptActorHeartbeatObserver observer);
    // Kept for the existing script-runtime caller contract. New game-runtime
    // code should use actor_heartbeat() to make the per-tick boundary clear.
    std::optional<ScriptActorBehaviorDecision> actor_behavior_decision(
        const ScriptActorBehaviorContext& context);
    std::optional<std::string> call_string(
        std::string_view module, std::string_view function);
    // Structured, JSON-compatible policy result for native adapters. Gameplay
    // modules use named fields instead of returning ROM-shaped integer slots.
    std::optional<ScriptPersistentValue> call_value(
        std::string_view module, std::string_view function,
        const std::vector<ScriptPersistentValue>& arguments);
    // Generic integer call: module.function(args...) must return an array of
    // integers (at most 64). Used for small numeric Lua policies such as the
    // world_links door placement; the Lua side owns the meaning of each slot.
    std::optional<std::vector<std::int64_t>> call_integers(
        std::string_view module, std::string_view function,
        const std::vector<std::int64_t>& arguments);
    // Read an ordered, unique ID array that has no display labels.
    std::optional<std::vector<std::string>> string_list(
        std::string_view module, std::string_view function);
    std::optional<ScriptStringMap> string_map(
        std::string_view module, std::string_view function);
    // module.function(argument) with one identifier argument.
    std::optional<ScriptStringMap> string_map(
        std::string_view module, std::string_view function,
        std::optional<std::string_view> argument);
    std::optional<ScriptSpeakerProfile> speaker_profile(
        std::string_view speaker_id);
    std::optional<ScriptDialogueTextKeys> dialogue_event_text_keys(
        std::string_view speaker_id, std::string_view text_profile,
        std::string_view event_id);
    std::optional<ScriptMenuDefinition> resolve_dialogue_menu(
        ScriptMenuDefinition menu);
    std::optional<ScriptShopDefinition> shop_definition(
        std::string_view shop_id);
    std::optional<ScriptShopMenuSnapshot> open_shop_menu(
        std::string_view shop_id);
    std::optional<ScriptShopMenuSnapshot> advance_shop_menu(
        std::uint16_t controller_events);
    std::optional<ScriptShopMenuSnapshot> reset_shop_menu();
    bool close_shop_menu();
    std::optional<ScriptItemCatalog> item_catalog();
    ScriptTaskStep start_task(
        std::string_view module, std::string_view function);
    ScriptTaskStep start_dialogue_task(
        ScriptDialogueSource source, std::string_view module,
        std::string_view function,
        const ScriptDialogueOpenRouteData& route_data = {});
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

    void set_menu_route_bindings(ScriptMenuRouteBindings bindings);
    bool open_menu_route(
        std::uint16_t root_screen,
        ScriptMenuRouteStartArguments start_arguments = {});
    void advance_menu_route(std::uint16_t events);
    void close_menu_route();
    ScriptMenuRouteSnapshot menu_route_snapshot() const noexcept;

    const std::string& failure() const noexcept;

private:
    std::optional<std::vector<ScriptWorldActivityActor>>
    world_activity_actors_for_activity_id_impl(
        std::optional<std::uint16_t> scene_id,
        std::string_view activity_id);
    std::optional<ScriptFieldEncounterOutcome>
    resolve_field_encounter_impl(std::uint16_t scene_id,
                                 double weighted_selection,
                                 std::optional<std::string_view>
                                     source_sequence_id = std::nullopt);
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace shadowrun::ghidra_conversion::scripting
