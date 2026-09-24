// Ghidra headless export
// Entry: 0x0001DF34
// Name: FUN_0001df34
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000133A6 FUN_000133a6
//   0x0001E514 clear_effect_slot_state
//   0x000530DE service_audio_if_enabled


void FUN_0001df34(void)

{
  ushort uVar1;
  ushort extraout_D1w;
  ushort uVar2;
  short sVar3;
  ushort *in_A0;
  short *psVar4;
  short *extraout_A0;
  int extraout_A0_00;
  bool bVar5;
  
  uVar1 = *in_A0;
  uVar2 = in_A0[2];
  psVar4 = &DAT_00ff164c;
  sVar3 = 9;
  do {
    if (((((*(byte *)(psVar4 + 8) & 1) != 0) && ((ushort)(*psVar4 - 0x100U) < uVar1)) &&
        (uVar1 <= (ushort)(*psVar4 + 0x100U))) &&
       (((ushort)(psVar4[2] - 0x100U) < uVar2 && (uVar2 <= (ushort)(psVar4[2] + 0x100U))))) {
      bVar5 = psVar4[1] < 0;
      uVar1 = FUN_000133a6();
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


