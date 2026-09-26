#pragma once

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

} // namespace shadowrun::ghidra_conversion
