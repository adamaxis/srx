#include "shadowrun_ghidra/field/field_hud_conversion.hpp"
#include "shadowrun_ghidra/menus/menu_service_sequence_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {
namespace {

bool slot_is_active(const FieldHudSlotState& slot) {
    return (slot.flags_byte & 0x01U) != 0U;
}

bool slot_is_armed(const FieldHudSlotState& slot) {
    return slot.equipped_inventory_slot >= 0 &&
           static_cast<std::uint8_t>(slot.equipped_inventory_slot) !=
               0x7FU &&
           slot.equipped_item_code != 0U &&
           slot.equipped_item_code <= kFieldHudItemCount;
}

bool slot_has_active_spell(const FieldHudSlotState& slot) {
    return slot.equipped_inventory_slot < 0 &&
           slot.equipped_item_code != 0U &&
           slot.equipped_item_code <= kFieldHudMagicItemCount;
}

std::uint8_t magic_item_palette_variant(const FieldHudSlotState& slot) {
    const auto flags = select_hud_glyph_attribute_flags(HudGlyphAttributeInput{
        static_cast<std::int8_t>(slot.equipped_item_code),
        static_cast<std::int8_t>(slot.class_code),
        slot.equipped_inventory_slot});
    return flags == 0x6000U ? 1U : 0U;
}

std::uint8_t ordinary_item_palette_variant(const FieldHudSlotState& slot) {
    const auto flags = select_hud_glyph_attribute_flags(HudGlyphAttributeInput{
        static_cast<std::int8_t>(slot.equipped_item_code),
        static_cast<std::int8_t>(slot.class_code),
        slot.equipped_inventory_slot});
    return flags == 0x6000U ? 2U : 0U;
}

std::uint8_t portrait_palette_variant(const std::uint16_t palette_word) {
    const auto palette_line = static_cast<std::uint8_t>(
        (palette_word & 0x6000U) >> 13U);
    return palette_line == 0U
        ? 0U
        : static_cast<std::uint8_t>(palette_line - 1U);
}

std::uint16_t slot_vram(const std::uint16_t base, const std::uint8_t slot) {
    return static_cast<std::uint16_t>(
        base + static_cast<std::uint16_t>(slot) * kFieldHudSlotVramStride);
}

std::uint8_t meter_asset_index(const std::int8_t meter) {
    if (meter <= 0) {
        return 0U;
    }
    return static_cast<std::uint8_t>(std::min<int>(
        static_cast<int>(meter),
        static_cast<int>(kFieldHudMeterLevelCount - 1U)));
}

FieldHudOverlay make_overlay(const FieldHudOverlayKind kind,
                             const std::uint8_t slot,
                             const std::uint16_t vram,
                             const std::uint8_t asset_index,
                             const std::uint8_t palette_variant = 0U) {
    return FieldHudOverlay{kind, slot, field_hud_vram_x(vram),
                           field_hud_vram_y(vram), asset_index,
                           palette_variant};
}

} // namespace

