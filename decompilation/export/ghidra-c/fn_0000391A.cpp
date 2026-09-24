// Ghidra headless export
// Entry: 0x0000391A
// Name: FUN_0000391a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000033AA FUN_000033aa
//   0x0001DC4C FUN_0001dc4c
//   0x0001E540 FUN_0001e540
//   0x000530DE service_audio_if_enabled


void FUN_0000391a(void)

{
  byte *pbVar1;
  undefined2 *puVar2;
  undefined2 *puVar3;
  
  DAT_00fff0cc = '\x01';
  service_audio_if_enabled();
  puVar2 = DAT_00fffc04;
  FUN_000033aa();
  puVar3 = DAT_00fffc04;
  if (*(int *)(DAT_00fffc04 + 0x51) != 0) {
    pbVar1 = (byte *)(*(int *)(DAT_00fffc04 + 0x51) + 0x11);
    *pbVar1 = *pbVar1 & 0xef;
  }
  do {
    if (puVar3[0x1f] == 2) {
      DAT_00fff0cc = DAT_00fff0cc + -1;
      *(byte *)(puVar3 + 8) = *(byte *)(puVar3 + 8) & 0xfd;
      DAT_00fffc04 = &DAT_00ff0100;
      if (DAT_00fff0cc < '\0') break;
    }
    else {
      *(byte *)(puVar3 + 8) = *(byte *)(puVar3 + 8) & 0xfd;
      DAT_00fffc04 = DAT_00fffc04 + 0x80;
    }
    puVar3 = DAT_00fffc04;
    *(byte *)(DAT_00fffc04 + 8) = *(byte *)(DAT_00fffc04 + 8) | 2;
  } while (((*(byte *)(puVar3 + 8) & 1) == 0) ||
          ((DAT_00fff09d == '\0' &&
           (((*(byte *)(puVar3 + 8) & 0x40) != 0 || ((*(byte *)(puVar3 + 8) & 0x10) != 0))))));
  DAT_00fff0cc = 0;
  if (puVar3 != puVar2) {
    FUN_0001e540();
  }
  FUN_0001dc4c();
  FUN_000033aa();
  FUN_00003548();
  FUN_000038fa();
  return;
}


