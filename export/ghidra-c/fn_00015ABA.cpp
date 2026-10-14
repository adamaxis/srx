// Ghidra headless export
// Entry: 0x00015ABA
// Name: find_colliding_entity
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void find_colliding_entity(void)

{
  ushort unaff_D5w;
  ushort unaff_D6w;
  short sVar1;
  short *in_A0;
  short *psVar2;
  
  psVar2 = &DAT_00ff0100;
  sVar1 = 9;
  do {
    if (((((*(byte *)(psVar2 + 8) & 1) != 0) && ((*(byte *)(psVar2 + 8) & 0x40) == 0)) &&
        ((*(byte *)(psVar2 + 8) & 0x10) == 0)) &&
       (((*(char *)(psVar2 + 0x26) != '\0' && ((ushort)(*psVar2 - 0x90U) <= unaff_D5w)) &&
        ((unaff_D5w < (ushort)(*psVar2 + 0x90U) &&
         (((ushort)(psVar2[2] - 0x90U) <= unaff_D6w && (unaff_D6w < (ushort)(psVar2[2] + 0x90U))))))
        ))) {
      if (_DAT_00ffdfd8 == 0) {
        if (*(char *)((int)in_A0 + 0x9f) == '\0') {
          if (((*(byte *)(in_A0 + 8) & 8) == 0) ||
             ((((*(byte *)(psVar2 + 0x58) & 0x80) == 0 && ((*(byte *)(psVar2 + 8) & 8) == 0)) &&
              ((*(byte *)(psVar2 + 0x4f) & 0x10) == 0)))) {
            *in_A0 = *psVar2;
            in_A0[2] = psVar2[2] + 0x10;
            return;
          }
        }
        else {
          if ((*(byte *)(in_A0 + 8) & 8) == 0) {
            return;
          }
          if ((((*(byte *)(psVar2 + 0x58) & 0x80) == 0) && ((*(byte *)(psVar2 + 8) & 8) == 0)) &&
             ((*(byte *)(psVar2 + 0x4f) & 0x10) == 0)) {
            return;
          }
        }
      }
      else if (psVar2 == _DAT_00ffdd3c) {
        return;
      }
    }
    psVar2 = psVar2 + 0x80;
    sVar1 = sVar1 + -1;
    if (sVar1 == -1) {
      return;
    }
  } while( true );
}


