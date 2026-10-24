#pragma once

#include "shadowrun_ghidra/field/field_map_event_system_conversion.hpp"
#include "shadowrun_ghidra/scripting/dialogue_outcome.hpp"

#include <array>
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
// scripts. These callbacks are narrow host services; page, row, posture focus,
// and the active route stack remain Lua-owned.
struct ScriptMenuRouteArgs {
    std::optional<int> page;
    std::optional<int> row;
    std::optional<int> column;
};

enum class ScriptNotebookBoundary : std::uint8_t {
    event_gates_unresolved,
    tips_flags_unresolved,
    current_run_unresolved,
    contacts_flags_unresolved,
    shadowrunners_flags_unresolved,
    mr_johnsons_flags_unresolved,
};

struct ScriptMenuRouteHost {
    std::function<void(std::uint16_t)> play_sound;
    std::function<void(std::uint16_t)> stop_sound;
    std::function<void(std::uint32_t)> redraw;
    std::function<void(std::uint16_t)> blank;
    std::function<void(std::uint16_t)> set_window;
    std::function<void(std::uint16_t)> set_screen;
    std::function<void(bool)> menu_closed;
    std::function<void(std::uint16_t, const ScriptMenuRouteArgs&)>
        legacy_open;
    std::function<std::pair<bool, bool>(std::uint16_t, std::uint16_t)>
        legacy_advance;
    std::function<void(ScriptNotebookBoundary)> notebook_boundary;
};

struct ScriptMenuPartyMember {
    // Source record +0x10 is a big-endian word; the +0x10 byte used by the
    // Clips rules is its high byte.
    std::uint8_t status = 0U;
    std::uint8_t clips = 0U;
    std::uint8_t rounds = 0U;
    int weapon_slot = -1;
    std::uint8_t weapon_code = 0U;
    std::uint8_t armor = 0U;
    std::uint8_t physical = 0U;
    std::uint8_t stun = 0U;
    // Typed runner fields used by the ROM Magic menu: +0x7F, +0x9B,
    // +0x9D, +0x4D, and +0xA0 respectively.
    std::uint8_t magic_resource = 0U;
    std::uint8_t archetype = 0U;
    std::uint8_t karma = 0U;
    std::uint8_t magic_force = 0U;
    std::uint8_t posture = 0U;
    std::uint16_t magic_table_index = 0U;
    std::uint32_t state_function = 0U;
    std::array<std::uint8_t, 8> inventory{};
    std::array<std::uint8_t, 8> phase{};
    std::array<std::uint8_t, 8> attributes{};
    std::array<std::uint8_t, 12> skills{};
};

