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

    return 0;
}
