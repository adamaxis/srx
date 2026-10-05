#include "attributes_skills_menu_runtime.hpp"

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
    AttributesSkillsMenuRuntime menu;
    RunnerRecordFields record{};
    record.species_index = 0U;
    record.archetype_index = 0U;
    record.attributes[0] = 1U;
    record.attributes[1] = 2U;
    record.skills[0] = 1U;
    record.auxiliary[2] = 10U;

    menu.open(record, AttributesSkillsMenuMode::command_view_only);
    auto grid = menu.grid(record);
    if (!expect(menu.is_open() && menu.state().cursor.column == 0U &&
                    menu.state().cursor.row == 0U &&
                    grid[0][0].label_index == 0U &&
                    grid[0][0].value == 1U &&
                    !menu.allows_karma_upgrades() &&
                    !menu.state().selection_data_available &&
                    menu.preview(record).status ==
                        AttributeSkillUpgradeStatus::available,
                "Command Attributes/Skills did not open read-only on Body") ||
        !expect(menu.advance(0x0200U, record) ==
                    AttributesSkillsRuntimeEvent::selection_moved &&
                    menu.state().cursor.row == 1U,
                "Attributes/Skills Down did not select the next row") ||
        !expect(menu.advance(0x0800U, record) ==
                    AttributesSkillsRuntimeEvent::selection_moved &&
                    menu.state().cursor.column == 1U &&
                    menu.state().cursor.row == 1U,
                "Attributes/Skills Right did not select the next column")) {
        return 1;
    }

    // ROM 0xD01C sets FFF116=-1. A/B leaves the command-menu view and cannot
    // reach the hotel-only Karma spending flow.
    const auto view_only_body = record.attributes[0];
    const auto view_only_karma = record.auxiliary[2];
    if (!expect(menu.advance(0x4000U, record) ==
                    AttributesSkillsRuntimeEvent::returned_to_parent &&
                    !menu.is_open() && record.attributes[0] == view_only_body &&
                    record.auxiliary[2] == view_only_karma,
                "Command Attributes/Skills A spent Karma instead of returning")) {
        return 2;
    }

    // The Hotel option enters at 0xD024 and retains the upgrade-enabled
    // selection data. Its prompt starts on Yes; Down reaches No before B.
    menu.open(record, AttributesSkillsMenuMode::hotel_karma_upgrade);
    const auto starting_karma = record.auxiliary[2];
    if (!expect(menu.allows_karma_upgrades() &&
                    menu.state().selection_data_available &&
                    menu.advance(0x4000U, record) ==
                    AttributesSkillsRuntimeEvent::upgrade_prompt_opened &&
                    menu.upgrade_prompt_open() &&
                    menu.upgrade_prompt_selection() == 0U,
                "Hotel Karma screen did not open the default-Yes prompt") ||
        !expect(menu.advance(0x0200U, record) ==
                    AttributesSkillsRuntimeEvent::upgrade_prompt_moved &&
                    menu.upgrade_prompt_selection() == 1U,
                "Hotel Karma prompt Down did not select No") ||
        !expect(menu.advance(0x1000U, record) ==
                    AttributesSkillsRuntimeEvent::upgrade_declined &&
                    record.attributes[0] == 1U &&
                    record.auxiliary[2] == starting_karma,
                "Hotel Karma No changed the active record")) {
        return 3;
    }

    if (!expect(menu.advance(0x4000U, record) ==
                    AttributesSkillsRuntimeEvent::upgrade_prompt_opened,
                "Hotel Karma screen could not reopen the upgrade prompt") ||
        !expect(menu.advance(0x1000U, record) ==
                    AttributesSkillsRuntimeEvent::upgrade_applied &&
                    record.attributes[0] == 2U &&
                    record.auxiliary[2] == 8U,
                "Hotel Karma Yes did not apply source cost/value rules")) {
        return 4;
    }

    record.auxiliary[2] = 0U;
    menu.open(record, AttributesSkillsMenuMode::hotel_karma_upgrade);
    if (!expect(menu.advance(0x4000U, record) ==
                    AttributesSkillsRuntimeEvent::insufficient_karma &&
                    menu.message() ==
                        AttributesSkillsMenuMessage::insufficient_karma &&
                    record.attributes[0] == 2U &&
                    record.auxiliary[2] == 0U,
                "Insufficient Karma did not preserve the active record") ||
        !expect(menu.advance(0x8000U, record) ==
                    AttributesSkillsRuntimeEvent::returned_to_parent &&
                    !menu.is_open(),
                "Hotel Karma Start did not return to Hotel Options")) {
        return 5;
    }

    record.attributes[0] = 6U;
    record.auxiliary[2] = 20U;
    menu.open(record, AttributesSkillsMenuMode::hotel_karma_upgrade);
    if (!expect(menu.message() ==
                    AttributesSkillsMenuMessage::maximum_level &&
                    menu.advance(0x4000U, record) ==
                        AttributesSkillsRuntimeEvent::upgrade_blocked,
                "Maximum attribute level did not take its source gate")) {
        return 6;
    }

    record.archetype_index = 2U;
    menu.open(record, AttributesSkillsMenuMode::hotel_karma_upgrade);
    for (int row = 0; row < 4; ++row) {
        (void)menu.advance(0x0200U, record);
    }
    if (!expect(menu.state().cursor.row == 4U &&
                    menu.message() == AttributesSkillsMenuMessage::none &&
                    menu.advance(0x4000U, record) ==
                        AttributesSkillsRuntimeEvent::upgrade_blocked &&
                    menu.message() ==
                        AttributesSkillsMenuMessage::mage_only_skill,
                "Mage-only rejection appeared before action or missed A")) {
        return 7;
    }

    return 0;
}
