// Ghidra headless export
// Entry: 0x00013C20
// Name: select_entity_action_lane
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void select_entity_action_lane(void)

{
  undefined1 uVar1;
  
  uVar1 = 0;
  if (((3 < DAT_00ff014c) && ((uVar1 = 1, DAT_00fffc0c < '\0' || (3 < DAT_00ff024c)))) &&
     ((DAT_00fffc0d < '\0' || (uVar1 = 2, 3 < DAT_00ff034c)))) {
    return;
  }
  DAT_00ffe190 = uVar1;
  return;
}


