// Ghidra headless export
// Entry: 0x000266E4
// Name: initialize_world_tile_buffers
// Decompile status: source-shaped annotation
//
// Native conversion: ghidra_conversion::initialize_world_tile_buffers.
// The selected resource record and the primary/secondary buffer dispatches
// are host callbacks. The old A5/A6 pointer frame is not part of the runtime.
//
// Calls recovered from direct call instructions:
//   0x00024E20 dispatch_indexed_handler
//   0x00026696 copy_tile_blocks

void initialize_world_tile_buffers(void)
{
  dispatch_indexed_handler();
  FFE192 = FFE19A = *selected_resource_record;
  FFE194 = FFE19C = selected_resource_record[1];
  if (scene_record_is_compact == 0) {
    FFE19E = 0x0E;
    FFE1A0 = 8;
  } else {
    FFE19E = 0x0C;
    FFE1A0 = 10;
  }
  FFE196 = 0;
  FFE198 = 0;
  FFE1A2 = 0;
  FFE1A4 = 0;
  FFE1A8 = 1;
  FFE1A6 = 1;
  copy_tile_blocks();
  dispatch_indexed_handler(primary_tile_buffer);
  dispatch_indexed_handler(secondary_tile_buffer);
  --FFB740;
  FFDD28 = 0x11E;
  FFDD2A = 0xE4;
}
