// Ghidra headless export
// Entry: 0x00057C7A
// Name: convert_intro_routine_0x00057c7a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x0000C0DA render_encoded_title_text
//   0x0000C216 build_grouped_decimal_glyphs
//   0x00013228 convert_menu_routine_0x00013228
//   0x00057610 convert_intro_routine_0x00057610
//   0x00057628 convert_intro_routine_0x00057628
//   0x0005787C convert_intro_routine_0x0005787c
//   0x00057A6E convert_intro_routine_0x00057a6e
//   0x00057AF6 convert_intro_routine_0x00057af6
//   0x00057B42 convert_intro_routine_0x00057b42
//   0x00057C46 convert_intro_routine_0x00057c46
//   0x00057C52 convert_intro_routine_0x00057c52
//   0x00057D7E convert_intro_routine_0x00057d7e
//   0x000580FE convert_intro_routine_0x000580fe
//   0x0005825C convert_intro_routine_0x0005825c
//   0x00058316 convert_intro_routine_0x00058316
//   0x000586CA convert_intro_routine_0x000586ca
//   0x0005878E convert_intro_routine_0x0005878e


void convert_intro_routine_0x00057c7a(void)

{
  byte bVar1;
  undefined4 uVar2;
  ushort uVar4;
  uint uVar3;
  uint extraout_D1;
  byte *extraout_A0;
  char *extraout_A0_00;
  char *extraout_A0_01;
  char *pcVar5;
  undefined4 extraout_A1;
  undefined4 extraout_A1_00;
  undefined4 extraout_A1_01;
  
  convert_intro_routine_0x00057c46();
  convert_intro_routine_0x00057c52();
  if (DAT_00fff0b7 < '\0') {
    if (((DAT_00fff0b8 == '\0') || ((char)(DAT_00fff0b8 + -2) < '\0')) || (DAT_00fff0b8 == '\x02'))
    goto LAB_00057d08;
    if ((char)(DAT_00fff0b8 + -4) < '\0') {
      if ((char)(DAT_00fff0d0 - 6) < '\0') {
        render_encoded_title_text();
        convert_intro_routine_0x00057a6e();
        build_grouped_decimal_glyphs();
        write_vdp_grid();
        render_encoded_title_text();
        render_encoded_title_text();
      }
      goto LAB_0005804a;
    }
    if (DAT_00fff0b8 == '\x04') {
      DAT_00fff0b1 = -1;
    }
  }
  else {
    if (DAT_00fff0b7 != '\0') {
      if (DAT_00fff0b7 == '\x01') {
        write_vdp_grid();
        render_encoded_title_text();
        render_encoded_title_text();
        convert_intro_routine_0x00057a6e();
        build_grouped_decimal_glyphs();
        build_grouped_decimal_glyphs();
        build_grouped_decimal_glyphs();
        build_grouped_decimal_glyphs();
        build_grouped_decimal_glyphs();
        render_encoded_title_text();
        render_encoded_title_text();
        goto LAB_0005804a;
      }
      if ((char)(DAT_00fff0b8 + -3) < '\0') {
        convert_intro_routine_0x00057628();
        DAT_00fff0d6 = (&DAT_00fffbcc)[(short)(ushort)*extraout_A0] + 1;
        convert_intro_routine_0x00057610();
        if ((int)(((extraout_D1 & 0xffff) * (uint)DAT_00fff0d6 & 0xffff) * 0x1e + -0x10000) < 0) {
          convert_intro_routine_0x00057a6e();
          pcVar5 = extraout_A0_00;
        }
        else {
          convert_intro_routine_0x00057b42();
          pcVar5 = extraout_A0_01;
        }
        if (-1 < *pcVar5) {
          render_encoded_title_text();
          render_encoded_title_text();
          convert_intro_routine_0x000580fe(extraout_A1);
        }
        goto LAB_000580ec;
      }
      if (DAT_00fff0b8 == '\x03') {
        uVar4 = DAT_00fff0b6 - 6;
        if (uVar4 != 0) {
          uVar4 = 1;
        }
        if (-1 < (char)(&DAT_001ab196)[(uint)DAT_00fff0d0 * 0xe + (uint)uVar4 * 0x70]) {
          convert_intro_routine_0x0005825c
                    (&DAT_001ab118 +
                     (uint)(byte)(&DAT_001ab196)[(uint)DAT_00fff0d0 * 0xe + (uint)uVar4 * 0x70] *
                     0x18);
        }
        goto LAB_0005804a;
      }
      if (DAT_00fff0b8 != '\x05') {
        convert_intro_routine_0x00058316();
        return;
      }
      if (-1 < (char)(DAT_00fff0d0 - 5)) {
        return;
      }
      DAT_00fff0b3 = '\0';
      if (DAT_00fff0d0 == 0) {
        bVar1 = (&DAT_001ab190)[(short)(ushort)(byte)(DAT_00fffbcb + 1)];
        if ((&DAT_001ab190)[(short)(ushort)(byte)(DAT_00fffbcb + 1)] != DAT_00fffbbe)
        goto LAB_0005842a;
LAB_00058412:
        uVar2 = render_encoded_title_text();
        DAT_00fff0b3 = -1;
      }
      else {
        DAT_00fff0d6 = (&DAT_00fffbc6)[(short)(DAT_00fff0d0 - 1)];
        if ((char)DAT_00fffbbe <= (char)DAT_00fff0d6) goto LAB_00058412;
        bVar1 = DAT_00fff0d6 + 1;
LAB_0005842a:
        DAT_00fff0d6 = bVar1;
        uVar2 = convert_intro_routine_0x0005787c();
      }
      if (DAT_00fff0d0 == 0) {
        render_encoded_title_text();
        render_encoded_title_text();
        uVar2 = extraout_A1_00;
      }
      else {
        if (-1 < (short)((short)CONCAT31((int3)((uint)uVar2 >> 8),DAT_00fff0d0) + -5))
        goto LAB_000580ec;
        render_encoded_title_text();
        render_encoded_title_text();
        uVar2 = extraout_A1_01;
      }
      render_encoded_title_text(uVar2);
      if (-1 < DAT_00fff0b3) {
        render_encoded_title_text(uVar2);
        build_grouped_decimal_glyphs();
        build_grouped_decimal_glyphs();
      }
      write_vdp_grid();
      if (DAT_00fff0d0 == 0) {
        return;
      }
LAB_000580ec:
      render_encoded_title_text();
      return;
    }
    if (DAT_00fff0b8 == '\x01') {
LAB_00057d08:
      convert_intro_routine_0x000586ca();
      return;
    }
    if ((char)(DAT_00fff0b8 + -1) < '\0') {
      if ((char)(&DAT_001aaf40)[(uint)DAT_00fff0d0 * 4 + (uint)(ushort)(DAT_00fff0b6 - 3) * 0x20] <
          '\0') goto LAB_0005804a;
      convert_intro_routine_0x00057d7e();
      goto LAB_000585e8;
    }
    if (DAT_00fff0b8 != '\x03') goto LAB_00057d08;
  }
  uVar4 = (ushort)*(byte *)((uint)DAT_00fff0d0 + DAT_00fffc04 + 0x66);
  if ((-1 < (short)(uVar4 - 1)) && (((short)(uVar4 - 0x29) < 0 || (-1 < (short)(uVar4 - 0x2c))))) {
    uVar3 = convert_menu_routine_0x00013228();
    convert_intro_routine_0x0005878e();
    if (DAT_00fff0b1 == '\0') {
      uVar3 = render_encoded_title_text();
    }
    if (DAT_00fff0b1 == '\0') {
      convert_intro_routine_0x00057af6(&DAT_000d4540 + (uVar3 & 0xffff) * 0x30);
      build_grouped_decimal_glyphs();
    }
    DAT_00fff0b1 = '\0';
LAB_000585e8:
    render_encoded_title_text();
    return;
  }
LAB_0005804a:
  render_encoded_title_text();
  return;
}


