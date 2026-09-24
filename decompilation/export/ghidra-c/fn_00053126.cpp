// Ghidra headless export
// Entry: 0x00053126
// Name: prepare_mode_entry_service
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000C24 wait_for_vblank_frames
//   0x00052FA0 flush_with_audio_bus
//   0x00053156 service_audio_tick_wrapper
//   0x0005315E FUN_0005315e


undefined8 prepare_mode_entry_service(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  short sVar1;
  
  sVar1 = 5;
  do {
    FUN_0005315e();
    wait_for_vblank_frames();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  flush_with_audio_bus();
  service_audio_tick_wrapper();
  return CONCAT44(in_D0,in_D1);
}


