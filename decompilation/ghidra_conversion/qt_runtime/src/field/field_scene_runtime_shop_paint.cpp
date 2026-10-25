#include "field_scene_runtime.hpp"
#include "field_scene_runtime_internal.hpp"

#include "enums.hpp"
#include "shadowrun_ghidra/field/scene_profile_conversion.hpp"
#include "shadowrun_ghidra/field/field_mode_conversion.hpp"

#include "shadowrun_ghidra/audio/audio_command_batch_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_coordinate_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_proximity_collision_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_route_primitives_conversion.hpp"
#include "shadowrun_ghidra/field/field_mode_scene_runtime_preparation_conversion.hpp"
#include "shadowrun_ghidra/field/field_exit_orchestrator_conversion.hpp"
#include "shadowrun_ghidra/field/field_exit_prompt_conversion.hpp"
#include "shadowrun_ghidra/field/field_mode_service_conversion.hpp"
#include "shadowrun_ghidra/field/field_action_progression_purchase_conversion.hpp"
#include "shadowrun_ghidra/field/field_equipment_upgrade_conversion.hpp"
#include "shadowrun_ghidra/field/field_resource_sequence_conversion.hpp"
#include "shadowrun_ghidra/world/post_intro_mode_conversion.hpp"
#include "shadowrun_ghidra/field/field_action_detail_text_conversion.hpp"
#include "shadowrun_ghidra/field/field_scene_entry_conversion.hpp"
#include "shadowrun_ghidra/field/field_script_route_conversion.hpp"
#include "shadowrun_ghidra/field/stoker_motel_followup_commands_conversion.hpp"
#include "shadowrun_ghidra/field/field_entity_render_batch_conversion.hpp"
#include "shadowrun_ghidra/integration/field_session_source_assets_conversion.hpp"
#include "shadowrun_ghidra/session/runner_status_conversion.hpp"
#include "shadowrun_ghidra/core/armor_catalog_conversion.hpp"
#include "shadowrun_ghidra/core/item_catalog_conversion.hpp"
#include "shadowrun_ghidra/field/vid_phone_service_conversion.hpp"
#include "shadowrun_ghidra/core/magic_spell_catalog_conversion.hpp"
#include "shadowrun_ghidra/core/random_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_action_math_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_animation_conversion.hpp"
#include "shadowrun_ghidra/entities/heading_animation_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_state_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_metric_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_delay_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_combat_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_transition_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_animation_spawn_probe_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_animation_player_action_conversion.hpp"
#include "shadowrun_ghidra/menus/clips_menu_conversion.hpp"
#include "shadowrun_ghidra/menus/menu_transfer_batch_conversion.hpp"
#include "shadowrun_ghidra/menus/cyberware_detail_conversion.hpp"
#include "genesis_image_compositor.hpp"
#include "numeric_visual_renderer.hpp"
#include "field_map_composition.hpp"
#include "matrix_map_presenter.hpp"
#include "matrix_datastore_facts.hpp"

#include <QPainter>
#include <QColor>
#include <QRegion>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <fstream>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <cstdio>
#include <string>
#include <string_view>
#include <utility>

