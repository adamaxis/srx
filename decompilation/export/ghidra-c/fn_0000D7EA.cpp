// Ghidra headless export
// Entry: 0x0000D7EA
// Name: compute_entity_delay_factor
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000D81C average_entity_delay_attributes
// Behavior annotation from the ROM disassembly:
//   A0+0x90 is the entity capability word. D2 scans bits 12, 13, and 14 in
//   that order; the first hit becomes (bit - 11) * 4, otherwise D2 is zero.
//   0xD81C returns the wrapped average of A0+0x78 and A0+0x7B in D1. The
//   routine adds D2 to that average, subtracts one, clamps negative values
//   to zero, and returns the factor in D1. Ghidra's output omits that return
//   arithmetic, so the disassembly is the source authority for this leaf.


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


