#pragma once

#include "shadowrun_ghidra/field/field_map_event_system_conversion.hpp"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace shadowrun::ghidra_conversion::scripting {

using StoryFacts = std::unordered_map<std::string, bool>;

struct ScriptMenuChoice {
    std::string id;
    std::string label_key;
};

struct ScriptMenuDefinition {
    std::string id;
    std::string title_key;
    std::vector<ScriptMenuChoice> choices;
};

// Lua modules use the global game facade. Story/location tables are copied in
// before a script call and returned for host validation afterward; Lua never
// receives pointers to mutable engine, save, or Qt objects.
class GameScriptRuntime final {
public:
    GameScriptRuntime();
    ~GameScriptRuntime();

    GameScriptRuntime(const GameScriptRuntime&) = delete;
    GameScriptRuntime& operator=(const GameScriptRuntime&) = delete;
    GameScriptRuntime(GameScriptRuntime&&) noexcept;
    GameScriptRuntime& operator=(GameScriptRuntime&&) noexcept;

    bool load_package(const std::filesystem::path& script_directory);
    std::optional<std::vector<FieldMapEventRule>> map_event_rules();
    void set_story_state(const StoryFacts& facts);
    StoryFacts story_state() const;
    void set_location_context(std::string_view type, std::string_view id);
    std::optional<std::string> call_string(
        std::string_view module, std::string_view function);
    std::optional<ScriptMenuDefinition> menu(
        std::string_view module, std::string_view function,
        std::string_view text_profile);

    const std::string& failure() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace shadowrun::ghidra_conversion::scripting
