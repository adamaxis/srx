// Ghidra headless export
// Entry: 0x00013B12
// Name: FUN_00013b12
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D3A FUN_00000d3a
//   0x00003404 FUN_00003404
//   0x0000ABFC update_entity_animation_delay
//   0x000125FA FUN_000125fa
//   0x00013C20 FUN_00013c20
//   0x00013C74 FUN_00013c74
//   0x00013CDE FUN_00013cde


void FUN_00013b12(void)

{
  undefined1 uVar1;
  short sVar2;
  undefined1 extraout_D1b;
  short unaff_D3w;
  int unaff_D4;
  undefined4 uVar3;
  int unaff_D7;
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A0_01;
  int iVar4;
  bool bVar5;
  short sStack_6;
  undefined4 uStack_4;
  
  uStack_4 = 0x13b26;
  FUN_00013c74();
  if (unaff_D3w != 0) {
    uVar3 = CONCAT31((int3)((uint)unaff_D4 >> 8),DAT_00ffdd66._0_1_);
    bVar5 = false;
    uStack_4 = CONCAT22((short)CONCAT31((int3)((uint)unaff_D7 >> 8),0xb),uStack_4._2_2_);
    FUN_00013cde();
    unaff_D4 = (int)(short)uVar3;
    unaff_D7 = (int)sStack_6;
    register0x0000003c = (BADSPACEBASE *)&uStack_4;
    if (!bVar5) {
      FUN_00013c20();
      iVar4 = extraout_A0;
      if (bVar5) goto LAB_00013b7a;
    }
  }
  if (DAT_00ffdd28 != 0) {
    *(undefined4 *)((int)register0x0000003c + -4) = 0x13b60;
    sVar2 = FUN_00000d3a();
    uVar1 = *(undefined1 *)((int)&DAT_00ffdd2a + (short)(sVar2 * 2) + 1);
    bVar5 = false;
    *(short *)((int)register0x0000003c + -4) =
         (short)CONCAT31((int3)((uint)unaff_D7 >> 8),
                         *(undefined1 *)((int)&DAT_00ffdd2a + (int)(short)(sVar2 * 2)));
    *(short *)((int)register0x0000003c + -8) = (short)CONCAT31((int3)((uint)unaff_D4 >> 8),uVar1);
    *(undefined4 *)((int)register0x0000003c + -0xc) = 0x13b72;
    FUN_00013cde();
    unaff_D4 = (int)*(short *)((int)register0x0000003c + -8);
    unaff_D7 = (int)*(short *)((int)register0x0000003c + -6);
    register0x0000003c = (BADSPACEBASE *)((int)register0x0000003c + -4);
    iVar4 = extraout_A0_00;
    if (!bVar5) {
LAB_00013b7a:
      if ((*(char *)(iVar4 + 0x57) != '\0') && (*(char *)(iVar4 + 0x57) < '\r')) {
        *(undefined4 *)((int)register0x0000003c + -4) = 0x13b90;
        FUN_000125fa();
        *(undefined1 *)(extraout_A0_01 + 0x4e) = extraout_D1b;
        iVar4 = extraout_A0_01;
      }
      *(undefined1 *)(iVar4 + 0x56) = 0xff;
      *(char *)(iVar4 + 0x57) = (char)unaff_D7 + '\x01';
      *(char *)(iVar4 + 0x9d) = (char)unaff_D4;
      *(undefined4 *)((int)register0x0000003c + -4) = 0x13bac;
      update_entity_animation_delay();
      *(undefined4 *)((int)register0x0000003c + -4) = 0x13bb2;
      FUN_00003404();
      return;
    }
  }
  return;
}


