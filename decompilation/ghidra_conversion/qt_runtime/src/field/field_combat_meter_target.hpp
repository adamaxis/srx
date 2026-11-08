#pragma once

#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

#include <cstdint>
#include <optional>
#include <string_view>

namespace shadowrun::ghidra_conversion::qt_runtime {

// ROM 0x4618 walks actors by their live record slot. The initializer at
// 0x44BC stores entity kind separately at +0x0E, so both facts must be checked
// against their own typed identity when committing an actor-to-actor result.
inline bool field_combat_meter_update_matches_world_actor(
    const scripting::ScriptCombatMeterUpdate& update,
    const std::string_view actor_instance_id,
    const bool actor_active,
    const std::optional<std::uint8_t> actor_record_slot,
    const std::uint16_t actor_record_type) noexcept {
    return actor_active && update.record_type >= 3U &&
           std::string_view(update.target_id) == actor_instance_id &&
           actor_record_slot.has_value() &&
           *actor_record_slot == update.source_record_slot &&
           actor_record_type == update.record_type;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
