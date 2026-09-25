// Ghidra headless export
// Entry: 0x0000600E
// Name: advance_entity_sprite_descriptor
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000060F4 convert_graphics_routine_0x000060f4
//   0x000060FA append_entity_render_sprite


void advance_entity_sprite_descriptor(void)

{
  ushort *puVar1;
  ushort uVar2;
  short sVar3;
  short sVar4;
  int in_A0;
  int extraout_A0;
  int extraout_A0_00;
  bool bVar5;
  
  puVar1 = *(ushort **)
            (*(int *)(&DAT_000062ac + (short)(*(short *)(in_A0 + 0xe) << 2)) +
            (int)(short)(*(ushort *)(in_A0 + 0x16) & 0x7fff));
  uVar2 = *puVar1;
  if ((short)uVar2 < 0) {
    if (*(short *)(in_A0 + 0x1e) < (short)(uVar2 & 0x7fff)) {
      sVar3 = *(short *)(in_A0 + 0x1e);
      *(ushort *)(in_A0 + 0x18) = puVar1[1];
    }
    else {
      *(ushort *)(in_A0 + 0x16) = *(ushort *)(in_A0 + 0x16) | 0x8000;
      *(ushort *)(in_A0 + 0x1e) = (uVar2 & 0x7fff) - 1;
      sVar3 = *(short *)(in_A0 + 0x1e);
      *(ushort *)(in_A0 + 0x18) = puVar1[1];
    }
  }
  else {
    if ((short)uVar2 <= *(short *)(in_A0 + 0x1e)) {
      *(undefined2 *)(in_A0 + 0x1e) = 0;
    }
    sVar3 = *(short *)(in_A0 + 0x1e);
    if (*(short *)(in_A0 + 0xe) == 0xd) {
      sVar4 = 0x180;
      if (sVar3 == 2) {
        sVar4 = 0x90;
      }
    }
    else {
      uVar2 = *(ushort *)(in_A0 + 0xca);
      if (uVar2 < 0x16) {
        sVar4 = uVar2 * 0x1c;
      }
      else {
        sVar4 = uVar2 * 0x10;
      }
    }
    *(short *)(in_A0 + 0x18) = sVar4;
  }
  puVar1 = *(ushort **)((int)puVar1 + (short)(sVar3 << 2) + 4);
  uVar2 = *puVar1;
  bVar5 = uVar2 < 0x8002;
  if ((uVar2 == 0x8002) && ((*(byte *)(in_A0 + 0x11) & 1) != 0)) {
    convert_graphics_routine_0x000060f4();
    if (!bVar5) {
      *(undefined2 *)(extraout_A0 + 0x1e) = DAT_00ffdd3e;
      *(undefined2 *)(extraout_A0 + 0x1a) = DAT_00ffdd3c;
      return;
    }
    *(byte *)(extraout_A0 + 0x11) = *(byte *)(extraout_A0 + 0x11) & 0xfe;
    append_entity_render_sprite();
    in_A0 = extraout_A0_00;
  }
  *(ushort **)(in_A0 + 0x12) = puVar1;
  return;
}


