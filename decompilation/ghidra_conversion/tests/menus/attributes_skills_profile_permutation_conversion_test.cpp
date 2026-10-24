#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"
#include "shadowrun_ghidra/menus/attributes_skills_menu_conversion.hpp"
#include "shadowrun_ghidra/session/runner_record_access_conversion.hpp"
#include "shadowrun_ghidra/session/startup_record_assets_conversion.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

namespace {

using namespace shadowrun::ghidra_conversion;

constexpr std::size_t kRecordSize = 0x100U;
constexpr std::size_t kJoshuaOffset = 0x7291AU;
constexpr std::size_t kRecruitedOffset = 0x72C1AU;
constexpr std::size_t kRecruitedCount = 10U;
constexpr std::size_t kKindOffset = 0x0EU;
constexpr std::size_t kSpeciesMaximumsOffset = 0xC22A6U;
constexpr std::array<std::array<std::uint8_t, 2U>, 13U> kProfileIdentity{{
    {{0U, 2U}}, {{0U, 3U}}, {{0U, 1U}},
    {{2U, 1U}}, {{4U, 2U}}, {{0U, 0U}}, {{2U, 3U}}, {{3U, 1U}},
    {{1U, 3U}}, {{0U, 2U}}, {{1U, 0U}}, {{1U, 3U}}, {{0U, 2U}},
}};

struct ExpectedUpgrade {
    AttributeSkillUpgradeStatus status =
        AttributeSkillUpgradeStatus::invalid_selection;
    std::uint8_t next_value = 0U;
    std::uint16_t karma_cost = 0U;
    bool suppresses_error_message = false;
};

bool fail(const std::string& message) {
    std::cerr << "Attributes/Skills profile permutation: " << message << '\n';
    return false;
}

bool read_file(const std::filesystem::path& path,
               std::vector<std::uint8_t>& bytes) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return false;
    }
    bytes.assign(std::istreambuf_iterator<char>(input),
                 std::istreambuf_iterator<char>());
    return !input.bad();
}

std::uint16_t read_be16(const std::vector<std::uint8_t>& bytes,
                        const std::size_t offset) {
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(bytes[offset]) << 8U) |
        bytes[offset + 1U]);
}

ExpectedUpgrade expected_upgrade(const RunnerRecordFields& record,
                                 const std::vector<std::uint8_t>& rom,
                                 const std::size_t column,
                                 const std::size_t row) {
    ExpectedUpgrade expected{};
    if (row < 4U) {
        const auto attribute_index = row + column * 4U;
        expected.suppresses_error_message = attribute_index >= 6U;
        if (attribute_index >= 6U) {
            expected.status =
                AttributeSkillUpgradeStatus::attribute_not_upgradeable;
            return expected;
        }
        const auto maximum = rom[kSpeciesMaximumsOffset +
                                 record.species_index * 6U + attribute_index];
        const auto current = record.attributes[attribute_index];
        const auto next = static_cast<unsigned>(current) + 1U;
        if (next > maximum) {
            expected.status = AttributeSkillUpgradeStatus::at_limit;
            return expected;
        }
        expected.status = AttributeSkillUpgradeStatus::available;
        expected.next_value = static_cast<std::uint8_t>(next);
        expected.karma_cost = static_cast<std::uint16_t>(next);
        return expected;
    }

    const auto skill_index = (row - 4U) + column * 6U;
    expected.suppresses_error_message =
        skill_index == 0U && record.archetype_index >= 2U;
    if (expected.suppresses_error_message) {
        expected.status = AttributeSkillUpgradeStatus::class_restricted;
        return expected;
    }

    auto next_base = static_cast<unsigned>(record.skills[skill_index]);
    const bool reduced_cost = column == 0U && skill_index >= 2U &&
                              skill_index <= 4U;
    if (reduced_cost && record.skills[skill_index] == 0U) {
        next_base = record.skills[1U];
    }
    const auto next = next_base + 1U;
    if (next >= 0x0DU) {
        expected.status = AttributeSkillUpgradeStatus::at_limit;
        return expected;
    }
    expected.status = AttributeSkillUpgradeStatus::available;
    expected.next_value = static_cast<std::uint8_t>(next);
    expected.karma_cost = static_cast<std::uint16_t>(
        reduced_cost ? (next * 15U + 9U) / 10U : next * 2U);
    return expected;
}