struct ScriptMenuSpellEntry {
    std::uint8_t code = 0U;
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
    std::optional<std::array<std::uint32_t, 3>> tip_flags;
    std::optional<std::uint8_t> referral_flags;
    std::optional<std::uint32_t> contact_flags;
    std::optional<std::uint16_t> shadowrunner_flags;
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

// Mutable world facts touched by Matrix node actions. Missing values mean the
// active host state has not been recovered (for example, a restored run).
struct ScriptMatrixRunState {
    std::int8_t state = 0;
    std::int8_t objective = 0;
    std::int8_t value = 0;
    std::uint8_t size = 0U;
    std::uint8_t completion_flags = 0U;
    std::uint8_t run_flags = 0U;
    // ROM FFFBF0: the active type-6 Matrix objective's target system.
    std::optional<std::uint8_t> target_system_id;
};

struct ScriptMatrixFieldState {
    std::optional<bool> alarm;
    std::optional<bool> security_off;
    std::optional<bool> maglocks_off;
};

struct ScriptMatrixFieldActorState {
    std::string actor_id;
    std::uint8_t flags_9e = 0U;
};

// Typed subset read by the ROM special-datastore handler at 0x55058. Lua
// supplies the active system ID from its Matrix session; FFFC22 and the
// wallet are optional because either source fact may be unavailable.
struct ScriptMatrixDatastoreFacts {
    std::uint8_t system_id = 0U;           // Current Matrix system, FFFBBC.
    std::optional<std::uint32_t> tips_page3_flags; // Story tips, FFFC38.
    std::optional<std::uint32_t> tips_page2_flags; // Story tips, FFFC34.
    std::uint32_t world_event_flags = 0U;   // World-event facts, FFFC1E.
    std::optional<std::uint32_t> group_item_flags; // Group items, FFFC22.
    std::optional<std::uint32_t> nuyen;
    std::optional<std::uint8_t> mr_johnsons_met_flags; // FFFC27.
};

// ROM FFFB30 stores one byte per Matrix map cell: the high nibble is node
// knowledge and the low nibble is the remembered security color. ROM 0x53290
// clears the 0x8C visible cells when the selected system changes.
struct ScriptMatrixMapMemory {
    std::uint8_t system_id = 0U;
    std::array<std::uint8_t, 0x8C> cells{};
    std::array<std::uint8_t, 0x8C> revealed{};
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
    // FFFB30 is transient Matrix RAM. It survives reopening the same system,
    // resets when its system changes, and is not part of the host save format.
    std::optional<ScriptMatrixMapMemory> matrix_map_memory;
};

struct ScriptMenuRouteGame {
    std::function<void()> reselect_leader;
    std::function<int()> leader_posture;
    std::function<void(int)> set_leader_posture;
    std::function<int()> leader_archetype;
    std::function<std::array<ScriptMenuPartyMember, 3>()> party;
    std::function<std::optional<ScriptMenuSpellTable>(int)> spell_table;
    std::function<void(int, const ScriptMenuSpellTable&)> set_spell_table;
    std::function<int()> active_slot;
    std::function<int(int)> magazine_capacity;
    std::function<void(int, int, int)> set_runner_ammo;
    std::function<std::array<bool, 3>()> slot_active_set;
    std::function<void(const std::array<ScriptMenuPartyMember, 3>&)>
        commit_party;
    std::function<int(int, int)> roll_successes;
    std::function<bool()> pocket_extra_row;
    std::function<bool()> in_combat;
    // FFDF40 field scene mode (0 = outside). ROM 0xEFC4 gates Save/Load on it.
    std::function<int()> scene_mode;
    std::function<bool(int)> save_game;
    std::function<bool(int)> load_game;
    std::function<int(int)> save_status;
    std::function<std::optional<std::uint32_t>()> nuyen;
    std::function<void(std::uint32_t)> set_nuyen;
    std::function<bool(int)> request_taxi_destination;
    std::function<void()> consume_debug_unlock;
    std::function<void(std::uint32_t)> set_contact_flags;
    std::function<void()> set_walk_fast;
    std::function<void()> install_test_deck;
    std::function<int(int)> source_random_bound;
    // Matrix 0x54A8E uses the same source RNG stream; this purpose-specific
    // binding keeps route QA able to script alert draws independently.
    std::function<int(int)> matrix_alert_random_bound;
    std::function<void(int)> dismiss_runner;
    std::function<std::optional<std::uint32_t>()> group_item_flags;
    std::function<std::optional<ScriptNotebookFacts>()> notebook_facts;
    std::function<std::optional<bool>()> cyberdeck_available;
    std::function<int()> deck_enter;
    std::function<void(int)> deck_leave;
    std::function<void(int)> set_deck_mode;
    std::function<bool()> matrix_online;
    std::function<std::optional<ScriptMatrixNodeFacts>()> matrix_node;
    std::function<int()> alert;
    std::function<bool()> world_effect_gate_negative;
    std::function<int()> map_seen;
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
    // Optional FFFB30 high-nibble seed for a progressed Matrix QA snapshot.
    std::optional<std::uint8_t> matrix_node_knowledge_rank;
    std::optional<std::uint8_t> taxi_current_region;
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
    int knowledge_rank = 0;
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
    int source_initializer = -1;
    int context = -1;
    int run_type = -1;
    int payment_amount = -1;
    std::string johnson_label;
    std::string johnson_name;
    std::string run_type_label;
    std::string run_type_name;
    std::string payment_label;
    std::string currency_symbol;
    std::vector<std::string> body_lines;
};

struct ScriptMenuRouteSnapshot {
    bool open = false;
    std::uint16_t top_screen = 0U;
    std::string top_title;
    std::string top_text;
    std::vector<std::string> top_option_labels;
    std::optional<int> page;
    std::optional<int> row;
    std::optional<int> top_column;
    std::optional<int> top_row;
    std::optional<int> top_selection;
    std::optional<int> top_program_index;
    std::optional<int> top_program_slot;
    std::optional<int> top_program_choice;
    std::optional<bool> top_enter_callback_present;
    std::optional<bool> top_enter_called;
    std::optional<int> top_enter_transition;
    std::optional<bool> top_caption_visible;
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
    std::optional<int> top_detail_index;
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
    std::vector<int> top_visible_source_ids;
    struct MagicView {
        int row = 0;
        int column = 0;
        int table_entry_index = 0;
        int spell_code = -1;
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

// Dialogue content selects a semantic presentation contract. The game owns
// page flow; the active host decides how that view is drawn and timed.
enum class ScriptDialoguePresentation : std::uint8_t {
    field_npc,
    field_mode_text,
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

struct ScriptFieldModeContext {
    std::uint8_t mode = 0;
    std::uint8_t event_code = 0;
    std::int8_t alternate_result = 0;
    // Source transition facts exposed only when the field host knows them.
    // Scene mode is FFFC0E; transition context is FFFC10.
    std::optional<std::uint16_t> alternate_profile;
    std::optional<std::uint16_t> transition_context;
    // Entity JSON chooses the named host service; Lua uses this ID to select
    // its matching field-mode narrative profile.
    std::string service_id;
    // A named modal variant lets a shared field-mode presenter resolve
    // service-specific notices without encoding content keys in C++.
    std::string prompt_variant;
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
    std::uint8_t actor_group_area = 0;
    std::uint8_t actor_group_event = 0;
    std::uint8_t activity_scene_id = 0;
    std::uint32_t actor_list_offset = 0;
    std::uint32_t actor_resource_profile_offset = 0;
};

struct ScriptActorDefeatFacts {
    std::int8_t race_or_state = 0; // live actor record +0x9B
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

enum class ScriptFieldEncounterOutcomeKind : std::uint8_t {
    dialogue_event,
    dynamic_world_effect_sequence,
};

enum class ScriptFieldEncounterPlacementPolicy : std::uint8_t {
    source_actor_positions,
    screen_edge_candidates,
    map_derived_candidates,
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
    std::uint8_t source_record_flags = 0;
    bool source_record_flags_updated = false;
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
    std::uint8_t source_record_flags = 0;
    bool source_record_flags_changed = false;
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
    std::optional<std::uint16_t> run_target_spawn_x;
    std::optional<std::uint16_t> run_target_spawn_y;
    std::string actor_name;
    std::string profile_id;
    std::uint16_t entity_kind = 0;
    ScriptActorActionKind action_kind = ScriptActorActionKind::none;
    std::uint8_t action_code = 0;
    // ROM 0x44BC copies template +0x08/+0x06/+0x07 into the live action
    // slot, remaining item uses, and action counter at +0x56/+0x4E/+0x4F.
    std::uint8_t source_action_slot = 0;
    std::uint8_t source_action_ammo = 0;
    std::uint8_t source_action_counter = 0;
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

// A source-selected field script list is installed by the world-activity
// selector and consumed by the next actor-service pass. It remains separate
// from random-encounter actors because its placement and lifecycle come from
// the scene-script list at 0x167AE / 0x43B6.
struct ScriptWorldActivityActor {
    std::uint16_t scene_id = 0;
    std::int16_t script_index = 0;
    std::uint16_t source_order = 0;
    std::uint32_t list_offset = 0;
    std::uint32_t source_record_offset = 0;
    std::uint32_t tagged_template = 0;
    std::uint32_t template_offset = 0;
    std::uint32_t position_offset = 0;
    std::string actor_name;
    std::string profile_id;
    std::uint16_t entity_kind = 0;
    std::string sprite_stem;
    ScriptActorActionKind action_kind = ScriptActorActionKind::none;
    std::uint8_t action_code = 0;
    std::uint8_t source_action_slot = 0;
    std::uint8_t source_action_ammo = 0;
    std::uint8_t source_action_counter = 0;
    std::uint8_t palette_line = 0;
    std::uint16_t world_x = 0;
    std::uint16_t world_y = 0;
    std::uint16_t heading = 0;
    std::string ordered_overrides;
    std::uint8_t source_spawn_state_flags = 0;
    std::uint8_t source_interaction_flags = 0;
    std::uint8_t physical_meter = 0;
    std::uint8_t delay_primary_attribute = 0;
    std::uint8_t delay_secondary_attribute = 0;
    std::uint16_t delay_capability_flags = 0;
    ScriptCombatantFacts combatant{};
};

// The selector result is data, not an event-specific C++ switch. Dialogue
// handler continuations remain source-owned until their respective branches
// have been traced and ported; dynamic actor lists are fully typed here.
struct ScriptFieldEncounterOutcome {
    ScriptFieldEncounterOutcomeKind kind =
        ScriptFieldEncounterOutcomeKind::dialogue_event;
    ScriptFieldEncounterPlacementPolicy placement_policy =
        ScriptFieldEncounterPlacementPolicy::screen_edge_candidates;
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

enum class ScriptActorItemSpawnResult : std::uint8_t {
    spawned,
    blocked,
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
    burst_followup_restarted,
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
    std::uint16_t heading = 0;
    std::uint16_t saved_position_x = 0;
    std::uint16_t saved_position_y = 0;
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
    // ROM 0x141E8 reads the linked record's +0x2E heading before jumping to
    // 0xB39A. `heading` above is the relative bearing used by route policy;
    // keep the linked record's facing separate for C042 pose selection.
    std::optional<std::uint16_t> source_record_heading;
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
    // ROM 0xAD86: a negative +0x56 sends the field player's held-A action
    // to 0xAE6E/0x1033E. The host offers the equipped D4EA0 row code
    // (+0x57, spell row + 1) and the B-selected +0xA2 target in
    // ai_target_candidates[1]; Lua owns the cast policy.
    bool player_spell_available = false;
    std::uint8_t player_spell_code = 0;
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
    bool walk_fast_enabled = false;
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
    // ROM world-activity script lists may create non-encounter actors in the
    // same runtime pool. Keep that provenance explicit for Lua profile rules.
    bool world_activity_script_actor = false;
    std::optional<std::uint8_t> source_record_slot;
    std::uint8_t source_record_flags = 0;
    std::uint8_t source_interaction_flags = 0;
    std::uint8_t source_behavior_flags = 0;
    std::uint8_t source_spawn_state_flags = 0;
    std::uint8_t source_action_slot = 0;
    std::uint8_t source_action_ammo = 0;
    std::uint8_t source_action_counter = 0;
    // Live +0x16 selector. C042 is the directional item-action pose family;
    // C052/C062 are the idle and movement families.
    std::uint16_t source_animation_selector = 0;
    std::optional<ScriptActorItemSpawnResult> source_item_spawn_result;
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
    // Directional C042 item-action frames extracted from the actor state list.
    std::uint8_t item_action_frame_count = 0;
    std::uint16_t item_action_increment = 0;
    std::uint16_t item_action_threshold = 0;
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
    // Source +0x4C physical meter. Lua retains this separate field for the
    // Scene-55 ending scan without exposing combatant facts to static actors.
    std::optional<std::uint8_t> source_physical_meter;
};

enum class ScriptTarislarTombOutroStep {
    inactive,
    armed,
    progressed,
    actor_target_present,
    completion_ready,
    unresolved,
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
    std::uint8_t source_behavior_flags = 0;
    std::uint16_t source_animation_selector = 0;
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

using ScriptActorHeartbeatObserver =
    std::function<void(std::string_view actor_id,
                       std::string_view profile_id,
                       ScriptActorHeartbeatPhase phase)>;

struct ScriptNativeOperation {
    enum class Type {
        actor_walk_heading,
        ui_say,
        ui_story_page,
        ui_credits_page,
        ui_menu,
        service_call,
    };

