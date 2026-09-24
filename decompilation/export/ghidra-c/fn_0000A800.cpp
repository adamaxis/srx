// Ghidra headless export
// Entry: 0x0000A800
// Name: prepare_pending_entity_template_list
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D3A FUN_00000d3a
//   0x0005BA98 FUN_0005ba98


void prepare_pending_entity_template_list(void)

{
  char cVar1;
  undefined4 *unaff_A3;
  
  if (DAT_00ffe180 == '\0') {
    if (DAT_00ffe182 == '\0') {
      if (((char)DAT_00fffbec == '\x05') && (DAT_00ffdf40 == 2)) {
        if (DAT_00ffe183 == '\0') {
          cVar1 = FUN_00000d3a();
          DAT_00ffe183 = cVar1 + '\x01';
        }
        if ((DAT_00ffe12c == DAT_00fff0c8) && (DAT_00fffc3c == 0)) {
          FUN_0005ba98();
          DAT_00ffe110 = *unaff_A3;
          DAT_00ffe124 = unaff_A3;
        }
      }
    }
    else {
      DAT_00ffe124 = &DAT_0000a8cc;
      DAT_00ffe110 = DAT_0000a8cc;
    }
  }
  else if (DAT_00fff0c8 == 4) {
    DAT_00ffe124 = &DAT_0000a8ae;
    DAT_00ffe110 = DAT_0000a8ae;
  }
  return;
}


