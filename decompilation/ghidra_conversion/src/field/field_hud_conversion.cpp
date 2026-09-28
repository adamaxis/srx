#include "shadowrun_ghidra/field/field_hud_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

bool slot_is_active(const FieldHudSlotState& slot) {
    return (slot.flags_byte & 0x01U) != 0U;
}

bool slot_is_armed(const FieldHudSlotState& slot) {
    return slot.equipped_inventory_slot >= 0 &&
           static_cast<std::uint8_t>(slot.equipped_inventory_slot) !=
               0x7FU &&
           slot.equipped_item_code != 0U;
}

std::uint16_t slot_vram(const std::uint16_t base, const std::uint8_t slot) {
    return static_cast<std::uint16_t>(
        base + static_cast<std::uint16_t>(slot) * kFieldHudSlotVramStride);
}

FieldHudOverlay make_overlay(const FieldHudOverlayKind kind,
                             const std::uint8_t slot,
                             const std::uint16_t vram,
                             const std::uint8_t asset_index) {
    return FieldHudOverlay{kind, slot, field_hud_vram_x(vram),
                           field_hud_vram_y(vram), asset_index};
}

} // namespace

std::vector<FieldHudOverlay> compose_field_hud_overlays(
    const std::array<FieldHudSlotState, kFieldHudSlotCount>& slots) {
    std::vector<FieldHudOverlay> overlays;
    for (std::uint8_t slot = 0; slot < kFieldHudSlotCount; ++slot) {
        if (!slot_is_active(slots[slot])) {
            continue;
        }
        overlays.push_back(make_overlay(FieldHudOverlayKind::portrait, slot,
                                        slot_vram(kFieldHudPortraitVram, slot),
                                        slot));
        overlays.push_back(
            make_overlay(FieldHudOverlayKind::physical_bar, slot,
                         slot_vram(kFieldHudPhysicalBarVram, slot), slot));
        overlays.push_back(
            make_overlay(FieldHudOverlayKind::mental_bar, slot,
                         slot_vram(kFieldHudMentalBarVram, slot), slot));
        if (slot_is_armed(slots[slot])) {
            overlays.push_back(
                make_overlay(FieldHudOverlayKind::item, slot,
                             slot_vram(kFieldHudItemVram, slot), slot));
        } else {
            overlays.push_back(
                make_overlay(FieldHudOverlayKind::unarmed_item, slot,
                             slot_vram(kFieldHudItemVram, slot), slot));
        }
        if (slots[slot].ammo_rounds < kFieldHudAmmoCounterCount) {
            overlays.push_back(make_overlay(
                FieldHudOverlayKind::ammo, slot,
                slot_vram(kFieldHudAmmoVram, slot),
                slots[slot].ammo_rounds));
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

} // namespace shadowrun::ghidra_conversion
