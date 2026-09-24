#include "shadowrun_ghidra/menu_asset_transfer_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

constexpr std::uint16_t kMenuTransferCommand = 0x8134U;
constexpr std::uint16_t kMenuDisplayCommand = 0x8174U;

void set_command(
    MenuGraphicsState& state,
    const MenuAssetBatchCallbacks& callbacks,
    const std::uint16_t command) {
    state.display_command = command;
    if (callbacks.set_display_command) {
        callbacks.set_display_command(command);
    }
}

} // namespace

void prepare_menu_graphics(
    MenuGraphicsState& state,
    const MenuGraphicsPreparationCallbacks& callbacks) {
    state.display_command = kMenuTransferCommand;
    if (callbacks.set_display_command) {
        callbacks.set_display_command(kMenuTransferCommand);
    }
    if (callbacks.prepare_menu_buffers) {
        callbacks.prepare_menu_buffers();
    }
    if (callbacks.copy_resource_metadata) {
        callbacks.copy_resource_metadata();
    }
    state.resource_metadata_ready = true;
    state.resource_upload_requested = true;
    if (callbacks.decode_and_upload_resource) {
        callbacks.decode_and_upload_resource();
    }
}

void transfer_menu_asset_batch(
    MenuGraphicsState& state,
    const MenuAssetBatchPlan& plan,
    const MenuAssetBatchCallbacks& callbacks) {
    if (plan.command_before_prepare) {
        set_command(state, callbacks, kMenuTransferCommand);
    }
    if (callbacks.prepare_menu_graphics) {
        callbacks.prepare_menu_graphics();
    }
    if (plan.command_after_prepare) {
        set_command(state, callbacks, kMenuTransferCommand);
    }
    if (plan.copy_buffer_before_upload && callbacks.copy_buffer_before_upload) {
        callbacks.copy_buffer_before_upload();
    }

    if (callbacks.upload_tile_block) {
        for (std::size_t block = 0; block < plan.tile_blocks; ++block) {
            callbacks.upload_tile_block();
            ++state.uploaded_tile_blocks;
        }
        if (plan.optional_block_enabled && plan.optional_block_selected) {
            callbacks.upload_tile_block();
            ++state.uploaded_tile_blocks;
            state.optional_tile_uploaded = true;
        }
    }
    set_command(state, callbacks, kMenuDisplayCommand);
}

} // namespace shadowrun::ghidra_conversion
