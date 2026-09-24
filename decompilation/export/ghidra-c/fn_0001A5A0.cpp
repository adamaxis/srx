// Ghidra headless export
// Entry: 0x0001A5A0
// Name: FUN_0001a5a0
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000013BE build_title_sprite_records_with_attribute_offset
//   0x000061CC append_sprite_record
//   0x0001B654 attach_behavior_descriptor_d
//   0x0001B680 attach_behavior_descriptor_e
//   0x0001B7D0 FUN_0001b7d0
//   0x000530EA service_audio_command_frame
//   0x000562EC FUN_000562ec
//   0x0005668C FUN_0005668c


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001a5a0(void)

{
  short extraout_D1w;
  int extraout_A0;
  
  if (DAT_00ffe076 == 2) {
LAB_0001a64c:
    DAT_00ffe07c = DAT_00ffe07c + -1;
    if (-1 < DAT_00ffe07c) {
      append_sprite_record();
      DAT_00ffe078 = DAT_00ffe078 + -6;
      DAT_00ffe07a = DAT_00ffe07a + 0x1a;
LAB_0001a62a:
      build_title_sprite_records_with_attribute_offset();
      return;
    }
    FUN_000562ec();
    if (_DAT_00ffe07e != 0) {
      DAT_00ffe07c = 0x10;
      DAT_00ffe076 = 4;
LAB_0001a6e6:
      DAT_00ffe07c = DAT_00ffe07c + -1;
      if (DAT_00ffe07c < 0) {
        attach_behavior_descriptor_e();
        DAT_00ffe076 = 0;
        return;
      }
      if (_DAT_00ffe0d6 == 0) {
        attach_behavior_descriptor_d();
        return;
      }
      return;
    }
    service_audio_command_frame();
    DAT_00ffe07c = 8;
    DAT_00ffe076 = 3;
    _DAT_00ffe0fc = extraout_D1w;
    if (DAT_00fff0d2 != '\0') {
      DAT_00ffe0f0 = 8;
      _DAT_00ffe0ee = 1;
    }
  }
  else if (DAT_00ffe076 != 3) {
    if (DAT_00ffe076 == 5) goto LAB_0001a6f8;
    if (DAT_00ffe076 == 4) goto LAB_0001a6e6;
    if (DAT_00ffe076 == 6) {
      DAT_00ffe07c = DAT_00ffe07c + -1;
      if (DAT_00ffe07c < 0) {
        FUN_0005668c();
        DAT_00ffe076 = 0;
        return;
      }
      append_sprite_record();
      DAT_00ffe078 = DAT_00ffe078 + -2;
      DAT_00ffe07a = DAT_00ffe07a + -0xe;
      goto LAB_0001a62a;
    }
    DAT_00ffe07c = DAT_00ffe07c + -1;
    if (-1 < DAT_00ffe07c) {
      append_sprite_record();
      DAT_00ffe078 = DAT_00ffe078 + 6;
      DAT_00ffe07a = DAT_00ffe07a + -2;
      goto LAB_0001a62a;
    }
    DAT_00ffe076 = 2;
    DAT_00ffe07c = 0x1e;
    goto LAB_0001a64c;
  }
  DAT_00ffe07c = DAT_00ffe07c + -1;
  if (-1 < DAT_00ffe07c) {
    if (_DAT_00ffe0fc < 0) {
      build_title_sprite_records();
      build_title_sprite_records();
      return;
    }
    return;
  }
  DAT_00ffe076 = 5;
  DAT_00ffe07c = 0x10;
LAB_0001a6f8:
  DAT_00ffe07c = DAT_00ffe07c + -1;
  if (DAT_00ffe07c < 0) {
    attach_behavior_descriptor_e();
    DAT_00ffe076 = 0;
    return;
  }
  if (_DAT_00ffe0d6 == 0) {
    FUN_0001b7d0();
    *(undefined **)(extraout_A0 + 8) = &DAT_0001c2ae;
    *(undefined **)(extraout_A0 + 4) = &DAT_0001c2a8;
    return;
  }
  return;
}


