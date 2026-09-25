// Ghidra headless export
// Entry: 0x00013000
// Name: FUN_00013000
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA render_encoded_title_text
//   0x00012148 read_entity_action_code


void FUN_00013000(void)

{
  uint uVar1;
  uint uVar2;
  short sVar3;
  int extraout_A0;
  int iVar4;
  
  render_encoded_title_text();
  if ((char)(DAT_00fff0d0 + -2) < '\0') {
    uVar1 = read_entity_action_code();
    iVar4 = extraout_A0;
  }
  else {
    if (*(char *)(DAT_00fffc04 + 0x56) < '\0') goto LAB_00013094;
    uVar1 = (uint)*(byte *)(DAT_00fffc04 + 0x57);
    iVar4 = DAT_00fffc04;
  }
  if ((short)((short)uVar1 + -1) < 0) {
    sVar3 = 3;
    uVar2 = 5;
    do {
      if ((CONCAT22((short)(uVar1 >> 0x10),*(undefined2 *)(iVar4 + 0x90)) & 1 << (uVar2 & 0x1f)) !=
          0) break;
      uVar2 = (uint)(ushort)((short)uVar2 + 1);
      sVar3 = sVar3 + -1;
    } while (sVar3 != -1);
  }
LAB_00013094:
  render_encoded_title_text();
  return;
}


