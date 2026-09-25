// Ghidra headless export
// Entry: 0x00001E4E
// Name: reveal_title_logo_column
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000530DE service_audio_if_enabled
//   0x0005C8C6 convert_intro_routine_0x0005c8c6


ushort reveal_title_logo_column(void)

{
  ushort uVar1;
  short sVar2;
  undefined2 *puVar3;
  undefined2 *puVar4;
  
  uVar1 = DAT_00fff1e6 & 1;
  if (uVar1 == 0) {
    uVar1 = service_audio_if_enabled();
  }
  if (DAT_00fff1e6 < 0x26) {
    uVar1 = convert_intro_routine_0x0005c8c6();
    DAT_00fff1e6 = DAT_00fff1e6 + 1;
    DAT_00fff1e2 = DAT_00fff1e2 + 2;
  }
  if (DAT_00fff1e6 == 0x25) {
    DAT_00ffdd5a = 1;
    sVar2 = 0xf;
    puVar3 = &DAT_00002070;
    puVar4 = &DAT_00ffd93e;
    do {
      *puVar4 = *puVar3;
      sVar2 = sVar2 + -1;
      puVar3 = puVar3 + 1;
      puVar4 = puVar4 + 1;
    } while (sVar2 != -1);
  }
  return uVar1;
}


