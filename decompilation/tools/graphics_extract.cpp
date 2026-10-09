#include "shadowrun/rendering/graphics.hpp"
#include "shadowrun/core/rom_layout.hpp"
#include "shadowrun/rendering/asset_taxonomy.hpp"
#include "shadowrun/rendering/resource_manifest.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <initializer_list>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

namespace {

std::uint64_t parse_number(const std::string& value) {
    std::size_t consumed = 0;
    const auto number = std::stoull(value, &consumed, 0);
    if (consumed != value.size()) {
        throw std::invalid_argument("invalid numeric argument: " + value);
    }
    return number;
}

std::uint16_t read_big_endian_word(
    const std::vector<std::uint8_t>& bytes,
    const std::size_t offset) {
    if (offset > bytes.size() || bytes.size() - offset < 2U) {
        throw std::runtime_error("source word exceeds input image");
    }
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(bytes[offset]) << 8U) |
        static_cast<std::uint16_t>(bytes[offset + 1U]));
}

std::uint32_t read_big_endian_long(
    const std::vector<std::uint8_t>& bytes,
    const std::size_t offset) {
    if (offset > bytes.size() || bytes.size() - offset < 4U) {
        throw std::runtime_error("source long exceeds input image");
    }
    return (static_cast<std::uint32_t>(bytes[offset]) << 24U) |
           (static_cast<std::uint32_t>(bytes[offset + 1U]) << 16U) |
           (static_cast<std::uint32_t>(bytes[offset + 2U]) << 8U) |
           static_cast<std::uint32_t>(bytes[offset + 3U]);
}

std::vector<std::uint8_t> read_file(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("cannot open ROM: " + path);
    }
    return std::vector<std::uint8_t>(
        std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

void write_rom_record(const std::vector<std::uint8_t>& rom,
                      const std::filesystem::path& root,
                      const std::string& relative_name,
                      const std::size_t source_offset,
                      const std::size_t byte_count) {
    if (source_offset > rom.size() || byte_count > rom.size() - source_offset) {
        throw std::runtime_error("ROM record exceeds input image: " +
                                 relative_name);
    }
    const auto path = root / relative_name;
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) {
        throw std::runtime_error("cannot create ROM record: " + path.string());
    }
    output.write(reinterpret_cast<const char*>(rom.data() + source_offset),
                 static_cast<std::streamsize>(byte_count));
    if (!output) {
        throw std::runtime_error("cannot write ROM record: " + path.string());
    }
}

void write_host_record(const std::vector<std::uint8_t>& bytes,
                       const std::filesystem::path& root,
                       const std::string& relative_name) {
    const auto path = root / relative_name;
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) {
        throw std::runtime_error("cannot create host record: " + path.string());
    }
    output.write(reinterpret_cast<const char*>(bytes.data()),
                 static_cast<std::streamsize>(bytes.size()));
    if (!output) {
        throw std::runtime_error("cannot write host record: " + path.string());
    }
}

std::vector<std::pair<std::string, std::string>>
write_field_service_routes(
    const std::vector<std::uint8_t>& rom,
    const std::filesystem::path& root,
    const shadowrun::native::NativePortAssets& assets) {
    constexpr std::array<std::string_view, 13> kMagicSpellIds{{
        "flame_dart", "flame_bolt", "hellblast", "invisibility", "sleep",
        "stink", "super_barrier", "mana_zap", "mana_blast", "mana_storm",
        "rockskin", "heal_wounds", "confusion"}};
    const auto path = root / "menu" / "field" / "service-routes.tsv";
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) {
        throw std::runtime_error("cannot create field service route catalog: " +
                                 path.string());
    }
    output << "key\ttext\n";
    std::vector<std::pair<std::string, std::string>> script_text;
    const auto emit = [&output, &script_text](const std::string& key,
                                             const std::string& text) {
        if (text.find_first_of("\t\r\n") != std::string::npos) {
            throw std::runtime_error(
                "field service route text contains a TSV control character: " +
                key);
        }
        if (key.rfind("field-service.title-destination.", 0U) == 0U) {
            output << key << '\t' << text << '\n';
        } else {
            script_text.emplace_back(key, text);
        }
    };

    const auto service_titles =
        shadowrun::graphics::read_genesis_field_service_titles(rom);
    for (std::size_t index = 0; index < service_titles.size(); ++index) {
        emit("field-service.title-destination." + std::to_string(index),
             std::to_string(service_titles[index].destination_word));
    }
    const auto escape_field_text = [](std::string text) {
        std::size_t position = 0U;
        while ((position = text.find('\n', position)) !=
               std::string::npos) {
            text.replace(position, 1U, "\\n");
            position += 2U;
        }
        return text;
    };
    const auto cyberdeck_unavailable_text =
        shadowrun::graphics::read_genesis_cyberdeck_unavailable_text(rom);
    for (std::size_t index = 0;
         index < cyberdeck_unavailable_text.size(); ++index) {
        emit("pocket-secretary.cyberdeck.line." + std::to_string(index),
             cyberdeck_unavailable_text[index]);
    }
    const auto group_item_names =
        shadowrun::graphics::read_genesis_group_item_catalog(rom);
    for (std::size_t index = 0; index < group_item_names.size(); ++index) {
        emit("pocket-secretary.group-item." + std::to_string(index),
             group_item_names[index]);
    }
    const auto group_items_empty_text =
        shadowrun::graphics::read_genesis_group_items_empty_text(rom);
    for (std::size_t index = 0; index < group_items_empty_text.size(); ++index) {
        emit("pocket-secretary.group-items.empty." + std::to_string(index),
             group_items_empty_text[index]);
    }
    const auto dismiss_runner_labels =
        shadowrun::graphics::read_genesis_dismiss_runner_labels(rom);
    for (std::size_t index = 0; index < dismiss_runner_labels.size(); ++index) {
        emit("dismiss.label." + std::to_string(index),
             dismiss_runner_labels[index]);
    }
    emit("dismiss.separator",
         shadowrun::graphics::read_genesis_dismiss_runner_separator(rom));
    const auto notebook_tips_text =
        shadowrun::graphics::read_genesis_notebook_tips_text(rom);
    emit("notebook.tips.title", notebook_tips_text.title);
    emit("notebook.tips.separator", notebook_tips_text.separator);
    emit("notebook.tips.page-count", notebook_tips_text.page_count_label);
    emit("notebook.tips.page-number-clear",
         notebook_tips_text.page_number_clear);
    constexpr std::array<std::string_view, 6> kNotebookLabelKeys{{
        "notebook.options.title",
        "notebook.option.current_run",
        "notebook.option.tips_and_clues",
        "notebook.option.contacts",
        "notebook.option.mr_johnsons",
        "notebook.option.shadowrunners",
    }};
    for (std::size_t index = 0; index < assets.notebook_labels.size(); ++index) {
        emit(std::string(kNotebookLabelKeys[index]),
             assets.notebook_labels[index]);
    }
    for (std::size_t index = 0; index < assets.notebook_no_run_lines.size();
         ++index) {
        emit("notebook.no-run." + std::to_string(index),
             assets.notebook_no_run_lines[index]);
    }
    for (std::size_t index = 0;
         index < assets.notebook_mr_johnson_names.size(); ++index) {
        emit("notebook.mr-johnson.name." + std::to_string(index),
             assets.notebook_mr_johnson_names[index]);
    }
    const auto mr_johnson_descriptions =
        shadowrun::graphics::read_genesis_notebook_mr_johnson_descriptions(
            rom);
    for (std::size_t index = 0; index < mr_johnson_descriptions.size();
         ++index) {
        emit("notebook.mr-johnson.detail." + std::to_string(index),
             escape_field_text(mr_johnson_descriptions[index]));
    }
    const auto command_names =
        shadowrun::graphics::read_genesis_command_overview_names(rom);
    for (std::size_t index = 0; index < command_names.races.size(); ++index) {
        emit("race." + std::to_string(index), command_names.races[index]);
    }
    for (std::size_t index = 0; index < command_names.classes.size(); ++index) {
        emit("class." + std::to_string(index), command_names.classes[index]);
    }
    const auto attribute_skill_labels =
        shadowrun::graphics::read_genesis_attribute_skill_labels(rom);
    for (std::size_t index = 0; index < attribute_skill_labels.size();
         ++index) {
        emit("attribute.skill." + std::to_string(index),
             attribute_skill_labels[index]);
    }
    const auto attributes_skills_text =
        shadowrun::graphics::read_genesis_attributes_skills_text(rom);
    emit("attributes.upgrade.header",
         attributes_skills_text.upgrade_header);
    emit("attributes.upgrade.maximum-level",
         attributes_skills_text.maximum_level_message);
    emit("attributes.upgrade.mage-only",
         attributes_skills_text.mage_only_message);
    emit("attributes.upgrade.insufficient-karma",
         attributes_skills_text.insufficient_karma_message);
    emit("attributes.upgrade.prompt",
         attributes_skills_text.upgrade_prompt);
    const auto cyberware_text =
        shadowrun::graphics::read_genesis_cyberware_text(rom);
    const auto title_break = cyberware_text.title.find('\n');
    if (title_break == std::string::npos ||
        cyberware_text.title.find('\n', title_break + 1U) !=
            std::string::npos) {
        throw std::runtime_error(
            "field menu Cyberware title must contain exactly two source lines");
    }
    emit("cyberware.title.0", cyberware_text.title.substr(0, title_break));
    emit("cyberware.title.1",
         cyberware_text.title.substr(title_break + 1U));
    emit("cyberware.empty", cyberware_text.empty_message);
    for (std::size_t index = 0; index < cyberware_text.entries.size();
         ++index) {
        emit("cyberware.entry." + std::to_string(index),
             cyberware_text.entries[index]);
    }
    // ROM 0xFBA6 passes C89C0 to the encoded-title writer as the fallback
    // heading. Keep the source's leading space for its original cell offset.
    emit("magic.defense.title", " Magical Defense");
    const auto magic_text =
        shadowrun::graphics::read_genesis_magic_menu_text(rom);
    if (magic_text.spell_names.size() != kMagicSpellIds.size() ||
        magic_text.spell_descriptions.size() != kMagicSpellIds.size()) {
        throw std::runtime_error(
            "Magic spell text count no longer matches its stable IDs");
    }
    emit("magic.heading.force-posture", magic_text.set_force_and_posture);
    emit("magic.label.success", magic_text.success_label);
    emit("magic.label.force", magic_text.force_label);
    emit("magic.label.availability", magic_text.availability_label);
    const auto escape_newlines = [](std::string value) {
        std::size_t position = 0U;
        while ((position = value.find('\n', position)) != std::string::npos) {
            value.replace(position, 1U, "\\n");
            position += 2U;
        }
        return value;
    };
    emit("magic.panel.metric-labels",
         escape_newlines(magic_text.panel_metric_labels));
    emit("magic.panel.force-posture-labels",
         escape_newlines(magic_text.force_posture_labels));
    for (std::size_t index = 0; index < magic_text.spell_names.size();
         ++index) {
        const auto spell_id = std::string(kMagicSpellIds[index]);
        emit("magic.spells.option." + spell_id,
             magic_text.spell_names[index]);
        emit("magic.spells.description." + spell_id,
             escape_newlines(magic_text.spell_descriptions[index]));
    }
    for (std::size_t index = 0; index < assets.intro.archetypes.size();
         ++index) {
        emit("archetype." + std::to_string(index),
             assets.intro.archetypes[index]);
    }
    for (std::size_t index = 0; index < assets.field_menu_runner_names.size();
         ++index) {
        emit("runner." + std::to_string(index),
             assets.field_menu_runner_names[index]);
    }
    for (std::size_t index = 0; index < assets.field_menu_status_names.size();
         ++index) {
        emit("status." + std::to_string(index),
             assets.field_menu_status_names[index]);
    }
    const auto field_labels =
        shadowrun::graphics::read_genesis_field_menu_labels(rom);
    emit("metric.physical", field_labels.physical);
    emit("metric.mental", field_labels.mental);
    emit("metric.attack", field_labels.attack);
    emit("metric.defense", field_labels.defense);
    emit("metric.damage", field_labels.damage);
    emit("metric.drain", field_labels.drain);
    emit("metric.none", field_labels.none);
    emit("metric.no-armor", field_labels.no_armor);
    emit("metric.weapons", field_labels.weapons);
    emit("metric.guns", field_labels.guns);
    emit("metric.power", field_labels.power);
    const auto action_text =
        shadowrun::graphics::read_genesis_field_menu_action_text(rom);
    emit("action.item-equipped.prefix", action_text.item_equipped_prefix);
    emit("action.item-equipped.suffix", action_text.item_equipped_suffix);
    emit("action.discard-prompt", action_text.discard_prompt);
    emit("action.cannot-discard", action_text.cannot_discard);
    emit("action.cannot-trade", action_text.cannot_trade);
    emit("ui.option.yes", action_text.choice_yes);
    emit("ui.option.no", action_text.choice_no);
    emit("action.automatic-use.message",
         escape_newlines(action_text.automatic_use_message));
    emit("first-aid.prompt", escape_newlines(action_text.first_aid_prompt));
    emit("first-aid.label", action_text.first_aid_label);
    emit("first-aid.cancel", action_text.first_aid_cancel);
    emit("first-aid.not-injured",
         escape_newlines(action_text.first_aid_not_injured));
    emit("first-aid.slap-patch-used",
         action_text.first_aid_slap_patch_used);
    emit("first-aid.medkit-used", action_text.first_aid_medkit_used);
    emit("action.trade-recipient", action_text.trade_recipient_label);
    emit("action.trade-full-inventory",
         escape_newlines(action_text.trade_full_inventory_message));
    const auto clips_text =
        shadowrun::graphics::read_genesis_field_menu_clips_text(rom);
    constexpr std::array<std::string_view, 4> kClipsCommandTextKeys{{
        "clips.option.reload", "clips.option.trade",
        "clips.option.distribute", "clips.option.cancel"}};
    constexpr std::array<std::string_view, 4> kClipsTradeCommandTextKeys{{
        "clips.trade.option.increase_clips",
        "clips.trade.option.decrease_clips",
        "clips.trade.option.trade_clips",
        "clips.trade.option.change_recipient"}};
    for (std::size_t index = 0; index < clips_text.commands.size(); ++index) {
        emit(std::string(kClipsCommandTextKeys[index]),
             clips_text.commands[index]);
        emit(std::string(kClipsTradeCommandTextKeys[index]),
             clips_text.trade_commands[index]);
    }
    for (std::size_t index = 0; index < clips_text.trade_prompt.size();
         ++index) {
        emit("clips.trade-prompt." + std::to_string(index),
             clips_text.trade_prompt[index]);
    }
    for (std::size_t index = 0; index < clips_text.exchange_complete.size();
         ++index) {
        emit("clips.exchange-complete." + std::to_string(index),
             clips_text.exchange_complete[index]);
    }
    emit("clips.title", clips_text.title);
    emit("clips.separator", clips_text.separator);
    emit("clips.counter", clips_text.counter_label);
    for (std::size_t index = 0; index < clips_text.reload_prompt.size();
         ++index) {
        emit("clips.reload." + std::to_string(index),
             clips_text.reload_prompt[index]);
    }
    for (std::size_t index = 0; index < clips_text.reload_success.size();
         ++index) {
        emit("clips.success." + std::to_string(index),
             clips_text.reload_success[index]);
    }
    for (std::size_t index = 0; index < clips_text.out_of_ammo.size();
         ++index) {
        emit("clips.out-of-ammo." + std::to_string(index),
             clips_text.out_of_ammo[index]);
    }
    for (std::size_t index = 0;
         index < clips_text.item_does_not_use_clips.size(); ++index) {
        emit("clips.item-no-clips." + std::to_string(index),
             clips_text.item_does_not_use_clips[index]);
    }
    for (std::size_t index = 0; index < clips_text.no_party_members.size();
         ++index) {
        emit("clips.no-party." + std::to_string(index),
             clips_text.no_party_members[index]);
    }
    for (std::size_t index = 0; index < assets.field_menu_power_labels.size();
         ++index) {
        emit("power." + std::to_string(index),
             assets.field_menu_power_labels[index]);
    }
    for (std::size_t armor = 0; armor < assets.field_menu_armor_labels.size();
         ++armor) {
        for (std::size_t row = 0;
             row < assets.field_menu_armor_labels[armor].size(); ++row) {
            emit("armor." + std::to_string(armor) + "." +
                     std::to_string(row),
                 assets.field_menu_armor_labels[armor][row]);
        }
    }
    if (!output) {
        throw std::runtime_error("cannot write field service route catalog: " +
                                 path.string());
    }
    return script_text;
}

std::vector<std::pair<std::string, std::string>>
read_notebook_tips_script_text(
    const shadowrun::native::NativePortAssets& assets) {
    std::vector<std::pair<std::string, std::string>> script_text;
    script_text.reserve(assets.notebook_tip_text.size());
    for (std::size_t index = 0; index < assets.notebook_tip_text.size();
         ++index) {
        auto text = assets.notebook_tip_text[index];
        if (text.find_first_of("\t\r") != std::string::npos) {
            throw std::runtime_error(
                "Notebook tip contains an unsupported TSV control character: " +
                std::to_string(index));
        }
        std::size_t position = 0U;
        while ((position = text.find('\n', position)) != std::string::npos) {
            text.replace(position, 1U, "\\n");
            position += 2U;
        }
        script_text.emplace_back("notebook.tip." + std::to_string(index),
                                 std::move(text));
    }
    return script_text;
}

void remove_obsolete_menu_text_assets(const std::filesystem::path& root) {
    // These generated TSVs were replaced by keyed Lua text. Remove them when
    // refreshing an existing asset directory so stale copies cannot linger.
    std::filesystem::remove(root / "menu" / "field" / "text.tsv");
    std::filesystem::remove(root / "menu" / "pocket-secretary" /
                            "notebook-tips.tsv");
}

