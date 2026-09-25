// Ghidra headless export
// Entry: 0x0001D048
// Name: convert_world_routine_0x0001d048
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void convert_world_routine_0x0001d048(void)

{
  short sVar1;
  char cVar2;
  byte bVar3;
  short sVar4;
  short *in_A0;
  
  bVar3 = *(byte *)((int)in_A0 + 0xb) - 1;
  *(byte *)((int)in_A0 + 0xb) = bVar3;
  if ((short)((ushort)bVar3 << 8) < 0) {
    *(undefined1 *)(in_A0 + 3) = 0;
  }
  bVar3 = *(byte *)(in_A0 + 2) - 1;
  *(byte *)(in_A0 + 2) = bVar3;
  if ((short)((ushort)bVar3 << 8) < 0) {
    *(undefined1 *)(in_A0 + 2) = 0x14;
    cVar2 = *(char *)((int)in_A0 + 5) + '\x01';
    if (cVar2 < '\x03') {
      *(char *)((int)in_A0 + 5) = cVar2;
    }
  }
  sVar1 = in_A0[8];
  in_A0[8] = in_A0[6] + sVar1;
  sVar4 = (short)((uint)((int)*(short *)((int)&DAT_00015e56 +
                                        (int)(short)((0x3c - *(byte *)((int)in_A0 + 0xb) & 0x3f) <<
                                                    2)) *
                         (int)(short)(ushort)*(byte *)((int)in_A0 + 0xb) * 0x40) >> 0x10);
  if (in_A0[9] == 0) {
    sVar4 = -sVar4;
  }
  *in_A0 = sVar4 + in_A0[6] + sVar1;
  in_A0[1] = in_A0[7] + in_A0[1];
  bVar3 = *(byte *)(in_A0 + 5) - 1;
  *(byte *)(in_A0 + 5) = bVar3;
  if ((short)((ushort)bVar3 << 8) < 0) {
    *(undefined1 *)(in_A0 + 5) = 2;
    *(char *)((int)in_A0 + 9) = *(char *)((int)in_A0 + 9) + '\x01';
    if ('\x02' < *(char *)((int)in_A0 + 9)) {
      *(undefined1 *)((int)in_A0 + 9) = 0;
    }
  }
  return;
}