std::vector<FieldHudOverlay> compose_field_hud_overlays(
    const std::array<FieldHudSlotState, kFieldHudSlotCount>& slots) {
    std::vector<FieldHudOverlay> overlays;
    for (std::uint8_t slot = 0; slot < kFieldHudSlotCount; ++slot) {
        if (!slot_is_active(slots[slot])) {
            continue;
        }
        // The face family follows the runner's selected portrait. The item
        // family follows the runner's +0x2C word; ordinary item art follows
        // the live +0x57/+0x56/+0x9B rule in 0x3602. Spells use the shared
        // D4EA0 row when +0x56 is negative.
        const auto family = slots[slot].asset_family_index;
        overlays.push_back(make_overlay(FieldHudOverlayKind::portrait, slot,
                                        slot_vram(kFieldHudPortraitVram, slot),
                                        family,
                                        portrait_palette_variant(
                                            slots[slot].portrait_palette_word)));
        overlays.push_back(
            make_overlay(FieldHudOverlayKind::physical_bar, slot,
                         slot_vram(kFieldHudPhysicalBarVram, slot),
                         meter_asset_index(slots[slot].delay_meter)));
        overlays.push_back(
            make_overlay(FieldHudOverlayKind::mental_bar, slot,
                         slot_vram(kFieldHudMentalBarVram, slot),
                         static_cast<std::uint8_t>(
                             (slots[slot].primary_runner ? 0U
                                 : kFieldHudMeterLevelCount) +
                             meter_asset_index(slots[slot].behavior_meter))));
        if (slot_has_active_spell(slots[slot])) {
            overlays.push_back(make_overlay(
                FieldHudOverlayKind::magic_item, slot,
                slot_vram(kFieldHudItemVram, slot),
                static_cast<std::uint8_t>(
                    slots[slot].equipped_item_code - 1U),
                magic_item_palette_variant(slots[slot])));
        } else if (slot_is_armed(slots[slot])) {
            overlays.push_back(
                make_overlay(FieldHudOverlayKind::item, slot,
                             slot_vram(kFieldHudItemVram, slot),
                             static_cast<std::uint8_t>(
                                 slots[slot].equipped_item_code - 1U),
                             ordinary_item_palette_variant(slots[slot])));
        } else {
            overlays.push_back(
                make_overlay(FieldHudOverlayKind::unarmed_item, slot,
                             slot_vram(kFieldHudItemVram, slot),
                             slots[slot].unarmed_item_variant,
                             ordinary_item_palette_variant(slots[slot])));
        }
        if (slots[slot].ammo_display_value < kFieldHudAmmoCounterCount) {
            overlays.push_back(make_overlay(
                FieldHudOverlayKind::ammo, slot,
                slot_vram(kFieldHudAmmoVram, slot),
                slots[slot].ammo_display_value));
        }
    }
    return overlays;
}


void upload_field_asset_sequence(
    const FieldHudAssetTransferCallbacks& callbacks) {
    if (callbacks.write_wrapped_transfer) {
        callbacks.write_wrapped_transfer();
    }
    if (callbacks.write_grid) {
        for (int grid = 0; grid < 5; ++grid) {
            callbacks.write_grid();
        }
    }
}

void upload_field_asset_setup(
    const FieldHudAssetTransferCallbacks& callbacks) {
    if (callbacks.write_wrapped_transfer) {
        for (int group = 0; group < 2; ++group) {
            for (int transfer = 0; transfer < 14; ++transfer) {
                callbacks.write_wrapped_transfer();
            }
        }
    }
    if (callbacks.draw_hud_panel_triplet) {
        callbacks.draw_hud_panel_triplet();
    }
}

FieldHudRenderResult render_field_hud(
    const bool mode_panel_enabled,
    const FieldHudRenderCallbacks& callbacks) {
    FieldHudRenderResult result{};
    if (callbacks.prepare_display) {
        callbacks.prepare_display();
    }
    if (callbacks.render_sprite_stream) {
        callbacks.render_sprite_stream();
    }
    if (callbacks.upload_tile_block) {
        for (std::uint32_t block = 0U; block < 5U; ++block) {
            callbacks.upload_tile_block();
            ++result.uploaded_tile_blocks;
        }
    }
    if (callbacks.refresh_hud_state) {
        callbacks.refresh_hud_state();
    }

    if (mode_panel_enabled && callbacks.select_hud_mode &&
        callbacks.select_hud_mode() == 8U) {
        if (callbacks.draw_mode_panel) {
            callbacks.draw_mode_panel();
        }
        if (callbacks.advance_mode_panel) {
            callbacks.advance_mode_panel();
        }
        if (callbacks.draw_mode_footer) {
            callbacks.draw_mode_footer();
        }
        result.mode_panel_rendered = true;
    }
    return result;
}

void draw_hud_panel_triplet(const std::function<void()>& draw_panel) {
    if (!draw_panel) {
        return;
    }
    for (int pass = 0; pass < 3; ++pass) {
        draw_panel();
    }
}

void draw_hud_panel(
    const FieldHudPanelPlan& plan,
    const WordTransferCallbacks& callbacks) {
    auto tile = static_cast<std::uint16_t>(
        plan.tile_base | plan.palette_or_flip);
    auto destination = static_cast<std::uint16_t>(plan.nametable);
    for (std::uint16_t row = 0; row < plan.rows; ++row) {
        if (callbacks.begin_transfer) {
            callbacks.begin_transfer(make_vdp_transfer_command(destination));
        }
        for (std::uint16_t column = 0; column < plan.columns; ++column) {
            if (callbacks.write_word) {
                callbacks.write_word(tile);
            }
            ++tile;
        }
        destination = static_cast<std::uint16_t>(destination + 0x80U);
    }
}

