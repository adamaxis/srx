// Ghidra headless export
// Entry: 0x0001D0DE
// Name: resolve_world_effect_sprite
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)
//
// ROM trace: animation phase indexes one of three direct three-entry tables:
// variant 0 -> 0x1D134, variant 1 -> 0x1D11C, and every other variant ->
// 0x1D128. The selected stream is returned with record X/Y shifted right by
// four for the sprite builder.


void resolve_world_effect_sprite(void)

{
  return;
}

