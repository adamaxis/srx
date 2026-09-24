// Ghidra headless export
// Entry: 0x000594E0
// Name: FUN_000594e0
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003074 FUN_00003074
//   0x000042F2 FUN_000042f2
//   0x0000431C FUN_0000431c
//   0x0001E4AC spawn_world_effect_at_player_position
//   0x0005966E FUN_0005966e


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000594e0(void)

{
  undefined2 *puVar1;
  short sVar2;
  undefined2 *puVar3;
  undefined2 *puVar4;
  
  DAT_00ffdf40 = 0;
  sVar2 = 0x247;
  puVar3 = &DAT_00fff9fa;
  puVar1 = (undefined2 *)&DAT_00fff5be;
  do {
    puVar4 = puVar1;
    *(undefined1 *)puVar3 = *(undefined1 *)puVar4;
    sVar2 = sVar2 + -1;
    puVar3 = (undefined2 *)((int)puVar3 + 1);
    puVar1 = (undefined2 *)((int)puVar4 + 1);
  } while (sVar2 != -1);
  DAT_00ffddc4 = *(undefined2 *)((int)puVar4 + 1);
  FUN_00003074();
  DAT_00ff0110 = 0xf;
  if (-1 < DAT_00fffc0c) {
    FUN_000042f2();
  }
  if (-1 < DAT_00fffc0d) {
    FUN_0000431c();
  }
  DAT_00ffddc6 = *(undefined2 *)((int)puVar4 + 3);
  DAT_00ffddc8 = *(undefined2 *)((int)puVar4 + 5);
  DAT_00ff0100 = *(undefined2 *)((int)puVar4 + 7);
  DAT_00ff0104 = *(undefined2 *)((int)puVar4 + 9);
  DAT_00ff0200 = *(undefined2 *)((int)puVar4 + 0xb);
  DAT_00ff0204 = *(undefined2 *)((int)puVar4 + 0xd);
  DAT_00ff0300 = *(undefined2 *)((int)puVar4 + 0xf);
  DAT_00ff0304 = *(undefined2 *)((int)puVar4 + 0x11);
  FUN_0005966e();
  if (DAT_00fffc0c < '\0') {
    DAT_00ff0210 = 0;
  }
  else {
    DAT_00ff0210 = 0xd;
  }
  if (DAT_00fffc0d < '\0') {
    DAT_00ff0310 = 0;
  }
  else {
    DAT_00ff0310 = 0xd;
  }
  DAT_00ff0110 = DAT_00ff0110 | 2;
  DAT_00fffc04 = &DAT_00ff0100;
  _DAT_00ff0150 = &DAT_000720fa;
  if ((DAT_00ff019b != '\x02') &&
     (_DAT_00ff0150 = (undefined2 *)&UNK_0007223a, DAT_00ff019b != '\x03')) {
    _DAT_00ff0150 = (undefined2 *)&DAT_0007219a;
  }
  if (DAT_00fffc3c != 0) {
    spawn_world_effect_at_player_position();
  }
  return;
}


