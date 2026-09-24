// Ghidra headless export
// Entry: 0x0000BB86
// Name: evaluate_entity_movement_route
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000B732 apply_movement_result_policy


undefined4 evaluate_entity_movement_route(void)

{
  char cVar1;
  undefined4 in_D0;
  uint uVar2;
  short sVar4;
  undefined4 uVar3;
  int in_A0;
  
  uVar2 = CONCAT22((short)((uint)in_D0 >> 0x10),*(ushort *)(in_A0 + 0x2e) >> 0xd) & 0xffff0006;
  sVar4 = (short)uVar2 << 1;
  cVar1 = *(char *)**(undefined4 **)(&DAT_0000bbf6 + sVar4);
  if ((cVar1 != '\0') && (-1 < cVar1)) {
    cVar1 = *(char *)(*(undefined4 **)(&DAT_0000bbf6 + sVar4))[1];
    if ((cVar1 != '\0') && (-1 < cVar1)) {
      return CONCAT31((int3)(CONCAT22((short)(uVar2 >> 0x10),sVar4) >> 8),cVar1);
    }
  }
  uVar3 = apply_movement_result_policy();
  if (((short)uVar3 != 0) && (-1 < (short)uVar3)) {
    return uVar3;
  }
  return uVar3;
}


