// Ghidra headless export
// Entry: 0x000038AC
// Name: service_entity_ammo
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000036FC advance_entity_asset_meter


void service_entity_ammo(void)

{
  int in_A0;
  
  if ((*(char *)(in_A0 + 0x4e) != '\0') && (*(char *)(in_A0 + 0x57) != '\0')) {
    if (*(short *)(in_A0 + 0x3e) < 3) {
      advance_entity_asset_meter();
      return;
    }
    *(char *)(in_A0 + 0x4e) = *(char *)(in_A0 + 0x4e) + -1;
  }
  return;
}


