// Ghidra headless export
// Entry: 0x0000F5F4
// Name: render_bitmask_text_batch
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA render_encoded_title_text
//
// Source behavior: when FFF112 is clear, scan the DBF-expanded source list
// through the D0 selection mask into parallel FFEF16/FFEF56 records. The
// seventh selected slot inserts 0xC661D; exactly nine records duplicate slot 8
// over slot 7, while larger lists append 0xC662B. Render the first eight
// records, or the continuation records beginning at slot 8, with a 0x100
// destination stride through 0xC0DA.


void render_bitmask_text_batch(void)

{
  uint in_D0;
  uint uVar1;
  short unaff_D7w;
  ushort uVar2;
  undefined4 *in_A1;
  undefined4 *unaff_A2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  undefined4 *puVar6;
  
  if (DAT_00fff112 == '\0') {
    puVar3 = &DAT_00ffef16;
    puVar5 = &DAT_00ffef56;
    DAT_00fff0cf = '\0';
    DAT_00fff0b9 = 0xff;
    uVar1 = 0;
    do {
      if ((in_D0 & 1 << (uVar1 & 0x1f)) != 0) {
        puVar4 = puVar3;
        puVar6 = puVar5;
        if (DAT_00fff0cf == '\a') {
          puVar4 = puVar3 + 1;
          *puVar3 = &DAT_000c661d;
          puVar6 = puVar5 + 1;
          *puVar5 = 0;
          DAT_00fff0b9 = DAT_00fff0b9 + 1;
          DAT_00fff0cf = DAT_00fff0cf + '\x01';
        }
        puVar3 = puVar4 + 1;
        *puVar4 = *in_A1;
        puVar5 = puVar6 + 1;
        *puVar6 = *unaff_A2;
        DAT_00fff0b9 = DAT_00fff0b9 + 1;
        DAT_00fff0cf = DAT_00fff0cf + '\x01';
      }
      unaff_A2 = unaff_A2 + 1;
      in_A1 = in_A1 + 1;
      uVar1 = (uint)(ushort)((short)uVar1 + 1);
      unaff_D7w = unaff_D7w + -1;
    } while (unaff_D7w != -1);
    if (-1 < (char)(DAT_00fff0cf + -9)) {
      if (DAT_00fff0cf == '\t') {
        puVar3[-2] = puVar3[-1];
        puVar5[-2] = puVar5[-1];
      }
      else {
        *puVar3 = &DAT_000c662b;
        *puVar5 = 0;
      }
    }
  }
  uVar2 = (ushort)DAT_00fff0b9;
  if (DAT_00fff112 == '\0') {
    if (-1 < (short)(DAT_00fff0b9 - 8)) {
      uVar2 = 7;
    }
  }
  else {
    uVar2 = DAT_00fff0b9 - 7;
  }
  do {
    render_encoded_title_text();
    uVar2 = uVar2 - 1;
  } while (uVar2 != 0xffff);
  return;
}


