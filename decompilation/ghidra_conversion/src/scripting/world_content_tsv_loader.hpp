#pragma once

#include <filesystem>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

extern "C" {
#include "lua.h"
}

namespace shadowrun::ghidra_conversion::scripting::detail {

bool load_world_content_tsv(lua_State* state, int modules_reference,
                            const std::filesystem::path& world_directory,
                            std::string& failure,
                            const std::filesystem::path& field_asset_directory = {},
                            std::vector<ScriptConditionalActorCandidate>*
                                conditional_actor_catalog = nullptr,
                            std::vector<ScriptActivitySelectorBinding>*
                                activity_selector_bindings = nullptr,
                            std::vector<ScriptWorldActivityGroupBinding>*
                                activity_group_bindings = nullptr,
                            std::vector<std::pair<std::string, std::uint16_t>>*
                                action_profile_bindings = nullptr);

} // namespace shadowrun::ghidra_conversion::scripting::detail
