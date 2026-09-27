// Ghidra headless export
// Entry: 0x0000ECCC
// Name: resolve_entity_pair_collision_event
// Decompile status: reviewed against export/disassembly.s and the US ROM.
// Calls recovered from direct call instructions:
//   0x0001E2E0 begin_world_activity_if_idle
//   0x0001E2EC begin_world_activity

// Source behavior: a bit-4 special record paired with the other record's
// entity bit 1 selects the idle-gated special activity actor and scene/mode
// variant. Independently, bit-3 special records set their own +0xB0 bit 7
// once when the other record carries entity bit 1, then begin ordinary
// activity. The ordinary handler priority is world-effect active,
// pending-world-effect, then the default collision handler. The native
// conversion represents the source handler pointers with a typed enum.


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void resolve_entity_pair_collision_event(void)

{
  byte bVar1;
  int iVar2;
  int in_A0;
  int extraout_A0;
  int in_A1;
  int extraout_A1;
  
  if ((*(byte *)(in_A1 + 0x9e) & 0x10) == 0) {
    if ((*(byte *)(in_A0 + 0x9e) & 0x10) == 0) goto LAB_0000ed3c;
    bVar1 = *(byte *)(in_A1 + 0x10);
    iVar2 = in_A0;
  }
  else {
    bVar1 = *(byte *)(in_A0 + 0x10);
    iVar2 = in_A1;
  }
  if (((bVar1 & 2) != 0) && (_DAT_00ffe128 = iVar2, DAT_00ffe176 == '\0')) {
    _DAT_00ffdd2c = &UNK_00021174;
    begin_world_activity_if_idle();
    in_A0 = extraout_A0;
    in_A1 = extraout_A1;
    if ((short)(DAT_00ffddc4 + -10) == 0) {
      DAT_00fff0ae = 4;
      if (DAT_00fffbf9 == '\x06') {
        DAT_00fff0ae = 2;
      }
    }
    else {
      DAT_00fff0ae = (&DAT_0000eefe)[(short)(DAT_00ffddc4 + -10)];
    }
  }
LAB_0000ed3c:
  if ((*(byte *)(in_A1 + 0x9e) & 8) == 0) {
    if ((*(byte *)(in_A0 + 0x9e) & 8) == 0) {
      return;
    }
    if ((*(byte *)(in_A0 + 0xb0) & 0x80) != 0) {
      return;
    }
    if ((*(byte *)(in_A1 + 0x10) & 2) == 0) {
      return;
    }
    *(byte *)(in_A0 + 0xb0) = *(byte *)(in_A0 + 0xb0) | 0x80;
  }
  else {
    if (((*(byte *)(in_A1 + 0xb0) & 0x80) != 0) || ((*(byte *)(in_A0 + 0x10) & 2) == 0)) {
      return;
    }
    *(byte *)(in_A1 + 0xb0) = *(byte *)(in_A1 + 0xb0) | 0x80;
  }
  DAT_00fffc3c = DAT_00ffe110;
  _DAT_00ffdd2c = &UNK_0001f146;
  if (DAT_00ffe180 == '\0') {
    if (DAT_00ffe182 != '\0') {
      DAT_00ffe182 = '\0';
      _DAT_00ffdd2c = &UNK_00054402;
    }
  }
  else {
    DAT_00ffe180 = '\0';
    _DAT_00ffdd2c = &UNK_00053c90;
  }
  begin_world_activity();
  return;
}


