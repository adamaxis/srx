// Ghidra headless export
// Entry: 0x000037EC
// Name: advance_entity_behavior_meter
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x00003674 advance_entity_delay_meter


undefined8 advance_entity_behavior_meter(void)

{
  char cVar1;
  char cVar2;
  undefined4 in_D0;
  undefined4 in_D1;
  undefined4 extraout_D1;
  int in_A0;
  int extraout_A0;
  
  if ((*(byte *)(in_A0 + 0x10) & 1) != 0) {
    cVar1 = *(char *)(in_A0 + 0x9c);
    cVar2 = (char)in_D1 + cVar1;
    *(char *)(in_A0 + 0x9c) = cVar2;
    if (cVar2 != '\0' && SCARRY1((char)in_D1,cVar1) == cVar2 < '\0') {
      if ('\n' < *(char *)(in_A0 + 0x9c)) {
        *(undefined1 *)(in_A0 + 0x9c) = 10;
      }
      goto LAB_00003890;
    }
    advance_entity_delay_meter();
    in_D1 = extraout_D1;
    in_A0 = extraout_A0;
  }
  *(undefined1 *)(in_A0 + 0x9c) = 0;
LAB_00003890:
  write_vdp_grid();
  return CONCAT44(in_D0,in_D1);
}