bool check_rendered_numeric_cell(const LiveRunnerRecord& source,
                                 const RunnerRecordFields& record,
                                 const std::vector<std::uint8_t>& rom,
                                 const std::size_t profile_index,
                                 const std::size_t column,
                                 const std::size_t row) {
    std::vector<int> order;
    std::uint8_t shown_cost = 0xFFU;
    std::uint8_t shown_next = 0xFFU;
    std::uint8_t shown_karma = 0xFFU;
    AttributeSkillCell shown_label{};
    AttributeSkillRenderCallbacks callbacks{};
    callbacks.clear_upgrade_error_marker = [&] { order.push_back(0); };
    callbacks.write_attributes_skills_grid = [&] { order.push_back(1); };
    callbacks.render_attributes_skills_heading = [&] { order.push_back(2); };
    callbacks.render_invalid_upgrade_message = [&] { order.push_back(3); };
    callbacks.render_upgrade_cost = [&](const std::uint8_t value) {
        order.push_back(4);
        shown_cost = value;
    };
    callbacks.render_next_value = [&](const std::uint8_t value) {
        order.push_back(5);
        shown_next = value;
    };
    callbacks.render_selected_label = [&](const AttributeSkillCell& cell) {
        order.push_back(6);
        shown_label = cell;
    };
    callbacks.render_selection_overlay_grid = [&] { order.push_back(7); };
    callbacks.render_karma_total = [&](const std::uint8_t value) {
        order.push_back(8);
        shown_karma = value;
    };

    const auto cursor = AttributeSkillCursor{
        static_cast<std::uint8_t>(column), static_cast<std::uint8_t>(row)};
    const auto result = render_attributes_and_skills(
        record, cursor, true, callbacks);
    const auto expected = expected_upgrade(record, rom, column, row);
    const auto& label = result.selected_cell;
    const auto expected_value = row < 4U
        ? source.bytes[0x77U + row + column * 4U]
        : source.bytes[(row - 4U + column * 6U) < 6U
                           ? 0x80U + row - 4U + column * 6U
                           : 0x87U + row - 4U + column * 6U - 6U];
    const auto expected_index = row < 4U
        ? row + column * 4U
        : row - 4U + column * 6U;
    const auto expected_record_offset = row < 4U
        ? 0x77U + expected_index
        : expected_index < 6U ? 0x80U + expected_index
                              : 0x87U + expected_index - 6U;
    const auto expected_cell_state =
        row >= 4U && expected_index == 0U && record.archetype_index >= 2U
            ? AttributeSkillCellState::class_restricted
            : AttributeSkillCellState::value;

    std::vector<int> expected_order{0, 1};
    if (!expected.suppresses_error_message) {
        expected_order.push_back(2);
        if (expected.status == AttributeSkillUpgradeStatus::available) {
            expected_order.push_back(4);
            expected_order.push_back(5);
        } else {
            expected_order.push_back(3);
        }
    }
    expected_order.push_back(6);
    expected_order.push_back(7);
    expected_order.push_back(8);

    const auto expected_outcome =
        expected.status == AttributeSkillUpgradeStatus::available
            ? AttributeSkillRenderOutcome::rendered_upgrade_preview
            : AttributeSkillRenderOutcome::invalid_upgrade_selection;
    if (label.value != expected_value || label.value_index != expected_index ||
        label.record_offset != expected_record_offset ||
        label.label_index != (row < 4U ? expected_index : 8U + expected_index) ||
        label.state != expected_cell_state || result.karma_available !=
            source.bytes[0x8DU] || result.outcome != expected_outcome ||
        order != expected_order || shown_label.value != expected_value ||
        shown_karma != source.bytes[0x8DU]) {
        return fail("rendered value/label/order mismatch at profile " +
                    std::to_string(profile_index) + " cell " +
                    std::to_string(column) + "," + std::to_string(row));
    }

    if (!expected.suppresses_error_message &&
        (result.upgrade.status != expected.status ||
         result.upgrade.current_value != expected_value ||
         result.upgrade.next_value != expected.next_value ||
         result.upgrade.karma_cost != expected.karma_cost)) {
        return fail("numeric preview mismatch at profile " +
                    std::to_string(profile_index) + " cell " +
                    std::to_string(column) + "," + std::to_string(row));
    }
    if (expected.status == AttributeSkillUpgradeStatus::available &&
        (shown_cost != expected.karma_cost ||
         shown_next != expected.next_value)) {
        return fail("visible cost/next-value mismatch at profile " +
                    std::to_string(profile_index) + " cell " +
                    std::to_string(column) + "," + std::to_string(row));
    }
    return true;
}

