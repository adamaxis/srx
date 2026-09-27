// Ghidra headless export
// Entry: 0x00055F44
// Name: calculate_runner_action_delay
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000D81C average_entity_delay_attributes
// Direct disassembly is authoritative for the byte modifiers, FFF0B7 mode
// branches, including the negative-mode request switch from two-thirds to
// one-third, and the request-dependent profile-floor clamp omitted by this
// decompiler output; the typed host conversion is in runner_action_delay.


undefined8 calculate_runner_action_delay(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  
  average_entity_delay_attributes();
  return CONCAT44(in_D0,in_D1);
}

