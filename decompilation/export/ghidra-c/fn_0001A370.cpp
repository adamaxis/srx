// Ghidra headless export
// Entry: 0x0001A370
// Name: advance_world_effect_controller
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000E9C make_vdp_transfer_command
//   0x00017E78 calculate_menu_row_offset
//   0x0001A538 convert_world_routine_0x0001a538
//   0x0001A54E initialize_effect_defaults
//   0x0001A818 begin_world_effect_descriptor_transition
//   0x0001A8F4 reset_effect_entity
//   0x0001D3F4 expand_random_nibble_mask
//   0x000561E8 select_world_effect_animation_mode
//
// Direct disassembly is authoritative for the branch at 0x1A380: FFE082
// selects the inline descriptor-transition/noise path, while a clear FFE082
// performs the runner-effect and countdown selection below. The typed host
// conversion is in ghidra_conversion/world_effect_controller_conversion.


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void advance_world_effect_controller(void)

{
  ushort uVar2;
  uint uVar1;
  short sVar3;
  uint *extraout_A1;
  bool bVar4;
  
  if (DAT_00ff164a == 0) {
    return;
  }
  bVar4 = _DAT_00ffe082 == 0;
  if (!bVar4) {
    if (_DAT_00ffe0f2 != 0) {
      return;
    }
    DAT_00ff1642 = DAT_00ff1642 - 1;
    if ((short)((ushort)DAT_00ff1642 << 8) < 0) {
      DAT_00ff1642 = 2;
      DAT_00ff1644 = DAT_00ff1644 - 1;
      if ((short)((ushort)DAT_00ff1644 << 8) < 0) {
        reset_effect_entity();
        return;
      }
    }
    wait_for_vblank();
    make_vdp_transfer_command();
    sVar3 = 0x1ff;
    do {
      uVar1 = expand_random_nibble_mask();
      Ram00c00000 = *extraout_A1 & uVar1;
      sVar3 = sVar3 + -1;
    } while (sVar3 != -1);
    return;
  }
  convert_world_routine_0x0001a538();
  if (!bVar4) {
    if (((DAT_00ffdf84 != '\0') && (DAT_00ffe076 == 0)) &&
       (DAT_00ffe074 = DAT_00ffe074 + -1, DAT_00ffe074 < 0)) {
      DAT_00ffe074 = calculate_menu_row_offset();
      _DAT_00ffe07e = select_world_effect_animation_mode();
      if (-1 < _DAT_00ffe07e) {
        if (_DAT_00ffe07e == 2) {
          uVar1 = calculate_menu_row_offset();
          DAT_00ffe0c4 = (undefined2)(0xa00 / (uVar1 & 0xffff));
          _DAT_00ffe0c2 = 1;
          DAT_00ffe0c6 = 0x11f0;
        }
        else if (_DAT_00ffe07e == 3) {
          uVar2 = (ushort)DAT_00fff0e7;
          if (uVar2 == 0) {
            uVar2 = 1;
          }
          DAT_00ffe0c0 = (undefined2)(0x2800 / ((uint)uVar2 * 0xf0 & 0xffff));
          if (DAT_00ffe0bc == 6) {
            DAT_00ffe0b8 = 2;
          }
          else if (DAT_00ffe0bc == 7) {
            DAT_00ffe0b6 = 2;
          }
        }
        else {
          initialize_effect_defaults();
          DAT_00ffe04e = 2;
        }
      }
    }
    return;
  }
  begin_world_effect_descriptor_transition();
  return;
}

