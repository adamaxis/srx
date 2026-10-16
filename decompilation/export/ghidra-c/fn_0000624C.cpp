// Ghidra headless export
// Entry: 0x0000624C
// Name: expand_entity_sprite_descriptor_list
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void expand_entity_sprite_descriptor_list(void)

{
  short sVar1;
  short sVar2;
  ushort uVar3;
  char unaff_D5b;
  short unaff_D6w;
  int in_A0;
  short *unaff_A2;
  ushort *puVar4;
  short *unaff_A3;
  
  while( true ) {
    if (*unaff_A2 == -0x8000) break;
    sVar1 = unaff_A2[1];
    sVar2 = *(short *)(in_A0 + 0xc);
    unaff_A3[3] = *(short *)(in_A0 + 10) + *unaff_A2;
    *unaff_A3 = sVar2 + sVar1;
    puVar4 = (ushort *)(unaff_A2 + 3);
    *(char *)(unaff_A3 + 1) = (char)unaff_A2[2];
    *(char *)((int)unaff_A3 + 3) = unaff_D5b;
    unaff_A2 = unaff_A2 + 4;
    uVar3 = *puVar4;
    if ((*(byte *)(in_A0 + 0x11) & 0x80) == 0) {
      uVar3 = *(ushort *)(in_A0 + 0x2c) | uVar3;
    }
    if (unaff_D6w != 0) {
      uVar3 = *(short *)(in_A0 + 0x20) + uVar3;
    }
    unaff_A3[2] = uVar3;
    unaff_D5b = unaff_D5b + '\x01';
    DAT_00ffdd02 = DAT_00ffdd02 + 1;
    unaff_A3 = unaff_A3 + 4;
    _DAT_00ffdd18 = unaff_A3;
    _DAT_00ffdd1c = DAT_00ffdd02;
  }
  return;
}