void draw_menu_hud_panel(const WordTransferCallbacks& callbacks) {
    draw_hud_panel(kMenuHudPanelPlan, callbacks);
}

std::uint16_t select_hud_glyph_attribute_flags(const FieldHudPanelSlot& slot) {
    if (slot.source_type == 0U) {
        return 0x2000U;
    }
    if (slot.transition_rate < 2U && slot.equipped_inventory_slot < 0) {
        return slot.source_type >= 6U ? 0x6000U : 0x2000U;
    }
    return slot.source_type < 0x0FU ? 0x6000U : 0x2000U;
}

FieldHudPanelPair refresh_hud_panel_variant(
    const std::uint16_t slot_index,
    const FieldHudPanelSlot& slot) {
    const std::uint16_t portrait_palette =
        slot.palette_word != 0U ? slot.palette_word : 0x2000U;
    const std::uint16_t item_palette =
        select_hud_glyph_attribute_flags(slot);
    if (slot_index == 0U) {
        return {{0x86A4U, 0xF448U, 3U, 2U, item_palette},
                {0x8694U, 0xF1C6U, 4U, 4U, portrait_palette}};
    }
    if (slot_index == 1U) {
        return {{0x86BAU, 0xF848U, 3U, 2U, item_palette},
                {0x86AAU, 0xF5C6U, 4U, 4U, portrait_palette}};
    }
    return {{0x86D0U, 0xFC48U, 3U, 2U, item_palette},
            {0x86C0U, 0xF9C6U, 4U, 4U, portrait_palette}};
}

FieldHudDelayMeterPlan advance_field_hud_delay_meter(
    FieldHudDelayMeterState& state,
    const std::int8_t delta) {
    FieldHudDelayMeterPlan plan{};
    if (state.slot_word == 0U) {
        plan.nametable = 0xF1C4U;
    } else if (state.slot_word == 1U) {
        plan.nametable = 0xF5C4U;
    } else {
        plan.nametable = 0xF9C4U;
    }
    if ((state.flags_byte & 0x01U) != 0U) {
        state.meter = static_cast<std::int8_t>(state.meter + delta);
        if (state.meter > 0) {
            if (state.meter > 10) {
                state.meter = 10;
            }
            plan.meter = static_cast<std::uint8_t>(state.meter);
            plan.tiles = kFieldHudDelayMeterTiles[plan.meter];
            return plan;
        }
    }
    state.meter = 0;
    plan.meter = 0;
    plan.tiles = kFieldHudDelayMeterTiles[0];
    return plan;
}

namespace {

std::array<std::uint16_t, 2> ammo_tiles_for(const std::uint8_t displayed) {
    const auto index = displayed < kFieldHudAmmoMeterTiles.size()
                           ? displayed
                           : static_cast<std::uint8_t>(
                                 kFieldHudAmmoMeterTiles.size() - 1U);
    return kFieldHudAmmoMeterTiles[index];
}

} // namespace

