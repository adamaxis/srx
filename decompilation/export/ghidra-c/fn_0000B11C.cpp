// Ghidra headless export
// Entry: 0x0000B11C
// Name: FUN_0000b11c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000ABFC update_entity_animation_delay
//   0x0000AF6C reset_entity_animation_state
//   0x0000AFF2 FUN_0000aff2
//   0x00015A8C FUN_00015a8c
//   0x0001DBAA FUN_0001dbaa
//   0x00055B16 FUN_00055b16


void FUN_0000b11c(void)

{
  undefined2 uVar1;
  ushort uVar2;
  undefined4 extraout_A0;
  undefined2 *extraout_A0_00;
  int extraout_A0_01;
  int in_A1;
  int extraout_A1;
  undefined2 *extraout_A1_00;
  undefined1 uVar3;
  byte bVar4;
  byte in_stack_fffffffb;
  
  bVar4 = *(byte *)(in_A1 + 0x9c);
  FUN_00055b16();
  uVar3 = in_stack_fffffffb < *(byte *)(extraout_A1 + 0x4c);
  if ((in_stack_fffffffb != *(byte *)(extraout_A1 + 0x4c)) ||
     (uVar3 = bVar4 < *(byte *)(extraout_A1 + 0x9c), bVar4 != *(byte *)(extraout_A1 + 0x9c))) {
    FUN_0000aff2(extraout_A0,extraout_A1);
    uVar1 = FUN_00015a8c();
    if (!(bool)uVar3) {
      extraout_A1_00[0x1f] = uVar1;
      *(byte *)((int)extraout_A1_00 + 0x11) = *(byte *)((int)extraout_A1_00 + 0x11) | 1;
      extraout_A1_00[0xb] = 4;
      extraout_A1_00[7] = 1;
      *(undefined1 **)(extraout_A1_00 + 0x1d) = &LAB_0000ac4e;
      *(byte *)(extraout_A1_00 + 8) = *(byte *)(extraout_A1_00 + 8) | 1;
      *extraout_A1_00 = *extraout_A0_00;
      extraout_A1_00[2] = extraout_A0_00[2] + 0x10;
      extraout_A1_00[0x16] = 0x2000;
      *(byte *)((int)extraout_A1_00 + 0x11) = *(byte *)((int)extraout_A1_00 + 0x11) | 1;
    }
  }
  update_entity_animation_delay();
  FUN_0001dbaa();
  *(undefined1 *)(extraout_A0_01 + 2) = 6;
  *(undefined1 *)(extraout_A0_01 + 3) = 0;
  *(undefined1 **)(extraout_A0_01 + 0x3a) = &LAB_0000b3ee;
  uVar2 = *(ushort *)(extraout_A0_01 + 0x2e) << 3 | *(ushort *)(extraout_A0_01 + 0x2e) >> 0xd;
  if ((short)uVar2 < 0) {
    uVar2 = uVar2 + 1;
  }
  *(undefined2 *)(extraout_A0_01 + 0x16) =
       *(undefined2 *)((int)L"@DHLPTX\\" + (int)(short)((uVar2 & 7) << 1));
  reset_entity_animation_state();
  return;
}


