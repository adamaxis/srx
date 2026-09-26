// Ghidra headless export
// Entry: 0x0001DF1A
// Name: resolve_nearby_effects
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001DF34 convert_world_routine_0x0001df34


void resolve_nearby_effects(void)

{
  ushort uVar1;
  ushort extraout_D1w;
  ushort uVar2;
  short sVar3;
  short *psVar4;
  short *extraout_A0;
  int extraout_A0_00;
  bool bVar5;
  
  convert_world_routine_0x0001df34();
  convert_world_routine_0x0001df34();
  psVar4 = &DAT_00ff164c;
  sVar3 = 9;
  uVar1 = DAT_00ff0300;
  uVar2 = DAT_00ff0304;
  do {
    if (((((*(byte *)(psVar4 + 8) & 1) != 0) && ((ushort)(*psVar4 - 0x100U) < uVar1)) &&
        (uVar1 <= (ushort)(*psVar4 + 0x100U))) &&
       (((ushort)(psVar4[2] - 0x100U) < uVar2 && (uVar2 <= (ushort)(psVar4[2] + 0x100U))))) {
      bVar5 = psVar4[1] < 0;
      uVar1 = apply_world_effect_reward();
      uVar2 = extraout_D1w;
      psVar4 = extraout_A0;
      if (!bVar5) {
        service_audio_if_enabled();
        *(byte *)(extraout_A0_00 + 0x10) = *(byte *)(extraout_A0_00 + 0x10) & 0xfe;
        if (*(short *)(extraout_A0_00 + 2) != 0x65) {
          return;
        }
        clear_effect_slot_state(extraout_A0_00);
        return;
      }
    }
    psVar4 = psVar4 + 9;
    sVar3 = sVar3 + -1;
  } while (sVar3 != -1);
  return;
}