void write_dialogue_text_script(const std::vector<std::uint8_t>& rom,
                                const std::filesystem::path& root,
                                const shadowrun::native::NativePortAssets& assets,
                                const std::vector<std::pair<std::string,
                                                            std::string>>&
                                    field_text) {
    const auto path = root / "dialogue" / "script-text.lua";
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) {
        throw std::runtime_error("cannot create Lua dialogue content: " +
                                 path.string());
    }

    const auto write_lua_string = [&output](const std::string_view text) {
        output << '"';
        for (const auto character : text) {
            switch (character) {
            case '\\': output << "\\\\"; break;
            case '"': output << "\\\""; break;
            case '\n': output << "\\n"; break;
            case '\r': output << "\\r"; break;
            case '\t': output << "\\t"; break;
            default:
                if (static_cast<unsigned char>(character) < 0x20U) {
                    output << '\\' << static_cast<char>(
                        '0' + ((static_cast<unsigned char>(character) / 100U) % 10U))
                           << static_cast<char>(
                        '0' + ((static_cast<unsigned char>(character) / 10U) % 10U))
                           << static_cast<char>(
                        '0' + (static_cast<unsigned char>(character) % 10U));
                } else {
                    output << character;
                }
                break;
            }
        }
        output << '"';
    };

    const std::array<std::pair<const char*, std::size_t>, 44> records{{
        // Stoker prompt pointers consumed by the source event/profile path.
        {"stoker.motel.arrival.title", 0x1167EBU},
        {"stoker.motel.arrival.more", 0x0FE5BFU},
        {"stoker.motel.manager.title", 0x116962U},
        {"stoker.motel.option.crash", 0x103A17U},
        {"stoker.motel.option.leave", 0x104FEBU},
        {"stoker.motel.street-ambush.title", 0x1168FCU},
        {"stoker.motel.room.title", 0x1169B8U},
        {"stoker.motel.option.pay", 0x103A7AU},
        {"stoker.motel.package-handoff.title", 0x1168A3U},
        {"stoker.motel.package-question.title", 0x112765U},
        {"stoker.motel.option.package", 0x103A55U},
        {"stoker.motel.option.debt-payment", 0x103B93U},
        {"stoker.motel.option.jump-house", 0x103BBFU},
        {"stoker.motel.leave.title", 0x0F4134U},
        {"stoker.motel.debt.title", 0x1169E9U},
        {"stoker.motel.room.paid.title", 0x0F4241U},
        {"stoker.motel.room.insufficient.title", 0x0F42C1U},
        {"stoker.motel.jump-house.title", 0x0F41A1U},
        // Mode-2 field object prompts and outcomes selected by 0x20E48,
        // 0x20EC2, and their source mode-command handlers.
        {"field.maglock.door.title", 0x11C882U},
        {"field.maglock.more", 0x0FE5BFU},
        {"field.maglock.no-passkey", 0x11C8C8U},
        {"field.maglock.insufficient-passkey", 0x11C91CU},
        {"field.interaction.option.electronics", 0x11C984U},
        {"field.maglock.option.grenade", 0x11C9A1U},
        {"field.interaction.option.walk-away", 0x103B53U},
        {"field.maglock.passkey.result", 0x11C9E6U},
        {"field.maglock.electronics.no-skill", 0x11CA59U},
        {"field.maglock.electronics.success", 0x11CAB2U},
        {"field.maglock.electronics.failure", 0x11CAF6U},
        {"field.maglock.electronics.alert", 0x11CB5CU},
        {"field.maglock.grenade.unavailable", 0x11CBF5U},
        {"field.maglock.grenade.result", 0x11CC26U},
        {"field.maglock.already-unlocked", 0x0F4837U},
        {"field.security-terminal.title", 0x11CDBAU},
        {"field.security-terminal.failed", 0x11CDE9U},
        {"field.security-terminal.alarm-disabled", 0x11CE2AU},
        {"field.security-terminal.system-disabled", 0x11CE68U},
        {"field.security-terminal.files", 0x11CEC0U},
        {"field.security-terminal.floor.0", 0x11CEF4U},
        {"field.security-terminal.floor.1", 0x11CF02U},
        {"field.security-terminal.floor.2", 0x11CF11U},
        {"field.security-terminal.floor.3", 0x11CF1FU},
        {"field.security-terminal.floor.4", 0x11CF2EU},
        {"field.security-terminal.floor.5", 0x11CF3CU},
    }};

    output << "-- Generated from the US ROM dialogue and menu text records.\n"
              "-- Pipe characters preserve source segment/page boundaries.\n"
              "return {\n    texts = {\n";
    constexpr std::array<std::string_view, 7> kCommandOverviewTextKeys{{
        "field.command_overview.option.inventory_left",
        "field.command_overview.option.inventory_right",
        "field.command_overview.option.clips",
        "field.command_overview.option.attributes_skills",
        "field.command_overview.option.cyberware",
        "field.command_overview.option.magic",
        "field.command_overview.option.pocket_secretary"}};
    const auto command_labels =
        shadowrun::graphics::read_genesis_command_overview_labels(rom);
    for (std::size_t index = 0; index < command_labels.size(); ++index) {
        output << "        [";
        write_lua_string(kCommandOverviewTextKeys[index]);
        output << "] = ";
        write_lua_string(command_labels[index]);
        output << ",\n";
    }
    for (std::size_t record_index = 0; record_index < records.size();
         ++record_index) {
        const auto& [key, source_offset] = records[record_index];
        const auto segments =
            shadowrun::graphics::read_genesis_dialogue_segments(
                rom, source_offset);
        if (segments.empty()) {
            throw std::runtime_error("empty dialogue text: " +
                                     std::string(key));
        }
        std::string text;
        for (std::size_t segment_index = 0;
             segment_index < segments.size(); ++segment_index) {
            if (segment_index != 0U) {
                text.push_back('|');
            }
            text += segments[segment_index];
        }
        output << "        [";
        write_lua_string(key);
        output << "] = ";
        // The two security-terminal floor records are single-string records
        // in the source continuation; retaining only their first segment
        // matches the former import's record-specific handling.
        if (record_index >= 42U) {
            text = segments.front();
        }
        write_lua_string(text);
        output << ",\n";
    }
    const auto hotel_text =
        shadowrun::graphics::read_genesis_hotel_options_text(rom);
    const auto write_text_entry = [&output, &write_lua_string](
                                      const std::string_view key,
                                      const std::string_view text) {
        output << "        [";
        write_lua_string(key);
        output << "] = ";
        write_lua_string(text);
        output << ",\n";
    };
    // Startup layout text at ROM 0x1B6498, 0x1B64CF, 0x1B6518,
    // 0x1B6527, and 0x1B6577. Native C++ retains row placement and typed
    // controller state; Lua owns the rendered copy and ordered choice labels.
    write_text_entry("startup.options.title", " Start Options");
    write_text_entry("startup.options.separator",
                     std::string("[") + std::string(13U, '\\'));
    write_text_entry("startup.option.new_game", "  New Game");
    write_text_entry("startup.option.load_game", "  Load Game");
    write_text_entry("startup.archetype.heading.choose", "  Choose");
    write_text_entry("startup.archetype.heading.archetype", " Archetype");
    write_text_entry("startup.archetype.separator",
                     std::string("[") + std::string(10U, '\\'));
    write_text_entry("startup.option.samurai", "  Samurai");
    write_text_entry("startup.option.decker", "  Decker");
    write_text_entry("startup.option.gator_shaman", "  Gator");
    write_text_entry("startup.archetype.gator_shaman.suffix", "    Shaman");
    write_text_entry("startup.option.slot_one", "Game 1");
    write_text_entry("startup.option.slot_two", "Game 2");
    write_text_entry("startup.message.no_save.0", "Jack out and smell the");
    write_text_entry("startup.message.no_save.1", "   soycaf, chummer!");
    write_text_entry("startup.message.no_save.2", "   There's no game");
    write_text_entry("startup.message.no_save.3", "     saved here!");
    write_text_entry("startup.message.save_success.0",
                     " Successful download!");
    write_text_entry("startup.message.save_success.1",
                     "   Press any button");
    write_text_entry("startup.message.save_success.2",
                     "to run in the shadows.");
    // Generic Ares mode-text prompt paragraphs selected by ROM 0x1FA78.
    // These adjacent variable-length records end with 0x80/0xFF controls, so
    // decode their known visible spans rather than scanning into the next
    // record with the standalone dialogue-segment reader.
    const auto read_mode_prompt_paragraphs = [&rom](
        const std::initializer_list<std::pair<std::size_t, std::size_t>>
            records_to_read) {
        const auto read_record = [&rom](const std::size_t source_offset,
                                        const std::size_t byte_count) {
            if (source_offset > rom.size() ||
                byte_count > rom.size() - source_offset) {
                throw std::runtime_error(
                    "Ares mode-text prompt record exceeds the ROM");
            }
            std::string record;
            record.reserve(byte_count);
            for (std::size_t index = 0; index < byte_count; ++index) {
                const auto byte = rom[source_offset + index];
                if (byte < 0x20U || byte > 0x7EU) {
                    throw std::runtime_error(
                        "Ares mode-text prompt span contains a control byte");
                }
                record.push_back(static_cast<char>(byte));
            }
            return record;
        };
        std::string text;
        for (const auto& [offset, length] : records_to_read) {
            if (!text.empty()) {
                text += "\n\n";
            }
            text += read_record(offset, length);
        }
        return text;
    };
    write_text_entry(
        "field.mode.ares.store_intro",
        read_mode_prompt_paragraphs(
            {{0x118934U, 85U}, {0x11898BU, 67U}, {0x1189D0U, 33U}}));
    write_text_entry(
        "field.mode.ares.package_handoff",
        read_mode_prompt_paragraphs(
            {{0x1189F2U, 42U}, {0x118A1EU, 38U}, {0x118A46U, 37U}}));
    write_text_entry(
        "field.mode.ares.package_payment",
        read_mode_prompt_paragraphs(
            {{0x118A6CU, 54U}, {0x118AA4U, 80U}}));
    constexpr std::array<std::string_view, 12> kServiceTitleIds{{
        "weapons_world", "ares_weapon_emporium", "crime_mall",
        "medicine_lodge", "riannons", "merlins_lore", "microtronics",
        "wire_masters", "little_chiba", "dr_bobs",
        "council_island_hospital", "seattle_general_hospital"}};
    const auto service_titles =
        shadowrun::graphics::read_genesis_field_service_titles(rom);
    for (std::size_t index = 0; index < service_titles.size(); ++index) {
        write_text_entry("shop.service.option." +
                             std::string(kServiceTitleIds[index]),
                         service_titles[index].text);
    }
    constexpr std::array<std::string_view, 32> kShopActionHeadingIds{{
        "ares_weapons_display", "ares_weapons_available",
        "ares_armor_display", "ares_armor_available",
        "ares_other_items_display", "ares_other_items_equipment",
        "ares_weapon_accessories_display",
        "ares_weapon_accessories_accessories",
        "ares_firearm_accessories_select",
        "ares_firearm_accessories_to_accessorize", "ares_sell_item_from",
        "ares_sell_inventory", "combat_utilities_display",
        "combat_utilities", "defense_utilities_display", "defense_utilities",
        "mask_sense_utilities_display", "mask_sense_utilities",
        "cyberdecks_display", "cyberdecks", "cyberdeck_upgrades_display",
        "cyberdeck_upgrades", "cyberdeck_upgrades_second_display",
        "cyberdeck_upgrades_second", "magic_spells_display", "magic_spells",
        "magic_items_display", "magic_items", "magic_foci_display",
        "magic_foci", "sell_item_from_secondary",
        "sell_inventory_secondary"}};
    const auto service_action_titles =
        shadowrun::graphics::read_genesis_field_service_action_titles(rom);
    for (std::size_t index = 0; index < service_action_titles.size(); ++index) {
        const auto key = "shop.action_heading.option." +
                         std::string(kShopActionHeadingIds[index]);
        write_text_entry(key, service_action_titles[index]);
    }
    const auto shop_text =
        shadowrun::graphics::read_genesis_field_menu_action_text(rom);
    const auto universal_clips_text =
        shadowrun::graphics::read_genesis_field_service_universal_clips_text(
            rom);
    write_text_entry("shop.universal_clips.title",
                     universal_clips_text.title);
    write_text_entry("shop.universal_clips.description",
                     universal_clips_text.description);
    write_text_entry("shop.transaction.prompt",
                     shop_text.transaction_prompt);
    write_text_entry("shop.transaction.no_inventory_slot",
                     shop_text.no_inventory_slot_message);
    write_text_entry("shop.transaction.insufficient_funds",
                     shop_text.insufficient_funds_message);
    write_text_entry("shop.transaction.special_action_limit",
                     shop_text.special_action_limit_message);
    write_text_entry("shop.transaction.protected_sale",
                     shop_text.protected_sale_message);
    write_text_entry("shop.transaction.sale_quote_heading",
                     shop_text.sale_quote_heading);
    write_text_entry("shop.transaction.purchase_price_heading",
                     shop_text.purchase_price_heading);
    const auto item_detail_labels =
        shadowrun::graphics::read_genesis_field_menu_item_detail_labels(rom);
    constexpr std::array<std::string_view, 3> kItemDetailLabelKeys{{
        "items.detail_label.ammo", "items.detail_label.uses",
        "items.detail_label.rating"}};
    for (std::size_t index = 0; index < item_detail_labels.size(); ++index) {
        write_text_entry(kItemDetailLabelKeys[index], item_detail_labels[index]);
    }
    for (std::size_t index = 0; index < assets.inventory_items.size(); ++index) {
        const auto code = std::to_string(index + 1U);
        write_text_entry("items.name." + code,
                         assets.inventory_items[index].name);
        write_text_entry("items.category." + code,
                         assets.inventory_items[index].category);
    }
    const auto item_detail_texts =
        shadowrun::graphics::read_genesis_field_action_detail_texts(rom);
    for (std::size_t index = 0; index < item_detail_texts.size(); ++index) {
        write_text_entry("items.action_detail." + std::to_string(index),
                         item_detail_texts[index]);
    }
    write_text_entry("hotel.options.title", hotel_text.title);
    write_text_entry("hotel.options.separator", hotel_text.separator);
    write_text_entry("hotel.option.karma_screen", hotel_text.options[0]);
    write_text_entry("hotel.option.check_out", hotel_text.options[1]);
    write_text_entry(
        "hotel.checkout.title",
        hotel_text.checkout_question[0] + "|" +
            hotel_text.checkout_question[1]);
    write_text_entry("hotel.option.checkout_yes",
                     hotel_text.checkout_options[0]);
    write_text_entry("hotel.option.checkout_no",
                     hotel_text.checkout_options[2]);
    write_text_entry("hotel.status.medical_attention_suffix",
                     hotel_text.medical_attention_suffix);
    for (std::size_t index = 2U;
         index < hotel_text.recovery_message.size();
         ++index) {
        write_text_entry("hotel.status.recovery." + std::to_string(index),
                         hotel_text.recovery_message[index]);
    }
    const auto pocket_secretary_text =
        shadowrun::graphics::read_genesis_pocket_secretary_text(rom);
    write_text_entry("pocket_secretary.root.title",
                     pocket_secretary_text.title);
    write_text_entry("pocket_secretary.root.separator",
                     pocket_secretary_text.separator);
    constexpr std::array<std::string_view, 5> kPocketSecretaryFieldChoiceIds{{
        "notebook", "group_items", "cyberdeck", "dismiss_runner",
        "save_load"}};
    for (std::size_t index = 0;
         index < kPocketSecretaryFieldChoiceIds.size(); ++index) {
        write_text_entry(
            "pocket_secretary.option." +
                std::string(kPocketSecretaryFieldChoiceIds[index]),
            pocket_secretary_text.options[index]);
    }
    for (const auto& [key, text] : field_text) {
        write_text_entry(key, text);
    }
    output << "    },\n}\n";
    if (!output) {
        throw std::runtime_error("cannot write Lua dialogue content: " +
                                 path.string());
    }
}

struct DecodedSourceByteStream {
    std::vector<std::uint8_t> bytes;
    std::size_t source_end = 0;
};

DecodedSourceByteStream decode_source_byte_stream_record(
    const std::vector<std::uint8_t>& rom, const std::size_t source_offset) {
    // ROM 0x01610 uses a DBF count: the leading word describes the number of
    // decrements, so a valid stream produces count + 1 output bytes. Literal,
    // repeat, and back-reference commands are kept here as an offline import
    // operation; no source decoder belongs in the native runtime.
    const auto read_byte = [&rom](const std::size_t offset) -> std::uint8_t {
        if (offset >= rom.size()) {
            throw std::runtime_error("source byte stream exceeds ROM");
        }
        return rom[offset];
    };
    const auto read_word = [&read_byte](const std::size_t offset) {
        return static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(read_byte(offset)) << 8U) |
            static_cast<std::uint16_t>(read_byte(offset + 1U)));
    };

    std::size_t cursor = source_offset;
    int remaining = static_cast<int>(read_word(cursor++));
    cursor++;
    if (remaining < 0) {
        throw std::runtime_error("source byte stream count is invalid");
    }
    DecodedSourceByteStream result;
    result.bytes.reserve(static_cast<std::size_t>(remaining) + 1U);

    std::uint8_t flag_byte = 0;
    int flag_bits_remaining = 0;
    const auto emit = [&result](const std::uint8_t value) {
        result.bytes.push_back(value);
    };
    const auto finish_dbf = [&remaining]() {
        if (remaining == 0) {
            return true;
        }
        --remaining;
        return false;
    };

    while (true) {
        if (flag_bits_remaining == 0) {
            flag_byte = read_byte(cursor++);
            flag_bits_remaining = 8;
        }
        const bool compressed = (flag_byte & 0x80U) != 0U;
        flag_byte = static_cast<std::uint8_t>(flag_byte << 1U);
        --flag_bits_remaining;

        if (!compressed) {
            emit(read_byte(cursor++));
            if (finish_dbf()) {
                break;
            }
            continue;
        }

        const auto control = read_byte(cursor++);
        if ((control & 0x80U) == 0U) {
            const auto run_length = static_cast<int>((control & 0x7FU) + 2U);
            remaining -= run_length;
            if (remaining < 0) {
                throw std::runtime_error(
                    "source byte stream repeat exceeds declared output");
            }
            const auto value = read_byte(cursor++);
            for (int index = 0; index <= run_length; ++index) {
                emit(value);
            }
        } else {
            const auto distance_high = read_byte(cursor++);
            const auto distance = static_cast<std::size_t>(
                (((static_cast<std::uint16_t>(distance_high) << 3U) |
                  static_cast<std::uint16_t>((control >> 4U) & 0x07U)) &
                 0x07FFU));
            const auto run_length = static_cast<int>((control & 0x0FU) + 2U);
            remaining -= run_length;
            if (remaining < 0 || distance == 0U ||
                distance > result.bytes.size()) {
                throw std::runtime_error(
                    "source byte stream back-reference is invalid");
            }
            for (int index = 0; index <= run_length; ++index) {
                emit(result.bytes[result.bytes.size() - distance]);
            }
        }
        if (finish_dbf()) {
            break;
        }
    }

    result.source_end = cursor;
    return result;
}

