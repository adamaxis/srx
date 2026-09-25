// Ghidra headless export
// Entry: 0x0005D3E6
// Name: convert_intro_routine_0x0005d3e6
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000AFF2 prepare_entity_auxiliary_behavior
//   0x00015930 convert_entity_routine_0x00015930


undefined8 convert_intro_routine_0x0005d3e6(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  int in_A1;
  
  *(byte *)(in_A1 + 0x11) = *(byte *)(in_A1 + 0x11) | 1;
  convert_entity_routine_0x00015930();
  prepare_entity_auxiliary_behavior();
  return CONCAT44(in_D0,in_D1);
}


