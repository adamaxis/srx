#include "field_combat_meter_target.hpp"

#include <iostream>
#include <optional>

using namespace shadowrun::ghidra_conversion::qt_runtime;

namespace {

bool expect(const bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
        return false;
    }
    return true;
}

} // namespace

int main() {
    shadowrun::ghidra_conversion::scripting::ScriptCombatMeterUpdate update{};
    update.target_id = "field.actor.scene54.yakuza.4";
    update.source_record_slot = 7U;
    update.record_type = 8U;

    // The slot and kind are deliberately different: 0x4618 addresses a live
    // slot, while 0x44BC stores the entity kind at +0x0E.
    if (!expect(field_combat_meter_update_matches_world_actor(
                    update, "field.actor.scene54.yakuza.4", true, 7U, 8U),
                "world combat result rejected matching slot 7 / kind 8") ||
        !expect(!field_combat_meter_update_matches_world_actor(
                    update, "field.actor.scene54.yakuza.4", true, 8U, 8U),
                "world combat result accepted the wrong record slot") ||
        !expect(!field_combat_meter_update_matches_world_actor(
                    update, "field.actor.scene54.yakuza.4", true, 7U, 9U),
                "world combat result accepted the wrong entity kind") ||
        !expect(!field_combat_meter_update_matches_world_actor(
                    update, "field.actor.scene54.yakuza.4", false, 7U, 8U),
                "world combat result accepted an inactive actor") ||
        !expect(!field_combat_meter_update_matches_world_actor(
                    update, "field.actor.scene54.yakuza.5", true, 7U, 8U),
                "world combat result accepted a different actor identity") ||
        !expect(!field_combat_meter_update_matches_world_actor(
                    update, "field.actor.scene54.yakuza.4", true,
                    std::nullopt, 8U),
                "world combat result accepted an actor without a live slot")) {
        return 1;
    }

    return 0;
}