std::vector<std::uint8_t> read_field_scene_metatile_record(
    const std::vector<std::uint8_t>& rom,
    const shadowrun::graphics::GenesisFieldModeResources& resources) {
    if (resources.metatile_encoding ==
        shadowrun::graphics::GenesisMetatileEncoding::CompressedResource) {
        // The source pointer is an offline extraction input. The native
        // runtime receives the expanded bytes and never runs 0x01610.
        return decode_source_byte_stream_record(
                   rom, resources.metatile_stream_offset)
            .bytes;
    }

    const auto map_entry_count = static_cast<std::size_t>(
        resources.tilemap_width) * resources.tilemap_height;
    std::size_t required_metatile_count = 0U;
    const auto scan_tilemap = [&](const std::size_t tilemap_offset) {
        if (tilemap_offset > rom.size() ||
            map_entry_count > (rom.size() - tilemap_offset) / 2U) {
            throw std::runtime_error(
                "field scene tilemap exceeds input image");
        }
        for (std::size_t entry = 0; entry < map_entry_count; ++entry) {
            const auto name_word = read_big_endian_word(
                rom, tilemap_offset + entry * sizeof(std::uint16_t));
            required_metatile_count = std::max(
                required_metatile_count,
                static_cast<std::size_t>(name_word) + 1U);
        }
    };
    scan_tilemap(resources.primary_tilemap_offset);
    scan_tilemap(resources.secondary_tilemap_offset);

    const auto byte_count = required_metatile_count * 32U;
    if (resources.metatile_stream_offset > rom.size() ||
        byte_count > rom.size() - resources.metatile_stream_offset) {
        throw std::runtime_error(
            "field scene raw metatile table exceeds input image");
    }
    return std::vector<std::uint8_t>(
        rom.begin() + static_cast<std::ptrdiff_t>(
                          resources.metatile_stream_offset),
        rom.begin() + static_cast<std::ptrdiff_t>(
                          resources.metatile_stream_offset + byte_count));
}

std::vector<std::uint8_t> read_field_scene_collision_records(
    const std::vector<std::uint8_t>& rom,
    const shadowrun::graphics::GenesisFieldModeResources& resources) {
    std::vector<std::uint8_t> records;
    auto cursor = resources.terrain_record_offset;
    for (;;) {
        const auto x_start = read_big_endian_word(rom, cursor);
        if ((x_start & 0x8000U) != 0U) {
            return records;
        }
        if (cursor > rom.size() || rom.size() - cursor < 10U) {
            throw std::runtime_error(
                "field scene terrain collision record is truncated");
        }
        records.insert(records.end(),
                       rom.begin() + static_cast<std::ptrdiff_t>(cursor),
                       rom.begin() + static_cast<std::ptrdiff_t>(cursor + 10U));
        cursor += 10U;
    }
}

std::string field_scene_record_directory(const std::uint16_t scene_id) {
    std::ostringstream result;
    result << "records/field/scenes/scene-" << std::setfill('0')
           << std::setw(2) << scene_id;
    return result.str();
}

void extract_field_session_record_assets(
    const std::vector<std::uint8_t>& rom,
    const std::filesystem::path& root) {
    // These are source records consumed by the converted field-session
    // bootstrap. They remain raw, individually named host records; the
    // conversion layer owns their typed interpretation.
    write_rom_record(rom, root,
                     "records/field/action-selection-profiles.bin",
                     0x166AD8U, 5U * 0x22U);
    write_rom_record(rom, root, "records/field/mode-selection-state.bin",
                     0x14C1F4U, 0x17U * sizeof(std::uint16_t));
    // 0x03084 preserves FFDDC4 in D0 after the 0x3032 classifier and indexes
    // all 56 entries at 0x3232. Materialize the complete 48-word rows once
    // so the conversion runtime can select a scene by typed id without
    // carrying a ROM pointer table.
    std::vector<std::uint8_t> scene_profiles;
    scene_profiles.reserve(shadowrun::rom::kFieldModeCount * 0x60U);
    std::vector<std::uint8_t> elevator_scene_groups;
    elevator_scene_groups.reserve(
        shadowrun::rom::kFieldModeCount * 6U);
    for (std::uint16_t scene_id = 0;
         scene_id < shadowrun::rom::kFieldModeCount; ++scene_id) {
        const auto table_entry =
            0x3232U + static_cast<std::size_t>(scene_id) * 4U;
        const auto profile_offset = static_cast<std::size_t>(
            read_big_endian_long(rom, table_entry));
        if (profile_offset > rom.size() ||
            rom.size() - profile_offset < 0x60U) {
            throw std::runtime_error(
                "field scene profile row exceeds input image");
        }
        scene_profiles.insert(
            scene_profiles.end(),
            rom.begin() + static_cast<std::ptrdiff_t>(profile_offset),
            rom.begin() +
                static_cast<std::ptrdiff_t>(profile_offset + 0x60U));

        // 0x58DE0 reads a scene group from the selected profile's +0x58
        // resource pointer: its first longword points to a word-counted list
        // of child scene IDs. Resolve the list now, preserving empty rows for
        // profiles without an elevator group so the runtime consumes only
        // typed native IDs.
        std::array<std::uint8_t, 6U> elevator_group{};
        const auto resource_bundle = static_cast<std::size_t>(
            read_big_endian_long(rom, profile_offset + 0x58U));
        if (resource_bundle != 0U && resource_bundle <= rom.size() &&
            rom.size() - resource_bundle >= sizeof(std::uint32_t)) {
            const auto group_offset = static_cast<std::size_t>(
                read_big_endian_long(rom, resource_bundle));
            if (group_offset != 0U && group_offset <= rom.size() &&
                rom.size() - group_offset >= sizeof(std::uint16_t)) {
                const auto count = read_big_endian_word(rom, group_offset);
                if (count >= 2U && count <= 5U &&
                    rom.size() - group_offset >=
                        (1U + static_cast<std::size_t>(count)) *
                            sizeof(std::uint16_t)) {
                    std::array<std::uint16_t, 5U> child_scenes{};
                    bool valid_group = true;
                    for (std::size_t child = 0; child < count; ++child) {
                        child_scenes[child] = read_big_endian_word(
                            rom, group_offset +
                                     (1U + child) * sizeof(std::uint16_t));
                        valid_group = valid_group &&
                            child_scenes[child] <
                                shadowrun::rom::kFieldModeCount;
                    }
                    if (valid_group) {
                        elevator_group[0] = static_cast<std::uint8_t>(count);
                        for (std::size_t child = 0; child < count; ++child) {
                            elevator_group[1U + child] =
                                static_cast<std::uint8_t>(child_scenes[child]);
                        }
                    }
                }
            }
        }
        elevator_scene_groups.insert(elevator_scene_groups.end(),
                                     elevator_group.begin(),
                                     elevator_group.end());

        // ROM 0x167AE resolves FFDE1E from the selected resource-scene
        // profile, dereferences the first bundle longword, and passes that
        // word to 0x0D3A before publishing FFE12C. Resolve this source chain
        // once into a named two-byte host value; the runtime never follows
        // the profile or bundle pointer.
        if (scene_id >= 0x15U && scene_id < 0x2CU) {
            const auto bundle_offset = static_cast<std::size_t>(
                read_big_endian_long(rom, profile_offset + 0x58U));
            if (bundle_offset == 0U || bundle_offset > rom.size() ||
                rom.size() - bundle_offset < 0x18U) {
                throw std::runtime_error(
                    "world activity scene has no complete resource bundle");
            }
            const auto bound_offset = static_cast<std::size_t>(
                read_big_endian_long(rom, bundle_offset));
            const auto bound = read_big_endian_word(rom, bound_offset);
            if (bound == 0U || bound > 6U) {
                throw std::runtime_error(
                    "world activity random-index bound changed");
            }
            const std::vector<std::uint8_t> bound_record{
                static_cast<std::uint8_t>(bound >> 8U),
                static_cast<std::uint8_t>(bound)};
            write_host_record(
                bound_record, root,
                field_scene_record_directory(scene_id) +
                    "/world-activity-random-index-bound.bin");
            const auto cursor_word = read_big_endian_word(
                rom, bundle_offset + 0x14U);
            const auto resource_index = read_big_endian_word(
                rom, bundle_offset + 0x16U);
            const std::vector<std::uint8_t> resource_cursor_record{
                static_cast<std::uint8_t>(cursor_word),
                static_cast<std::uint8_t>(resource_index >> 8U),
                static_cast<std::uint8_t>(resource_index)};
            write_host_record(
                resource_cursor_record, root,
                field_scene_record_directory(scene_id) +
                    "/world-activity-resource-cursor.bin");
        }
    }
    write_host_record(scene_profiles, root,
                      "records/field/scene-profiles.bin");
    write_host_record(elevator_scene_groups, root,
                      "records/field/elevator-scene-groups.bin");
    // ROM 0x0BABC loads the eight signed fixed-point movement vectors from
    // 0x0C002 before 0x0B77E probes the candidate position. Keep this as one
    // direct source record; the conversion loader owns its typed pairs.
    write_rom_record(
        rom, root, "records/field/entity-movement-direction-table.bin",
        0x0C002U, 8U * 4U);
    // ROM 0xBC36 selects one of four three-point collision-probe banks from
    // the signed FFEF10..12 sample bytes. Keep the complete 4 x 3 x 2-word
    // source table as one direct record; the conversion loader interprets the
    // big-endian signed offsets and the runtime never reads ROM addresses.
    write_rom_record(
        rom, root, "records/field/entity-collision-probe-offset-banks.bin",
        0x0BD34U, 4U * 3U * 2U * sizeof(std::uint16_t));
    // ROM 0x195C indexes this sixteen-word D-pad/heading table before the
    // movement heading is slewed and converted to a vector-table ordinal.
    write_rom_record(
        rom, root, "records/field/entity-controller-heading-table.bin",
        0x019D2U, 16U * sizeof(std::uint16_t));
    // ROM 0xBB86 selects one of four descriptors through the long-pointer
    // table at 0xBBF6. Each descriptor stores pointers to FFEF10..13 and two
    // fallback direction slots. Resolve those pointers once during offline
    // extraction into four six-byte semantic records:
    // primary sample index, secondary sample index, primary fallback, and
    // secondary fallback. The runtime never follows the source RAM pointers.
    const std::array<std::uint32_t, 4> route_sample_pointers{
        0xFFEF10U, 0xFFEF11U, 0xFFEF12U, 0xFFEF13U};
    std::vector<std::uint8_t> movement_route_table;
    movement_route_table.reserve(4U * 6U);
    for (std::size_t route_index = 0; route_index < 4U; ++route_index) {
        const auto descriptor_pointer = read_big_endian_long(
            rom, 0x0BBF6U + route_index * sizeof(std::uint32_t));
        if (descriptor_pointer < 0x0BC06U ||
            descriptor_pointer + 12U > rom.size()) {
            throw std::runtime_error(
                "field movement route descriptor pointer changed");
        }
        const auto primary_pointer =
            read_big_endian_long(rom, descriptor_pointer);
        const auto secondary_pointer =
            read_big_endian_long(rom, descriptor_pointer + 4U);
        const auto primary = std::find(
            route_sample_pointers.begin(), route_sample_pointers.end(),
            primary_pointer);
        const auto secondary = std::find(
            route_sample_pointers.begin(), route_sample_pointers.end(),
            secondary_pointer);
        if (primary == route_sample_pointers.end() ||
            secondary == route_sample_pointers.end()) {
            throw std::runtime_error(
                "field movement route sample pointer changed");
        }
        movement_route_table.push_back(static_cast<std::uint8_t>(
            std::distance(route_sample_pointers.begin(), primary)));
        movement_route_table.push_back(static_cast<std::uint8_t>(
            std::distance(route_sample_pointers.begin(), secondary)));
        const auto primary_fallback =
            read_big_endian_word(rom, descriptor_pointer + 8U);
        const auto secondary_fallback =
            read_big_endian_word(rom, descriptor_pointer + 10U);
        movement_route_table.push_back(
            static_cast<std::uint8_t>(primary_fallback >> 8U));
        movement_route_table.push_back(
            static_cast<std::uint8_t>(primary_fallback));
        movement_route_table.push_back(
            static_cast<std::uint8_t>(secondary_fallback >> 8U));
        movement_route_table.push_back(
            static_cast<std::uint8_t>(secondary_fallback));
    }
    write_host_record(movement_route_table, root,
                      "records/field/entity-movement-route-table.bin");
    // 0x03084/0x0313A can select every row in the 0x3232 scene table. Keep
    // the complete direct resource set available to the conversion layer:
    // fixed terrain work buffers, offline-expanded metatile bytes, the map
    // descriptor, collision records through the source sentinel, and the
    // optional event table. The runtime consumes these files by scene id and
    // never follows the source pointers or decodes 0x01610.
    std::filesystem::create_directories(root / "records/field/scenes");
    std::ofstream scene_index(
        root / "records/field/scenes/index.tsv", std::ios::binary);
    if (!scene_index) {
        throw std::runtime_error("cannot create field scene resource index");
    }
    scene_index
        << "scene_id\trecord_offset\tmetatile_stream\tmetatile_encoding"
           "\tlayer_a\tlayer_b\tterrain_stream\tdescriptor"
           "\tcollision_rectangles\tevent_table\n";
    for (std::uint16_t scene_id = 0;
         scene_id < shadowrun::rom::kFieldModeCount; ++scene_id) {
        const auto resources =
            shadowrun::graphics::read_genesis_field_mode_resources(rom, scene_id);
        const auto directory = field_scene_record_directory(scene_id);
        const auto layer_byte_count =
            0xE10U * sizeof(std::uint16_t);
        const auto layer_a = directory + "/terrain-layer-a.bin";
        const auto layer_b = directory + "/terrain-layer-b.bin";
        const auto terrain_stream = directory + "/terrain-stream.bin";
        const auto descriptor = directory + "/descriptor.bin";
        const auto collision = directory + "/terrain-collision-rectangles.bin";
        const auto event = directory + "/event-table.bin";
        write_rom_record(rom, root, layer_a,
                         resources.primary_tilemap_offset, layer_byte_count);
        write_rom_record(rom, root, layer_b,
                         resources.secondary_tilemap_offset, layer_byte_count);
        write_host_record(read_field_scene_metatile_record(rom, resources),
                          root, terrain_stream);
        write_rom_record(rom, root, descriptor, resources.record_offset, 0x58U);
        write_host_record(read_field_scene_collision_records(rom, resources),
                          root, collision);
        if (resources.event_table_offset != 0U) {
            write_rom_record(rom, root, event, resources.event_table_offset, 32U);
        }
        scene_index << scene_id << "\t0x" << std::uppercase << std::hex
                    << resources.record_offset << "\t0x"
                    << resources.metatile_stream_offset << "\t"
                    << (resources.metatile_encoding ==
                                shadowrun::graphics::GenesisMetatileEncoding::
                                    RawNameWords
                            ? "raw-name-words"
                            : "compressed-resource")
                    << std::dec << "\t" << layer_a << "\t" << layer_b << "\t"
                    << terrain_stream << "\t" << descriptor << "\t" << collision;
        if (resources.event_table_offset != 0U) {
            scene_index << "\t" << event;
        }
        scene_index << '\n';
    }
}

struct CroppedAsset {
    shadowrun::graphics::RgbaImage image;
    std::int32_t origin_x = 0;
    std::int32_t origin_y = 0;
};

CroppedAsset crop_transparent_bounds(
    const shadowrun::graphics::RgbaImage& source) {
    if (source.width == 0 || source.height == 0 || source.pixels.empty()) {
        throw std::invalid_argument("cannot crop an empty asset");
    }
    std::int32_t left = source.width;
    std::int32_t top = source.height;
    std::int32_t right = -1;
    std::int32_t bottom = -1;
    for (std::int32_t y = 0; y < source.height; ++y) {
        for (std::int32_t x = 0; x < source.width; ++x) {
            const auto& pixel = source.pixels[
                static_cast<std::size_t>(y) * source.width + x];
            if (pixel.alpha == 0U) {
                continue;
            }
            left = std::min(left, x);
            top = std::min(top, y);
            right = std::max(right, x);
            bottom = std::max(bottom, y);
        }
    }
    if (right < left || bottom < top) {
        throw std::runtime_error("asset contains no opaque pixels");
    }
    CroppedAsset result;
    result.origin_x = left;
    result.origin_y = top;
    result.image.width = static_cast<std::uint16_t>(right - left + 1);
    result.image.height = static_cast<std::uint16_t>(bottom - top + 1);
    result.image.pixels.resize(static_cast<std::size_t>(result.image.width) *
                               result.image.height);
    for (std::int32_t y = top; y <= bottom; ++y) {
        for (std::int32_t x = left; x <= right; ++x) {
            result.image.pixels[
                static_cast<std::size_t>(y - top) * result.image.width +
                (x - left)] = source.pixels[
                    static_cast<std::size_t>(y) * source.width + x];
        }
    }
    return result;
}

