// Ghidra headless export
// Entry: 0x00059798
// Name: serialize_runner_record_population
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000597EE project_live_runner_record_to_compact


uint serialize_runner_record_population(void)

{
  uint uVar1;
  
  project_live_runner_record_to_compact();
  if (-1 < DAT_00fffc0c) {
    project_live_runner_record_to_compact();
  }
  uVar1 = (uint)DAT_00fffc0d;
  if (-1 < (char)DAT_00fffc0d) {
    uVar1 = project_live_runner_record_to_compact();
  }
  return uVar1;
}


