#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <vector>

namespace shadowrun::ghidra_conversion {

constexpr std::size_t kFieldHudSlotCount = 3U;
constexpr std::size_t kFieldHudAmmoCounterCount = 64U;
constexpr std::uint16_t kFieldHudPlaneBase = 0xF000U;
constexpr std::uint16_t kFieldHudPlaneWidthTiles = 64U;
constexpr std::uint16_t kFieldHudSlotVramStride = 0x0400U;
// 0x35D4 4x4 portrait at F1C6. 0x3674 one-column bar at F1C4. 0x35F0 3x2
// item at F448. 0x36FC two-tile ammo at F4C4. Horizontal bar is F3C6.
constexpr std::uint16_t kFieldHudPortraitVram = 0xF1C6U;
constexpr std::uint16_t kFieldHudPhysicalBarVram = 0xF1C4U;
constexpr std::uint16_t kFieldHudMentalBarVram = 0xF3C6U;
constexpr std::uint16_t kFieldHudItemVram = 0xF448U;
constexpr std::uint16_t kFieldHudAmmoVram = 0xF4C4U;

constexpr int field_hud_vram_x(const std::uint16_t vram) {
    const auto word =
        static_cast<unsigned>((vram - kFieldHudPlaneBase) / 2U);
    return static_cast<int>((word % kFieldHudPlaneWidthTiles) * 8U);
}

constexpr int field_hud_vram_y(const std::uint16_t vram) {
    const auto word =
        static_cast<unsigned>((vram - kFieldHudPlaneBase) / 2U);
    return static_cast<int>((word / kFieldHudPlaneWidthTiles) * 8U);
}

enum class FieldHudOverlayKind {
    portrait,
    physical_bar,
    mental_bar,
    item,
    unarmed_item,
    ammo,
};

struct FieldHudOverlay {
    FieldHudOverlayKind kind = FieldHudOverlayKind::portrait;
    std::uint8_t slot = 0;
    int x = 0;
    int y = 0;
    std::uint8_t asset_index = 0;
};

struct FieldHudSlotState {
    // Live record +0x10. Bit 0 is the 0x3674/0x36FC/0x35D4 draw gate.
    std::uint8_t flags_byte = 0;
    // Live record +0x56. 0x7F is the source unarmed sentinel.
    std::int8_t equipped_inventory_slot = 0;
    std::uint8_t equipped_item_code = 0;
    std::uint8_t ammo_rounds = 0;
};

// ROM 0x35D4/0x35F0/0x36FC/0x3674: emit the nametable-backed overlays for
// each live field record whose +0x10 bit 0 is set. Inactive companion slots
// produce no overlays.
std::vector<FieldHudOverlay> compose_field_hud_overlays(
    const std::array<FieldHudSlotState, kFieldHudSlotCount>& slots);


struct FieldHudRenderCallbacks {
    std::function<void()> prepare_display;
    std::function<void()> render_sprite_stream;
    std::function<void()> upload_tile_block;
    std::function<void()> refresh_hud_state;
    std::function<std::uint8_t()> select_hud_mode;
    std::function<void()> draw_mode_panel;
    std::function<void()> advance_mode_panel;
    std::function<void()> draw_mode_footer;
};

struct FieldHudRenderResult {
    std::uint32_t uploaded_tile_blocks = 0;
    bool mode_panel_rendered = false;
};

struct FieldHudAssetTransferCallbacks {
    std::function<void()> write_wrapped_transfer;
    std::function<void()> write_grid;
    std::function<void()> draw_hud_panel_triplet;
};

// ROM 0x334C: emit one wrapped transfer followed by five grid writes.
void upload_field_asset_sequence(
    const FieldHudAssetTransferCallbacks& callbacks);

// ROM 0x3404: emit two groups of fourteen wrapped transfers, then draw the
// three HUD panels.
void upload_field_asset_setup(
    const FieldHudAssetTransferCallbacks& callbacks);

// ROM 0x17708: perform the field HUD render choreography. The source writes
// VDP state, streams five tile blocks, and enters the extra panel path only
// when the HUD flag is set and the selected mode is 8; host presentation is
// represented by typed callbacks instead of Genesis register writes.
FieldHudRenderResult render_field_hud(
    bool mode_panel_enabled,
    const FieldHudRenderCallbacks& callbacks);

// ROM 0x3548: invoke the source HUD-panel drawing leaf three times.
void draw_hud_panel_triplet(const std::function<void()>& draw_panel);

struct FieldHudPanelSlot {
    std::uint16_t palette_word = 0; // +0x2C
    std::uint8_t source_type = 0; // +0x57
    std::int8_t equipped_inventory_slot = 0; // +0x56
    std::uint8_t transition_rate = 0; // +0x9B
};

struct FieldHudPanelPlan {
    std::uint16_t tile_base = 0; // D5
    std::uint32_t nametable = 0; // D4
    std::uint16_t columns = 0; // D7+1
    std::uint16_t rows = 0; // D1+1
    std::uint16_t palette_or_flip = 0; // D2
};

struct FieldHudPanelPair {
    FieldHudPanelPlan item;
    FieldHudPanelPlan portrait;
};

// ROM 0x3602: D2 is 0x2000 or 0x6000 from +0x57/+0x56/+0x9B.
std::uint16_t select_hud_glyph_attribute_flags(const FieldHudPanelSlot& slot);

// ROM 0x3560: two nametable stamps per runner slot 0/1/2.
FieldHudPanelPair refresh_hud_panel_variant(
    std::uint16_t slot_index,
    const FieldHudPanelSlot& slot);

constexpr std::uint32_t kFieldHudDelayMeterTable = 0x0006AB3EU;
constexpr std::array<std::array<std::uint16_t, 4>, 11> kFieldHudDelayMeterTiles{{
    {{0x86E4U, 0x86E4U, 0x86E4U, 0x86E4U}},
    {{0x86E4U, 0x86E4U, 0x86E4U, 0x86E5U}},
    {{0x86E4U, 0x86E4U, 0x86E4U, 0x86E6U}},
    {{0x86E4U, 0x86E4U, 0x86E5U, 0x86E7U}},
    {{0x86E4U, 0x86E4U, 0x86E6U, 0x86E7U}},
    {{0x86E4U, 0x86E4U, 0x86E7U, 0x86E7U}},
    {{0x86E4U, 0x86E5U, 0x86E7U, 0x86E7U}},
    {{0x86E4U, 0x86E7U, 0x86E7U, 0x86E7U}},
    {{0x86E4U, 0x86E7U, 0x86E7U, 0x86E7U}},
    {{0x86E5U, 0x86E7U, 0x86E7U, 0x86E7U}},
    {{0x86E7U, 0x86E7U, 0x86E7U, 0x86E7U}},
}};

struct FieldHudDelayMeterState {
    std::uint16_t slot_word = 0; // +0x3E
    std::uint8_t flags_byte = 0; // +0x10
    std::int8_t meter = 0; // +0x4C
};

struct FieldHudDelayMeterPlan {
    std::uint32_t nametable = 0;
    std::uint8_t meter = 0;
    std::array<std::uint16_t, 4> tiles{};
};

// ROM 0x3674: +0x10 bit 0 gates ADD.B D1 into +0x4C, cap 10, else clear.
// Table 0x6AB3E is four words per meter. Slot word 0/1/else -> F1C4/F5C4/F9C4.
FieldHudDelayMeterPlan advance_field_hud_delay_meter(
    FieldHudDelayMeterState& state,
    std::int8_t delta);

constexpr std::array<std::uint8_t, 12> kFieldHudAmmoTypeCaps{{
    0x06U, 0x0CU, 0x0BU, 0x19U, 0x06U, 0x0AU, 0x0FU, 0x1EU, 0x1CU, 0x18U,
    0x05U, 0x08U}};

constexpr std::array<std::array<std::uint16_t, 2>, 31> kFieldHudAmmoMeterTiles{{
    {{0x86D7U, 0x8ED7U}}, {{0x86D7U, 0x86D8U}}, {{0x86D7U, 0x86D9U}},
    {{0x86D7U, 0x86DAU}}, {{0x86D7U, 0x86DBU}}, {{0x86D7U, 0x86DCU}},
    {{0x86D7U, 0x86DDU}}, {{0x86D7U, 0x86DEU}}, {{0x86D7U, 0x86DFU}},
    {{0x86D7U, 0x86E0U}}, {{0x86E1U, 0x8ED7U}}, {{0x86E1U, 0x86D8U}},
    {{0x86E1U, 0x86D9U}}, {{0x86E1U, 0x86DAU}}, {{0x86E1U, 0x86DBU}},
    {{0x86E1U, 0x86DCU}}, {{0x86E1U, 0x86DDU}}, {{0x86E1U, 0x86DEU}},
    {{0x86E1U, 0x86DFU}}, {{0x86E1U, 0x86E0U}}, {{0x86E2U, 0x8ED7U}},
    {{0x86E2U, 0x86D8U}}, {{0x86E2U, 0x86D9U}}, {{0x86E2U, 0x86DAU}},
    {{0x86E2U, 0x86DBU}}, {{0x86E2U, 0x86DCU}}, {{0x86E2U, 0x86DDU}},
    {{0x86E2U, 0x86DEU}}, {{0x86E2U, 0x86DFU}}, {{0x86E2U, 0x86E0U}},
    {{0x86E3U, 0x8ED7U}},
}};

struct FieldHudAmmoMeterState {
    std::uint16_t slot_word = 0; // +0x3E
    std::uint8_t flags_byte = 0; // +0x10
    std::int8_t equipped_inventory_slot = 0; // +0x56
    std::uint8_t source_type = 0; // +0x57
    std::uint8_t ammo = 0; // +0x4E
    std::array<std::uint8_t, 8> clip_counts{}; // +0x6E
    std::array<std::uint8_t, 8> clip_aux{}; // +0x66
};

struct FieldHudAmmoMeterPlan {
    std::uint32_t nametable = 0;
    std::uint8_t displayed = 0;
    std::array<std::uint16_t, 2> tiles{};
    bool reload_assets = false;
};

// ROM 0x36FC: F4C4/F8C4/FCC4. Unarmed or +0x10 bit 0 clear uses table
// 0x6AC46. Type < 0xD adds D1 into +0x4E and caps from 0xD4540+0x24.
// Type 0xD..0xF adds into +0x6E[slot] and reloads assets on underflow.
FieldHudAmmoMeterPlan advance_field_hud_ammo_meter(
    FieldHudAmmoMeterState& state,
    std::int8_t delta);

constexpr std::array<std::array<std::uint16_t, 4>, 11>
    kFieldHudMentalMeterTiles{{
        {{0x86F3U, 0x86F3U, 0x86F3U, 0x86F3U}},
        {{0x86F8U, 0x86F3U, 0x86F3U, 0x86F3U}},
        {{0x86F9U, 0x86F3U, 0x86F3U, 0x86F3U}},
        {{0x86FAU, 0x86F8U, 0x86F3U, 0x86F3U}},
        {{0x86FAU, 0x86FAU, 0x86F3U, 0x86F3U}},
        {{0x86FAU, 0x86FAU, 0x86F8U, 0x86F3U}},
        {{0x86FAU, 0x86FAU, 0x86F9U, 0x86F3U}},
        {{0x86FAU, 0x86FAU, 0x86FAU, 0x86F3U}},
        {{0x86FAU, 0x86FAU, 0x86FAU, 0x86F8U}},
        {{0x86FAU, 0x86FAU, 0x86FAU, 0x86F9U}},
        {{0x86FAU, 0x86FAU, 0x86FAU, 0x86FAU}},
    }};

constexpr std::array<std::array<std::uint16_t, 4>, 11>
    kFieldHudPrimaryMentalMeterTiles{{
        {{0x86EEU, 0x86EEU, 0x86EEU, 0x86EEU}},
        {{0x86EFU, 0x86EEU, 0x86EEU, 0x86EEU}},
        {{0x86F0U, 0x86EEU, 0x86EEU, 0x86EEU}},
        {{0x86F1U, 0x86EFU, 0x86EEU, 0x86EEU}},
        {{0x86F1U, 0x86F1U, 0x86EEU, 0x86EEU}},
        {{0x86F1U, 0x86F1U, 0x86EFU, 0x86EEU}},
        {{0x86F1U, 0x86F1U, 0x86F0U, 0x86EEU}},
        {{0x86F1U, 0x86F1U, 0x86F1U, 0x86EEU}},
        {{0x86F1U, 0x86F1U, 0x86F1U, 0x86EFU}},
        {{0x86F1U, 0x86F1U, 0x86F1U, 0x86F0U}},
        {{0x86F1U, 0x86F1U, 0x86F1U, 0x86F1U}},
    }};

struct FieldHudMentalMeterState {
    std::uint16_t slot_word = 0; // +0x3E
    std::uint8_t flags_byte = 0; // +0x10
    std::int8_t meter = 0; // +0x9C
    bool primary_runner = false; // A0 == FFFC04
};

struct FieldHudMentalMeterPlan {
    std::uint32_t nametable = 0;
    std::uint8_t meter = 0;
    std::array<std::uint16_t, 4> tiles{};
    std::int8_t delay_delta = 0;
    bool forward_delay_meter = false;
};

// ROM 0x37EC: F3C6/F7C6/FBC6. +0x10 bit 0 gates ADD.B into +0x9C, cap 10.
// Underflow forwards D1 into 0x3674 then clears +0x9C. FFFC04 uses 0x6ABEE.
FieldHudMentalMeterPlan advance_field_hud_mental_meter(
    FieldHudMentalMeterState& state,
    std::int8_t delta);

} // namespace shadowrun::ghidra_conversion