namespace shadowrun::ghidra_conversion::qt_runtime {

using field_scene_detail::field_shop_cost_context;
using field_scene_detail::draw_command_overview_number;
using field_scene_detail::make_field_shop_progression_input;

void FieldSceneRuntime::install_test_deck() {
    // ROM 0x59B7E sets FF018F bit 0, FFFC28, and these FFFBBE fields.
    cyberdeck_available_ = true;
    if (field_runner_feature_flags_.has_value()) {
        *field_runner_feature_flags_ |= 0x01U;
    }
    field_matrix_passcode_flags_ = 0x0FFEU;

    auto deck = field_matrix_deck_record_.value_or(CyberdeckRecord{});
    deck.mpcp = 6U;
    deck.hardening = 1U;
    // ROM 0x59B7E writes FFFBBE+2=0x00C8, +4=0x03E8, +6=0x32,
    // +7=1, and +0x0D=0xFF. It then writes rating 4 to each of the twelve
    // program bytes at +0x0E..+0x19.
    deck.memory_megapulses = 0x00C8U;
    deck.storage_megapulses = 1000U;
    deck.load_io_speed = 0x32U;
    deck.response = 1U;
    deck.model_index = static_cast<std::int8_t>(-1);
    std::array<std::uint8_t, 12> program_ratings{};
    program_ratings.fill(4U);
    deck.program_ratings = program_ratings;
    // The ROM routine leaves persona, loaded programs, slot animation state,
    // and stored files untouched. Preserve those existing typed fields.
    field_matrix_deck_record_ = std::move(deck);
}

std::optional<std::uint8_t> FieldSceneRuntime::selected_shop_item_code() const {
    if (!field_shop_menu_.offers_panel_active()) {
        return std::nullopt;
    }
    std::uint8_t item_code = 0U;
    if (field_shop_menu_.selected_group() == ShopActionGroup::sell) {
        const auto cursor = field_shop_menu_.selected_cursor();
        const auto& item_codes = active_runner_record().inventory_item_codes;
        if (cursor >= item_codes.size()) {
            return std::nullopt;
        }
        item_code = item_codes[cursor];
    } else {
        const auto offer = field_shop_menu_.offer(
            field_shop_menu_.selected_group(),
            field_shop_menu_.selected_cursor());
        if (!offer.has_value() ||
            offer->type == static_cast<std::uint8_t>('E')) {
            return std::nullopt;
        }
        item_code = static_cast<std::uint8_t>(offer->type + 1U);
    }
    if (item_code == 0U || item_code > field_menu_text_.items.size()) {
        return std::nullopt;
    }
    return item_code;
}

void FieldSceneRuntime::paint_field_shop(QPainter& painter) const {
    if (field_service_background_.isNull() ||
        !field_shop_active_definition_.has_value()) {
        return;
    }
    painter.drawImage(0, 0, field_service_background_);

    // The Lua service definition chooses the ROM title and category labels;
    // this presenter places those named text records over shared service art.
    const auto* title = game_scripts_.dialogue_text(
        field_shop_active_definition_->title_key);
    if (title != nullptr) {
        draw_command_overview_text(painter, 64, 16, *title);
    }
    const auto group = field_shop_menu_.selected_group();
    const auto group_index = field_shop_menu_.selected_group_index();
    if (group_index >= field_shop_active_definition_->groups.size()) {
        return;
    }
    const auto& group_definition =
        field_shop_active_definition_->groups[group_index];
    const bool sell_group = group_definition.id == "sell";
    const bool progression_group =
        !group_definition.progression_offers.empty();
    const bool wire_upgrade_group =
        !group_definition.equipment_upgrade_offers.empty();
    if (!wire_upgrade_group &&
        group_index < field_service_tab_images_.size()) {
        painter.drawImage(64, 128, field_service_tab_images_[group_index]);
    }
    if (field_shop_menu_.offers_panel_active()) {
            const bool transaction_replaces_detail =
                !wire_upgrade_group &&
                (field_shop_menu_.transaction_confirmation_open() ||
                 field_shop_menu_.transaction_message() !=
                     ShopTransactionMessage::none);
            const auto cursor = field_shop_menu_.selected_cursor();
            const auto selected_upgrade_offer = wire_upgrade_group
                ? field_shop_menu_.equipment_upgrade_offer(group, cursor)
                : std::optional<scripting::ScriptShopEquipmentUpgradeOffer>{};
            const auto selected_offer = sell_group || progression_group
                ? std::optional<FieldActionCostRecord>{}
                : field_shop_menu_.offer(group, cursor);
            const auto selected_progression_offer = progression_group
                ? field_shop_menu_.progression_offer(group, cursor)
                : std::optional<scripting::ScriptShopProgressionOffer>{};
            std::uint8_t selected_item_code = 0U;
            if (sell_group) {
                if (cursor < active_runner_record().inventory_item_codes.size()) {
                    selected_item_code =
                        active_runner_record().inventory_item_codes[cursor];
                }
            } else if (selected_offer.has_value() &&
                       selected_offer->type !=
                           static_cast<std::uint8_t>('E')) {
                selected_item_code = static_cast<std::uint8_t>(
                    selected_offer->type + 1U);
            }

            if (wire_upgrade_group && selected_upgrade_offer.has_value()) {
                const auto& upgrade = *selected_upgrade_offer;
                if (upgrade.category < field_service_cyberware_icons_.size()) {
                    painter.drawImage(
                        80, 32,
                        field_service_cyberware_icons_[upgrade.category]);
                }
                const auto* selected_name = game_scripts_.dialogue_text(
                    upgrade.selected_name_key);
                if (selected_name != nullptr) {
                    draw_command_overview_text(painter, 128, 32,
                                               *selected_name);
                }
                const auto* description = game_scripts_.dialogue_text(
                    upgrade.description_key);
                if (description != nullptr) {
                    draw_command_overview_text(painter, 24, 56,
                                               *description);
                }

                const auto show_upgrade_quote =
                    !field_shop_menu_.transaction_confirmation_open() &&
                    field_shop_menu_.transaction_message() ==
                        ShopTransactionMessage::none;
                if (show_upgrade_quote) {
                    const auto* price_heading = game_scripts_.dialogue_text(
                        field_shop_active_definition_->text_keys
                            .purchase_price_heading);
                    if (price_heading != nullptr) {
                        draw_command_overview_text(painter, 80, 112,
                                                   *price_heading);
                    }
                    const auto cost = apply_field_action_cost_discounts(
                        upgrade.base_cost,
                        field_shop_cost_context(
                            field_shop_menu_.menu_mode(),
                            active_runner_record(),
                            field_runner_feature_flags_.value_or(0U)));
                    draw_command_overview_number(
                        painter, command_overview_font_, 176, 112, cost,
                        NumericVisualAlignment::right, true);

                    const auto& thresholds =
                        field_shop_active_definition_
                            ->equipment_upgrade_meter_thresholds;
                    if (thresholds.size() == 16U) {
                        FieldEquipmentUpgradeTable table{};
                        std::copy(thresholds.begin(), thresholds.end(),
                                  table.meter_thresholds.begin());
                        const auto essence_cost =
                            field_equipment_upgrade_display_essence_cost(
                                table, upgrade.category,
                                active_runner_record().cyberware_flags);
                        const auto* essence_heading =
                            game_scripts_.dialogue_text(
                                field_shop_active_definition_->text_keys
                                    .cyberware_essence_heading);
                        const auto* essence_cost_heading =
                            game_scripts_.dialogue_text(
                                field_shop_active_definition_->text_keys
                                    .cyberware_essence_cost_heading);
                        if (essence_heading != nullptr) {
                            draw_command_overview_text(
                                painter, 72, 88, *essence_heading);
                        }
                        if (essence_cost_heading != nullptr) {
                            draw_command_overview_text(
                                painter, 72, 96, *essence_cost_heading);
                        }
                        const auto& runner = active_runner_record();
                        draw_command_overview_number(
                            painter, command_overview_font_, 184, 88,
                            runner.cyberware_essence_tens());
                        draw_command_overview_number(
                            painter, command_overview_font_, 200, 88,
                            runner.cyberware_essence_ones());
                        if (essence_cost.has_value()) {
                            draw_command_overview_number(
                                painter, command_overview_font_, 184, 96,
                                *essence_cost);
                            draw_command_overview_number(
                                painter, command_overview_font_, 200, 96,
                                0U);
                        }
                    }
                }
            } else if (progression_group &&
                       selected_progression_offer.has_value()) {
                // ROM 0x57D7E uses the offer byte directly as the D4EA0 row
                // index for its name, details, and glyph.
                const auto spell_code = static_cast<std::size_t>(
                    selected_progression_offer->action_code);
                std::optional<FieldActionProgressionQuoteOutcome>
                    progression_quote;
                const auto* const spell_table = field_magic_spell_table(
                    active_runner_record().magic_table_index);
                if (!transaction_replaces_detail && spell_table != nullptr) {
                    auto progression = make_field_shop_progression_input(
                        field_shop_menu_, active_runner_record(),
                        field_funds_.value_or(0U), *spell_table,
                        field_runner_feature_flags_.value_or(0U));
                    if (progression.has_value()) {
                        progression_quote =
                            prepare_field_action_progression_quote(
                                progression->tables, progression->state);
                    }
                }

                const bool runner_unavailable = progression_quote.has_value() &&
                    progression_quote->result ==
                        FieldActionProgressionQuoteResult::runner_unavailable;
                if (!runner_unavailable &&
                    spell_code < command_overview_magic_spell_icons_.size()) {
                    painter.drawImage(
                        128, 32,
                        command_overview_magic_spell_icons_[spell_code]);
                }
                if (!runner_unavailable &&
                    spell_code < field_menu_text_.magic_spell_names.size()) {
                    draw_command_overview_text(
                        painter, 104, 144,
                        field_menu_text_.magic_spell_names[spell_code]);
                }
                if (!transaction_replaces_detail && !runner_unavailable &&
                    spell_code <
                        field_menu_text_.magic_spell_descriptions.size()) {
                    draw_command_overview_text(
                        painter, 24, 56,
                        field_menu_text_.magic_spell_descriptions[spell_code]);
                }

                if (!transaction_replaces_detail &&
                    progression_quote.has_value()) {
                    const auto draw_shop_text = [this, &painter](
                        const std::string& key, const int x, const int y) {
                        const auto* text = game_scripts_.dialogue_text(key);
                        if (text != nullptr) {
                            draw_command_overview_text(painter, x, y, *text);
                        }
                    };
                    switch (progression_quote->result) {
                    case FieldActionProgressionQuoteResult::runner_unavailable:
                        draw_shop_text(
                            field_shop_active_definition_->text_keys
                                .progression_training_unavailable,
                            64, 88);
                        break;
                    case FieldActionProgressionQuoteResult::progression_limit:
                        draw_shop_text(
                            field_shop_active_definition_->text_keys
                                .progression_maximum_level,
                            64, 104);
                        break;
                    case FieldActionProgressionQuoteResult::quoted:
                        draw_shop_text(
                            field_shop_active_definition_->text_keys
                                .progression_level_label,
                            112, 64);
                        draw_command_overview_number(
                            painter, command_overview_font_, 160, 64,
                            progression_quote->quote->next_progression_level);
                        draw_shop_text(
                            field_shop_active_definition_->text_keys
                                .progression_cost_heading,
                            56, 112);
                        draw_command_overview_number(
                            painter, command_overview_font_, 208, 112,
                            progression_quote->quote->cost,
                            NumericVisualAlignment::right, true);
                        break;
                    case FieldActionProgressionQuoteResult::invalid_offer_selection:
                    case FieldActionProgressionQuoteResult::missing_record_slot:
                    case FieldActionProgressionQuoteResult::offer_unavailable:
                        break;
                    }
                }
            }

            if (!wire_upgrade_group && selected_offer.has_value() &&
                selected_offer->type == static_cast<std::uint8_t>('E')) {
                // ROM 0x586F6 uses the dedicated CE28E grid, 1AACE2
                // description, 1AACCE title, and record quantity for the
                // Universal Gun Clips row. These are source text/assets, not
                // a fabricated inventory item-code mapping.
                const auto& clips_description =
                    field_menu_text_.field_service_universal_clips_description;
                painter.drawImage(128, 32,
                                  field_service_universal_clips_icon_);
                if (!transaction_replaces_detail) {
                    draw_command_overview_text(
                        painter, 24, 56, clips_description);
                }
                draw_command_overview_text(
                    painter, 104, 144,
                    field_menu_text_.field_service_universal_clips_title);
                if (!transaction_replaces_detail) {
                    // The ROM-derived Lua description places its currency
                    // glyph on the final line. Anchor the amount immediately
                    // to its left so the last digit cannot overwrite it.
                    const auto currency_marker = clips_description.rfind('$');
                    if (currency_marker != std::string::npos) {
                        const auto line_break =
                            clips_description.rfind('\n', currency_marker);
                        const auto line_begin = line_break == std::string::npos
                            ? 0U
                            : line_break + 1U;
                        const auto marker_x = 24 + static_cast<int>(
                            (currency_marker - line_begin) * 8U);
                        const auto marker_row = static_cast<int>(
                            std::count(clips_description.begin(),
                                       clips_description.begin() +
                                           static_cast<std::ptrdiff_t>(
                                               currency_marker),
                                       '\n'));
                        draw_command_overview_number(
                            painter, command_overview_font_, marker_x,
                            56 + marker_row * 8, selected_offer->quantity,
                            NumericVisualAlignment::right);
                    }
                }
            }
            else if (!wire_upgrade_group && selected_item_code > 0U &&
                     selected_item_code <= field_menu_text_.items.size()) {
                const auto item_index =
                    static_cast<std::size_t>(selected_item_code - 1U);
                const auto& item_name = field_menu_text_.items[item_index].name;
                const auto& item_category =
                    field_menu_text_.items[item_index].category;
                const auto space = item_name.find(' ');
                if (!item_category.empty()) {
                    draw_command_overview_text(
                        painter,
                        (320 - static_cast<int>(item_name.size() * 8U)) / 2,
                        144, item_name);
                    draw_command_overview_text(
                        painter,
                        (320 - static_cast<int>(item_category.size() * 8U)) / 2,
                        152, item_category);
                } else if (space != std::string::npos && space > 0U &&
                           space + 1U < item_name.size()) {
                    const auto first_line =
                        std::string_view(item_name).substr(0U, space);
                    const auto second_line =
                        std::string_view(item_name).substr(space + 1U);
                    draw_command_overview_text(
                        painter, (320 - static_cast<int>(first_line.size() * 8U)) / 2,
                        144, first_line);
                    draw_command_overview_text(
                        painter, (320 - static_cast<int>(second_line.size() * 8U)) / 2,
                        152, second_line);
                } else {
                    draw_command_overview_text(
                        painter,
                        (320 - static_cast<int>(item_name.size() * 8U)) / 2,
                        144, item_name);
                }
                if (selected_item_code <=
                    command_overview_item_details_.size()) {
                    painter.drawImage(
                        124, 32,
                        command_overview_item_details_[item_index]);
                }

                if (!transaction_replaces_detail) {
                    const auto weapon_capacity =
                        selected_shop_weapon_capacity();
                    const auto item_uses = selected_shop_item_uses();
                    if (weapon_capacity.has_value()) {
                        // A named Lua weapon template supplies stock capacity;
                        // the active runner's loaded rounds stay in the HUD.
                        draw_command_overview_text(
                            painter, 72, 56,
                            field_menu_text_.item_detail_labels[0]);
                        draw_command_overview_number(
                            painter, command_overview_font_, 104, 56,
                            *weapon_capacity);
                    } else if (item_uses.has_value()) {
                        draw_command_overview_text(
                            painter, 72, 56,
                            field_menu_text_.item_detail_labels[1]);
                        draw_command_overview_number(
                            painter, command_overview_font_, 104, 56,
                            *item_uses);
                    }
                    const auto detail_text = select_field_action_detail_text(
                        static_cast<std::uint8_t>(selected_item_code - 1U));
                    if (detail_text.has_value() &&
                        detail_text->table_index <
                            field_menu_text_.field_action_detail_texts.size()) {
                        draw_command_overview_text(
                            painter, 24, 80,
                            field_menu_text_.field_action_detail_texts[
                                detail_text->table_index]);
                    }
                }

                if (!transaction_replaces_detail && !sell_group &&
                    !progression_group &&
                    selected_offer.has_value()) {
                    draw_command_overview_text(
                        painter, 80, 112,
                        field_menu_text_.shop_purchase_price_heading);
                    const auto amount = calculate_field_action_record_cost(
                        *selected_offer,
                        field_shop_cost_context(field_shop_menu_.menu_mode(),
                                          active_runner_record(),
                                          field_runner_feature_flags_.value_or(
                                              0U)),
                        item_base_cost_lookup());
                    draw_command_overview_number(
                        painter, command_overview_font_, 184, 112, amount,
                        NumericVisualAlignment::right, true);
                }
            }
    } else {
        for (std::size_t row = 0; row < group_definition.title_keys.size();
             ++row) {
            const auto* heading = game_scripts_.dialogue_text(
                group_definition.title_keys[row]);
            if (heading != nullptr) {
                draw_command_overview_text(
                    painter, 64, 144 + static_cast<int>(row) * 8, *heading);
            }
        }
    }

    const auto stock_group = field_shop_menu_.offers_panel_active()
        ? group
        : field_shop_menu_.displayed_offer_group();
    const auto stock_group_index =
        field_shop_menu_.offers_panel_active()
            ? field_shop_menu_.selected_group_index()
            : field_shop_menu_.displayed_offer_group_index();
    const bool stock_is_sell =
        stock_group_index < field_shop_active_definition_->groups.size() &&
        field_shop_active_definition_->groups[stock_group_index].id == "sell";
    const bool stock_is_progression =
        stock_group_index < field_shop_active_definition_->groups.size() &&
        !field_shop_active_definition_->groups[stock_group_index]
             .progression_offers.empty();
    const bool stock_is_wire_upgrade =
        stock_group_index < field_shop_active_definition_->groups.size() &&
        !field_shop_active_definition_->groups[stock_group_index]
             .equipment_upgrade_offers.empty();
    const auto count = stock_is_wire_upgrade
        ? field_shop_menu_.equipment_upgrade_offer_count(stock_group)
        : stock_is_sell
              ? active_runner_record().inventory_item_codes.size()
              : stock_is_progression
                    ? field_shop_menu_.progression_offer_count(stock_group)
                    : field_shop_menu_.offer_count(stock_group);
    for (std::size_t index = 0; index < count && index < 8U; ++index) {
        if (stock_is_wire_upgrade) {
            if (index < field_service_cyberware_icons_.size()) {
                painter.drawImage(
                    16 + static_cast<int>(index) * 32, 168,
                    field_service_cyberware_icons_[index]);
            }
            continue;
        }
        if (stock_is_sell) {
            const auto item_code =
                active_runner_record().inventory_item_codes[index];
            if (item_code > 0U &&
                item_code <= command_overview_inventory_icons_.size()) {
                painter.drawImage(
                    16 + static_cast<int>(index) * 32, 168,
                    command_overview_inventory_icons_[item_code - 1U][0]);
            }
            continue;
        }
        if (stock_is_progression) {
            const auto offer =
                field_shop_menu_.progression_offer(stock_group, index);
            if (!offer.has_value()) {
                continue;
            }
            const auto spell_code = static_cast<std::size_t>(
                offer->action_code);
            if (spell_code < command_overview_magic_spell_icons_.size()) {
                painter.drawImage(
                    16 + static_cast<int>(index) * 32, 168,
                    command_overview_magic_spell_icons_[spell_code]);
            }
            continue;
        }
        const auto offer = field_shop_menu_.offer(stock_group, index);
        if (!offer.has_value()) {
            continue;
        }
        if (offer->type == static_cast<std::uint8_t>('E')) {
            painter.drawImage(
                16 + static_cast<int>(index) * 32, 168,
                field_service_universal_clips_icon_);
            continue;
        }
        const auto item_code = static_cast<std::uint8_t>(offer->type + 1U);
        if (item_code > 0U &&
            item_code <= command_overview_inventory_icons_.size()) {
            painter.drawImage(
                16 + static_cast<int>(index) * 32, 168,
                command_overview_inventory_icons_[item_code - 1U][0]);
        }
    }

    if (field_shop_menu_.offers_panel_active()) {
        // The Genesis selector hugs each 24x16 item image. Draw a single
        // warm-yellow pixel around that footprint; the surrounding 32x24
        // cell and neighboring controls remain clear.
        const auto cursor = field_shop_menu_.selected_cursor();
        const auto selectable = stock_is_wire_upgrade || !sell_group ||
            (cursor < active_runner_record().inventory_item_codes.size() &&
             active_runner_record().inventory_item_codes[cursor] != 0U);
        if (selectable && cursor < count && cursor < 8U) {
            constexpr int kIconWidth = 24;
            constexpr int kIconHeight = 16;
            const auto x = 16 + static_cast<int>(cursor) * 32;
            constexpr int y = 168;
            const QColor selector_color(196, 190, 39);
            painter.fillRect(x, y, kIconWidth, 1, selector_color);
            painter.fillRect(x, y + kIconHeight - 1, kIconWidth, 1,
                             selector_color);
            painter.fillRect(x, y, 1, kIconHeight, selector_color);
            painter.fillRect(x + kIconWidth - 1, y, 1, kIconHeight,
                             selector_color);
        }
    }

    paint_field_hud_overlays(painter);
    if (field_funds_.has_value()) {
        draw_command_overview_number(
            painter, command_overview_font_, 144, 200, *field_funds_,
            NumericVisualAlignment::left, true);
        draw_command_overview_text(painter, 200, 200, "$");
    }

    if (field_shop_menu_.transaction_confirmation_open()) {
        if (wire_upgrade_group) {
            const auto* heading = game_scripts_.dialogue_text(
                field_shop_active_definition_->text_keys
                    .cyberware_purchase_heading);
            if (heading != nullptr) {
                draw_command_overview_text(painter, 48, 88, *heading);
            }
            draw_command_overview_number(
                painter, command_overview_font_, 216, 88,
                field_shop_menu_.transaction_amount(),
                NumericVisualAlignment::right, true);
        } else {
            draw_command_overview_text(
                painter, 24, 80, field_menu_text_.shop_transaction_prompt);
        }
        if (sell_group && !wire_upgrade_group) {
            draw_command_overview_text(
                painter, 56, 88, field_menu_text_.shop_sale_quote_heading);
            draw_command_overview_number(
                painter, command_overview_font_, 224, 88,
                field_shop_menu_.transaction_amount(),
                NumericVisualAlignment::right, true);
        }
        draw_command_overview_text(
            painter, 128, 96, field_menu_text_.action_choice_yes);
        draw_command_overview_text(
            painter, 128, 112, field_menu_text_.action_choice_no);
        draw_command_overview_text(
            painter, 112,
            field_shop_menu_.transaction_confirmation().selected_option == 0U
                ? 96
                : 112,
            ">");
    } else {
        switch (field_shop_menu_.transaction_message()) {
        case ShopTransactionMessage::no_inventory_slot:
            draw_command_overview_text(
                painter, 96, 88, field_menu_text_.shop_no_inventory_slot);
            break;
        case ShopTransactionMessage::insufficient_funds:
            draw_command_overview_text(
                painter, 56, 88, field_menu_text_.shop_insufficient_funds);
            break;
        case ShopTransactionMessage::special_action_limit:
            draw_command_overview_text(
                painter, 56, 88,
                field_menu_text_.shop_special_action_limit);
            break;
        case ShopTransactionMessage::protected_sale:
            draw_command_overview_text(
                painter, 48, 96, field_menu_text_.shop_protected_sale);
            break;
        case ShopTransactionMessage::training_unavailable: {
            const auto* message = game_scripts_.dialogue_text(
                field_shop_active_definition_->text_keys
                    .progression_training_unavailable);
            if (message != nullptr) {
                draw_command_overview_text(painter, 64, 88, *message);
            }
            break;
        }
        case ShopTransactionMessage::cyberware_already_owned: {
            const auto* message = game_scripts_.dialogue_text(
                field_shop_active_definition_->text_keys
                    .cyberware_already_owned);
            if (message != nullptr) {
                draw_command_overview_text(painter, 64, 88, *message);
            }
            break;
        }
        case ShopTransactionMessage::cyberware_spurs_conflict: {
            const auto* message = game_scripts_.dialogue_text(
                field_shop_active_definition_->text_keys
                    .cyberware_spurs_conflict);
            if (message != nullptr) {
                draw_command_overview_text(painter, 72, 88, *message);
            }
            break;
        }
        case ShopTransactionMessage::cyberware_insufficient_essence: {
            const auto* message = game_scripts_.dialogue_text(
                field_shop_active_definition_->text_keys
                    .cyberware_insufficient_essence);
            if (message != nullptr) {
                draw_command_overview_text(painter, 64, 88, *message);
            }
            break;
        }
        case ShopTransactionMessage::none:
            break;
        }
    }
}

void FieldSceneRuntime::paint_field_shop_notice(QPainter& painter) const {
    if (!field_shop_notice_active_ || field_service_background_.isNull()) {
        return;
    }
    painter.drawImage(0, 0, field_service_background_);
    const auto* title = game_scripts_.dialogue_text(
        "shop.service.option." + field_shop_notice_service_id_);
    if (title != nullptr) {
        draw_command_overview_text(painter, 64, 16, *title);
    }
    const auto* message = game_scripts_.dialogue_text(
        field_shop_notice_text_key_);
    if (message != nullptr) {
        // ROM 0x56C12 writes the message at name-table word 0xC316. On the
        // source 64-cell plane this is pixel origin (88, 48).
        draw_command_overview_text(painter, 88, 48, *message);
    }
    paint_field_hud_overlays(painter);
    if (field_funds_.has_value()) {
        draw_command_overview_number(
            painter, command_overview_font_, 144, 200, *field_funds_,
            NumericVisualAlignment::left, true);
        draw_command_overview_text(painter, 200, 200, "$");
    }
}

void FieldSceneRuntime::draw_tinted_field_text(
    QPainter& painter, const int x, const int y,
    const std::string_view text, const QColor color,
    const bool opaque_palette_pixels) const {
    if (command_overview_font_.isNull() || command_overview_font_.width() < 8 ||
        command_overview_font_.height() < 8) {
        return;
    }
    auto cursor = x;
    auto cursor_y = y;
    for (const unsigned char code : text) {
        if (code == '\n') {
            cursor = x;
            cursor_y += 8;
            continue;
        }
        if (code < 0x20U || code > 0x7EU) {
            cursor += 8;
            continue;
        }
        const auto glyph = static_cast<int>(code - 0x20U);
        const auto source_x = (glyph % 16) * 8;
        const auto source_y = (glyph / 16) * 8;
        if (source_x + 8 > command_overview_font_.width() ||
            source_y + 8 > command_overview_font_.height()) {
            cursor += 8;
            continue;
        }
        QImage tinted_glyph(8, 8, QImage::Format_ARGB32_Premultiplied);
        tinted_glyph.fill(Qt::transparent);
        for (int glyph_y = 0; glyph_y < 8; ++glyph_y) {
            for (int glyph_x = 0; glyph_x < 8; ++glyph_x) {
                const auto source_pixel = command_overview_font_.pixelColor(
                    source_x + glyph_x, source_y + glyph_y);
                const auto coverage = std::max(
                    {source_pixel.red(), source_pixel.green(),
                     source_pixel.blue()});
                if (source_pixel.alpha() == 0 || coverage == 0) {
                    continue;
                }
                const auto alpha = static_cast<int>(
                    (static_cast<std::uint32_t>(source_pixel.alpha()) *
                     static_cast<std::uint32_t>(coverage)) /
                    255U);
                auto tinted_color = color;
                if (opaque_palette_pixels) {
                    const auto covered_channel = [alpha](const int channel) {
                        return (channel * alpha + 127) / 255;
                    };
                    tinted_color = QColor(
                        covered_channel(color.red()),
                        covered_channel(color.green()),
                        covered_channel(color.blue()));
                } else {
                    tinted_color.setAlpha(alpha);
                }
                tinted_glyph.setPixelColor(glyph_x, glyph_y,
                                           tinted_color);
            }
        }
        painter.drawImage(cursor, cursor_y, tinted_glyph);
        cursor += 8;
    }
}

void FieldSceneRuntime::draw_field_menu_font_text(
    QPainter& painter, const int x, const int y,
    const std::string_view text, const QImage& font) const {
    if (font.isNull() || font.width() < 8 || font.height() < 8) {
        return;
    }
    auto cursor = x;
    auto cursor_y = y;
    for (const auto character : text) {
        if (character == '\n') {
            cursor = x;
            cursor_y += 8;
            continue;
        }
        const auto code = static_cast<unsigned char>(character);
        if (code < 0x20U || code > 0x7FU) {
            cursor += 8;
            continue;
        }
        const auto glyph = static_cast<int>(code - 0x20U);
        const auto source_x = (glyph % 16) * 8;
        const auto source_y = (glyph / 16) * 8;
        if (source_x + 8 > font.width() || source_y + 8 > font.height()) {
            cursor += 8;
            continue;
        }
        painter.drawImage(cursor, cursor_y, font,
                          source_x, source_y, 8, 8);
        cursor += 8;
    }
}

void FieldSceneRuntime::draw_hotel_menu_text(
    QPainter& painter, const int x, const int y,
    const std::string_view text,
    const HotelMenuPaletteLine palette_line) const {
    const auto& font = palette_line == HotelMenuPaletteLine::checkout
                           ? hotel_checkout_font_
                           : command_overview_font_;
    if (font.isNull() || font.width() < 8 || font.height() < 8) {
        return;
    }
    auto cursor = x;
    auto cursor_y = y;
    for (const auto character : text) {
        if (character == '\n') {
            cursor = x;
            cursor_y += 8;
            continue;
        }
        const auto code = static_cast<unsigned char>(character);
        if (code < 0x20U || code > 0x7FU) {
            cursor += 8;
            continue;
        }
        const auto glyph = static_cast<int>(code - 0x20U);
        const auto source_x = (glyph % 16) * 8;
        const auto source_y = (glyph / 16) * 8;
        if (source_x + 8 <= font.width() &&
            source_y + 8 <= font.height()) {
            // Hotel pages select the CRAM line in the source text writer.
            // Keep the extracted RGBA colors intact for both green options
            // text and the purple nested checkout prompt.
            painter.drawImage(cursor, cursor_y, font, source_x, source_y, 8,
                              8);
        }
        cursor += 8;
    }
}

void FieldSceneRuntime::refresh_command_overview_combat_bars() {
    if (!command_overview_.open()) {
        command_overview_combat_bars_ = EquipmentCombatBarResult{};
        return;
    }

    const auto input = command_overview_combat_input();
    command_overview_combat_bars_ = compute_equipment_combat_bars(input);
}

EquipmentCombatInput FieldSceneRuntime::command_overview_combat_input() const {
    const auto& record = active_runner_record();
    EquipmentCombatInput input{};
    input.attributes = record.attributes;
    // The source combat adapter names the sparse +0x81..+0x8C window. The
    // live-record projection stores +0x80..+0x85 and +0x87..+0x8C as the
    // typed skill array, with +0x86 retained in auxiliary[1].
    input.skills = {record.skills[1],  record.skills[2], record.skills[3],
                    record.skills[4],  record.skills[5], record.auxiliary[1],
                    record.skills[6],  record.skills[7], record.skills[8],
                    record.skills[9],  record.skills[10], record.skills[11]};
    input.inventory_item_codes = record.inventory_item_codes;
    input.inventory_phase_bytes = record.inventory_phase_bytes;
    input.display_state = record.equipped_inventory_slot;
    input.active_item_code = record.equipped_item_code;
    input.posture = record.posture;
    // The command overview's source selector is outside the eight-cell item
    // grid. This keeps the normal higher-mode defense branch from treating a
    // highlighted possession cell as an action lookup.
    input.selected_inventory_slot =
        static_cast<std::uint8_t>(record.inventory_item_codes.size());
    input.cyberware_flags = record.cyberware_flags;
    input.action_flags = static_cast<std::uint8_t>(record.flags & 0xFFU);
    // ROM 0x12CEE's ordinary command-overview caller is in the higher-mode
    // branch. It uses +0x56/+0x57 directly for the attack strip and still
    // performs the typed defense calculation below it.
    input.source_mode = 2U;
    // When +0x56 is negative, 0x12D74/0x12FA4 uses the active action's
    // Sorcery, Magic cap, and current force before applying item-focus and
    // slot-7 totem data. Keep those values sourced from the selected live
    // runner record so class/race presets and the action's force remain part
    // of the displayed number.
    input.special_item18_base = record.skills[0U]; // +0x80 Sorcery.
    input.roll_count_cap = record.auxiliary[0U]; // +0x7F Magic cap.
    input.auxiliary_roll_bonus = record.magic_force; // +0x9D current force.
    if (input.display_state < 0 && input.active_item_code > 0U) {
        const auto* spell = find_us_magic_spell(static_cast<std::uint8_t>(
            input.active_item_code - 1U));
        if (spell != nullptr) {
            input.special_magic_spell = *spell;
        }
    }
    input.item_catalog = &us_item_catalog();
    input.selector_table = &us_special_item_modifier_selector_table();
    return input;
}

void FieldSceneRuntime::refresh_command_overview_item_detail() {
    command_overview_item_detail_ = ItemActionDetailResult{};
    if (!command_overview_.open() ||
        command_overview_.view().source_state.primary_page >= 2U) {
        return;
    }

    const auto slot =
        command_overview_.view().source_state.secondary_page * 2U +
        command_overview_.view().source_state.primary_page;
    if (slot >= active_runner_record().inventory_item_codes.size()) {
        return;
    }
    ItemActionDetailInput input{};
    input.item_code = active_runner_record().inventory_item_codes[slot];
    for (std::size_t index = 0;
         index < active_runner_record().inventory_phase_bytes.size();
         ++index) {
        input.phase_bytes[index] =
            active_runner_record().inventory_phase_bytes[index];
    }
    // The ninth byte in the source phase span is active record +0x76, the
    // one-based armor ordinal. Preserve it as typed data for 0x12A8C's
    // item-detail value selection.
    input.phase_bytes[8] = active_runner_record().equipped_armor_index;
    input.phase_slot = 8U;
    command_overview_item_detail_ = render_item_action_details(
        input, us_item_catalog(), command_overview_resource_variant_);
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
