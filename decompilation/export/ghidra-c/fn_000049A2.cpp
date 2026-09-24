// Ghidra headless export
// Entry: 0x000049A2
// Name: FUN_000049a2
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00004B6C FUN_00004b6c
//   0x000061A0 append_entity_sprite_record
//   0x0001582A thunk_FUN_0001a2cc
//   0x000530DE FUN_000530de


void FUN_000049a2(void)

{
  ushort uVar1;
  short sVar2;
  ushort uVar3;
  short sVar4;
  ushort extraout_D1w;
  short *extraout_A0;
  ushort *extraout_A1;
  short *psVar5;
  ushort *puVar6;
  undefined1 uVar7;
  
  uVar7 = 0;
  if (DAT_00ffdca0 != 0) {
    FUN_00004b6c();
  }
  uVar3 = thunk_FUN_0001a2cc();
  if (!(bool)uVar7) {
    extraout_A1[0x19] = uVar3;
    extraout_A1[0x15] = extraout_D1w;
    extraout_A1[1] = 1;
    extraout_A1[7] = (ushort)DAT_00fff0dd * 400;
    uVar3 = extraout_A0[0x17];
    extraout_A1[3] = uVar3 >> 0xd;
    sVar4 = (uVar3 >> 0xd) << 2;
    uVar3 = (extraout_A0[2] + *(short *)(&DAT_00004aae + sVar4) & 0xfe00U) + 0x100;
    extraout_A1[2] = uVar3;
    uVar1 = (*extraout_A0 + *(short *)(&DAT_00004aac + sVar4) & 0xfe00U) + 0x100;
    *extraout_A1 = uVar1;
    uVar3 = (uVar3 >> 4 & 0x7fe0) * 2 + (uVar1 >> 9);
    psVar5 = *(short **)((int)&PTR_DAT_00004bb0 + (int)(short)(extraout_A1[3] << 2));
    sVar4 = 4;
    extraout_A1[0x1a] = uVar3;
    puVar6 = extraout_A1 + 0x1b;
    do {
      sVar2 = *psVar5 + uVar3;
      *(undefined1 *)puVar6 = (&DAT_00ff1700)[sVar2];
      (&DAT_00ff1700)[sVar2] = 0x7e;
      if (*(char *)((int)extraout_A0 + 0x57) < '\n') {
        (&DAT_00ff1700)[sVar2] = 0x7f;
      }
      sVar4 = sVar4 + -1;
      psVar5 = psVar5 + 1;
      puVar6 = (ushort *)((int)puVar6 + 1);
    } while (sVar4 != -1);
    append_entity_sprite_record(extraout_A0,extraout_A1);
    FUN_000530de();
  }
  return;
}