CroppedAsset crop_source_region(
    const shadowrun::graphics::RgbaImage& source, const std::int32_t origin_x,
    const std::int32_t origin_y, const std::uint16_t width,
    const std::uint16_t height) {
    if (width == 0U || height == 0U || origin_x < 0 || origin_y < 0 ||
        origin_x + width > source.width || origin_y + height > source.height) {
        throw std::invalid_argument("source asset region exceeds image");
    }
    CroppedAsset result;
    result.origin_x = origin_x;
    result.origin_y = origin_y;
    result.image.width = width;
    result.image.height = height;
    result.image.pixels.resize(static_cast<std::size_t>(width) * height);
    for (std::int32_t y = 0; y < height; ++y) {
        for (std::int32_t x = 0; x < width; ++x) {
            result.image.pixels[static_cast<std::size_t>(y) * width + x] =
                source.pixels[static_cast<std::size_t>(origin_y + y) *
                                  source.width + origin_x + x];
        }
    }
    return result;
}

void write_scene_asset(const std::filesystem::path& root,
                       const std::string& relative_name,
                       const shadowrun::graphics::RgbaImage& source,
                       std::ofstream& manifest, bool crop) {
    const auto asset = crop ? crop_transparent_bounds(source)
                            : CroppedAsset{source, 0, 0};
    const auto path = root / relative_name;
    std::filesystem::create_directories(path.parent_path());
    shadowrun::graphics::write_bmp32(path.string(), asset.image);
    manifest << relative_name << '\t' << asset.origin_x << '\t'
             << asset.origin_y << '\t' << asset.image.width << '\t'
             << asset.image.height << '\n';
}

void write_circuit_backgrounds(
    const std::filesystem::path& root,
    const std::array<shadowrun::graphics::RgbaImage, 7>& phases,
    std::ofstream& manifest) {
    // Each decoder result is already the complete 320x224 background with
    // one ROM-selected circuit-light phase applied. Store those seven frames
    // directly so runtime animation is a small frame selection, not a
    // reconstruction from hundreds of cropped fragments.
    for (std::size_t phase_index = 0; phase_index < phases.size();
         ++phase_index) {
        write_scene_asset(root,
                          "title/circuit-background-" +
                              std::to_string(phase_index) + ".bmp",
                          phases[phase_index], manifest, false);
    }
}

void remove_legacy_circuit_fragments(const std::filesystem::path& root) {
    const auto title_root = root / "title";
    if (!std::filesystem::exists(title_root)) {
        return;
    }
    for (const auto& entry :
         std::filesystem::directory_iterator(title_root)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        const auto name = entry.path().filename().string();
        if (name.rfind("circuit-light-", 0) != 0 ||
            entry.path().extension() != ".bmp") {
            continue;
        }
        std::error_code error;
        std::filesystem::remove(entry.path(), error);
        if (error) {
            throw std::runtime_error(
                "cannot remove legacy circuit light fragment: " +
                entry.path().string());
        }
    }
}

void extract_title_boot_assets(const std::vector<std::uint8_t>& rom,
                               const std::filesystem::path& root) {
    std::filesystem::create_directories(root);
    std::ofstream manifest(root / "manifest.tsv", std::ios::binary);
    if (!manifest) {
        throw std::runtime_error("cannot create title/boot asset manifest");
    }
    manifest << "asset\tx\ty\twidth\theight\n";

    const auto title_background =
        shadowrun::graphics::decode_genesis_title_background(rom);
    write_scene_asset(root, "title/background.bmp", title_background,
                      manifest, false);
    const auto title_lights =
        shadowrun::graphics::decode_genesis_title_circuit_lights(rom);
    remove_legacy_circuit_fragments(root);
    write_circuit_backgrounds(root, title_lights, manifest);
    write_scene_asset(root, "title/logo.bmp",
                      shadowrun::graphics::decode_genesis_title_logo_plane(rom),
                      manifest, true);
    write_scene_asset(root, "title/text.bmp",
                      shadowrun::graphics::decode_genesis_title_text(rom),
                      manifest, true);
    const auto title_sprites =
        shadowrun::graphics::decode_genesis_title_sprite_frames(rom);
    for (std::size_t index = 0; index < title_sprites.size(); ++index) {
        write_scene_asset(root,
                          "title/sprite-" + std::to_string(index) + ".bmp",
                          title_sprites[index], manifest, true);
    }

    write_scene_asset(root, "boot/snow.bmp",
                      shadowrun::graphics::decode_genesis_boot_static(rom),
                      manifest, false);
    write_scene_asset(root, "boot/sega-background.bmp",
                      shadowrun::graphics::decode_genesis_boot_background(rom),
                      manifest, true);
    write_scene_asset(root, "boot/guy.bmp",
                      shadowrun::graphics::decode_genesis_boot_guy(rom),
                      manifest, true);
    const auto sega = shadowrun::graphics::decode_genesis_boot_sega_frames(rom);
    for (std::size_t index = 0; index < sega.size(); ++index) {
        write_scene_asset(root,
                          "boot/sega-lightning-" + std::to_string(index) +
                              ".bmp",
                          sega[index], manifest, true);
    }
    const auto strobe =
        shadowrun::graphics::decode_genesis_boot_logo_strobe(rom);
    for (std::size_t index = 0; index < strobe.size(); ++index) {
        write_scene_asset(root,
                          "boot/sega-strobe-" + std::to_string(index) +
                              ".bmp",
                          strobe[index], manifest, true);
    }
    write_scene_asset(root, "boot/fasa.bmp",
                      shadowrun::graphics::decode_genesis_boot_fasa(rom),
                      manifest, true);
}

shadowrun::graphics::RgbaImage to_rgba_image(
    const shadowrun::native::NativeImage& source) {
    return {source.width, source.height, source.pixels};
}

std::string format_resource_source(
    const shadowrun::graphics::GenesisGraphicsResource& resource) {
    std::ostringstream output;
    output << "table=0x" << std::hex << resource.table_offset << " index="
           << std::dec << resource.table_index << " sprite="
           << resource.sprite_index << " source=0x" << std::hex
           << resource.source_offset << " attr=0x" << resource.attr_or;
    return output.str();
}

void write_catalog_asset(const std::filesystem::path& root,
                         const std::string& relative_name,
                         const shadowrun::graphics::RgbaImage& image,
                         const std::string& kind,
                         const std::string& provenance, bool crop,
                         std::ofstream& manifest, std::size_t& written,
                         std::size_t& missing, std::size_t& blank,
                         const std::int32_t source_origin_x = 0,
                         const std::int32_t source_origin_y = 0,
                         const std::uint16_t source_width = 0,
                         const std::uint16_t source_height = 0) {
    if (image.width == 0U || image.height == 0U || image.pixels.empty()) {
        ++missing;
        manifest << relative_name << '\t' << kind
                 << "\t0\t0\t0\t0\t" << provenance << "\tmissing\n";
        return;
    }
    auto status = "extracted";
    CroppedAsset asset;
    if (source_width != 0U && source_height != 0U) {
        asset = crop_source_region(image, source_origin_x, source_origin_y,
                                   source_width, source_height);
    } else if (crop) {
        try {
            asset = crop_transparent_bounds(image);
            // Descriptor origins locate the uncropped frame relative to its
            // world anchor. Cropping removes transparent pixels from the
            // image, so carry both offsets into the direct-asset catalog.
            asset.origin_x += source_origin_x;
            asset.origin_y += source_origin_y;
        } catch (const std::runtime_error& error) {
            if (std::string(error.what()) != "asset contains no opaque pixels") {
                throw;
            }
            // A state-specific source can legitimately be fully transparent
            // (for example, an empty initial HUD counter). Preserve it as an
            // individual transparent sprite instead of silently dropping the
            // resource or treating it as a decoder failure.
            asset = CroppedAsset{image, source_origin_x, source_origin_y};
            status = "extracted-empty";
            ++blank;
        }
    } else {
        asset = CroppedAsset{image, source_origin_x, source_origin_y};
    }
    const auto path = root / relative_name;
    std::filesystem::create_directories(path.parent_path());
    shadowrun::graphics::write_bmp32(path.string(), asset.image);
    manifest << relative_name << '\t' << kind << '\t' << asset.origin_x
             << '\t' << asset.origin_y << '\t' << asset.image.width << '\t'
             << asset.image.height << '\t' << provenance << '\t' << status
             << '\n';
    ++written;
}

void append_title_boot_catalog(const std::filesystem::path& root,
                               std::ofstream& manifest) {
    std::ifstream source(root / "manifest.tsv", std::ios::binary);
    if (!source) {
        throw std::runtime_error("title/boot manifest was not generated");
    }
    std::string line;
    std::getline(source, line);
    while (std::getline(source, line)) {
        if (line.empty()) {
            continue;
        }
        const auto kind = line.find("background.bmp") != std::string::npos ||
                line.find("circuit-background-") != std::string::npos ||
                line.find("snow.bmp") != std::string::npos
            ? "background"
            : "sprite";
        manifest << line.substr(0, line.find('\t')) << '\t' << kind
                 << '\t' << line.substr(line.find('\t') + 1)
                 << '\t' << "ROM title/boot resource\textracted\n";
    }
}

void write_asset_readme(const std::filesystem::path& root) {
    std::ofstream readme(root / "README.md", std::ios::binary);
    if (!readme) {
        throw std::runtime_error("cannot create direct asset README");
    }
    readme << "# Direct Shadowrun assets\n\n"
           << "Generated from the US Genesis ROM by "
              "shadowrun_graphics_extract --all-assets.\n\n"
           << "The native and Ghidra-conversion runtimes consume these "
              "individually named BMPs, direct text records, native/state.bin, "
              "and resources.tsv directly. This tree deliberately contains "
              "semantic folders: menu, intro, field, and item categories. "
              "Static intro plates and independently extracted intro sprites "
              "remain separate; "
              "it does not contain full-screen flipbook samples or an aggregate "
              "serializer. All ROM-derived dialogue, field-menu labels, "
              "startup menu copy, feedback, and Notebook Tips are keyed in "
              "`dialogue/script-text.lua`; `menu/field/service-routes.tsv` "
              "retains only "
              "numeric service destinations. Command labels, menu dialogue, "
              "service titles, shop category headings, item names/categories/details, "
              "startup labels/messages, and transaction copy are emitted in "
              "`dialogue/script-text.lua`; `gameplay/scripts/shop.lua` binds "
              "the 12 service titles, 32 source-order action headings, Ares "
              "categories, and message keys, while "
              "`gameplay/scripts/items.lua` maps source item IDs to their "
              "name/category/detail keys. The spell-capable Magic panel is an individually "
              "named RGBA overlay. Item art is grouped under items/weapons, "
              "items/first-aid, items/miscellaneous, items/magic, and items/armor. "
              "The shared field dialogue frames and A/B/C marker sprites are "
              "grouped under dialogue/panel, with speaker portraits under "
              "dialogue/portraits. ROM-derived Stoker and field interaction "
              "text, Hotel menu and status text, Ares mode-event dialogue, "
              "Pocket Secretary root/Notebook menu text, field-service titles, shop "
              "text, and item descriptions are emitted as "
              "`dialogue/script-text.lua` and consumed by the gameplay script "
              "runtime. `gameplay/scripts/hotel.lua`, "
              "`gameplay/scripts/pocket_secretary.lua`, "
              "`gameplay/scripts/clips.lua`, and "
              "`gameplay/scripts/startup.lua` provide cursor-menu choice IDs; "
              "`gameplay/scripts/ui.lua` provides shared confirmation choices. "
              "`gameplay/scripts/magic.lua` provides ROM-ordered spell IDs. "
              "Separately traced cinematic resources belong under movies/<scene>; "
              "they must not be mixed into intro or field assets.\n\n"
           << "Audio is emitted here only as complete Windows-playable lossless "
              "FLAC files under\n"
              "`audio/`. The offline Z80/VGM extraction sources and intermediate "
              "event data\n"
              "remain outside this runtime asset tree. Verified repeating audio "
              "uses\n"
              "`audio/loop-points.tsv` for exact PCM frame boundaries. Use "
              "`resources.tsv` for stable aliases,\n"
              "such as `assets.audio_path(\"audio.intro.title.stage-1\")`; do "
              "not add raw ROM\n"
              "slices or split event fragments here. A missing loop-point entry "
              "is\n"
              "intentional: the native player must not guess a repeat boundary "
              "from an\n"
              "internal channel restart.\n\n"
           << "Startup/session records are imported separately under "
               "`records/startup/` by `shadowrun_ghidra_startup_record_extract` "
               "and listed in `resources.tsv`. Field startup records remain "
               "individually named under `records/field/`; the complete 56-row "
               "scene profile table is `records/field/scene-profiles.bin`, and "
               "the complete field resource set is grouped under "
               "`records/field/scenes/scene-00` through `scene-55`. Each scene "
               "has its fixed terrain layers, offline-expanded metatile bytes, "
               "descriptor, collision records, and optional event table listed "
               "by a stable alias in `resources.tsv`. The conversion loader "
               "interprets each into typed field state and host collision maps "
               "without making the runtime read ROM addresses. The source "
               "movement vector table at `0xC002` is likewise one named direct "
               "record consumed as typed directional offsets by the conversion "
               "layer. The `0xBD34` collision-probe offset banks are kept as "
               "one four-bank direct record beside it. The `0x19D2` "
               "controller-heading table is kept beside it "
               "as one sixteen-word direct record. The `0xBBF6` aligned-route "
               "table is kept as four six-byte semantic records after its "
               "FFEF10..13 sample pointers are resolved offline.\n";
}

