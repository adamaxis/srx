// Ghidra headless export
// Entry: 0x000530A0
// Name: controller_access_pair
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00052DC2 begin_controller_bus_access
//   0x00052DD6 end_controller_bus_access


void controller_access_pair(void)

{
  begin_controller_bus_access();
  end_controller_bus_access();
  return;
}


