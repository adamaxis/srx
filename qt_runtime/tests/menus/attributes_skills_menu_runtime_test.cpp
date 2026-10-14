#include "attributes_skills_menu_runtime.hpp"

#include <iostream>
#include <type_traits>

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
    static_assert(!std::is_same_v<CommandMenuKarmaViewRuntime,
                                  HotelKarmaUpgradeRuntime>);
    static_assert(!CommandMenuKarmaViewRuntime::kAllowsKarmaUpgrades);
    static_assert(HotelKarmaUpgradeRuntime::kAllowsKarmaUpgrades);
    CommandMenuKarmaViewRuntime command_view;
    HotelKarmaUpgradeRuntime hotel_upgrade;
    RunnerRecordFields record{};
    record.species_index = 0U;
    record.archetype_index = 0U;
    record.attributes[0] = 1U;
    record.attributes[1] = 2U;
    record.skills[0] = 1U;
    record.auxiliary[2] = 10U;

    command_view.open(record);
    auto grid = command_view.grid(record);
    if (!expect(command_view.is_open() &&
                    command_view.state().cursor.column == 0U &&
                    command_view.state().cursor.row == 0U &&
                    grid[0][0].label_index == 0U &&
                    grid[0][0].value == 1U &&
                    !command_view.allows_karma_upgrades() &&
                    !command_view.state().selection_data_available &&
                    command_view.preview(record).status ==
                        AttributeSkillUpgradeStatus::available,
                "Command Attributes/Skills did not open read-only on Body") ||
        !expect(command_view.advance(0x0200U, record) ==
                    AttributesSkillsRuntimeEvent::selection_moved &&
                    command_view.state().cursor.row == 1U,
                "Attributes/Skills Down did not select the next row") ||
        !expect(command_view.advance(0x0800U, record) ==
                    AttributesSkillsRuntimeEvent::selection_moved &&
                    command_view.state().cursor.column == 1U &&
                    command_view.state().cursor.row == 1U,
                "Attributes/Skills Right did not select the next column")) {
        return 1;
    }

    // ROM 0xD01C sets FFF116=-1. A/B leaves the command-menu view and cannot
    // reach the hotel-only Karma spending flow.
    const auto view_only_body = record.attributes[0];
    const auto view_only_karma = record.auxiliary[2];
    if (!expect(command_view.advance(0x4000U, record) ==
                    AttributesSkillsRuntimeEvent::returned_to_parent &&
                    !command_view.is_open() &&
                    record.attributes[0] == view_only_body &&
                    record.auxiliary[2] == view_only_karma,
                "Command Attributes/Skills A spent Karma instead of returning")) {
        return 2;
    }

    // The Hotel option enters at 0xD024 and retains the upgrade-enabled
    // selection data. Its prompt starts on Yes; Down reaches No before B.
    hotel_upgrade.open(record);
    const auto starting_karma = record.auxiliary[2];
    if (!expect(hotel_upgrade.allows_karma_upgrades() &&
                    hotel_upgrade.state().selection_data_available &&
                    hotel_upgrade.advance(0x4000U, record) ==
                    AttributesSkillsRuntimeEvent::upgrade_prompt_opened &&
                    hotel_upgrade.upgrade_prompt_open() &&
                    hotel_upgrade.upgrade_prompt_selection() == 0U,
                "Hotel Karma screen did not open the default-Yes prompt") ||
        !expect(hotel_upgrade.advance(0x0200U, record) ==
                    AttributesSkillsRuntimeEvent::upgrade_prompt_moved &&
                    hotel_upgrade.upgrade_prompt_selection() == 1U,
                "Hotel Karma prompt Down did not select No") ||
        !expect(hotel_upgrade.advance(0x1000U, record) ==
                    AttributesSkillsRuntimeEvent::upgrade_declined &&
                    record.attributes[0] == 1U &&
                    record.auxiliary[2] == starting_karma,
                "Hotel Karma No changed the active record")) {
        return 3;
    }

    if (!expect(hotel_upgrade.advance(0x4000U, record) ==
                    AttributesSkillsRuntimeEvent::upgrade_prompt_opened,
                "Hotel Karma screen could not reopen the upgrade prompt") ||
        !expect(hotel_upgrade.advance(0x1000U, record) ==
                    AttributesSkillsRuntimeEvent::upgrade_applied &&
                    record.attributes[0] == 2U &&
                    record.auxiliary[2] == 8U,
                "Hotel Karma Yes did not apply source cost/value rules")) {
        return 4;
    }

    record.auxiliary[2] = 0U;
    hotel_upgrade.open(record);
    if (!expect(hotel_upgrade.advance(0x4000U, record) ==
                    AttributesSkillsRuntimeEvent::insufficient_karma &&
                    hotel_upgrade.message() ==
                        AttributesSkillsMenuMessage::insufficient_karma &&
                    record.attributes[0] == 2U &&
                    record.auxiliary[2] == 0U,
                "Insufficient Karma did not preserve the active record") ||
        !expect(hotel_upgrade.advance(0x8000U, record) ==
                    AttributesSkillsRuntimeEvent::returned_to_parent &&
                    !hotel_upgrade.is_open(),
                "Hotel Karma Start did not return to Hotel Options")) {
        return 5;
    }

    record.attributes[0] = 6U;
    record.auxiliary[2] = 20U;
    hotel_upgrade.open(record);
    if (!expect(hotel_upgrade.message() ==
                    AttributesSkillsMenuMessage::maximum_level &&
                    hotel_upgrade.advance(0x4000U, record) ==
                        AttributesSkillsRuntimeEvent::upgrade_blocked,
                "Maximum attribute level did not take its source gate")) {
        return 6;
    }

    record.archetype_index = 2U;
    hotel_upgrade.open(record);
    for (int row = 0; row < 4; ++row) {
        (void)hotel_upgrade.advance(0x0200U, record);
    }
    if (!expect(hotel_upgrade.state().cursor.row == 4U &&
                    hotel_upgrade.message() ==
                        AttributesSkillsMenuMessage::none &&
                    hotel_upgrade.advance(0x4000U, record) ==
                        AttributesSkillsRuntimeEvent::upgrade_blocked &&
                    hotel_upgrade.message() ==
                        AttributesSkillsMenuMessage::mage_only_skill,
                "Mage-only rejection appeared before action or missed A")) {
        return 7;
    }

    return 0;
}