void write_runtime_resource_manifest(const std::filesystem::path& root) {
    std::ifstream catalog(root / "catalog.tsv", std::ios::binary);
    if (!catalog) {
        throw std::runtime_error("cannot read generated asset catalog");
    }
    // Graphics extraction is an explicit import step, but it must not erase
    // independently imported runtime resources. Preserve existing state and
    // audio rows while regenerating the image aliases from catalog.tsv.
    std::vector<std::string> preserved_state_rows;
    std::vector<std::string> preserved_runtime_rows;
    std::ifstream prior_manifest(
        root / std::string(shadowrun::resources::kManifestFile),
        std::ios::binary);
    if (prior_manifest) {
        std::string prior_line;
        std::getline(prior_manifest, prior_line);
        while (std::getline(prior_manifest, prior_line)) {
            if (prior_line.empty()) {
                continue;
            }
            std::vector<std::string> fields;
            std::size_t start = 0;
            for (;;) {
                const auto end = prior_line.find('\t', start);
                fields.push_back(prior_line.substr(
                    start, end == std::string::npos ? std::string::npos
                                                     : end - start));
                if (end == std::string::npos) {
                    break;
                }
                start = end + 1U;
            }
            const auto legacy_field_scene_record =
                fields.size() == 5U &&
                (fields[0].rfind("records.field.mode.0.", 0U) == 0U ||
                 fields[0].rfind("records.field.mode.2.", 0U) == 0U);
            if (fields.size() == 5U && fields[0] != "state" &&
                fields[2] != "image" && !legacy_field_scene_record &&
                std::filesystem::is_regular_file(root / fields[1])) {
                if (fields[2] == "state") {
                    preserved_state_rows.push_back(prior_line);
                } else {
                    preserved_runtime_rows.push_back(prior_line);
                }
            }
        }
    }
    std::ofstream output(root / std::string(
                              shadowrun::resources::kManifestFile),
                         std::ios::binary);
    if (!output) {
        throw std::runtime_error("cannot create runtime resource manifest");
    }
    output << "alias\tpath\tkind\torigin_x\torigin_y\n"
           << "state\tnative/state.bin\tstate\t0\t0\n";
    std::unordered_set<std::string> aliases{"state"};
    const auto emit_preserved_rows = [&output, &aliases](
                                         const std::vector<std::string>& rows) {
        for (const auto& preserved : rows) {
            const auto separator = preserved.find('\t');
            if (separator == std::string::npos) {
                continue;
            }
            const auto alias = preserved.substr(0, separator);
            if (aliases.insert(alias).second) {
                output << preserved << '\n';
            }
        }
    };
    emit_preserved_rows(preserved_state_rows);
    const auto emit_generated_state_row = [&output, &aliases, &root](
                                              const std::string& alias,
                                              const std::string& relative) {
        if (!std::filesystem::is_regular_file(root / relative)) {
            throw std::runtime_error("generated source record is missing: " +
                                     std::string(relative));
        }
        if (aliases.insert(alias).second) {
            output << alias << '\t' << relative << "\tstate\t0\t0\n";
        }
    };
    emit_generated_state_row(
        "records.field.action.selection.profiles",
        "records/field/action-selection-profiles.bin");
    emit_generated_state_row(
        "records.field.mode.selection.state",
        "records/field/mode-selection-state.bin");
    emit_generated_state_row(
        "records.field.scene.profiles",
        "records/field/scene-profiles.bin");
    emit_generated_state_row(
        "records.field.elevator.scene.groups",
        "records/field/elevator-scene-groups.bin");
    emit_generated_state_row(
        "records.field.entity.movement.direction.table",
        "records/field/entity-movement-direction-table.bin");
    emit_generated_state_row(
        "records.field.entity.collision.probe.offset.banks",
        "records/field/entity-collision-probe-offset-banks.bin");
    emit_generated_state_row(
        "records.field.entity.controller.heading.table",
        "records/field/entity-controller-heading-table.bin");
    emit_generated_state_row(
        "records.field.entity.movement.route.table",
        "records/field/entity-movement-route-table.bin");
    for (std::uint16_t scene_id = 0;
         scene_id < shadowrun::rom::kFieldModeCount; ++scene_id) {
        const auto directory = field_scene_record_directory(scene_id);
        const auto emit_scene_record = [&](const std::string& suffix) {
            const auto relative = directory + "/" + suffix + ".bin";
            emit_generated_state_row(
                shadowrun::resources::alias_for_path(relative), relative);
        };
        emit_scene_record("terrain-layer-a");
        emit_scene_record("terrain-layer-b");
        emit_scene_record("terrain-stream");
        emit_scene_record("descriptor");
        emit_scene_record("terrain-collision-rectangles");
        if (scene_id >= 0x15U && scene_id < 0x2CU) {
            emit_scene_record("world-activity-random-index-bound");
            emit_scene_record("world-activity-resource-cursor");
        }
        const auto event_relative = directory + "/event-table.bin";
        if (std::filesystem::is_regular_file(root / event_relative)) {
            emit_generated_state_row(
                shadowrun::resources::alias_for_path(event_relative),
                event_relative);
        }
    }
    // Startup/session records are extracted by the separate conversion-side
    // record tool, but once present they must remain first-class manifest rows
    // even if --all-assets is rerun from the native graphics importer.
    const auto emit_optional_state_row = [&output, &aliases, &root](
                                             const char* alias,
                                             const char* relative) {
        if (std::filesystem::is_regular_file(root / relative) &&
            aliases.insert(alias).second) {
            output << alias << '\t' << relative << "\tstate\t0\t0\n";
        }
    };
    emit_optional_state_row(
        "records.startup.joshua.archetypes",
        "records/startup/joshua-archetypes.bin");
    emit_optional_state_row(
        "records.startup.recruited.runners",
        "records/startup/recruited-runner-records.bin");
    emit_optional_state_row(
        "records.startup.save.roster.header",
        "records/startup/save-roster-header.bin");
    const auto emit_generated_resource_row = [&output, &aliases, &root](
                                                const char* alias,
                                                const char* relative,
                                                const char* kind) {
        if (!std::filesystem::is_regular_file(root / relative)) {
            throw std::runtime_error("generated runtime resource is missing: " +
                                     std::string(relative));
        }
        if (aliases.insert(alias).second) {
            output << alias << '\t' << relative << '\t' << kind << "\t0\t0\n";
        }
    };
    emit_generated_resource_row(
        "menu.field.service.routes", "menu/field/service-routes.tsv", "text");
    emit_generated_resource_row(
        "field.entities.default.actors",
        "field/entities/default-actors.tsv", "state");
    std::string line;
    std::getline(catalog, line);
    while (std::getline(catalog, line)) {
        if (line.empty()) {
            continue;
        }
        std::vector<std::string> fields;
        std::size_t start = 0;
        for (;;) {
            const auto end = line.find('\t', start);
            fields.push_back(line.substr(
                start, end == std::string::npos ? std::string::npos
                                                 : end - start));
            if (end == std::string::npos) {
                break;
            }
            start = end + 1U;
        }
        if (fields.size() < 8U) {
            throw std::runtime_error("invalid generated asset catalog row: " +
                                     line);
        }
        const auto& relative = fields[0];
        if (relative.find("-gray-") != std::string::npos) {
            continue;
        }
        const auto alias = shadowrun::resources::alias_for_path(relative);
        if (!aliases.insert(alias).second) {
            throw std::runtime_error("duplicate generated resource alias: " +
                                     alias);
        }
        output << alias << '\t' << relative << "\timage\t" << fields[2]
               << '\t' << fields[3] << '\n';
    }
    emit_preserved_rows(preserved_runtime_rows);
}

