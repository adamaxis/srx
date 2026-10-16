// Ghidra headless export
// Entry: 0x0000BCAE
// Name: apply_mode_five_boundary_motion
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void apply_mode_five_boundary_motion(void)

{
  ushort *in_A0;
  
  if ((((DAT_00ffddc4 == 5) && (0x11ff < in_A0[2])) && (in_A0[2] < 0x1f80)) && (0x25ff < *in_A0)) {
    if (*in_A0 < 0x2800) {
      in_A0[2] = in_A0[2] - 8;
      return;
    }
    if ((0x3bff < *in_A0) && (*in_A0 < 0x3e00)) {
      in_A0[2] = in_A0[2] + 8;
      return;
    }
  }
  return;
}