bool check_input_mask(const RunnerRecordFields& record,
                      const std::size_t profile_index,
                      const std::size_t column,
                      const std::size_t row,
                      const std::uint8_t mask,
                      const unsigned state_mode,
                      std::vector<int>& callback_order,
                      const AttributesSkillsMenuInputCallbacks& callbacks) {
    AttributesSkillsMenuState state{};
    state.open = true;
    state.selection_data_available = state_mode != 1U;
    state.upgrade_error_marker = state_mode == 2U;
    state.cursor = {static_cast<std::uint8_t>(column),
                    static_cast<std::uint8_t>(row)};

    AttributesSkillsMenuInputFrame input{};
    input.move_up = (mask & 0x01U) != 0U;
    input.move_down = (mask & 0x02U) != 0U;
    input.move_left = (mask & 0x04U) != 0U;
    input.move_right = (mask & 0x08U) != 0U;
    input.upgrade = (mask & 0x50U) != 0U;
    input.active_runner_selection = (mask & 0x20U) != 0U;
    input.exit = (mask & 0x80U) != 0U;
    callback_order.clear();

    const auto event = apply_attributes_skills_menu_input(
        state, record, input, callbacks);
    auto expected_column = column;
    auto expected_row = row;
    auto expected_open = true;
    AttributesSkillsMenuInputEvent expected_event =
        AttributesSkillsMenuInputEvent::waiting;
    std::vector<int> expected_callbacks;

    if ((mask & 0x04U) != 0U) {
        expected_column = 0U;
        expected_event = AttributesSkillsMenuInputEvent::selection_moved;
        expected_callbacks = {1};
    } else if ((mask & 0x08U) != 0U) {
        expected_column = 1U;
        expected_event = AttributesSkillsMenuInputEvent::selection_moved;
        expected_callbacks = {1};
    } else if ((mask & 0x01U) != 0U) {
        if (expected_row > 0U) {
            --expected_row;
        }
        expected_event = AttributesSkillsMenuInputEvent::selection_moved;
        expected_callbacks = {1};
    } else if ((mask & 0x02U) != 0U) {
        if (expected_row < 9U) {
            ++expected_row;
        }
        expected_event = AttributesSkillsMenuInputEvent::selection_moved;
        expected_callbacks = {1};
    } else if ((mask & 0x50U) != 0U) {
        if (state_mode == 1U) {
            expected_open = false;
            expected_event = AttributesSkillsMenuInputEvent::exited_to_field;
            expected_callbacks = {6};
        } else if (state_mode == 2U) {
            expected_event = AttributesSkillsMenuInputEvent::upgrade_blocked;
        } else {
            expected_callbacks.push_back(2);
            const bool non_upgradeable_attribute =
                expected_column == 1U &&
                (expected_row == 2U || expected_row == 3U);
            const bool restricted_first_skill =
                expected_column == 0U && expected_row == 4U &&
                record.archetype_index >= 2U;
            if (non_upgradeable_attribute) {
                expected_event =
                    AttributesSkillsMenuInputEvent::upgrade_blocked;
                expected_callbacks.push_back(3);
            } else if (restricted_first_skill) {
                expected_event =
                    AttributesSkillsMenuInputEvent::upgrade_blocked;
                expected_callbacks.push_back(4);
            } else {
                expected_event =
                    AttributesSkillsMenuInputEvent::upgrade_requested;
            }
        }
    } else if ((mask & 0x20U) != 0U) {
        expected_column = 0U;
        expected_event = AttributesSkillsMenuInputEvent::
            active_runner_selection_requested;
        expected_callbacks = {1, 5};
    } else if ((mask & 0x80U) != 0U) {
        expected_open = false;
        expected_event = AttributesSkillsMenuInputEvent::exited_to_field;
        expected_callbacks = {6};
    }

    if (event != expected_event || state.open != expected_open ||
        state.cursor.column != expected_column ||
        state.cursor.row != expected_row || callback_order != expected_callbacks) {
        return fail("controller mask mismatch at profile " +
                    std::to_string(profile_index) + " cell " +
                    std::to_string(column) + "," + std::to_string(row) +
                    " mask " + std::to_string(mask) + " mode " +
                    std::to_string(state_mode));
    }
    return true;
}

} // namespace

