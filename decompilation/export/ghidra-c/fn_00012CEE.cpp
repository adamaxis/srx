// Ghidra headless export
// Entry: 0x00012CEE
// Name: convert_menu_routine_0x00012cee
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x000104F6 convert_menu_routine_0x000104f6
//   0x00012148 read_entity_action_code
//   0x00012C5A write_menu_repeated_grid_with_metadata
//   0x00012C74 convert_menu_routine_0x00012c74
//   0x00012CD6 render_menu_text_asset
//   0x00012FA4 advance_entity_action_menu
//   0x00055E62 convert_intro_routine_0x00055e62
//   0x00055ED6 convert_intro_routine_0x00055ed6
//   0x00055F02 convert_intro_routine_0x00055f02
//   0x00055F20 convert_intro_routine_0x00055f20


/* WARNING: Removing unreachable block (ram,0x00012ddc) */
/* WARNING: Removing unreachable block (ram,0x00012d50) */

void convert_menu_routine_0x00012cee(void)

{
  undefined1 uVar1;
  undefined2 extraout_D0u;
  short sVar2;
  char cVar3;
  uint uVar4;
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A0_01;
  int extraout_A0_02;
  int extraout_A0_03;
  
  write_menu_repeated_grid_with_metadata();
  sVar2 = convert_intro_routine_0x00055ed6();
  if (sVar2 == 0) {
    if ((char)(DAT_00fff0d0 + -2) < '\0') {
      convert_intro_routine_0x00055f20();
      cVar3 = read_entity_action_code();
      if (cVar3 != '\x17') {
        if ((char)(cVar3 + -0xd) < '\0') {
          uVar1 = *(undefined1 *)(extraout_A0_00 + 0x57);
          *(char *)(extraout_A0_00 + 0x57) = cVar3;
          convert_menu_routine_0x00012c74();
          *(undefined1 *)(extraout_A0_01 + 0x57) = uVar1;
        }
        else if (-1 < (char)(cVar3 + -0x10)) {
          return;
        }
      }
    }
    else {
      if (*(char *)(extraout_A0 + 0x56) < '\0') {
        advance_entity_action_menu();
        convert_intro_routine_0x00055f02();
        convert_intro_routine_0x00055f20();
        convert_menu_routine_0x000104f6();
        goto LAB_00012e7a;
      }
      convert_intro_routine_0x00055f20();
      if ((*(char *)(extraout_A0_02 + 0x56) != '\x7f') &&
         ((char)(*(char *)(extraout_A0_02 + 0x57) + -0xd) < '\0')) {
        convert_menu_routine_0x00012c74();
      }
    }
    render_menu_text_asset();
  }
  else {
    if ((((char)(DAT_00fff0d0 + -2) < '\0') && (cVar3 = read_entity_action_code(), cVar3 != '\x17'))
       && (-1 < (char)(cVar3 + -0x10))) {
      return;
    }
    convert_intro_routine_0x00055f20();
    convert_intro_routine_0x00055e62();
    sVar2 = 2;
    uVar4 = 9;
    do {
      if ((CONCAT22(extraout_D0u,*(undefined2 *)(extraout_A0_03 + 0x90)) & 1 << (uVar4 & 0x1f)) != 0
         ) break;
      uVar4 = (uint)(ushort)((short)uVar4 + 1);
      sVar2 = sVar2 + -1;
    } while (sVar2 != -1);
    render_menu_text_asset();
  }
LAB_00012e7a:
  write_vdp_grid();
  return;
}