void extract_all_assets(const std::vector<std::uint8_t>& rom,
                        const std::filesystem::path& root) {
    std::filesystem::create_directories(root);
    remove_obsolete_menu_text_assets(root);
    extract_title_boot_assets(rom, root);

    std::ofstream manifest(root / "catalog.tsv", std::ios::binary);
    if (!manifest) {
        throw std::runtime_error("cannot create all-resource catalog");
    }
    manifest << "asset\tkind\tx\ty\twidth\theight\tsource\tstatus\n";
    write_asset_readme(root);
    append_title_boot_catalog(root, manifest);
    std::size_t written = 0;
    std::size_t missing = 0;
    std::size_t blank = 0;
    const auto assets = shadowrun::graphics::decode_genesis_native_assets(rom);
    auto field_text = write_field_service_routes(rom, root, assets);
    auto notebook_tips = read_notebook_tips_script_text(assets);
    field_text.insert(field_text.end(), notebook_tips.begin(),
                      notebook_tips.end());
    write_dialogue_text_script(rom, root, assets, field_text);
    const auto write = [&](const std::string& name,
                           const shadowrun::native::NativeImage& image,
                           const std::string& kind,
                           const std::string& source, bool crop) {
        write_catalog_asset(root, name, to_rgba_image(image), kind, source,
                            crop, manifest, written, missing, blank);
    };
    const auto write_frame = [&](const std::string& name,
                                 const shadowrun::native::NativeEntityFrame& frame,
                                 const std::string& kind,
                                 const std::string& source, bool crop) {
        write(name,
              shadowrun::native::NativeImage{frame.width, frame.height,
                                             frame.pixels},
              kind, source, crop);
    };
    const auto write_array = [&](const std::string& prefix,
                                 const auto& images, const std::string& kind,
                                 const std::string& source, bool crop) {
        for (std::size_t index = 0; index < images.size(); ++index) {
            write(prefix + std::to_string(index) + ".bmp", images[index], kind,
                  source + "[" + std::to_string(index) + "]", crop);
        }
    };
    const auto write_rgba_array = [&](const std::string& prefix,
                                      const auto& images,
                                      const std::string& kind,
                                      const std::string& source, bool crop) {
        for (std::size_t index = 0; index < images.size(); ++index) {
            write_catalog_asset(
                root, prefix + std::to_string(index) + ".bmp", images[index],
                kind, source + "[" + std::to_string(index) + "]", crop,
                manifest, written, missing, blank);
        }
    };

    // Startup and opening source layers.
    write_array("title/animation-", assets.title_animation_frames, "background",
                "native title source composition", false);
    write("menu/startup/background.bmp", assets.startup_background, "background",
          "0x10B5A/0xCDCE6", false);
    write("menu/startup/font.bmp", assets.startup_font, "sprite", "0xC1320", true);
    write("menu/startup/cursor.bmp", assets.startup_cursor, "sprite", "0xD29F6", true);
    write("intro/aircraft-plane-a.bmp", assets.intro_aircraft_plane_a, "sprite",
          "0x5B528/0x5B7D8", true);
    write("intro/trivid-background.bmp", assets.intro_trivid_background,
          "background", "0x5AA96", false);
    const auto intro_backgrounds =
        shadowrun::graphics::decode_genesis_intro_backgrounds(rom);
    for (std::size_t index = 0; index < intro_backgrounds.size(); ++index) {
        write_catalog_asset(root, "intro/background-" + std::to_string(index) +
                                  ".bmp",
                            intro_backgrounds[index], "background", "0x59C5E",
                            false, manifest, written, missing, blank);
    }
    const auto intro_sprites =
        shadowrun::graphics::decode_genesis_intro_sprite_resources(rom);
    for (std::size_t index = 0; index < intro_sprites.size(); ++index) {
        const auto& resource = intro_sprites[index];
        write_catalog_asset(
            root, "intro/sprites/" + resource.role + "-" +
                      std::to_string(resource.table_index) + "-" +
                      std::to_string(resource.sprite_index) + ".bmp",
            resource.image, "sprite",
            format_resource_source(resource),
            true, manifest, written, missing, blank, resource.source_origin_x,
            resource.source_origin_y, resource.source_width,
            resource.source_height);
    }
    const auto intro_patches =
        shadowrun::graphics::decode_genesis_intro_ambush_background_resources(
            rom);
    for (std::size_t index = 0; index < intro_patches.size(); ++index) {
        const auto& resource = intro_patches[index];
        write_catalog_asset(
            root, "intro/background-patch-" + std::to_string(index) + ".bmp",
            resource.image, "background",
            format_resource_source(resource),
            true, manifest, written, missing, blank);
    }
    write("intro/cab-map.bmp", assets.cab_map, "background", "0x134D2/0xD2A2E",
          false);
    // The cab selection outline is the 0xD4446 eight-tile border stamped by
    // 0x13726; it is a live composition per destination, not part of the
    // map background.
    {
        const auto cursor_tiles =
            shadowrun::graphics::decode_genesis_cab_cursor_tiles(rom);
        constexpr std::array<std::string_view, 8> kCursorNames{{
            "top-left", "top-middle", "top-right", "left", "right",
            "bottom-left", "bottom-middle", "bottom-right"}};
        for (std::size_t index = 0; index < cursor_tiles.tiles.size();
             ++index) {
            const auto& tile = cursor_tiles.tiles[index];
            // Keep the full 8x8 tile cell; the border compositor places
            // these on the 8-pixel grid and a cropped sliver would carry an
            // origin the grid drawing does not apply.
            write_frame("intro/cab/cursor-" +
                            std::string(kCursorNames[index]) + ".bmp",
                        shadowrun::native::NativeEntityFrame{
                            tile.width, tile.height, tile.pixels},
                        "sprite", "0xD4446/0x13726", false);
        }
    }

    // 0x1F65A selects profile 0 for Stoker's Coffin Motel. The portrait is
    // written by the shared 7x9 resource stream at 0x122420 after its
    // profile-specific pattern upload at 0x1287C6. Keep this as one direct
    // sprite asset; the Qt conversion owns only its screen placement.
    write_catalog_asset(
        root, "dialogue/portraits/stoker-motel.bmp",
        shadowrun::graphics::decode_genesis_field_dialogue_portrait(rom, 0U),
        "sprite", "0x1F65A/0x121FA0/0x1287C6/0x122420", false, manifest,
        written, missing, blank);
    write_catalog_asset(
        root, "dialogue/panel/frame.bmp",
        shadowrun::graphics::decode_genesis_field_dialogue_panel_background(
            rom, false),
        "background", "0x10984/0x10918/0xCAB0C/0xCECD8-0xCEFA4", false,
        manifest, written, missing, blank);
    write_catalog_asset(
        root, "dialogue/panel/frame-alternate-left-panel.bmp",
        shadowrun::graphics::decode_genesis_field_dialogue_panel_background(
            rom, true),
        "background", "0x10984/0x10918/0xCAB0C/0xCEFF0 (FFF0AF)", false,
        manifest, written, missing, blank);
    constexpr std::array<std::string_view, 3> kDialogueButtonFiles{{
        "dialogue/panel/button-a.bmp",
        "dialogue/panel/button-b.bmp",
        "dialogue/panel/button-c.bmp",
    }};
    const auto dialogue_buttons =
        shadowrun::graphics::decode_genesis_field_dialogue_option_markers(rom);
    for (std::size_t index = 0; index < dialogue_buttons.size(); ++index) {
        write_catalog_asset(
            root, std::string(kDialogueButtonFiles[index]),
            dialogue_buttons[index], "sprite",
            "0x1F3C4/0x1232/0xCF11C-0xCF12C", false,
            manifest, written, missing, blank);
    }

    // Field HUD and menu source layers.
    write("field/hud/background.bmp", assets.field_hud, "background",
          "0x334C/0x39F2", false);
    const auto ares_mode_sprite =
        shadowrun::graphics::decode_genesis_field_mode_profile_sprite(
            rom, 0x1220C0U);
    write("field/mode-entry/ares-sprite.bmp",
          shadowrun::native::NativeImage{ares_mode_sprite.width,
                                         ares_mode_sprite.height,
                                         ares_mode_sprite.pixels},
          "sprite", "0x1FA78/0x1F246/0x1F292/0x1220C0", true);
    write_array("field/hud/face-", assets.field_hud_faces, "sprite", "0x3404",
                true);
    write_array("field/hud/item-", assets.field_hud_item_blocks, "sprite",
                "0x3404", true);
    write_array("field/hud/unarmed-item-",
                assets.field_hud_unarmed_item_blocks, "sprite",
                "0x3404/0xD1860-0xD1878", true);
    const auto field_hud_party =
        shadowrun::graphics::decode_genesis_field_hud_party_assets(rom);
    for (std::size_t family = 0;
         family < field_hud_party.portraits.size(); ++family) {
        for (std::size_t palette = 0;
             palette < field_hud_party.portraits[family].size(); ++palette) {
            write_catalog_asset(
                root,
                "field/hud/party/face-" + std::to_string(family) +
                    "-palette-" + std::to_string(palette) + ".bmp",
                field_hud_party.portraits[family][palette], "sprite",
                "0x3404/0x35D4/0x3602", false, manifest, written, missing,
                blank);
        }
    }
    for (std::size_t item = 0; item < field_hud_party.items.size(); ++item) {
        for (std::size_t palette = 0;
             palette < field_hud_party.items[item].size(); ++palette) {
            write_catalog_asset(
                root,
                "field/hud/party/item-" + std::to_string(item + 1U) +
                    "-palette-" + std::to_string(palette) + ".bmp",
                field_hud_party.items[item][palette], "sprite",
                "0x3404/0xD4540/0x35F0", false, manifest, written, missing,
                blank);
        }
    }
    for (std::size_t item = 0;
         item < field_hud_party.unarmed_items.size(); ++item) {
        for (std::size_t palette = 0;
             palette < field_hud_party.unarmed_items[item].size(); ++palette) {
            write_catalog_asset(
                root,
                "field/hud/party/unarmed-item-" + std::to_string(item) +
                    "-palette-" + std::to_string(palette) + ".bmp",
                field_hud_party.unarmed_items[item][palette], "sprite",
                "0x3404/0xD1860-0xD1878/0x35F0", false, manifest, written,
                missing, blank);
        }
    }
    for (std::size_t spell = 0;
         spell < assets.field_hud_magic_item_blocks.size(); ++spell) {
        for (std::size_t palette = 0;
             palette < assets.field_hud_magic_item_blocks[spell].size();
             ++palette) {
            const auto filename = "field/hud/magic-item-" +
                std::to_string(spell) + "-palette-" +
                std::to_string(palette) + ".bmp";
            write(filename,
                  assets.field_hud_magic_item_blocks[spell][palette],
                  "sprite", "0x3404/0xD4EA0/0x3602", true);
        }
    }
    write_array("field/hud/delay-meter-",
                assets.field_hud_delay_meter_counters, "sprite",
                "0x3674/0x6AB3E", true);
    for (std::size_t profile = 0;
         profile < assets.field_hud_behavior_meter_counters.size(); ++profile) {
        write_array(
            profile == 0U ? "field/hud/behavior-meter-primary-"
                          : "field/hud/behavior-meter-companion-",
            assets.field_hud_behavior_meter_counters[profile], "sprite",
            profile == 0U ? "0x37EC/0x6ABEE" : "0x37EC/0x6AB96", true);
    }
    write_array("field/hud/vertical-counter-", assets.field_hud_vertical_counters,
                "sprite", "0x35D4", true);
    write_array("field/hud/horizontal-counter-",
                assets.field_hud_horizontal_counters, "sprite", "0x35D4", true);
    write_array("field/hud/ammo-counter-", assets.field_hud_ammo_counters,
                "sprite", "0x36FC", true);
    write("menu/pocket-secretary/background.bmp",
          assets.pocket_secretary_background, "background", "0x10A7E", false);
    write_catalog_asset(
        root, "menu/pocket-secretary/grid.bmp",
        shadowrun::graphics::decode_genesis_pocket_secretary_grid(rom),
        "background", "0xC3E6/0x12C5A/0xC1320/0xC1064", false, manifest,
        written, missing, blank);
    write("menu/field/background.bmp", assets.field_menu_background, "background",
          "0x10F50", false);
    for (std::size_t frame = 0; frame < 17U; ++frame) {
        write_catalog_asset(
            root, "menu/field/elevator-selector-background-frame-" +
                      std::to_string(frame) + ".bmp",
            shadowrun::graphics::
                decode_genesis_field_elevator_selector_background(rom, frame),
            "background",
            "0x58DE0/0x566D8/0x1530/0x1728;0x5906E frame=" +
                std::to_string(frame),
            false, manifest, written, missing, blank);
    }
    for (std::size_t floor = 0; floor < 5U; ++floor) {
        write_catalog_asset(
            root, "menu/field/elevator-selector-current-floor-" +
                      std::to_string(floor) + ".bmp",
            shadowrun::graphics::
                decode_genesis_field_elevator_selector_floor_layer(
                    rom, floor, false),
            "overlay", "0x58FCC/0x1B6100",
            false, manifest, written, missing, blank);
        write_catalog_asset(
            root, "menu/field/elevator-selector-selected-floor-" +
                      std::to_string(floor) + ".bmp",
            shadowrun::graphics::
                decode_genesis_field_elevator_selector_floor_layer(
                    rom, floor, true),
            "overlay", "0x58FA8/0x1B60E8",
            false, manifest, written, missing, blank);
    }
    for (const bool previous_floor_input : {true, false}) {
        for (const bool active_phase : {true, false}) {
            const auto direction = previous_floor_input ? "previous" : "next";
            const auto phase = active_phase ? "active" : "rest";
            write_catalog_asset(
                root,
                "menu/field/elevator-selector-feedback-" +
                    std::string(direction) + "-" + std::string(phase) +
                    ".bmp",
                shadowrun::graphics::
                    decode_genesis_field_elevator_selector_feedback_layer(
                        rom, previous_floor_input, active_phase),
                "overlay",
                active_phase
                    ? (previous_floor_input
                           ? "0x58EBC/0x1B615C/0x1B6138/0xE8BA"
                           : "0x58EBC/0x1B615C/0x1B6130/0xE63A")
                    : (previous_floor_input
                           ? "0x58F1E/0x1B6140/0x1B6154/0xE8BA"
                           : "0x58F1E/0x1B6140/0x1B614C/0xE63A"),
                false, manifest, written, missing, blank);
        }
    }
    write_catalog_asset(
        root, "menu/field/service-background.bmp",
        shadowrun::graphics::decode_genesis_field_service_background(rom),
        "background", "0x10BC4/0x10918/0x1232", false, manifest, written,
        missing, blank);
    const auto service_selection_tabs =
        shadowrun::graphics::decode_genesis_field_service_selection_tabs(rom);
    for (std::size_t index = 0; index < service_selection_tabs.size(); ++index) {
        write_catalog_asset(
            root, "menu/field/service-tabs-selected-" +
                      std::to_string(index) + ".bmp",
            service_selection_tabs[index], "sprite",
            "0x58B5E/0x58BB6/0xCE17E/0xCE22E", false, manifest,
            written, missing, blank);
    }
    write_catalog_asset(
        root, "menu/field/service-universal-clips.bmp",
        shadowrun::graphics::decode_genesis_field_service_universal_clips_icon(
            rom),
        "sprite", "0x586F6/0xCE28E/0xD29D6/0x18AC56", false, manifest,
        written, missing, blank);
    write("menu/field/action-background.bmp",
          assets.field_menu_action_background, "background", "0x117A0", false);
    write("menu/field/action-confirm-background.bmp",
          assets.field_menu_action_confirmation_background, "background",
          "0x127B0/0xCD3B6", false);
    write_catalog_asset(
        root, "menu/field/clips-background.bmp",
        shadowrun::graphics::decode_genesis_field_menu_clips_background(rom),
        "background", "0x11B58/0x11B98/0xCD33E", false, manifest, written,
        missing, blank);
    write_catalog_asset(
        root, "menu/field/clips-trade-background.bmp",
        shadowrun::graphics::decode_genesis_field_menu_clips_trade_background(
            rom),
        "background", "0x11E7E/0xCD37A", false, manifest, written, missing,
        blank);
    write("menu/field/attributes-background.bmp",
          assets.field_menu_attributes_background, "background", "0x10C96", false);
    write("menu/field/inventory-boxes.bmp", assets.field_menu_inventory_boxes,
          "background", "0x10F50/0x1232", false);
    write("menu/field/font.bmp", assets.field_menu_font, "sprite",
          "0xC1064 palette-line=3", true);
    write_catalog_asset(
        root, "menu/field/font-palette-0.bmp",
        shadowrun::graphics::decode_genesis_field_menu_font(rom, 0U),
        "sprite", "0xC1064/0x10276/0x102D4 palette-line=0", true,
        manifest, written, missing, blank);
    write_catalog_asset(
        root, "menu/field/magic-font-palette-0.bmp",
        shadowrun::graphics::decode_genesis_field_magic_menu_font(rom),
        "sprite", "0xFBA6/0xC8A60/0xC1064/0x10276 palette-line=0", true,
        manifest, written, missing, blank);
    write_catalog_asset(
        root, "menu/field/font-palette-2.bmp",
        shadowrun::graphics::decode_genesis_field_menu_font(rom, 2U),
        "sprite", "0xC1064/0xD29D6/0x18AC56 palette-line=2", true,
        manifest, written, missing, blank);
    write_array("menu/field/runner-portrait-", assets.field_menu_runner_portraits,
                "sprite", "0x7291A", true);
    std::vector<shadowrun::graphics::GenesisFieldEntityTemplate>
        runner_preview_records;
    runner_preview_records.reserve(13U);
    for (std::size_t index = 0; index < 3U; ++index) {
        runner_preview_records.push_back(
            shadowrun::graphics::read_genesis_field_entity_template(
                rom, 0x7291AU + index * 0x100U));
    }
    const auto recruited_preview_records =
        shadowrun::graphics::read_genesis_recruited_roster(rom);
    runner_preview_records.insert(runner_preview_records.end(),
                                  recruited_preview_records.begin(),
                                  recruited_preview_records.end());
    std::vector<std::pair<std::uint32_t, std::uint16_t>>
        emitted_preview_sources;
    for (const auto& record : runner_preview_records) {
        const auto source_key = std::pair{
            record.hud_portrait_table_pointer,
            record.hud_portrait_table_offset};
        if (std::find(emitted_preview_sources.begin(),
                      emitted_preview_sources.end(), source_key) !=
            emitted_preview_sources.end()) {
            continue;
        }
        emitted_preview_sources.push_back(source_key);
        const auto panels =
            shadowrun::graphics::decode_genesis_field_menu_dismiss_runner_preview_panels(
                rom, record);
        for (std::size_t palette_line = 0;
             palette_line < panels.size(); ++palette_line) {
            write_catalog_asset(
                root,
                "menu/field/runner-preview-table-" +
                    std::to_string(record.hud_portrait_table_pointer) +
                    "-offset-" +
                    std::to_string(record.hud_portrait_table_offset) +
                    "-palette-" + std::to_string(palette_line) + ".bmp",
                panels[palette_line], "sprite",
                "0x3404/0x6ACFA/0x110E8/0x110EC/0x03648", false, manifest,
                written, missing, blank);
        }
    }
    const auto random_grid_effects =
        shadowrun::graphics::decode_genesis_field_menu_random_grid_effects(
            rom);
    for (std::size_t index = 0; index < 3U; ++index) {
        write_catalog_asset(
            root,
            "menu/field/random-grid-fixed-" + std::to_string(index) +
                ".bmp",
            random_grid_effects[index], "background",
            "0xE660/0xC13E8/0xCE126/0xCE12E/0xCE136", false,
            manifest, written, missing, blank);
    }
    for (std::size_t index = 0; index < 4U; ++index) {
        write_catalog_asset(
            root,
            "menu/field/random-grid-indexed-" + std::to_string(index) +
                ".bmp",
            random_grid_effects[3U + index], "background",
            "0xE660/0xC13F8/0xCE13E", false, manifest, written, missing,
            blank);
    }
    write_array("menu/field/selected-cell-", assets.field_menu_selected_cells,
                "sprite", "0x11678", true);
    write_array("menu/field/command-focus-", assets.field_menu_command_focus,
                "sprite", "0x11454", true);
    write_array("menu/field/action-focus-", assets.field_menu_action_focus,
                "sprite", "0x117A4", true);
    write_array("menu/field/action-confirm-focus-",
                assets.field_menu_action_confirmation_focus, "sprite",
                "0x127FC/0xCD47A", true);
    write_array("menu/field/clips-focus-", assets.field_menu_clips_focus, "sprite",
                "0x11B58", true);
    write_array("menu/field/clips-trade-focus-", assets.field_menu_clips_trade_focus,
                "sprite", "0x11E7E", true);
    write("menu/field/posture-focus.bmp", assets.field_menu_posture_focus, "sprite",
          "0x119CE", true);
    write("menu/field/cursor.bmp", assets.field_menu_cursor, "sprite", "0x11592",
          true);
    write_catalog_asset(
        root, "menu/field/hotel-cursor.bmp",
        shadowrun::graphics::decode_genesis_hotel_options_cursor(rom),
        "sprite", "0xC0C2/0xC376/0xDA6A/0xC1064/0x5FF", true, manifest,
        written, missing, blank);
    write_catalog_asset(
        root, "menu/field/hotel-checkout-cursor.bmp",
        shadowrun::graphics::decode_genesis_hotel_options_cursor(rom, 2U),
        "sprite", "0xC0C2/0xC376/0xDA6A/0xC1064/0x5FF palette-line=2", true,
        manifest, written, missing, blank);
    for (std::size_t item = 0; item < assets.field_menu_item_art.size(); ++item) {
        const auto item_code = item + 1U;
        write(shadowrun::resources::item_detail_asset_path(item_code),
              assets.field_menu_item_art[item], "sprite",
              "0x12A8C/0xD4540[item=" + std::to_string(item_code) + "]", true);
    }
    for (std::size_t item = 0; item < assets.field_menu_inventory_icons.size();
         ++item) {
        for (std::size_t variant = 0;
             variant < assets.field_menu_inventory_icons[item].size(); ++variant) {
            write(shadowrun::resources::item_icon_asset_path(item + 1U, variant),
                  assets.field_menu_inventory_icons[item][variant], "sprite",
                  "0x35D4/0xD4540[item=" + std::to_string(item + 1U) +
                      "][variant=" + std::to_string(variant) + "]", true);
        }
    }
    write_array("menu/field/status-bar-", assets.field_menu_status_bars, "sprite",
                "0x11928", true);
    write_array("menu/field/damage-bar-", assets.field_menu_damage_bars, "sprite",
                "0x12F70", true);
    write_array("menu/field/drain-bar-", assets.field_menu_drain_bars, "sprite",
                "0x12FA4", true);
    write_array("menu/field/armor-summary-bar-",
                assets.field_menu_armor_summary_bars, "sprite",
                "0x11352/0xCF884", true);
    write_array("menu/field/detail-value-", assets.field_menu_detail_values, "sprite",
                "0x13000", true);
    write_array("menu/field/attack-bar-", assets.field_menu_attack_bars, "sprite",
                "0x12CEE/0xCF3D4", true);
    write_array("menu/field/defense-bar-", assets.field_menu_defense_bars, "sprite",
                "0x12CEE/0xCF610", true);
    write_catalog_asset(
        root, "menu/field/magic-panel-background.bmp",
        shadowrun::graphics::decode_genesis_field_magic_panel_background(rom),
        "background", "0xFBA6/0xC6714/0xC8108", false,
        manifest, written, missing, blank);
    write_rgba_array(
        "menu/field/magic-metric-bar-",
        shadowrun::graphics::decode_genesis_field_magic_metric_bars(rom),
        "sprite", "0x1055E/0xC8784/0xC8A60", false);
    write_rgba_array(
        "menu/field/magic-spell-icon-",
        shadowrun::graphics::decode_genesis_field_magic_spell_icons(rom),
        "sprite", "0xFBA6/0xFCB4/0x11190", false);
    write_rgba_array(
        "menu/field/magic-cursor-frame-",
        shadowrun::graphics::decode_genesis_field_magic_cursor_frames(rom),
        "sprite", "0xFDFE/0xC8B3E/0x1378", false);
    write_rgba_array(
        "menu/field/magic-posture-bar-",
        shadowrun::graphics::decode_genesis_field_magic_posture_bars(rom),
        "sprite", "0x107CE/0xC8600", false);
    write_rgba_array(
        "menu/field/magic-force-meter-tile-",
        shadowrun::graphics::decode_genesis_field_magic_force_meter_tiles(rom),
        "sprite", "0x10172/0xC85A2", false);
    write_rgba_array(
        "menu/field/magic-availability-bar-",
        shadowrun::graphics::decode_genesis_field_magic_availability_bars(rom),
        "sprite", "0x1088C/0xC86C0", false);
    write_rgba_array(
        "menu/field/magic-damage-bar-",
        shadowrun::graphics::decode_genesis_field_magic_damage_bars(rom),
        "sprite", "0x105E6/0xC86F8", false);

    // Field maps are backgrounds; entity and walking frames are sprites.
    // The mode-2 map is already the first entry in field_areas; emitting the
    // legacy field_map field as a second row would overwrite the same asset
    // path and make the catalog appear to contain a duplicate resource.
    for (const auto& area : assets.field_areas) {
        write("field/maps/mode-" + std::to_string(area.mode_id) + ".bmp",
              area.map, "background", "mode resource catalog", false);
    }

    // Keep each rendered map tied to the descriptor that selected it. This is
    // separate from catalog.tsv: that file describes host images, while this
    // index audits the complete ROM table used by 0x3074.
    std::ofstream map_index(root / "field/maps/index.tsv", std::ios::binary);
    if (!map_index) {
        throw std::runtime_error("cannot create field map resource index");
    }
    map_index
        << "mode_id\ttable_entry\trecord_offset\tcamera_x_fixed"
           "\tcamera_y_fixed\tbound_x\tbound_y\tmin_x\tmin_y"
           "\tterrain_record\tsetup_block\tpalette\tpattern\tpattern_bytes"
           "\tmetatile_stream\tprimary_tilemap\tsecondary_tilemap"
           "\ttilemap_width\ttilemap_height\tmetatile_encoding"
           "\tevent_table\tobject_pattern\tobject_vram"
           "\tobject_pattern_bytes\tasset\timage_width\timage_height\n";
    const auto write_hex = [&map_index](std::size_t value) {
        map_index << "0x" << std::uppercase << std::hex << value << std::dec;
    };
    for (std::uint16_t mode_id = 0;
         mode_id < shadowrun::rom::kFieldModeCount; ++mode_id) {
        const auto resources =
            shadowrun::graphics::read_genesis_field_mode_resources(rom, mode_id);
        const auto area = std::find_if(
            assets.field_areas.begin(), assets.field_areas.end(),
            [mode_id](const auto& candidate) { return candidate.mode_id == mode_id; });
        if (area == assets.field_areas.end()) {
            throw std::runtime_error("field map index is missing mode " +
                                     std::to_string(mode_id));
        }
        map_index << mode_id << '\t';
        write_hex(shadowrun::rom::kFieldModePointerTable +
                  static_cast<std::size_t>(mode_id) * 4U);
        map_index << '\t';
        write_hex(resources.record_offset);
        map_index << '\t' << resources.initial_camera_x_fixed << '\t'
                  << resources.initial_camera_y_fixed << '\t'
                  << resources.terrain_x_exclusive_upper << '\t'
                  << resources.terrain_y_exclusive_upper << '\t'
                  << resources.terrain_x_inclusive_lower << '\t'
                  << resources.terrain_y_inclusive_lower << '\t';
        write_hex(resources.terrain_record_offset);
        map_index << '\t';
        write_hex(resources.setup_block_offset);
        map_index << '\t';
        write_hex(resources.palette_offset);
        map_index << '\t';
        write_hex(resources.pattern_data_offset);
        map_index << '\t' << resources.pattern_byte_count << '\t';
        write_hex(resources.metatile_stream_offset);
        map_index << '\t';
        write_hex(resources.primary_tilemap_offset);
        map_index << '\t';
        write_hex(resources.secondary_tilemap_offset);
        map_index << '\t' << resources.tilemap_width << '\t'
                  << resources.tilemap_height << '\t'
                  << (resources.metatile_encoding ==
                              shadowrun::graphics::GenesisMetatileEncoding::RawNameWords
                          ? "raw-name-words"
                          : "compressed-resource")
                  << '\t';
        write_hex(resources.event_table_offset);
        map_index << '\t';
        write_hex(resources.object_pattern_data_offset);
        map_index << '\t';
        write_hex(resources.object_pattern_vram_offset);
        map_index << '\t' << resources.object_pattern_byte_count
                  << "\tfield/maps/mode-" << mode_id << ".bmp\t"
                  << area->map.width << '\t' << area->map.height << '\n';
    }
    map_index.flush();

    // These profile-selected lists are the only default actor populations
    // currently traced end to end: scene 2 and scene 0x14. Read their template
    // and position pointers offline, preserving source order and ordered byte
    // overrides in a named host table. The game runtime never walks the ROM.
    struct DefaultActorSceneList {
        std::uint16_t scene_id;
        std::size_t list_offset;
        std::size_t expected_count;
    };
    struct DefaultActorRecord {
        std::uint16_t scene_id;
        std::uint16_t source_order;
        std::uint16_t entity_kind;
        std::uint16_t world_x;
        std::uint16_t world_y;
        std::uint16_t initial_heading;
        std::uint8_t palette_line;
        std::uint32_t template_offset;
        std::uint32_t position_offset;
        std::size_t list_offset;
        std::string overrides;
    };
    constexpr std::array<DefaultActorSceneList, 2> kDefaultActorLists{{
        {2U, 0x172D1CU, 3U},
        {0x14U, 0x1D91DAU, 8U},
    }};
    const auto format_offset = [](const std::uint32_t value) {
        std::ostringstream text;
        text << "0x" << std::uppercase << std::hex << value;
        return text.str();
    };
    std::vector<DefaultActorRecord> default_actor_records;
    for (const auto& source_list : kDefaultActorLists) {
        auto cursor = source_list.list_offset;
        const auto first_record = default_actor_records.size();
        for (std::uint16_t source_order = 0U; source_order < 32U;
             ++source_order) {
            const auto tagged_template =
                read_big_endian_long(rom, cursor);
            if (tagged_template == 0U) {
                break;
            }
            const auto template_offset = tagged_template & 0x7FFFFFFFU;
            const auto position_offset =
                read_big_endian_long(rom, cursor + 4U);
            if (template_offset > rom.size() ||
                rom.size() - template_offset < 4U) {
                throw std::runtime_error(
                    "default field actor template pointer exceeds ROM");
            }
            const auto entity_kind = read_big_endian_word(rom, template_offset);
            const auto palette_line = static_cast<std::uint8_t>(
                (read_big_endian_word(rom, template_offset + 2U) >> 13U) &
                0x03U);
            DefaultActorRecord record{
                source_list.scene_id,
                source_order,
                entity_kind,
                read_big_endian_word(rom, position_offset),
                read_big_endian_word(rom, position_offset + 2U),
                read_big_endian_word(rom, 0x7361AU + 0x2EU),
                palette_line,
                template_offset,
                position_offset,
                source_list.list_offset,
                "-",
            };

            auto next_record = cursor + 8U;
            if (read_big_endian_long(rom, next_record) == 2U) {
                auto pair_offset = next_record + 4U;
                std::ostringstream overrides;
                bool first_override = true;
                for (;;) {
                    const auto pair = read_big_endian_word(rom, pair_offset);
                    if (pair == 0U) {
                        pair_offset += 2U;
                        break;
                    }
                    if (!first_override) {
                        overrides << ',';
                    }
                    const auto byte_offset =
                        static_cast<std::uint8_t>(pair >> 8U);
                    const auto byte_value =
                        static_cast<std::uint8_t>(pair & 0x00FFU);
                    overrides << format_offset(byte_offset) << '='
                              << format_offset(byte_value);
                    if (byte_offset == 0x2EU) {
                        record.initial_heading = static_cast<std::uint16_t>(
                            (static_cast<std::uint16_t>(byte_value) << 8U) |
                            (record.initial_heading & 0x00FFU));
                    } else if (byte_offset == 0x2FU) {
                        record.initial_heading = static_cast<std::uint16_t>(
                            (record.initial_heading & 0xFF00U) | byte_value);
                    }
                    first_override = false;
                    pair_offset += 2U;
                }
                if (!first_override) {
                    record.overrides = overrides.str();
                }
                next_record = pair_offset;
            }
            default_actor_records.push_back(std::move(record));
            cursor = next_record;
        }
        if (default_actor_records.size() - first_record !=
            source_list.expected_count) {
            throw std::runtime_error(
                "profile-selected default actor list count changed at " +
                format_offset(static_cast<std::uint32_t>(
                    source_list.list_offset)));
        }
        if (read_big_endian_long(rom, cursor) != 0U) {
            throw std::runtime_error(
                "profile-selected default actor list has no null sentinel");
        }
    }

    {
        const auto actor_table_path = root / "field/entities/default-actors.tsv";
        std::filesystem::create_directories(actor_table_path.parent_path());
        std::ofstream actor_table(actor_table_path, std::ios::binary);
        if (!actor_table) {
            throw std::runtime_error(
                "cannot create profile-selected default actor table");
        }
        actor_table << "scene\tsource_order\tentity_kind\tworld_x\tworld_y"
                       "\tinitial_heading\tpalette_line\ttemplate_offset\tposition_offset"
                       "\tlist_offset\tordered_overrides\n";
        for (const auto& actor : default_actor_records) {
            actor_table << actor.scene_id << '\t' << actor.source_order << '\t'
                        << actor.entity_kind << '\t' << actor.world_x << '\t'
                        << actor.world_y << '\t'
                        << actor.initial_heading << '\t'
                        << static_cast<unsigned>(actor.palette_line) << '\t'
                        << format_offset(actor.template_offset) << '\t'
                        << format_offset(actor.position_offset) << '\t'
                        << format_offset(static_cast<std::uint32_t>(
                               actor.list_offset))
                        << '\t' << actor.overrides << '\n';
        }
        if (!actor_table) {
            throw std::runtime_error(
                "cannot write profile-selected default actor table");
        }
    }

    struct DefaultActorKindSource {
        std::uint16_t entity_kind;
        std::uint8_t palette_line;
        const char* asset_stem;
        const char* label;
    };
    constexpr std::array<DefaultActorKindSource, 2> kDefaultActorKinds{{
        {5U, 2U, "field/entities/actors/corp-guard", "corp guard"},
        {11U, 3U, "field/entities/actors/pedestrian", "pedestrian"},
    }};
    constexpr std::size_t kIdleSelectorTableOffset = 0xC052U;
    constexpr std::size_t kEntrySelectorTableOffset = 0xC0B2U;
    std::array<bool, 8> used_actor_directions{};
    for (const auto& actor : default_actor_records) {
        auto rotated = static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(actor.initial_heading << 3U) |
            (actor.initial_heading >> 13U));
        if ((rotated & 0x8000U) != 0U) {
            rotated = static_cast<std::uint16_t>(rotated + 1U);
        }
        used_actor_directions[rotated & 0x0007U] = true;
    }
    for (const auto& kind_source : kDefaultActorKinds) {
        const auto asset_stem = std::string(kind_source.asset_stem);
        const auto template_record = std::find_if(
            default_actor_records.begin(), default_actor_records.end(),
            [&kind_source](const DefaultActorRecord& actor) {
                return actor.entity_kind == kind_source.entity_kind &&
                       actor.palette_line == kind_source.palette_line;
            });
        if (template_record == default_actor_records.end()) {
            throw std::runtime_error(
                "default actor state frame has no selected scene template");
        }
        for (std::size_t direction = 0; direction < used_actor_directions.size();
             ++direction) {
            if (!used_actor_directions[direction]) {
                continue;
            }
            const auto idle_selector = read_big_endian_word(
                rom, kIdleSelectorTableOffset + direction * 2U);
            const auto idle_frame =
                shadowrun::graphics::decode_genesis_field_entity_state_frame(
                    rom, kind_source.entity_kind, kind_source.palette_line,
                    idle_selector);
            const auto idle_name = asset_stem +
                                   (direction == 0U
                                        ? std::string("-idle.bmp")
                                        : "-idle-dir-" +
                                              std::to_string(direction) +
                                              ".bmp");
            write_catalog_asset(
                root, idle_name, idle_frame.image, "sprite",
                "0xB45C/0xC052/0x600E kind=" +
                    std::to_string(kind_source.entity_kind) + " template=" +
                    format_offset(template_record->template_offset) +
                    " descriptor=" +
                    format_offset(static_cast<std::uint32_t>(
                        idle_frame.descriptor_offset)) +
                    " selector=" + format_offset(idle_selector) +
                    " palette-line=" +
                    std::to_string(kind_source.palette_line),
                true, manifest, written, missing, blank,
                idle_frame.origin_x, idle_frame.origin_y);

            const auto entry_selector = read_big_endian_word(
                rom, kEntrySelectorTableOffset + direction * 2U);
            const auto kind_state_pointer = static_cast<std::size_t>(
                read_big_endian_long(
                    rom, 0x62ACU +
                             static_cast<std::size_t>(kind_source.entity_kind) *
                                 4U));
            const auto entry_list = static_cast<std::size_t>(
                read_big_endian_long(rom, kind_state_pointer + entry_selector));
            if (read_big_endian_word(rom, entry_list) != 0x8002U) {
                throw std::runtime_error(
                    "profile-selected default actor entry animation is not "
                    "the traced two-frame list");
            }
            for (std::uint16_t frame_index = 0U; frame_index < 2U;
                 ++frame_index) {
                const auto frame =
                    shadowrun::graphics::decode_genesis_field_entity_state_frame(
                        rom, kind_source.entity_kind,
                        kind_source.palette_line, entry_selector, frame_index);
                const auto entry_name =
                asset_stem + "-entry-dir-" +
                    std::to_string(direction) + "-frame-" +
                    std::to_string(frame_index) + ".bmp";
                write_catalog_asset(
                    root, entry_name, frame.image, "sprite",
                    "0xB330/0xC0B2/0x5FDC/0x600E kind=" +
                        std::to_string(kind_source.entity_kind) +
                        " template=" +
                        format_offset(template_record->template_offset) +
                        " descriptor=" +
                        format_offset(static_cast<std::uint32_t>(
                            frame.descriptor_offset)) +
                        " selector=" + format_offset(entry_selector) +
                        " frame=" + std::to_string(frame_index) +
                        " palette-line=" +
                        std::to_string(kind_source.palette_line),
                    true, manifest, written, missing, blank, frame.origin_x,
                    frame.origin_y);
            }
        }
    }

    for (std::size_t index = 0; index < assets.entity_frames.size(); ++index) {
        write_frame("field/entities/idle-" + std::to_string(index) + ".bmp",
                    assets.entity_frames[index], "sprite", "0x5A62/0x5FDC",
                    true);
    }
    for (std::size_t direction = 0;
         direction < assets.field_entity_animations.size(); ++direction) {
        const auto& animation = assets.field_entity_animations[direction];
        for (std::size_t frame = 0; frame < animation.frames.size(); ++frame) {
            write_frame("field/entities/walk-" + std::to_string(direction) + "-" +
                            std::to_string(frame) + ".bmp",
                        animation.frames[frame], "sprite", "0x5A62/0x5FDC", true);
        }
    }
    const auto field_movement_animations =
        shadowrun::graphics::decode_genesis_field_entity_animation_sequences(
            rom, 0x7291A, 0x62AC, 0xC062, 0x3EDC, 0x172D8A, 8);
    for (std::size_t direction = 0;
         direction < field_movement_animations.size(); ++direction) {
        const auto& animation = field_movement_animations[direction];
        for (std::size_t frame = 0; frame < animation.frames.size(); ++frame) {
            const auto& source = animation.frames[frame];
            write_frame(
                "field/entities/movement-" + std::to_string(direction) + "-" +
                    std::to_string(frame) + ".bmp",
                shadowrun::native::NativeEntityFrame{
                    source.width, source.height, source.pixels},
                "sprite", "0xB6DE/0xC062/0x600E", true);
        }
    }
    const auto field_attack_animations =
        shadowrun::graphics::decode_genesis_field_entity_animation_sequences(
            rom, 0x7291A, 0x62AC, 0xC042, 0x3EDC, 0x172D8A, 8);
    for (std::size_t direction = 0;
         direction < field_attack_animations.size(); ++direction) {
        const auto& animation = field_attack_animations[direction];
        if (animation.frames.size() != 1U) {
            throw std::runtime_error(
                "source C042 attack pose did not resolve to one frame");
        }
        const auto& source = animation.frames.front();
        write_frame(
            "field/entities/attack-" + std::to_string(direction) + ".bmp",
            shadowrun::native::NativeEntityFrame{
                source.width, source.height, source.pixels},
            "sprite", "0xB39A/0xC042/0x600E", true);
    }

    // Weapon actions allocate a paired behavior/projectile record at
    // 0x1514E. The second record selects one of the 16 direction lists in
    // 0x62EC/0x62FC and 0x630C; export each descriptor assembly as its own
    // host image so the field runtime never needs to decode ROM lists.
    const auto field_spawn_animations =
        shadowrun::graphics::decode_genesis_field_spawn_animations(rom);
    for (const auto& animation : field_spawn_animations) {
        for (std::size_t frame = 0; frame < animation.frames.size(); ++frame) {
            const auto& source = animation.frames[frame];
            const auto relative_name =
                "field/weapons/projectile-" +
                std::to_string(animation.selector) + "-" +
                std::to_string(frame) + ".bmp";
            write_catalog_asset(
                root, relative_name, source.image, "sprite",
                "0x1514E/0x62EC/0x62FC/0x630C/0x600E/0x622C", true,
                manifest, written, missing, blank, source.origin_x,
                source.origin_y);
        }
    }

    extract_field_session_record_assets(rom, root);
    manifest.flush();
    std::filesystem::create_directories(root / "native");
    shadowrun::graphics::write_native_asset_state(
        (root / "native" / "state.bin").string(), assets);
    write_runtime_resource_manifest(root);
    std::cout << "all graphics assets written to " << root << "\n"
              << "extracted=" << written << " blank=" << blank
              << " missing=" << missing << "\n";
    if (missing != 0U) {
        throw std::runtime_error(
            "asset catalog contains missing image resources; extraction coverage is incomplete");
    }
}