    Type type = Type::actor_walk_heading;
    std::string actor_id;
    std::uint8_t heading = 0;
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
    std::optional<ScriptRunFieldEventDecision> resolve_active_run_field_event(
        std::uint8_t map_id, std::uint8_t event_id);
    std::optional<ScriptRunActorDefeatDecision>
    resolve_active_run_actor_defeat(const ScriptActorDefeatFacts& facts);
    std::optional<ScriptRunJohnsonReturnDecision>
    resolve_active_run_johnson_return(std::uint8_t context);
    std::optional<std::uint8_t> johnson_interaction_context(
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
    std::optional<ScriptFieldEncounterOutcome> resolve_field_encounter(
        std::uint16_t scene_id, std::uint8_t first_roll,
        std::uint8_t second_roll);
    std::optional<ScriptFieldEncounterOutcome> resolve_field_actor_sequence(
        std::uint16_t scene_id, std::uint32_t sequence_list_offset);
    std::optional<std::vector<ScriptWorldActivityActor>>
    world_activity_actors_for_script(std::uint16_t scene_id,
                                     std::uint16_t script_index);
    std::optional<std::vector<ScriptWorldActivityActor>>
    world_activity_actors_for_source_list(std::uint16_t scene_id,
                                          std::uint32_t list_offset);
    std::optional<std::vector<ScriptWorldActivityActor>>
    world_activity_actors_for_list_id(std::uint16_t scene_id,
                                     std::string_view list_id);
    std::optional<std::uint32_t>
    pending_world_activity_actor_list_for_scene(
        std::uint16_t scene_id, bool world_effect_active,
        bool pending_world_effect);
    std::optional<ScriptFieldEncounterOutcome>
    resolve_run_target_actor_group(std::uint8_t area_id,
                                   std::uint8_t event_id);
    // audio.lua scene_music: the music id Lua selects on scene init from
    // scene profile word +0x40 (FFDE06). nullopt = no music / failure.
    std::optional<std::uint16_t> scene_music(std::uint16_t scene_id,
                                             std::uint16_t profile_music);
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
                              std::string_view event_id = {},
                              std::optional<std::uint8_t> scene_id =
                                  std::nullopt);
    void set_field_mode_context(ScriptFieldModeContext context) noexcept;
    std::optional<ScriptActorBehaviorDecision> actor_heartbeat(
        const ScriptActorBehaviorContext& context);
    // ROM 0x5D29C checks the six live records at FF0500..FF0A00 on effect
    // frame 3. These addresses map to typed host slots 4..9 because FF0400
    // is slot 3. The array is in record order; an empty ID denotes an unused
    // host slot. Eligibility is read from Lua-owned actor state.
    std::optional<bool> has_world_scene_effect_target(
        const std::array<std::string, 6>& actor_ids);
    // Lua owns the Scene-55 outro progression from the 0x1DD94 mode change
    // through the 0x5D29C actor scan. The host supplies typed actor identities
    // and receives the point where 0x1E2E0 should queue its 60-frame callback.
    std::optional<ScriptTarislarTombOutroStep> advance_tarislar_tomb_outro(
        std::uint16_t scene_id, std::uint8_t scene_state,
        const std::array<std::string, 6>& actor_ids, bool arm);
    void set_actor_heartbeat_observer(
        ScriptActorHeartbeatObserver observer);
    // Kept for the existing script-runtime caller contract. New game-runtime
    // code should use actor_heartbeat() to make the per-tick boundary clear.
    std::optional<ScriptActorBehaviorDecision> actor_behavior_decision(
        const ScriptActorBehaviorContext& context);
    std::optional<std::string> call_string(
        std::string_view module, std::string_view function);
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
    world_activity_actors_for_selector(std::uint16_t scene_id,
                                       std::string_view selector_field,
                                       std::uint32_t selector_value,
                                       std::uint32_t selector_maximum,
                                       std::string_view lua_function,
                                       std::string_view selector_text = {});
    std::optional<ScriptFieldEncounterOutcome>
    resolve_field_encounter_impl(std::uint16_t scene_id,
                                 std::uint8_t first_roll,
                                 std::uint8_t second_roll,
                                 bool run_target_actor_group,
                                 std::optional<std::uint32_t>
                                     source_sequence_list_offset =
                                         std::nullopt);
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace shadowrun::ghidra_conversion::scripting
