// Ghidra headless export
// Entry: 0x00058D32
// Name: refresh_field_mode_selection_sprites
// Decompile status: source-shaped annotation
//
// Native conversion: ghidra_conversion::refresh_field_mode_selection_sprites.
// The source publishes the sprite-list and display dirty markers, builds the
// title/sprite records, and then enters the remaining sprite-selection
// service through an explicit host callback.
//
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records
//   0x00057C7A render_field_action_selection
