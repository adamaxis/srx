// Ghidra headless export
// Entry: 0x00003674
// Name: FUN_00003674
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 upload_hud_tile_block


undefined8 FUN_00003674(void)

{
  char cVar1;
  char cVar2;
  undefined4 in_D0;
  undefined4 in_D1;
  int in_A0;
  
  if ((*(byte *)(in_A0 + 0x10) & 1) != 0) {
    cVar1 = *(char *)(in_A0 + 0x4c);
    cVar2 = (char)in_D1 + cVar1;
    *(char *)(in_A0 + 0x4c) = cVar2;
    if (cVar2 != '\0' && SCARRY1((char)in_D1,cVar1) == cVar2 < '\0') {
      if ('\n' < *(char *)(in_A0 + 0x4c)) {
        *(undefined1 *)(in_A0 + 0x4c) = 10;
      }
      goto LAB_000036e4;
    }
  }
  *(undefined1 *)(in_A0 + 0x4c) = 0;
LAB_000036e4:
  upload_hud_tile_block();
  return CONCAT44(in_D0,in_D1);
}


