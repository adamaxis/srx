// Ghidra headless export
// Entry: 0x000019F2
// Name: read_controller_port
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00052DC2 begin_controller_bus_access
//   0x00052DD6 end_controller_bus_access


void read_controller_port(void)

{
  byte bVar1;
  byte bVar2;
  short sVar3;
  
  begin_controller_bus_access();
  IO_CT1_CTRL._1_1_ = 0x40;
  IO_CT1_DATA._1_1_ = 0;
  sVar3 = 10;
  do {
    sVar3 = sVar3 + -1;
  } while (sVar3 != -1);
  bVar1 = IO_CT1_DATA._1_1_;
  IO_CT1_DATA._1_1_ = 0x40;
  sVar3 = 10;
  do {
    sVar3 = sVar3 + -1;
  } while (sVar3 != -1);
  bVar2 = IO_CT1_DATA._1_1_;
  DAT_00ffef0e = ~(bVar2 & 0x3f | (bVar1 & 0x30) << 2);
  end_controller_bus_access();
  return;
}


