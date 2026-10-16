// Ghidra headless export
// Entry: 0x0001E302
// Name: build_world_effect_edge_candidates
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001E41E lookup_world_effect_tile (ten edge-coordinate probes)

/*
 * The source builds two five-point rings at the map edge selected from the
 * current world origin. A zero result from lookup_world_effect_tile means
 * that the coordinate is available for a world effect. The native conversion
 * is in ghidra_conversion/world_effect_edge_conversion.cpp; this file keeps
 * the original RAM-backed evidence readable and auditable.
 */

void build_world_effect_edge_candidates(void)
{
  short x_step;
  short y_step;
  ushort x_anchor;
  ushort y_anchor;
  ushort x;
  ushort y;
  short offset;
  short *out;

  out = &DAT_00ffdd50;
  x_step = 0xbe0;
  y_step = 0x8c0;
  x_anchor = DAT_00ffddc6 + 0xa00U;
  y_anchor = DAT_00ffddc8 + 0x700U;
  if ((ushort)(DAT_00ffddce >> 1) < x_anchor) {
    x_step = -0xbe0;
  }
  if ((ushort)(DAT_00ffddd0 >> 1) < y_anchor) {
    y_step = -0x8c0;
  }

  x = (x_anchor + x_step & 0xfe00) + 0x100;
  y = (y_anchor & 0xfe00) + 0x100;
  for (offset = 0; offset < 5; ++offset) {
    static const short ring_offsets[5] = {0, 0x200, 0x400, -0x200, -0x400};
    const ushort candidate_y = y + ring_offsets[offset];
    if (lookup_world_effect_tile(x, candidate_y) == 0) {
      *out++ = x;
      *out++ = candidate_y;
    }
  }

  y = (y_anchor + y_step & 0xfe00) + 0x100;
  for (offset = 0; offset < 5; ++offset) {
    static const short ring_offsets[5] = {0, 0x200, 0x400, -0x200, -0x400};
    const ushort candidate_x = x + ring_offsets[offset];
    if (lookup_world_effect_tile(candidate_x, y) == 0) {
      *out++ = candidate_x;
      *out++ = y;
    }
  }
  *out = 0;
}
