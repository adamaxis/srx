// Ghidra headless export
// Entry: 0x0000B442
// Name: FUN_0000b442
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000B48C FUN_0000b48c


void FUN_0000b442(void)

{
  ushort uVar1;
  int in_A0;
  
  *(byte *)(in_A0 + 0xb0) = *(byte *)(in_A0 + 0xb0) & 0xfd;
  *(undefined2 *)(in_A0 + 0x2e) = *(undefined2 *)(in_A0 + 0x34);
  *(undefined4 *)(in_A0 + 0x3a) = 0xb45c;
  *(byte *)(in_A0 + 0x11) = *(byte *)(in_A0 + 0x11) | 1;
  *(undefined2 *)(in_A0 + 0x38) = 0;
  uVar1 = *(ushort *)(in_A0 + 0x2e) << 3 | *(ushort *)(in_A0 + 0x2e) >> 0xd;
  if ((short)uVar1 < 0) {
    uVar1 = uVar1 + 1;
  }
  *(undefined2 *)(in_A0 + 0x16) =
       *(undefined2 *)((int)L"@DHLPTX\\" + (short)((uVar1 & 7) << 1) + 0x10);
  FUN_0000b48c();
  return;
}


