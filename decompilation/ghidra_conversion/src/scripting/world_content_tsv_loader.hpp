#pragma once

#include <filesystem>
#include <string>

extern "C" {
#include "lua.h"
}

namespace shadowrun::ghidra_conversion::scripting::detail {

bool load_world_content_tsv(lua_State* state, int modules_reference,
                            const std::filesystem::path& world_directory,
                            std::string& failure);

} // namespace shadowrun::ghidra_conversion::scripting::detail
