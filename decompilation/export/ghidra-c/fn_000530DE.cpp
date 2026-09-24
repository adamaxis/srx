// Ghidra headless export
// Entry: 0x000530DE
// Name: service_audio_if_enabled
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 service_audio_if_enabled(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  
  if (_DAT_00fffc0a == 0) {
    return CONCAT44(in_D0,in_D1);
  }
  service_audio_command_queue();
  return CONCAT44(in_D0,in_D1);
}


