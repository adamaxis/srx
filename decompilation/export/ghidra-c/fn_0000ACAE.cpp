// Ghidra headless export
// Entry: 0x0000ACAE
// Name: FUN_0000acae
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000AAD2 FUN_0000aad2
//   0x0000ABFC update_entity_animation_delay
//   0x0000AE6E FUN_0000ae6e
//   0x0000AF7C prepare_entity_delayed_animation_primary
//   0x0000AFAE prepare_entity_delayed_animation_alternate
//   0x0000B1DC FUN_0000b1dc
//   0x0000B39A set_entity_heading_with_field_animation
//   0x0000B3FE set_entity_heading_with_alternate_animation
//   0x0001514E FUN_0001514e
//   0x00015646 FUN_00015646
//   0x0001DC4C FUN_0001dc4c
//   0x000530DE FUN_000530de


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000acae(void)

{
  short sVar1;
  int in_A0;
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A0_01;
  int extraout_A0_02;
  bool bVar2;
  
  if (DAT_00ffe18a != '\0') goto LAB_0000ae40;
  if (_DAT_00ffdf3a != 0) {
    sVar1 = *(short *)(in_A0 + 0xcc) + -0x8000;
    *(short *)(in_A0 + 0x2e) = sVar1;
    *(short *)(in_A0 + 0x34) = sVar1;
    *(undefined4 *)(in_A0 + 0x3a) = 0xb45c;
    *(undefined1 **)(in_A0 + 0x3a) = &LAB_0000b574;
    return;
  }
  if (DAT_00ffe18b != '\0') {
    if (DAT_00ffe18c != '\0') {
      DAT_00ffe18c = DAT_00ffe18c + -1;
      goto LAB_0000ae40;
    }
    sVar1 = 0;
    goto LAB_0000ae50;
  }
  if ((DAT_00ffdd22 & 0x1000) != 0) {
    FUN_0001dc4c();
    in_A0 = extraout_A0;
  }
  if (*(short *)(in_A0 + 0xdc) < 0) {
    if (_DAT_00ffdfe6 == 0) {
      if ((DAT_00ffdd22 & 0x40) == 0) {
        if (DAT_00ffdd04 == -1) {
          update_entity_animation_delay();
          *(char *)(extraout_A0_00 + 0x43) = *(char *)(extraout_A0_00 + 0x43) + '\x01';
          in_A0 = extraout_A0_00;
        }
        else {
          *(undefined1 *)(in_A0 + 0x43) = 0;
        }
        goto LAB_0000ae26;
      }
    }
    else {
      _DAT_00ffdfe6 = 0;
    }
    *(undefined1 *)(in_A0 + 0x43) = 0;
    if (*(char *)(in_A0 + 0x56) < '\0') {
      FUN_0000ae6e();
      if (DAT_00fff0dd != '\0') {
        prepare_entity_delayed_animation_alternate();
        return;
      }
      prepare_entity_delayed_animation_primary();
      return;
    }
    if (('\f' < *(char *)(in_A0 + 0x57)) &&
       (bVar2 = *(byte *)(in_A0 + 0x57) < 0x10, (char)(*(byte *)(in_A0 + 0x57) - 0x10) < '\0')) {
      FUN_00015646();
      if (bVar2) {
        return;
      }
      set_entity_heading_with_alternate_animation();
      return;
    }
    if (*(char *)(in_A0 + 0x57) == '\0') {
      FUN_0000b1dc();
      return;
    }
    bVar2 = *(byte *)(in_A0 + 0x57) < 0xe;
    if ('\x0e' < (char)*(byte *)(in_A0 + 0x57)) {
      return;
    }
    FUN_0001514e();
    if (!bVar2) {
      set_entity_heading_with_field_animation();
      return;
    }
    FUN_0000aad2();
    in_A0 = extraout_A0_01;
    if (!bVar2) goto LAB_0000ae40;
    FUN_000530de();
    update_entity_animation_delay();
    in_A0 = extraout_A0_02;
  }
  else if ((DAT_00ffdd22 & 0x4000) != 0) {
    _DAT_00ffdfe6 = -1;
  }
LAB_0000ae26:
  sVar1 = DAT_00ffdd04;
  if (DAT_00ffdd04 != -1) {
LAB_0000ae50:
    *(short *)(in_A0 + 0x34) = sVar1;
    *(undefined1 **)(in_A0 + 0x3a) = &LAB_0000b574;
    return;
  }
  if (*(int *)(in_A0 + 0x3a) == 0xb45c) {
    return;
  }
LAB_0000ae40:
  *(byte *)(in_A0 + 0x11) = *(byte *)(in_A0 + 0x11) | 1;
  *(undefined4 *)(in_A0 + 0x3a) = 0xb45c;
  return;
}