shadowrun::graphics::RgbaImage make_field_entity_state_sheet(
    const std::vector<shadowrun::graphics::RgbaImage>& frames) {
    constexpr std::uint16_t kCellWidth = 40;
    constexpr std::uint16_t kCellHeight = 40;
    constexpr std::size_t kColumns = 4;
    const auto rows = std::max<std::size_t>(
        1U, (frames.size() + kColumns - 1U) / kColumns);
    shadowrun::graphics::RgbaImage sheet{
        static_cast<std::uint16_t>(kCellWidth * kColumns),
        static_cast<std::uint16_t>(kCellHeight * rows),
        std::vector<shadowrun::native::Rgba8>(
            static_cast<std::size_t>(kCellWidth * kColumns) *
                (kCellHeight * rows),
            {0x04, 0x08, 0x14, 0xFF}),
    };
    for (std::size_t index = 0; index < frames.size(); ++index) {
        const auto& frame = frames[index];
        const auto cell_x = static_cast<std::int32_t>(
            (index % kColumns) * kCellWidth);
        const auto cell_y = static_cast<std::int32_t>(
            (index / kColumns) * kCellHeight);
        const auto origin_x = cell_x +
            (static_cast<std::int32_t>(kCellWidth) - frame.width) / 2;
        const auto origin_y = cell_y +
            (static_cast<std::int32_t>(kCellHeight) - frame.height) / 2;
        for (std::int32_t y = 0; y < frame.height; ++y) {
            for (std::int32_t x = 0; x < frame.width; ++x) {
                const auto& pixel = frame.pixels[
                    static_cast<std::size_t>(y) * frame.width + x];
                if (pixel.alpha == 0) {
                    continue;
                }
                const auto target_x = origin_x + x;
                const auto target_y = origin_y + y;
                sheet.pixels[static_cast<std::size_t>(target_y) * sheet.width +
                             target_x] = pixel;
            }
        }
    }
    return sheet;
}

std::vector<shadowrun::graphics::RgbaImage> flatten_field_entity_animations(
    const std::vector<shadowrun::graphics::GenesisFieldEntityAnimation>&
        animations) {
    std::vector<shadowrun::graphics::RgbaImage> frames;
    for (const auto& animation : animations) {
        for (const auto& frame : animation.frames) {
            frames.push_back(frame);
        }
    }
    return frames;
}

void print_field_event_cells(
    const shadowrun::terrain::Map& terrain,
    const std::array<std::uint8_t, 32>& event_mode_table) {
    std::size_t count = 0;
    for (std::size_t row = 0; row < shadowrun::terrain::Map::kHeight; ++row) {
        for (std::size_t column = 0;
             column < shadowrun::terrain::Map::kWidth; ++column) {
            const auto status = terrain.statuses[
                row * shadowrun::terrain::Map::kWidth + column];
            if ((status & 0xE0U) != 0xC0U) {
                continue;
            }
            const auto event_index = static_cast<std::size_t>(status & 0x1FU);
            const auto destination = event_index < event_mode_table.size()
                ? event_mode_table[event_index]
                : 0U;
            std::cout << "cell (" << column * 32U << "," << row * 32U
                      << ") raw=0x" << std::hex << static_cast<unsigned>(status)
                      << " event=" << std::dec << event_index
                      << " destination=" << destination << "\n";
            ++count;
        }
    }
    std::cout << "C0 event cells: " << count << "\n";
}

shadowrun::graphics::RgbaImage render_native_field(
    shadowrun::native::NativePortAssets assets) {
    shadowrun::native::SoftwareScreen screen;
    shadowrun::native::NativePortShell shell("SHADOWRUN", std::move(assets));
    const auto press = [&shell](shadowrun::native::Button button) {
        const auto mask = shadowrun::native::button_mask(button);
        shell.update(shadowrun::native::InputState{mask, mask});
    };
    // Follow the reviewed title -> start options -> new game -> archetype
    // path. New Game enters 0x59C5E; skip that source-timed opening for this
    // field-only diagnostic, just as a user can with an action edge.
    press(shadowrun::native::Button::Start);
    press(shadowrun::native::Button::Start);
    press(shadowrun::native::Button::A);
    press(shadowrun::native::Button::A);
    if (shell.scene() == shadowrun::native::NativeScene::Intro) {
        press(shadowrun::native::Button::Start);
    }
    if (shell.scene() != shadowrun::native::NativeScene::Field) {
        throw std::runtime_error(
            "native field diagnostic did not reach the field scene");
    }
    shell.render(screen);

    shadowrun::graphics::RgbaImage image{
        screen.width(), screen.height(),
        std::vector<shadowrun::native::Rgba8>(screen.pixels().size()),
    };
    for (std::size_t index = 0; index < screen.pixels().size(); ++index) {
        const auto packed = screen.pixels()[index];
        image.pixels[index] = {
            static_cast<std::uint8_t>(packed >> 16U),
            static_cast<std::uint8_t>(packed >> 8U),
            static_cast<std::uint8_t>(packed), 255,
        };
    }
    return image;
}

shadowrun::graphics::RgbaImage render_native_field_menu(
    shadowrun::native::NativePortAssets assets, bool inventory) {
    shadowrun::native::SoftwareScreen screen;
    shadowrun::native::NativePortShell shell("SHADOWRUN", std::move(assets));
    const auto press = [&shell](shadowrun::native::Button button) {
        const auto mask = shadowrun::native::button_mask(button);
        shell.update(shadowrun::native::InputState{mask, mask});
    };
    press(shadowrun::native::Button::Start);
    press(shadowrun::native::Button::Start);
    press(shadowrun::native::Button::A);
    press(shadowrun::native::Button::A);
    if (shell.scene() == shadowrun::native::NativeScene::Intro) {
        press(shadowrun::native::Button::Start);
    }
    if (shell.scene() != shadowrun::native::NativeScene::Field) {
        throw std::runtime_error(
            "native field-menu diagnostic did not reach the field scene");
    }
    press(shadowrun::native::Button::Start);
    if (shell.scene() != shadowrun::native::NativeScene::FieldMenu) {
        throw std::runtime_error(
            "native field-menu diagnostic did not open the command screen");
    }
    if (inventory) {
        const auto press_menu_edge = [&shell](shadowrun::native::Button button) {
            const auto mask = shadowrun::native::button_mask(button);
            shell.update(shadowrun::native::InputState{mask, mask});
            shell.update(shadowrun::native::InputState{});
        };
        // The ROM starts at Pocket Secretary (selector 6). Six left edges
        // select the first Inventory column, which exercises the real
        // selected-cell, item-art, and item-stat destinations.
        for (int step = 0; step < 6; ++step) {
            press_menu_edge(shadowrun::native::Button::Left);
        }
    }
    shell.render(screen);

    shadowrun::graphics::RgbaImage image{
        screen.width(), screen.height(),
        std::vector<shadowrun::native::Rgba8>(screen.pixels().size()),
    };
    for (std::size_t index = 0; index < screen.pixels().size(); ++index) {
        const auto packed = screen.pixels()[index];
        image.pixels[index] = {
            static_cast<std::uint8_t>(packed >> 16U),
            static_cast<std::uint8_t>(packed >> 8U),
            static_cast<std::uint8_t>(packed), 255,
        };
    }
    return image;
}

