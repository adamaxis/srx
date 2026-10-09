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

struct ScriptMenuDefinition {
    std::string id;
    std::string text_profile;
    std::string speaker_id;
    std::string title_key;
    std::vector<ScriptMenuChoice> choices;
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

struct ScriptItemTextKeys {
    std::uint8_t code = 0;
    std::string name_key;
    std::string category_key;
};

struct ScriptItemCatalog {
    std::vector<ScriptItemTextKeys> items;
    std::vector<std::string> detail_text_keys;
    std::array<std::string, 3> detail_label_keys{};
};

struct ScriptDialogueRequest {
    std::string speaker_id;
    std::string text_profile;
    std::string event_id;
};

struct ScriptFieldModeContext {
    std::uint8_t mode = 0;
    std::uint8_t event_code = 0;
    std::int8_t alternate_result = 0;
};

enum class ScriptActorActionKind : std::uint8_t {
    none,
    item,
    spell,
};

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
    bool transition_started = false;
    bool transition_complete = false;
    std::optional<std::uint16_t> audio_event;
    std::optional<std::uint8_t> weapon_item_id;
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
    std::optional<std::vector<FieldMapEventRule>> map_event_rules();
    void set_story_state(const StoryFacts& facts);
    void set_story_fact_accessors(StoryFactReader read,
                                  StoryFactWriter write);
    StoryFacts story_state() const;
    void set_location_context(std::string_view type, std::string_view id,
                              std::string_view text_profile = {},
                              std::string_view speaker_id = {});
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
