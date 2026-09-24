// Ghidra headless export
// Entry: 0x0000D7EA
// Name: compute_entity_delay_factor
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000D81C average_entity_delay_attributes


void compute_entity_delay_factor(void)

{
  uint uVar1;
  undefined4 unaff_D5;
  short sVar2;
  int in_A0;
  
  sVar2 = 2;
  uVar1 = 0xc;
  do {
    if ((CONCAT22((short)((uint)unaff_D5 >> 0x10),*(undefined2 *)(in_A0 + 0x90)) &
        1 << (uVar1 & 0x1f)) != 0) break;
    uVar1 = (uint)(ushort)((short)uVar1 + 1);
    sVar2 = sVar2 + -1;
  } while (sVar2 != -1);
  average_entity_delay_attributes();
  return;
}