int main() {
#ifndef SHADOWRUN_CONVERSION_ASSET_ROOT
#error SHADOWRUN_CONVERSION_ASSET_ROOT must point to decompilation/assets
#endif
#ifndef SHADOWRUN_US_ROM_PATH
#error SHADOWRUN_US_ROM_PATH must point to the authoritative US ROM
#endif

    std::vector<std::uint8_t> rom;
    if (!read_file(std::filesystem::path(SHADOWRUN_US_ROM_PATH), rom) ||
        rom.size() < 0x166AD8U + 0x100U) {
        return fail("could not read the complete authoritative US ROM") ? 0 : 1;
    }

    const auto catalog = load_conversion_asset_catalog(
        std::filesystem::path(SHADOWRUN_CONVERSION_ASSET_ROOT));
    if (!catalog.complete()) {
        return fail("could not load named source assets") ? 0 : 2;
    }
    const auto recruited_asset =
        catalog.catalog.find(kRecruitedRunnerAssetAlias);
    if (recruited_asset == nullptr || recruited_asset->byte_size !=
            kRecruitedCount * kRecordSize) {
        return fail("recruited asset includes bytes beyond ten named records")
                   ? 0
                   : 3;
    }
    const auto startup = load_startup_record_assets(catalog.catalog);
    if (!startup.complete()) {
        return fail("could not load source runner record assets") ? 0 : 4;
    }

    const auto sentinel_offset =
        kRecruitedOffset + kRecruitedCount * kRecordSize + kKindOffset;
    if (read_be16(rom, sentinel_offset) != 0U) {
        return fail("raw ROM does not have the expected zero-kind sentinel")
                   ? 0
                   : 5;
    }

    std::array<LiveRunnerRecord, 13U> profiles{};
    for (std::size_t index = 0U; index < 3U; ++index) {
        const auto source_offset = kJoshuaOffset + index * kRecordSize;
        std::copy_n(rom.begin() + static_cast<std::ptrdiff_t>(source_offset),
                    kRecordSize,
                    profiles[index].bytes.begin());
        if (profiles[index].bytes !=
            startup.assets->joshua_archetypes[index].record) {
            return fail("Joshua archetype asset differs from its ROM record")
                       ? 0
                       : 6;
        }
    }
    for (std::size_t index = 0U; index < kRecruitedCount; ++index) {
        const auto source_offset = kRecruitedOffset + index * kRecordSize;
        std::copy_n(rom.begin() + static_cast<std::ptrdiff_t>(source_offset),
                    kRecordSize,
                    profiles[index + 3U].bytes.begin());
        if (profiles[index + 3U].bytes !=
            startup.assets->save_roster_templates[index + 1U].record) {
            return fail("recruited runner asset differs from its ROM record")
                       ? 0
                       : 7;
        }
        if (read_be16(rom, source_offset + kKindOffset) == 0U) {
            return fail("a zero-kind sentinel was loaded as a runner profile")
                       ? 0
                       : 8;
        }
    }
    for (std::size_t index = 11U; index < 13U; ++index) {
        if (std::any_of(startup.assets->save_roster_templates[index].record
                            .begin(),
                        startup.assets->save_roster_templates[index].record
                            .end(),
                        [](const std::uint8_t value) { return value != 0U; })) {
            return fail("unused save slot contains bytes from beyond the source sentinel")
                       ? 0
                       : 9;
        }
    }

    std::size_t displayed_cells = 0U;
    for (std::size_t profile = 0U; profile < profiles.size(); ++profile) {
        const auto record = read_runner_record_fields(profiles[profile]);
        const auto expected_species = kProfileIdentity[profile][0U];
        const auto expected_class = kProfileIdentity[profile][1U];
        if (record.species_index != expected_species ||
            record.archetype_index != expected_class) {
            return fail("source profile race/class identity mismatch at row " +
                        std::to_string(profile))
                       ? 0
                       : 10;
        }
        const auto grid = read_attributes_and_skills_grid(record);
        for (std::size_t column = 0U; column < 2U; ++column) {
            for (std::size_t row = 0U; row < 10U; ++row) {
                const auto skill_index = row >= 4U
                    ? row - 4U + column * 6U
                    : 0U;
                const auto value_index = row < 4U
                    ? row + column * 4U
                    : skill_index;
                const auto expected_offset = row < 4U
                    ? 0x77U + value_index
                    : skill_index < 6U ? 0x80U + skill_index
                                       : 0x87U + skill_index - 6U;
                const auto expected_label = row < 4U
                    ? value_index
                    : 8U + skill_index;
                const auto expected_section = row < 4U
                    ? AttributeSkillSection::attribute
                    : AttributeSkillSection::skill;
                const auto expected_cell_state =
                    row >= 4U && skill_index == 0U &&
                            record.archetype_index >= 2U
                        ? AttributeSkillCellState::class_restricted
                        : AttributeSkillCellState::value;
                const auto& cell = grid[column][row];
                if (cell.section != expected_section ||
                    cell.value_index != value_index ||
                    cell.record_offset != expected_offset ||
                    cell.label_index != expected_label ||
                    cell.value != profiles[profile].bytes[expected_offset] ||
                    cell.state != expected_cell_state ||
                    !check_rendered_numeric_cell(
                        profiles[profile], record, rom, profile, column, row)) {
                    return fail("full profile grid mismatch at row " +
                                std::to_string(profile) + " cell " +
                                std::to_string(column) + "," +
                                std::to_string(row))
                               ? 0
                               : 11;
                }
                ++displayed_cells;
            }
        }
    }

    if (displayed_cells != 260U) {
        return fail("profile grid sweep did not visit 13 x 20 cells") ? 0
                                                                      : 12;
    }

    std::vector<int> callback_order;
    AttributesSkillsMenuInputCallbacks input_callbacks{};
    input_callbacks.render_detail_panel = [&](const std::uint16_t command) {
        callback_order.push_back(command == 0x6000U ? 1 : 99);
    };
    input_callbacks.write_upgrade_grid = [&] { callback_order.push_back(2); };
    input_callbacks.render_unsupported_selection =
        [&](const AttributesSkillsUnsupportedSelection reason) {
            callback_order.push_back(
                reason == AttributesSkillsUnsupportedSelection::
                               non_upgradeable_attribute
                    ? 3
                    : 4);
        };
    input_callbacks.advance_active_runner_selection =
        [&] { callback_order.push_back(5); };
    input_callbacks.leave_to_field = [&] { callback_order.push_back(6); };

    std::size_t controller_cases = 0U;
    for (std::size_t profile = 0U; profile < profiles.size(); ++profile) {
        const auto record = read_runner_record_fields(profiles[profile]);
        for (std::size_t column = 0U; column < 2U; ++column) {
            for (std::size_t row = 0U; row < 10U; ++row) {
                for (unsigned state_mode = 0U; state_mode < 3U;
                     ++state_mode) {
                    for (unsigned byte = 0U; byte <= 0xFFU; ++byte) {
                        if (!check_input_mask(
                                record, profile, column, row,
                                static_cast<std::uint8_t>(byte), state_mode,
                                callback_order, input_callbacks)) {
                            return 13;
                        }
                        ++controller_cases;
                    }
                }
            }
        }
    }
    if (controller_cases != 199680U) {
        return fail("controller sweep did not visit all 256 masks per cell and state")
                   ? 0
                   : 14;
    }

    std::size_t attribute_boundary_cases = 0U;
    for (std::size_t species = 0U; species < 5U; ++species) {
        for (std::size_t attribute = 0U; attribute < 6U; ++attribute) {
            const auto maximum = rom[kSpeciesMaximumsOffset + species * 6U +
                                     attribute];
            const auto column = attribute / 4U;
            const auto row = attribute % 4U;
            for (unsigned current = 0U; current <= maximum + 1U; ++current) {
                RunnerRecordFields record{};
                record.species_index = static_cast<std::uint8_t>(species);
                record.auxiliary[2U] = 0xFFU;
                record.attributes[attribute] =
                    static_cast<std::uint8_t>(current);
                const auto preview = preview_attribute_skill_upgrade(
                    record, {static_cast<std::uint8_t>(column),
                             static_cast<std::uint8_t>(row)});
                if (current < maximum) {
                    const auto cost = static_cast<std::uint8_t>(current + 1U);
                    auto underfunded = record;
                    underfunded.auxiliary[2U] =
                        static_cast<std::uint8_t>(cost - 1U);
                    auto confirm = record;
                    confirm.auxiliary[2U] = cost;
                    if (preview.status !=
                            AttributeSkillUpgradeStatus::available ||
                        preview.current_value != current ||
                        preview.next_value != cost ||
                        preview.karma_cost != cost ||
                        apply_attribute_skill_upgrade(
                            underfunded,
                            {static_cast<std::uint8_t>(column),
                             static_cast<std::uint8_t>(row)},
                            true) !=
                            AttributeSkillUpgradeResult::insufficient_karma ||
                        apply_attribute_skill_upgrade(
                            confirm,
                            {static_cast<std::uint8_t>(column),
                             static_cast<std::uint8_t>(row)},
                            false) !=
                            AttributeSkillUpgradeResult::confirmation_required ||
                        apply_attribute_skill_upgrade(
                            confirm,
                            {static_cast<std::uint8_t>(column),
                             static_cast<std::uint8_t>(row)},
                            true) != AttributeSkillUpgradeResult::upgraded ||
                        confirm.attributes[attribute] != cost ||
                        confirm.auxiliary[2U] != 0U) {
                        return fail("race maximum lower-bound/upgrade transaction mismatch")
                                   ? 0
                                   : 15;
                    }
                } else if (preview.status !=
                               AttributeSkillUpgradeStatus::at_limit ||
                           apply_attribute_skill_upgrade(
                               record,
                               {static_cast<std::uint8_t>(column),
                                static_cast<std::uint8_t>(row)},
                               true) != AttributeSkillUpgradeResult::at_limit) {
                    return fail("race maximum did not stop an attribute upgrade")
                               ? 0
                               : 16;
                }
                ++attribute_boundary_cases;
            }
        }
    }

    std::size_t skill_boundary_cases = 0U;
    for (unsigned runner_class = 0U; runner_class < 4U; ++runner_class) {
        for (std::size_t skill = 0U; skill < 12U; ++skill) {
            const auto column = skill / 6U;
            const auto row = 4U + skill % 6U;
            for (unsigned current = 0U; current <= 13U; ++current) {
                const auto support_limit =
                    skill >= 2U && skill <= 4U && current == 0U ? 13U : 1U;
                for (unsigned firearms = 0U; firearms < support_limit;
                     ++firearms) {
                    RunnerRecordFields record{};
                    record.archetype_index =
                        static_cast<std::uint8_t>(runner_class);
                    record.skills[skill] =
                        static_cast<std::uint8_t>(current);
                    if (skill != 1U) {
                        record.skills[1U] =
                            static_cast<std::uint8_t>(firearms);
                    }
                    const auto preview = preview_attribute_skill_upgrade(
                        record, {static_cast<std::uint8_t>(column),
                                 static_cast<std::uint8_t>(row)});
                    const bool restricted = skill == 0U && runner_class >= 2U;
                    const bool reduced = skill >= 2U && skill <= 4U;
                    auto next_base = current;
                    if (reduced && current == 0U) {
                        next_base = firearms;
                    }
                    const auto next = next_base + 1U;
                    if (restricted) {
                        if (preview.status !=
                            AttributeSkillUpgradeStatus::class_restricted) {
                            return fail("class gate did not restrict Sorcery")
                                       ? 0
                                       : 17;
                        }
                    } else if (next >= 13U) {
                        if (preview.status !=
                            AttributeSkillUpgradeStatus::at_limit) {
                            return fail("skill rank 12/13 boundary mismatch")
                                       ? 0
                                       : 18;
                        }
                    } else {
                        const auto expected_cost = static_cast<std::uint16_t>(
                            reduced ? (next * 15U + 9U) / 10U : next * 2U);
                        if (preview.status !=
                                AttributeSkillUpgradeStatus::available ||
                            preview.next_value != next ||
                            preview.karma_cost != expected_cost) {
                            return fail("class/skill cost permutation mismatch")
                                       ? 0
                                       : 19;
                        }
                    }
                    ++skill_boundary_cases;
                }
            }
        }
    }

    RunnerRecordFields unknown_species{};
    unknown_species.species_index = 5U;
    if (preview_attribute_skill_upgrade(unknown_species, {0U, 0U}).status !=
        AttributeSkillUpgradeStatus::unknown_species) {
        return fail("unknown race did not remain an explicit source boundary")
                   ? 0
                   : 20;
    }

    std::cout << "source_profiles=" << profiles.size()
              << " displayed_cells=" << displayed_cells
              << " controller_states=" << controller_cases
              << " race_attribute_boundary_states="
              << attribute_boundary_cases
              << " class_skill_boundary_states=" << skill_boundary_cases
              << " recruited_templates=" << kRecruitedCount
              << "\n";
    return 0;
}
