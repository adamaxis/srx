// Ghidra headless export
// Entry: 0x000566EC
// Name: copy_field_mode_palette_bank
// Decompile status: source-shaped annotation
//
// The native conversion is
// ghidra_conversion::copy_field_mode_palette_bank. It copies eight longwords
// (sixteen CRAM colors) selected by the caller into FFD8BE, the colors 32..47
// portion of the full palette upload buffer, and ORs FFEF14 bit 0 so the next
// display service uploads the changed palette. The native routine uses typed
// arrays rather than the source A0/DAT addresses.
// Calls recovered from direct call instructions:
//   (none)


void copy_field_mode_palette_bank(void)

{
  short sVar1;
  undefined4 *in_A0;
  undefined4 *puVar2;
  
  sVar1 = 7;
  puVar2 = (undefined4 *)&DAT_00ffd8be;
  do {
    *puVar2 = *in_A0;
    sVar1 = sVar1 + -1;
    in_A0 = in_A0 + 1;
    puVar2 = puVar2 + 1;
  } while (sVar1 != -1);
  DAT_00ffef14 = DAT_00ffef14 | 1;
  return;
}

