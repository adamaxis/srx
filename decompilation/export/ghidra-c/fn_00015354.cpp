// Ghidra headless export
// Entry: 0x00015354
// Name: FUN_00015354
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000530DE service_audio_if_enabled


void FUN_00015354(void)

{
  undefined2 *in_A0;
  
  *(byte *)(in_A0 + 8) = *(byte *)(in_A0 + 8) | 0x80;
  in_A0[0xb] = 0xc;
  if (*(char *)((int)in_A0 + 0x57) == '\x0e') {
    in_A0[0xb] = 0x10;
  }
  *(undefined1 *)((int)in_A0 + 0x41) = 0;
  in_A0[1] = *in_A0;
  in_A0[3] = in_A0[2];
  in_A0[0x22] = 0;
  service_audio_if_enabled();
  return;
}


