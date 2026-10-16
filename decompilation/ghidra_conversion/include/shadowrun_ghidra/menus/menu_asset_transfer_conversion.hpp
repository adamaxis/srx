#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct MenuGraphicsState {
    std::uint16_t display_command = 0;
    std::size_t uploaded_tile_blocks = 0;
    bool optional_tile_uploaded = false;
    bool resource_metadata_ready = false;
    bool resource_upload_requested = false;
};

struct MenuGraphicsPreparationCallbacks {
    std::function<void(std::uint16_t)> set_display_command;
    std::function<void()> prepare_menu_buffers;
    std::function<void()> copy_resource_metadata;
    std::function<void()> decode_and_upload_resource;
};

// ROM 0x10918: prepare the common menu graphics state, copy the source
// resource metadata, and hand the decoded bytes to the presentation boundary.
void prepare_menu_graphics(
    MenuGraphicsState& state,
    const MenuGraphicsPreparationCallbacks& callbacks);

struct MenuAssetBatchPlan {
    std::size_t tile_blocks = 0;
    bool command_before_prepare = false;
    bool command_after_prepare = false;
    bool optional_block_enabled = false;
    bool optional_block_selected = false;
    bool copy_buffer_before_upload = false;
};

struct MenuAssetBatchCallbacks {
    std::function<void()> prepare_menu_graphics;
    std::function<void(std::uint16_t)> set_display_command;
    std::function<void()> copy_buffer_before_upload;
    std::function<void()> upload_tile_block;
};

// ROM 0x10984, 0x10A7E, 0x10AF0, 0x10B5A, 0x10BC4, and 0x10C96: run a
// source-specific menu transfer plan through one counted upload loop. The
// plan records the few wrapper differences without reproducing VDP state.
void transfer_menu_asset_batch(
    MenuGraphicsState& state,
    const MenuAssetBatchPlan& plan,
    const MenuAssetBatchCallbacks& callbacks);

struct MenuAssetGridWrite {
    std::uint32_t glyph_source = 0;
    std::uint16_t destination = 0;
    std::uint16_t height = 0; // D7
    std::uint16_t width = 0;  // D1
};

// ROM 0x10AF0: four 0x1232 writes after 0x10918, then VDP command 0x8174.
constexpr std::array<MenuAssetGridWrite, 4> kMenuAssetBatchFourA{{
    {0x000CDCE6U, 0xE000U, 0x21U, 0x03U},
    {0x000CDDF6U, 0xE200U, 0x04U, 0x10U},
    {0x000CDEA0U, 0xE23AU, 0x04U, 0x10U},
    {0x000CDF4AU, 0xEA80U, 0x21U, 0x06U},
}};

// ROM 0x10B5A: same sources, destinations +6.
constexpr std::array<MenuAssetGridWrite, 4> kMenuAssetBatchFourB{{
    {0x000CDCE6U, 0xE006U, 0x21U, 0x03U},
    {0x000CDDF6U, 0xE206U, 0x04U, 0x10U},
    {0x000CDEA0U, 0xE240U, 0x04U, 0x10U},
    {0x000CDF4AU, 0xEA86U, 0x21U, 0x06U},
}};

// ROM 0x10C96: seven 0x1232 writes, D7/D1 persist across pairs.
constexpr std::array<MenuAssetGridWrite, 7> kMenuAssetBatchSeven{{
    {0x000CE850U, 0xE000U, 0x13U, 0x06U},
    {0x000CE968U, 0xE028U, 0x13U, 0x06U},
    {0x000CEA80U, 0xE380U, 0x01U, 0x14U},
    {0x000CEAD4U, 0xE3CCU, 0x01U, 0x14U},
    {0x000CEB28U, 0xE584U, 0x23U, 0x01U},
    {0x000CEBB8U, 0xE984U, 0x23U, 0x01U},
    {0x000CEC48U, 0xED04U, 0x23U, 0x01U},
}};

} // namespace shadowrun::ghidra_conversion