shadowrun::graphics::RgbaImage render_native_startup_scene(
    shadowrun::native::NativePortAssets assets, const std::string& scene_name) {
    shadowrun::native::SoftwareScreen screen;
    shadowrun::native::NativePortShell shell("SHADOWRUN", std::move(assets));
    const auto press = [&shell](shadowrun::native::Button button) {
        const auto mask = shadowrun::native::button_mask(button);
        shell.update(shadowrun::native::InputState{mask, mask});
    };
    if (scene_name == "start-options") {
        press(shadowrun::native::Button::Start);
        press(shadowrun::native::Button::Start);
    } else if (scene_name == "save-load" || scene_name == "no-save") {
        press(shadowrun::native::Button::Start);
        press(shadowrun::native::Button::Start);
        press(shadowrun::native::Button::Down);
        press(shadowrun::native::Button::A);
        if (scene_name == "no-save") {
            press(shadowrun::native::Button::A);
        }
    } else if (scene_name == "archetype") {
        press(shadowrun::native::Button::Start);
        press(shadowrun::native::Button::Start);
        press(shadowrun::native::Button::A);
    } else if (scene_name == "intro") {
        press(shadowrun::native::Button::Start);
        press(shadowrun::native::Button::Start);
        press(shadowrun::native::Button::A);
        press(shadowrun::native::Button::A);
    } else {
        throw std::invalid_argument("unknown native startup scene");
    }
    shell.render(screen);
    shadowrun::graphics::RgbaImage image{
        screen.width(), screen.height(),
        std::vector<shadowrun::native::Rgba8>(screen.pixels().size())};
    for (std::size_t index = 0; index < screen.pixels().size(); ++index) {
        const auto packed = screen.pixels()[index];
        image.pixels[index] = {
            static_cast<std::uint8_t>(packed >> 16U),
            static_cast<std::uint8_t>(packed >> 8U),
            static_cast<std::uint8_t>(packed), 255};
    }
    return image;
}

shadowrun::graphics::RgbaImage render_native_title_frame(
    shadowrun::native::NativePortAssets assets, std::size_t frame) {
    shadowrun::native::SoftwareScreen screen;
    shadowrun::native::NativePortShell shell("SHADOWRUN", std::move(assets));
    for (std::size_t index = 0; index < frame; ++index) {
        shell.update(shadowrun::native::InputState{});
    }
    shell.render(screen);
    shadowrun::graphics::RgbaImage image{
        screen.width(), screen.height(),
        std::vector<shadowrun::native::Rgba8>(screen.pixels().size())};
    for (std::size_t index = 0; index < screen.pixels().size(); ++index) {
        const auto packed = screen.pixels()[index];
        image.pixels[index] = {
            static_cast<std::uint8_t>(packed >> 16U),
            static_cast<std::uint8_t>(packed >> 8U),
            static_cast<std::uint8_t>(packed), 255};
    }
    return image;
}

} // namespace

int main(int argc, char** argv) {
    if (argc == 4 && std::string(argv[3]) == "--field-event-cells") {
        try {
            const auto rom = read_file(argv[1]);
            const auto assets =
                shadowrun::graphics::decode_genesis_native_assets(rom);
            print_field_event_cells(assets.field_terrain,
                                    assets.field_event_mode_table);
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--field-event-cells") {
        try {
            const auto rom = read_file(argv[1]);
            const auto assets =
                shadowrun::graphics::decode_genesis_native_assets(rom);
            const auto mode_id = static_cast<std::uint16_t>(
                parse_number(argv[4]));
            const auto area = std::find_if(
                assets.field_areas.begin(), assets.field_areas.end(),
                [mode_id](const shadowrun::native::NativeFieldArea& candidate) {
                    return candidate.mode_id == mode_id;
                });
            if (area == assets.field_areas.end()) {
                throw std::out_of_range("field mode is not in the native catalog");
            }
            std::cout << "field mode " << mode_id << " event cells:\n";
            print_field_event_cells(area->terrain, area->event_mode_table);
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--title") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image = shadowrun::graphics::decode_genesis_title_screen(rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "startup scene: " << image.width << "x" << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--native-title-text") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image = shadowrun::graphics::decode_genesis_title_text(rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "native title text layer: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 &&
        (std::string(argv[3]) == "--native-start-options" ||
         std::string(argv[3]) == "--native-save-load" ||
         std::string(argv[3]) == "--native-no-save" ||
         std::string(argv[3]) == "--native-archetype" ||
         std::string(argv[3]) == "--native-intro")) {
        try {
            const auto rom = read_file(argv[1]);
            const auto name = std::string(argv[3]).substr(9);
            const auto image = render_native_startup_scene(
                shadowrun::graphics::decode_genesis_native_assets(rom), name);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "native startup scene: " << name << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--native-title-frame") {
        try {
            const auto rom = read_file(argv[1]);
            const auto frame = static_cast<std::size_t>(
                parse_number(argv[4]));
            const auto image = render_native_title_frame(
                shadowrun::graphics::decode_genesis_native_assets(rom), frame);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "native title frame: " << frame << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--intro-stage") {
        try {
            const auto rom = read_file(argv[1]);
            const auto index = static_cast<std::size_t>(parse_number(argv[4]));
            const auto stages = shadowrun::graphics::decode_genesis_intro_stages(rom);
            if (index >= stages.size()) {
                throw std::out_of_range("intro stage is outside the ROM stage set");
            }
            shadowrun::graphics::write_bmp32(argv[2], stages[index]);
            std::cout << "intro stage " << index << ": " << stages[index].width
                      << "x" << stages[index].height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--intro-heli") {
        try {
            const auto rom = read_file(argv[1]);
            const auto index = static_cast<std::size_t>(parse_number(argv[4]));
            const auto frames = shadowrun::graphics::decode_genesis_intro_heli_frames(rom);
            if (index >= frames.size()) {
                throw std::out_of_range("intro helicopter frame is outside the ROM frame set");
            }
            shadowrun::graphics::write_bmp32(argv[2], frames[index]);
            std::cout << "intro helicopter frame " << index << ": "
                      << frames[index].width << "x" << frames[index].height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--intro-aircraft-movement") {
        try {
            const auto rom = read_file(argv[1]);
            const auto index = static_cast<std::size_t>(parse_number(argv[4]));
            const auto frames =
                shadowrun::graphics::decode_genesis_intro_aircraft_movement_frames(
                    rom);
            if (index >= frames.size()) {
                throw std::out_of_range(
                    "intro aircraft movement frame is outside the ROM frame set");
            }
            shadowrun::graphics::write_bmp32(argv[2], frames[index]);
            std::cout << "intro aircraft movement frame " << index << ": "
                      << frames[index].width << "x" << frames[index].height
                      << "\noutput: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--intro-aircraft-plane-a") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image =
                shadowrun::graphics::decode_genesis_intro_aircraft_plane_a(rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "intro aircraft Plane-A: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--intro-wilderness-pan") {
        try {
            const auto rom = read_file(argv[1]);
            const auto index = static_cast<std::size_t>(parse_number(argv[4]));
            const auto frames =
                shadowrun::graphics::decode_genesis_intro_wilderness_pan_frames(
                    rom);
            if (index >= frames.size()) {
                throw std::out_of_range(
                    "intro wilderness pan frame is outside the ROM frame set");
            }
            shadowrun::graphics::write_bmp32(argv[2], frames[index]);
            std::cout << "intro wilderness pan frame " << index << ": "
                      << frames[index].width << "x" << frames[index].height
                      << "\noutput: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--intro-ambush") {
        try {
            const auto rom = read_file(argv[1]);
            const auto index = static_cast<std::size_t>(parse_number(argv[4]));
            const auto frames =
                shadowrun::graphics::decode_genesis_intro_ambush_frames(rom);
            if (index >= frames.size()) {
                throw std::out_of_range(
                    "intro ambush frame is outside the ROM frame set");
            }
            shadowrun::graphics::write_bmp32(argv[2], frames[index]);
            std::cout << "intro ambush frame " << index << ": "
                      << frames[index].width << "x" << frames[index].height
                      << "\noutput: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << error.what() << '\n';
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--cab-map") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image = shadowrun::graphics::decode_genesis_cab_map(rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "cab map: " << image.width << "x" << image.height
                      << "\noutput: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--startup-base") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image = shadowrun::graphics::decode_genesis_startup_screen(
                rom, 0x1F310EU, 0x67DF6U, 0x1FF0U);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "startup base scene: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--title-frame") {
        try {
            const auto rom = read_file(argv[1]);
            const auto frames =
                shadowrun::graphics::decode_genesis_title_animation_frames(rom);
            const auto frame_index = static_cast<std::size_t>(
                parse_number(argv[4]));
            if (frame_index >= frames.size()) {
                throw std::out_of_range("title frame is outside the ROM frame set");
            }
            shadowrun::graphics::write_bmp32(argv[2], frames[frame_index]);
            std::cout << "title animation frame " << frame_index << ": "
                      << frames[frame_index].width << "x"
                      << frames[frame_index].height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--native-field") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image = render_native_field(
                shadowrun::graphics::decode_genesis_native_assets(rom));
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "native field scene: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--native-field-menu") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image = render_native_field_menu(
                shadowrun::graphics::decode_genesis_native_assets(rom), false);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "native field menu scene: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--native-field-menu-inventory") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image = render_native_field_menu(
                shadowrun::graphics::decode_genesis_native_assets(rom), true);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "native field inventory menu scene: " << image.width
                      << "x" << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--native-menu") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image =
                shadowrun::graphics::decode_genesis_field_menu_background(rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "native field menu base: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 &&
        std::string(argv[3]) == "--field-service-background") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image =
                shadowrun::graphics::decode_genesis_field_service_background(
                    rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "field service background: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--field-dialogue-panel") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image =
                shadowrun::graphics::decode_genesis_field_dialogue_panel_background(
                    rom, false);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "field dialogue frame: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--field-dialogue-portrait") {
        try {
            const auto rom = read_file(argv[1]);
            const auto profile = static_cast<std::size_t>(
                parse_number(argv[4]));
            const auto image =
                shadowrun::graphics::decode_genesis_field_dialogue_portrait(
                    rom, profile);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "field dialogue portrait profile " << profile << ": "
                      << image.width << "x" << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) ==
                         "--field-dialogue-panel-alternate") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image =
                shadowrun::graphics::decode_genesis_field_dialogue_panel_background(
                    rom, true);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "alternate field dialogue frame: " << image.width
                      << "x" << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--field-dialogue-buttons") {
        try {
            const auto rom = read_file(argv[1]);
            constexpr std::array<std::string_view, 3> kButtonFiles{{
                "button-a.bmp", "button-b.bmp", "button-c.bmp",
            }};
            const auto output_directory = std::filesystem::path(argv[2]);
            std::filesystem::create_directories(output_directory);
            const auto buttons = shadowrun::graphics::
                decode_genesis_field_dialogue_option_markers(rom);
            for (std::size_t index = 0; index < buttons.size(); ++index) {
                shadowrun::graphics::write_bmp32(
                    (output_directory / kButtonFiles[index]).string(),
                    buttons[index]);
            }
            std::cout << "field dialogue button markers: " << buttons.size()
                      << " 16x16 images\n"
                      << "output directory: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--magic-panel") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image =
                shadowrun::graphics::decode_genesis_field_magic_panel_background(
                    rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "magic panel background: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--native-menu-inventory") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image = shadowrun::graphics::decode_genesis_field_menu_inventory_boxes(rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "native field inventory boxes: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--mode2-map") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image =
                shadowrun::graphics::decode_genesis_mode2_field_map(rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "mode-2 field map: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--mode-map") {
        try {
            const auto rom = read_file(argv[1]);
            const auto mode_id = static_cast<std::uint16_t>(
                parse_number(argv[4]));
            const auto resources =
                shadowrun::graphics::read_genesis_field_mode_resources(
                    rom, mode_id);
            const auto image = shadowrun::graphics::decode_genesis_field_mode_map(
                rom, resources);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "field mode " << mode_id << ": " << image.width
                      << "x" << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--entity-frame") {
        try {
            const auto rom = read_file(argv[1]);
            const auto image =
                shadowrun::graphics::decode_genesis_entity_frame(rom);
            shadowrun::graphics::write_bmp32(argv[2], image);
            std::cout << "entity frame: " << image.width << "x"
                      << image.height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--field-entity-states") {
        try {
            const auto rom = read_file(argv[1]);
            const auto frames =
                shadowrun::graphics::decode_genesis_field_entity_animation(rom);
            const auto sheet = make_field_entity_state_sheet(frames);
            shadowrun::graphics::write_bmp32(argv[2], sheet);
            std::cout << "field entity state sheet: " << sheet.width << "x"
                      << sheet.height << " (" << frames.size() << " states)\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--field-entity-walking") {
        try {
            const auto rom = read_file(argv[1]);
            const auto animations =
                shadowrun::graphics::decode_genesis_field_entity_animation_sequences(
                    rom);
            const auto frames = flatten_field_entity_animations(animations);
            const auto sheet = make_field_entity_state_sheet(frames);
            shadowrun::graphics::write_bmp32(argv[2], sheet);
            std::cout << "field entity behavior-transition sheet: "
                      << sheet.width << "x" << sheet.height << " ("
                      << frames.size() << " frames)\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--field-entity-movement") {
        try {
            const auto rom = read_file(argv[1]);
            const auto animations =
                shadowrun::graphics::decode_genesis_field_entity_animation_sequences(
                    rom, 0x7291A, 0x62AC, 0xC062, 0x3EDC, 0x172D8A, 8);
            const auto frames = flatten_field_entity_animations(animations);
            const auto sheet = make_field_entity_state_sheet(frames);
            shadowrun::graphics::write_bmp32(argv[2], sheet);
            std::cout << "field entity movement sheet: " << sheet.width << "x"
                      << sheet.height << " (" << frames.size() << " frames)\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 5 && std::string(argv[3]) == "--field-entity-state") {
        try {
            const auto rom = read_file(argv[1]);
            const auto frames =
                shadowrun::graphics::decode_genesis_field_entity_animation(rom);
            const auto index = static_cast<std::size_t>(parse_number(argv[4]));
            if (index >= frames.size()) {
                throw std::out_of_range("field entity state index is out of range");
            }
            shadowrun::graphics::write_bmp32(argv[2], frames[index]);
            std::cout << "field entity state " << index << ": "
                      << frames[index].width << "x" << frames[index].height << "\n"
                      << "output: " << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--title-boot-assets") {
        try {
            const auto rom = read_file(argv[1]);
            extract_title_boot_assets(rom, argv[2]);
            std::cout << "individual title/boot assets written to "
                      << argv[2] << "\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc == 4 && std::string(argv[3]) == "--all-assets") {
        try {
            const auto rom = read_file(argv[1]);
            extract_all_assets(rom, argv[2]);
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "error: " << error.what() << "\n";
            return 1;
        }
    }
    if (argc != 6 && argc != 7) {
        std::cerr << "usage: shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "<tile-offset> <tile-count> <palette-offset> [--resource]\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> --title\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--native-field\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--intro-stage <index>\n";
    std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--intro-heli <index>\n";
    std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--intro-aircraft-plane-a\n";
    std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--intro-wilderness-pan <index>\n";
    std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--intro-ambush <index>\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--cab-map\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--native-menu\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--field-service-background\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--field-dialogue-panel\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--field-dialogue-portrait <profile-index>\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--field-dialogue-panel-alternate\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out-dir> "
                      "--field-dialogue-buttons\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--magic-panel\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--native-menu-inventory\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                      "--native-field-menu-inventory\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <ignored> "
                     "--field-event-cells\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <ignored> "
                     "--field-event-cells <mode-id>\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--mode2-map\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--mode-map <mode-id>\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--entity-frame\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--field-entity-states\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--field-entity-walking\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--field-entity-movement\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <out.bmp> "
                     "--field-entity-state <index>\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <asset-dir> "
                     "--title-boot-assets\n";
        std::cerr << "       shadowrun_graphics_extract <rom.bin> <asset-dir> "
                     "--all-assets\n";
        std::cerr << "numeric offsets accept decimal or 0x-prefixed hex\n";
        return 2;
    }
    try {
        const auto rom = read_file(argv[1]);
        const auto tile_offset = static_cast<std::size_t>(parse_number(argv[3]));
        const auto tile_count = static_cast<std::size_t>(parse_number(argv[4]));
        const auto palette_offset = static_cast<std::size_t>(parse_number(argv[5]));
        const auto resource_stream = argc == 7 && std::string(argv[6]) == "--resource";
        if (argc == 7 && !resource_stream) {
            throw std::invalid_argument("unknown extractor mode");
        }
        const auto palette = shadowrun::graphics::read_genesis_palette(
            rom, palette_offset);
        const auto resource = resource_stream
            ? shadowrun::graphics::decode_genesis_resource_stream(rom, tile_offset)
            : std::vector<std::uint8_t>{};
        const auto& tile_source = resource_stream ? resource : rom;
        const auto image = shadowrun::graphics::decode_tile_atlas(
            tile_source, resource_stream ? 0U : tile_offset, tile_count, palette);
        shadowrun::graphics::write_bmp32(argv[2], image);
        std::cout << "tiles: " << tile_count << "\n"
                  << "image: " << image.width << "x" << image.height << "\n"
                  << (resource_stream ? "decoded bytes: " +
                                             std::to_string(resource.size()) +
                                             "\n" : "")
                  << "output: " << argv[2] << "\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << "\n";
        return 1;
    }
}
