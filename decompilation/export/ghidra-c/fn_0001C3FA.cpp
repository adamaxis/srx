// Ghidra headless export
// Entry: 0x0001C3FA
// Name: advance_title_sprite_layers
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001C506 render_title_sprite_layers
//
// ROM trace (the generic decompiler body below omits the source-register
// values): FFDF8C selects the fixed alternate pair A2=0x1D428 and
// A4=0x1D452. Otherwise the +0x08 behavior descriptor supplies the
// secondary limit and pointer array, while the +0x04 descriptor supplies the
// primary limit and pointer array. The source advances the secondary phase
// first, then the primary phase, and passes primary to A2 and secondary to A4
// at 0x1C474/0x1C48A before calling 0x1C506.


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void advance_title_sprite_layers(void)

{
  if (_DAT_00ffdf8c != 0) {
    render_title_sprite_layers();
    return;
  }
  if ((DAT_00ff158c < (char)*DAT_00ff1588) &&
     (DAT_00ff1582 = DAT_00ff1582 + -1, DAT_00ff1582 < '\0')) {
    DAT_00ff1582 = '\x04';
    if ((char)(DAT_00ff158c + '\x01') <= (char)*DAT_00ff1588) {
      DAT_00ff158c = DAT_00ff158c + '\x01';
    }
  }
  if ((DAT_00ff158d < (char)*DAT_00ff1584) &&
     (DAT_00ff1583 = DAT_00ff1583 + -1, DAT_00ff1583 < '\0')) {
    DAT_00ff1583 = '\x04';
    if ((char)(DAT_00ff158d + '\x01') <= (char)*DAT_00ff1584) {
      DAT_00ff158d = DAT_00ff158d + '\x01';
    }
  }
  render_title_sprite_layers();
  return;
}

