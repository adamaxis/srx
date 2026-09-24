// Ghidra headless export
// Entry: 0x00000C2C
// Name: clear_runtime_memory_and_vdp
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000CEC FUN_00000cec
//   0x00000DFC write_repeated_words
//   0x00000E34 FUN_00000e34
//   0x00000E9C make_vdp_transfer_command
//   0x00000EC0 upload_startup_palette
//   0x000531AA FUN_000531aa


void clear_runtime_memory_and_vdp(void)

{
  int in_D1;
  short sVar1;
  int *piVar2;
  undefined1 *puVar3;
  
  if (DAT_00fff9e8 == 0x12345678) {
    sVar1 = -0xe19;
    puVar3 = &DAT_00ff0000;
    do {
      *puVar3 = 0;
      sVar1 = sVar1 + -1;
      puVar3 = puVar3 + 1;
    } while (sVar1 != -1);
  }
  else {
    sVar1 = 0x3fdf;
    piVar2 = (int *)&DAT_00ff0000;
    do {
      in_D1 = *piVar2 + in_D1;
      *piVar2 = 0;
      sVar1 = sVar1 + -1;
      piVar2 = piVar2 + 1;
    } while (sVar1 != -1);
    DAT_00fff9f8 = (undefined2)in_D1;
  }
  upload_startup_palette();
  write_repeated_words();
  DAT_00ffdd02 = 1;
  FUN_00000e34();
  FUN_00000cec();
  make_vdp_transfer_command();
  sVar1 = 7;
  do {
    Ram00c00000 = 0;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  Ram00c00004 = 0x40000010;
  Ram00c00000 = 0;
  FUN_000531aa();
  FUN_000531aa();
  FUN_000531aa();
  FUN_000531aa();
  FUN_000531aa();
  return;
}