FieldHudAmmoMeterPlan advance_field_hud_ammo_meter(
    FieldHudAmmoMeterState& state,
    const std::int8_t delta) {
    FieldHudAmmoMeterPlan plan{};
    if (state.slot_word == 0U) {
        plan.nametable = 0xF4C4U;
    } else if (state.slot_word == 1U) {
        plan.nametable = 0xF8C4U;
    } else {
        plan.nametable = 0xFCC4U;
    }
    const auto unarmed =
        (state.flags_byte & 0x01U) == 0U ||
        state.equipped_inventory_slot < 0 ||
        static_cast<std::uint8_t>(state.equipped_inventory_slot) == 0x7FU;
    if (unarmed) {
        plan.tiles = kFieldHudAmmoMeterTiles[0];
        return plan;
    }
    if (state.source_type < 0x0DU) {
        state.ammo = static_cast<std::uint8_t>(
            static_cast<std::int8_t>(state.ammo) + delta);
        if (static_cast<std::int8_t>(state.ammo) <= 0) {
            state.ammo = 0U;
            plan.tiles = kFieldHudAmmoMeterTiles[0];
            return plan;
        }
        if (state.source_type >= 1U &&
            state.source_type <= kFieldHudAmmoTypeCaps.size()) {
            const auto cap = kFieldHudAmmoTypeCaps[state.source_type - 1U];
            if (state.ammo > cap) {
                state.ammo = cap;
            }
        }
        plan.displayed = state.ammo;
        plan.tiles = ammo_tiles_for(plan.displayed);
        return plan;
    }
    if (state.source_type >= 0x10U) {
        plan.tiles = kFieldHudAmmoMeterTiles[0];
        return plan;
    }
    const auto slot =
        static_cast<std::uint8_t>(state.equipped_inventory_slot);
    if (slot >= state.clip_counts.size()) {
        plan.tiles = kFieldHudAmmoMeterTiles[0];
        return plan;
    }
    state.clip_counts[slot] = static_cast<std::uint8_t>(
        static_cast<std::int8_t>(state.clip_counts[slot]) + delta);
    if (static_cast<std::int8_t>(state.clip_counts[slot]) <= 0) {
        state.clip_counts[slot] = 0U;
        state.clip_aux[slot] = 0U;
        state.equipped_inventory_slot = static_cast<std::int8_t>(0x7F);
        state.source_type = 0U;
        plan.reload_assets = true;
        plan.tiles = kFieldHudAmmoMeterTiles[0];
        return plan;
    }
    plan.displayed = state.clip_counts[slot];
    plan.tiles = ammo_tiles_for(plan.displayed);
    return plan;
}

FieldHudMentalMeterPlan advance_field_hud_mental_meter(
    FieldHudMentalMeterState& state,
    const std::int8_t delta) {
    FieldHudMentalMeterPlan plan{};
    if (state.slot_word == 0U) {
        plan.nametable = 0xF3C6U;
    } else if (state.slot_word == 1U) {
        plan.nametable = 0xF7C6U;
    } else {
        plan.nametable = 0xFBC6U;
    }
    const auto& tiles = state.primary_runner ? kFieldHudPrimaryMentalMeterTiles
                                             : kFieldHudMentalMeterTiles;
    if ((state.flags_byte & 0x01U) != 0U) {
        state.meter = static_cast<std::int8_t>(state.meter + delta);
        if (state.meter > 0) {
            if (state.meter > 10) {
                state.meter = 10;
            }
            plan.meter = static_cast<std::uint8_t>(state.meter);
            plan.tiles = tiles[plan.meter];
            return plan;
        }
        plan.delay_delta = state.meter;
        plan.forward_delay_meter = true;
    }
    state.meter = 0;
    plan.meter = 0;
    plan.tiles = tiles[0];
    return plan;
}

FieldHudAmmoMeterPlan service_field_hud_ammo(FieldHudAmmoMeterState& state) {
    if (state.ammo == 0U || state.source_type == 0U) {
        FieldHudAmmoMeterPlan plan{};
        if (state.slot_word == 0U) {
            plan.nametable = 0xF4C4U;
        } else if (state.slot_word == 1U) {
            plan.nametable = 0xF8C4U;
        } else {
            plan.nametable = 0xFCC4U;
        }
        return plan;
    }
    if (state.slot_word > 2U) {
        state.ammo = static_cast<std::uint8_t>(state.ammo - 1U);
        FieldHudAmmoMeterPlan plan{};
        plan.displayed = state.ammo;
        return plan;
    }
    const auto delta =
        (state.source_type >= 8U && state.source_type < 0x0BU) ? -3 : -1;
    return advance_field_hud_ammo_meter(state, static_cast<std::int8_t>(delta));
}

FieldHudMeterTripletPlan refresh_field_hud_meter_triplet(
    std::array<FieldHudMeterBundle, 3>& slots) {
    FieldHudMeterTripletPlan plan{};
    for (std::size_t index = 0; index < slots.size(); ++index) {
        plan.delay[index] = advance_field_hud_delay_meter(slots[index].delay, 0);
        plan.mental[index] =
            advance_field_hud_mental_meter(slots[index].mental, 0);
        plan.ammo[index] = advance_field_hud_ammo_meter(slots[index].ammo, 0);
    }
    return plan;
}

} // namespace shadowrun::ghidra_conversion
