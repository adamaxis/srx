// Ghidra headless export
// Entry: 0x0000ABB6
// Name: arm_controller_activity_delay
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void arm_controller_activity_delay(void)

{
  if ((((DAT_00ff0110 & 0x40) != 0) && (((DAT_00ff0210 & 1) == 0 || ((DAT_00ff0210 & 0x40) != 0))))
     && (((DAT_00ff0310 & 1) == 0 || ((DAT_00ff0310 & 0x40) != 0)))) {
    DAT_00ffe166 = 0x3c;
  }
  return;
}


