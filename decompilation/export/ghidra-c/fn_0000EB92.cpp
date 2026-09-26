// Ghidra headless export
// Entry: 0x0000EB92
// Name: resolve_entity_proximity_interactions
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D44 advance_source_prng
//   0x0000ECCC resolve_entity_pair_collision_event
//   0x00015D3C measure_relative_heading_and_distance


void resolve_entity_proximity_interactions(void)

{
  ushort uVar1;
  ushort uVar2;
  ushort uVar3;
  undefined2 unaff_D7w;
  ushort *in_A0;
  int extraout_A0;
  ushort *extraout_A0_00;
  int extraout_A1;
  ushort *extraout_A1_00;
  ushort *puVar4;
  undefined1 *puVar5;
  
  if ((in_A0[8] & 0x4000) != 0) {
    return;
  }
  if (in_A0 == (ushort *)0xff0a00) {
    return;
  }
  uVar2 = *in_A0;
  uVar3 = in_A0[2];
  puVar4 = in_A0 + 0x80;
  do {
    if (((puVar4[8] & 0x100) != 0) && ((puVar4[8] & 0x4000) == 0)) {
      if ((in_A0 == *(ushort **)(puVar4 + 0x51)) ||
         (((puVar4 == *(ushort **)(in_A0 + 0x51) || (in_A0 == *(ushort **)(puVar4 + 0x56))) ||
          (puVar4 == *(ushort **)(in_A0 + 0x56))))) {
        if ((((ushort)(*puVar4 - 0x100) <= uVar2) && (uVar2 < (ushort)(*puVar4 + 0x100))) &&
           ((ushort)(puVar4[2] - 0x100) <= uVar3)) {
          uVar1 = puVar4[2] + 0x100;
          goto joined_r0x0000ec14;
        }
      }
      else if ((((ushort)(*puVar4 - 0x40) <= uVar2) && (uVar2 < (ushort)(*puVar4 + 0x40))) &&
              ((ushort)(puVar4[2] - 0x40) <= uVar3)) {
        uVar1 = puVar4[2] + 0x40;
joined_r0x0000ec14:
        if (uVar3 < uVar1) {
          *(byte *)((int)in_A0 + 0x11) = *(byte *)((int)in_A0 + 0x11) | 2;
          *(byte *)((int)puVar4 + 0x11) = *(byte *)((int)puVar4 + 0x11) | 2;
          *(undefined2 *)((int)register0x0000003c + -4) = unaff_D7w;
          *(ushort *)((int)register0x0000003c + -8) = uVar3;
          *(ushort *)((int)register0x0000003c + -0xc) = uVar2;
          uVar1 = puVar4[2];
          *(undefined4 *)((int)register0x0000003c + -0x10) = 0xec6c;
          measure_relative_heading_and_distance();
          uVar2 = *(ushort *)((int)register0x0000003c + -0xc);
          uVar3 = *(ushort *)((int)register0x0000003c + -10);
          unaff_D7w = *(undefined2 *)((int)register0x0000003c + -8);
          puVar5 = (undefined1 *)((int)register0x0000003c + -6);
          *(ushort *)(extraout_A1 + 0x92) = uVar1 + 0x4000;
          *(ushort *)(extraout_A0 + 0x92) = uVar1 + 0xc000;
          *(undefined2 *)(extraout_A1 + 0x94) = *(undefined2 *)(extraout_A0 + 0x38);
          *(undefined2 *)(extraout_A0 + 0x94) = *(undefined2 *)(extraout_A1 + 0x38);
          *(undefined4 *)((int)register0x0000003c + -10) = 0xec90;
          resolve_entity_pair_collision_event();
          *(undefined4 *)((int)register0x0000003c + -10) = 0xec96;
          uVar1 = advance_source_prng();
          in_A0 = extraout_A0_00;
          puVar4 = extraout_A1_00;
          register0x0000003c = (BADSPACEBASE *)puVar5;
          if ((uVar1 & 0x7000) == 0) {
            extraout_A1_00[0x49] = extraout_A1_00[0x49] + 0x4000;
            extraout_A0_00[0x49] = extraout_A0_00[0x49] + 0xc000;
            extraout_A1_00[0x4a] = extraout_A1_00[0x4a] + extraout_A1_00[0x4a];
            extraout_A0_00[0x4a] = extraout_A0_00[0x4a] + extraout_A0_00[0x4a];
          }
        }
      }
    }
    puVar4 = puVar4 + 0x80;
    if ((ushort *)0xff0aff < puVar4) {
      return;
    }
  } while( true );
}


