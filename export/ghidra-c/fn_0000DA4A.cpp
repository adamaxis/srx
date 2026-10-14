// Ghidra headless export
// Entry: 0x0000DA4A
// Name: render_attributes_skills_heading
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA render_encoded_title_text
// Behavior annotation from the ROM disassembly:
//   This entry is called only by 0xD82A. It preserves all registers, loads
//   the fixed encoded Attributes/Skills heading at 0xC1B3B, submits it to the
//   text renderer at 0xCC90 with command 0x4000, and restores the caller's
//   register context.


undefined8 render_attributes_skills_heading(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  
  render_encoded_title_text();
  return CONCAT44(in_D0,in_D1);
}


