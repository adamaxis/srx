#include "shadowrun_ghidra/field/field_hud_conversion.hpp"

#include <array>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<int> order;
    std::uint32_t uploads = 0U;
    const auto callbacks = FieldHudRenderCallbacks{
        [&order] { order.push_back(1); },
        [&order] { order.push_back(2); },
        [&order, &uploads] {
            order.push_back(3);
            ++uploads;
        },
        [&order] { order.push_back(4); },
        [&order] {
            order.push_back(5);
            return static_cast<std::uint8_t>(8U);
        },
        [&order] { order.push_back(6); },
        [&order] { order.push_back(7); },
        [&order] { order.push_back(8); },
    };
    const auto rendered = render_field_hud(true, callbacks);
    if (!expect(rendered.uploaded_tile_blocks == 5U && uploads == 5U &&
                rendered.mode_panel_rendered &&
                order == std::vector<int>{1, 2, 3, 3, 3, 3, 3, 4, 5, 6,
                                           7, 8})) {
        return 1;
    }

    order.clear();
    const auto skipped = render_field_hud(
        false,
        {
            [&order] { order.push_back(1); },
            [&order] { order.push_back(2); },
            [&order] { order.push_back(3); },
            [&order] { order.push_back(4); },
            [&order] {
                order.push_back(5);
                return static_cast<std::uint8_t>(8U);
            },
            [&order] { order.push_back(6); },
            [&order] { order.push_back(7); },
            [&order] { order.push_back(8); },
        });
    if (!expect(skipped.uploaded_tile_blocks == 5U &&
                !skipped.mode_panel_rendered &&
                order == std::vector<int>{1, 2, 3, 3, 3, 3, 3, 4})) {
        return 2;
    }

    int panel_draws = 0;
    draw_hud_panel_triplet([&panel_draws] { ++panel_draws; });
    if (!expect(panel_draws == 3)) {
        return 3;
    }

    std::vector<char> asset_order;
    upload_field_asset_sequence({
        [&asset_order] { asset_order.push_back('w'); },
        [&asset_order] { asset_order.push_back('g'); },
        {}});
    if (!expect(asset_order == std::vector<char>{'w', 'g', 'g', 'g', 'g', 'g'})) {
        return 4;
    }

    int wrapped_transfers = 0;
    int panel_triplets = 0;
    upload_field_asset_setup({
        [&wrapped_transfers] { ++wrapped_transfers; },
        {},
        [&panel_triplets] { ++panel_triplets; }});
    if (!expect(wrapped_transfers == 28 && panel_triplets == 1)) {
        return 5;
    }

    if (!expect(field_hud_vram_x(kFieldHudPortraitVram) == 280 &&
                field_hud_vram_y(kFieldHudPortraitVram) == 24 &&
                field_hud_vram_x(kFieldHudPhysicalBarVram) == 272 &&
                field_hud_vram_y(kFieldHudPhysicalBarVram) == 24 &&
                field_hud_vram_x(kFieldHudMentalBarVram) == 280 &&
                field_hud_vram_y(kFieldHudMentalBarVram) == 56 &&
                field_hud_vram_x(kFieldHudItemVram) == 288 &&
                field_hud_vram_y(kFieldHudItemVram) == 64 &&
                field_hud_vram_x(kFieldHudAmmoVram) == 272 &&
                field_hud_vram_y(kFieldHudAmmoVram) == 72)) {
        return 6;
    }

    std::array<FieldHudSlotState, kFieldHudSlotCount> empty_slots{};
    if (!expect(compose_field_hud_overlays(empty_slots).empty())) {
        return 7;
    }

    std::array<FieldHudSlotState, kFieldHudSlotCount> joshua{};
    joshua[0].flags_byte = 0x0FU;
    joshua[0].equipped_inventory_slot = 0;
    joshua[0].equipped_item_code = 1U;
    joshua[0].ammo_rounds = 18U;
    joshua[1].equipped_item_code = 2U;
    joshua[1].ammo_rounds = 7U;
    const auto active = compose_field_hud_overlays(joshua);
    if (!expect(active.size() == 5U &&
                active[0].kind == FieldHudOverlayKind::portrait &&
                active[0].slot == 0U && active[0].x == 280 &&
                active[0].y == 24 && active[0].asset_index == 0U &&
                active[1].kind == FieldHudOverlayKind::physical_bar &&
                active[1].x == 272 && active[1].y == 24 &&
                active[2].kind == FieldHudOverlayKind::mental_bar &&
                active[2].x == 280 && active[2].y == 56 &&
                active[3].kind == FieldHudOverlayKind::item &&
                active[3].x == 288 && active[3].y == 64 &&
                active[4].kind == FieldHudOverlayKind::ammo &&
                active[4].x == 272 && active[4].y == 72 &&
                active[4].asset_index == 18U)) {
        return 8;
    }

    joshua[0].equipped_inventory_slot = 0x7F;
    joshua[0].equipped_item_code = 0U;
    const auto unarmed = compose_field_hud_overlays(joshua);
    if (!expect(unarmed.size() == 5U &&
                unarmed[3].kind == FieldHudOverlayKind::unarmed_item &&
                unarmed[3].x == 288 && unarmed[3].y == 64)) {
        return 9;
    }

    std::array<FieldHudSlotState, kFieldHudSlotCount> companion{};
    companion[1].flags_byte = 0x01U;
    companion[1].ammo_rounds = 4U;
    const auto secondary = compose_field_hud_overlays(companion);
    if (!expect(secondary.size() == 5U && secondary[0].slot == 1U &&
                secondary[0].y == 88 && secondary[4].asset_index == 4U &&
                secondary[4].y == 136)) {
        return 10;
    }

    joshua[0].equipped_inventory_slot = 0;
    joshua[0].equipped_item_code = 1U;
    joshua[0].ammo_rounds = 64U;
    const auto ammo_overflow = compose_field_hud_overlays(joshua);
    if (!expect(ammo_overflow.size() == 4U &&
                ammo_overflow[3].kind == FieldHudOverlayKind::item)) {
        return 11;
    }

    FieldHudPanelSlot empty_panel{};
    if (!expect(select_hud_glyph_attribute_flags(empty_panel) == 0x2000U)) {
        return 12;
    }
    FieldHudPanelSlot armed_panel{};
    armed_panel.source_type = 0x0EU;
    armed_panel.equipped_inventory_slot = 0;
    armed_panel.transition_rate = 2U;
    if (!expect(select_hud_glyph_attribute_flags(armed_panel) == 0x6000U)) {
        return 13;
    }
    FieldHudPanelSlot high_type{};
    high_type.source_type = 0x0FU;
    high_type.equipped_inventory_slot = 0;
    high_type.transition_rate = 2U;
    if (!expect(select_hud_glyph_attribute_flags(high_type) == 0x2000U)) {
        return 14;
    }
    FieldHudPanelSlot unarmed_low{};
    unarmed_low.source_type = 5U;
    unarmed_low.equipped_inventory_slot = -1;
    unarmed_low.transition_rate = 1U;
    if (!expect(select_hud_glyph_attribute_flags(unarmed_low) == 0x2000U)) {
        return 15;
    }
    unarmed_low.source_type = 6U;
    if (!expect(select_hud_glyph_attribute_flags(unarmed_low) == 0x6000U)) {
        return 16;
    }

    const auto slot0 = refresh_hud_panel_variant(0, empty_panel);
    if (!expect(slot0.item.tile_base == 0x86A4U &&
                slot0.item.nametable == 0xF448U && slot0.item.columns == 4U &&
                slot0.item.rows == 4U && slot0.item.palette_or_flip == 0x2000U &&
                slot0.portrait.tile_base == 0x8694U &&
                slot0.portrait.nametable == 0xF1C6U &&
                slot0.portrait.columns == 3U && slot0.portrait.rows == 2U)) {
        return 17;
    }
    FieldHudPanelSlot paletted{};
    paletted.palette_word = 0x4000U;
    paletted.source_type = 0x0EU;
    paletted.transition_rate = 2U;
    const auto slot2 = refresh_hud_panel_variant(2, paletted);
    if (!expect(slot2.item.tile_base == 0x86D0U &&
                slot2.item.nametable == 0xFC48U &&
                slot2.item.palette_or_flip == 0x4000U &&
                slot2.portrait.tile_base == 0x86C0U &&
                slot2.portrait.nametable == 0xF9C6U &&
                slot2.portrait.palette_or_flip == 0x6000U)) {
        return 18;
    }
    const auto slot1 = refresh_hud_panel_variant(1, empty_panel);
    if (!expect(slot1.item.nametable == 0xF848U &&
                slot1.portrait.nametable == 0xF5C6U)) {
        return 19;
    }

    FieldHudDelayMeterState delay{};
    delay.flags_byte = 0x01U;
    auto delay_plan = advance_field_hud_delay_meter(delay, 4);
    if (!expect(delay.meter == 4 && delay_plan.nametable == 0xF1C4U &&
                delay_plan.tiles[3] == 0x86E7U &&
                delay_plan.tiles[2] == 0x86E6U)) {
        return 20;
    }
    delay_plan = advance_field_hud_delay_meter(delay, 10);
    if (!expect(delay.meter == 10 &&
                delay_plan.tiles == kFieldHudDelayMeterTiles[10])) {
        return 21;
    }
    delay.flags_byte = 0;
    delay.slot_word = 1;
    delay_plan = advance_field_hud_delay_meter(delay, 4);
    if (!expect(delay.meter == 0 && delay_plan.nametable == 0xF5C4U &&
                delay_plan.tiles == kFieldHudDelayMeterTiles[0])) {
        return 22;
    }
    delay.slot_word = 2;
    delay.flags_byte = 0x01U;
    delay_plan = advance_field_hud_delay_meter(delay, -1);
    if (!expect(delay.meter == 0 && delay_plan.nametable == 0xF9C4U)) {
        return 23;
    }

    FieldHudAmmoMeterState ammo{};
    ammo.flags_byte = 0x01U;
    ammo.equipped_inventory_slot = 0;
    ammo.source_type = 1;
    auto ammo_plan = advance_field_hud_ammo_meter(ammo, 4);
    if (!expect(ammo.ammo == 4 && ammo_plan.nametable == 0xF4C4U &&
                ammo_plan.tiles == kFieldHudAmmoMeterTiles[4] &&
                !ammo_plan.reload_assets)) {
        return 24;
    }
    ammo_plan = advance_field_hud_ammo_meter(ammo, 10);
    if (!expect(ammo.ammo == 6 &&
                ammo_plan.tiles == kFieldHudAmmoMeterTiles[6])) {
        return 25;
    }
    ammo.equipped_inventory_slot = static_cast<std::int8_t>(0x7F);
    ammo_plan = advance_field_hud_ammo_meter(ammo, 1);
    if (!expect(ammo_plan.tiles == kFieldHudAmmoMeterTiles[0])) {
        return 26;
    }
    ammo = FieldHudAmmoMeterState{};
    ammo.flags_byte = 0x01U;
    ammo.equipped_inventory_slot = 2;
    ammo.source_type = 0x0EU;
    ammo.clip_counts[2] = 3;
    ammo.slot_word = 1;
    ammo_plan = advance_field_hud_ammo_meter(ammo, -1);
    if (!expect(ammo.clip_counts[2] == 2 && ammo_plan.nametable == 0xF8C4U &&
                ammo_plan.tiles == kFieldHudAmmoMeterTiles[2])) {
        return 27;
    }
    ammo_plan = advance_field_hud_ammo_meter(ammo, -5);
    if (!expect(ammo.clip_counts[2] == 0 &&
                ammo.equipped_inventory_slot == static_cast<std::int8_t>(0x7F) &&
                ammo.source_type == 0U && ammo_plan.reload_assets &&
                ammo_plan.tiles == kFieldHudAmmoMeterTiles[0])) {
        return 28;
    }

    FieldHudMentalMeterState mental{};
    mental.flags_byte = 0x01U;
    mental.primary_runner = true;
    auto mental_plan = advance_field_hud_mental_meter(mental, 4);
    if (!expect(mental.meter == 4 && mental_plan.nametable == 0xF3C6U &&
                mental_plan.tiles == kFieldHudPrimaryMentalMeterTiles[4] &&
                !mental_plan.forward_delay_meter)) {
        return 29;
    }
    mental_plan = advance_field_hud_mental_meter(mental, 10);
    if (!expect(mental.meter == 10 &&
                mental_plan.tiles == kFieldHudPrimaryMentalMeterTiles[10])) {
        return 30;
    }
    mental.primary_runner = false;
    mental.slot_word = 1;
    mental_plan = advance_field_hud_mental_meter(mental, -20);
    if (!expect(mental.meter == 0 && mental_plan.nametable == 0xF7C6U &&
                mental_plan.forward_delay_meter &&
                mental_plan.delay_delta < 0 &&
                mental_plan.tiles == kFieldHudMentalMeterTiles[0])) {
        return 31;
    }

    FieldHudAmmoMeterState fired{};
    fired.flags_byte = 0x01U;
    fired.equipped_inventory_slot = 0;
    fired.source_type = 1;
    fired.ammo = 5;
    auto fired_plan = service_field_hud_ammo(fired);
    if (!expect(fired.ammo == 4 &&
                fired_plan.tiles == kFieldHudAmmoMeterTiles[4])) {
        return 32;
    }
    fired.source_type = 8;
    fired.ammo = 5;
    fired_plan = service_field_hud_ammo(fired);
    if (!expect(fired.ammo == 2 &&
                fired_plan.tiles == kFieldHudAmmoMeterTiles[2])) {
        return 33;
    }
    fired.source_type = 0;
    fired.ammo = 5;
    fired_plan = service_field_hud_ammo(fired);
    if (!expect(fired.ammo == 5 && fired_plan.displayed == 0)) {
        return 34;
    }
    fired.source_type = 1;
    fired.slot_word = 3;
    fired_plan = service_field_hud_ammo(fired);
    if (!expect(fired.ammo == 4 && fired_plan.displayed == 4)) {
        return 35;
    }

    std::array<FieldHudMeterBundle, 3> meters{};
    meters[0].delay.flags_byte = 0x01U;
    meters[0].delay.meter = 4;
    meters[0].mental.flags_byte = 0x01U;
    meters[0].mental.meter = 3;
    meters[0].mental.primary_runner = true;
    meters[0].ammo.flags_byte = 0x01U;
    meters[0].ammo.equipped_inventory_slot = 0;
    meters[0].ammo.source_type = 1;
    meters[0].ammo.ammo = 6;
    const auto triplet = refresh_field_hud_meter_triplet(meters);
    if (!expect(meters[0].delay.meter == 4 && meters[0].mental.meter == 3 &&
                meters[0].ammo.ammo == 6 &&
                triplet.delay[0].tiles == kFieldHudDelayMeterTiles[4] &&
                triplet.mental[0].tiles == kFieldHudPrimaryMentalMeterTiles[3] &&
                triplet.ammo[0].tiles == kFieldHudAmmoMeterTiles[6])) {
        return 36;
    }

    return 0;
}
