// Ghidra headless export
// Entry: 0x000566EC
// Name: copy_field_mode_panel_snapshot
// Decompile status: source-shaped annotation
//
// The native conversion is
// ghidra_conversion::copy_field_mode_panel_snapshot. It copies the eight
// longwords selected by the caller into the field-mode panel staging record
// and ORs the display dirty flag. The native routine uses typed arrays rather
// than the source A0/DAT addresses.
// Calls recovered from direct call instructions:
//   (none)


void copy_field_mode_panel_snapshot(void)

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

