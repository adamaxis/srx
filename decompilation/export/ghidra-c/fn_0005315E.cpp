// Ghidra headless export
// Entry: 0x0005315E
// Name: run_fixed_audio_transaction_wrapper
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00052F6E run_audio_transaction_wrapper


undefined8 run_fixed_audio_transaction_wrapper(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  
  run_audio_transaction_wrapper();
  return CONCAT44(in_D0,in_D1);
}


