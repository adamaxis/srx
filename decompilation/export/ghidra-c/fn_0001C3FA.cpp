// Ghidra headless export
// Entry: 0x0001C3FA
// Name: advance_title_sprite_layers
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001C506 render_title_sprite_layers


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

