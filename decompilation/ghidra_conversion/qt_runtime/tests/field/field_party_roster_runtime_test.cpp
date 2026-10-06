#include "field_party_roster_runtime.hpp"

#include <iostream>

using namespace shadowrun::ghidra_conversion;
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
    integration::StartupIntegrationState new_game{};
    new_game.joshua_selection.emplace();
    new_game.joshua_archetype.selected =
        static_cast<JoshuaArchetypeChoice>(1U);
    const std::string joshua_name = "Joshua";
    for (std::size_t index = 0; index < joshua_name.size(); ++index) {
        new_game.joshua_selection->live_joshua_record[0x58U + index] =
            static_cast<std::uint8_t>(joshua_name[index]);
    }
    new_game.joshua_selection->typed_live_joshua_record.ammo_clips = 3U;

    FieldPartyRosterRuntime roster;
    if (!expect(roster.initialize(new_game) ==
                    FieldPartyRosterRuntime::Source::new_game_joshua,
                "new-game roster source was not identified") ||
        !expect(roster.active_slot() == 0U && roster.slot_active(0U) &&
                    !roster.slot_active(1U) && !roster.slot_active(2U),
                "new-game roster activated an invented recruited slot") ||
        !expect((roster.active_record().flags & 0x0101U) == 0x0101U &&
                    roster.active_record().ammo_clips == 3U &&
                    roster.name(0U) == "Joshua" &&
                    roster.portrait_asset_family(0U) == 1U,
                "new-game roster did not publish Joshua as active")) {
        return 1;
    }

    integration::StartupIntegrationState loaded{};
    loaded.restored_field_state.emplace();
    loaded.startup_records.emplace();
    loaded.startup_records->joshua_archetypes[0].record[0x9BU] = 2U;
    loaded.startup_records->joshua_archetypes[1].record[0x9BU] = 3U;
    loaded.startup_records->joshua_archetypes[2].record[0x9BU] = 1U;
    loaded.restored_field_state->roster.primary_marker = 1U;
    loaded.restored_field_state->roster.secondary_marker = 0xFFU;
    const std::string ricky_name = "Ricky";
    for (std::size_t index = 0; index < ricky_name.size(); ++index) {
        loaded.restored_field_state->roster.primary_record.bytes[0x58U + index] =
            static_cast<std::uint8_t>(ricky_name[index]);
    }
    const std::string winston_name = "Winston Marrs";
    for (std::size_t index = 0; index < winston_name.size(); ++index) {
        loaded.restored_field_state->roster.default_record.bytes[0x58U + index] =
            static_cast<std::uint8_t>(winston_name[index]);
    }
    loaded.restored_field_state->roster.default_record.bytes[0x9BU] = 3U;
    loaded.restored_field_state->roster.default_record.bytes[0x4FU] = 2U;
    loaded.restored_field_state->roster.primary_record.bytes[0x4FU] = 7U;
    loaded.restored_field_state->active_runner_slot = 1U;
    if (!expect(roster.initialize(loaded) ==
                    FieldPartyRosterRuntime::Source::restored_save,
                "save roster source was not identified") ||
        !expect(roster.slot_active(0U) && roster.slot_active(1U) &&
                    !roster.slot_active(2U) && roster.active_slot() == 1U,
                "save roster markers did not control active slots") ||
        !expect(roster.records()[0].ammo_clips == 2U &&
                    roster.records()[1].ammo_clips == 7U &&
                    roster.name(0U) == "Winston Marrs" &&
                    roster.name(1U) == "Ricky" &&
                    roster.portrait_asset_family(0U) == 1U &&
                    roster.portrait_asset_family(1U) == 3U,
                "save roster records were not projected into typed fields")) {
        return 2;
    }

    if (!expect(roster.deactivate_party_member(1U) &&
                    !roster.slot_active(1U) && roster.slot_active(0U) &&
                    !roster.deactivate_party_member(1U) &&
                    !roster.deactivate_party_member(0U) &&
                    (roster.records()[1].flags & 0x0100U) != 0U &&
                    roster.name(1U) == "Ricky",
                "dismissal erased the live record or changed the source flag")) {
        return 4;
    }

    integration::StartupIntegrationState unavailable{};
    if (!expect(roster.initialize(unavailable) ==
                    FieldPartyRosterRuntime::Source::unavailable &&
                    !roster.available(),
                "unavailable roster did not remain explicit")) {
        return 3;
    }
    return 0;
}
