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
    new_game.joshua_selection->typed_live_joshua_record.ammo_clips = 3U;

    FieldPartyRosterRuntime roster;
    if (!expect(roster.initialize(new_game) ==
                    FieldPartyRosterRuntime::Source::new_game_joshua,
                "new-game roster source was not identified") ||
        !expect(roster.active_slot() == 0U && roster.slot_active(0U) &&
                    !roster.slot_active(1U) && !roster.slot_active(2U),
                "new-game roster activated an invented recruited slot") ||
        !expect((roster.active_record().flags & 0x01U) != 0U &&
                    roster.active_record().ammo_clips == 3U,
                "new-game roster did not publish Joshua as active")) {
        return 1;
    }

    integration::StartupIntegrationState loaded{};
    loaded.restored_field_state.emplace();
    loaded.restored_field_state->roster.primary_marker = 1U;
    loaded.restored_field_state->roster.secondary_marker = 0xFFU;
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
                    roster.records()[1].ammo_clips == 7U,
                "save roster records were not projected into typed fields")) {
        return 2;
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
