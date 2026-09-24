// Ghidra headless export
// Entry: 0x0005672C
// Name: FUN_0005672c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


ushort FUN_0005672c(void)

{
  ushort in_D0w;
  short sVar1;
  
  if (DAT_00ffe18a != '\0') {
    return in_D0w;
  }
  if (DAT_00ffddc4 != 0x14) {
    if (DAT_00ffdf40 == 1) {
      DAT_00ffdf90 = DAT_00ffdf90 + -1;
      if (DAT_00ffdf90 < 0) {
        DAT_00ffdf90 = 6;
        DAT_00ffdf92 = DAT_00ffdf92 + 1;
        if (3 < DAT_00ffdf92) {
          DAT_00ffdf92 = 0;
        }
        sVar1 = DAT_00ffdf92 << 1;
        in_D0w = 0xc018;
        VDP_CTRL = 0xc018;
        VDP_CTRL = 0;
        VDP_DATA = *(word *)((int)&DAT_0018ad6a + (int)sVar1);
        VDP_DATA = *(word *)((int)&DAT_0018ad6c + (int)sVar1);
        VDP_DATA = *(word *)((int)&DAT_0018ad6e + (int)sVar1);
        VDP_DATA = *(word *)((int)&DAT_0018ad70 + (int)sVar1);
      }
    }
    else if (DAT_00ffdf40 == 2) {
      in_D0w = DAT_00ffdf88 & 0xf;
      if (in_D0w == 0xf) {
        if (DAT_00ffdf90 < 0) {
          DAT_00ffdf92 = DAT_00ffdf92 - 1 & 7;
          if (DAT_00ffdf92 == 7) {
            DAT_00ffdf90 = 0;
            DAT_00ffdf92 = 0;
          }
        }
        else {
          DAT_00ffdf92 = DAT_00ffdf92 + 1 & 7;
          if (DAT_00ffdf92 == 0) {
            DAT_00ffdf90 = -1;
            DAT_00ffdf92 = 7;
          }
        }
        in_D0w = 0xc01e;
        VDP_CTRL = 0xc01e;
        VDP_CTRL = 0;
        VDP_DATA = DAT_00ffdf92 << 9;
      }
    }
    else if (DAT_00ffdf40 == 0) {
      if ((((DAT_00ffddc4 == 6) || (DAT_00ffddc4 == 7)) || (DAT_00ffddc4 == 8)) ||
         (DAT_00ffddc4 == 9)) {
        in_D0w = 0xc01a;
        DAT_00ffdf90 = DAT_00ffdf90 + -1;
        if (DAT_00ffdf90 < 0) {
          DAT_00ffdf90 = 0xc;
          DAT_00ffdf92 = DAT_00ffdf92 + 1;
          if (2 < DAT_00ffdf92) {
            DAT_00ffdf92 = 0;
          }
          sVar1 = DAT_00ffdf92 << 3;
          VDP_CTRL = 0xc01a;
          VDP_CTRL = 0;
          VDP_DATA = *(word *)((int)&DAT_0018ad7a + (int)sVar1);
          VDP_DATA = *(word *)((int)&DAT_0018ad7c + (int)sVar1);
          VDP_DATA = *(word *)((int)&DAT_0018ad7e + (int)sVar1);
        }
      }
      else if (DAT_00ffddc4 == 4) {
        in_D0w = 0xc00c;
        DAT_00ffdf90 = DAT_00ffdf90 + -1;
        if (DAT_00ffdf90 < 0) {
          DAT_00ffdf90 = 10;
          DAT_00ffdf92 = DAT_00ffdf92 + 1;
          if (2 < DAT_00ffdf92) {
            DAT_00ffdf92 = 0;
          }
          sVar1 = DAT_00ffdf92 << 3;
          VDP_CTRL = 0xc00c;
          VDP_CTRL = 0;
          VDP_DATA = *(word *)((int)&DAT_0018ad92 + (int)sVar1);
          VDP_DATA = *(word *)((int)&DAT_0018ad94 + (int)sVar1);
          VDP_DATA = *(word *)((int)&DAT_0018ad96 + (int)sVar1);
          return in_D0w;
        }
      }
      else if (DAT_00ffddc4 == 5) {
        in_D0w = 0xc014;
        DAT_00ffdf90 = DAT_00ffdf90 + -1;
        if (DAT_00ffdf90 < 0) {
          DAT_00ffdf90 = 10;
          DAT_00ffdf92 = DAT_00ffdf92 + 1;
          if (2 < DAT_00ffdf92) {
            DAT_00ffdf92 = 0;
          }
          sVar1 = DAT_00ffdf92 << 1;
          VDP_CTRL = 0xc014;
          VDP_CTRL = 0;
          VDP_DATA = *(word *)((int)&DAT_0018ad4e + (int)sVar1);
          VDP_DATA = *(word *)((int)&DAT_0018ad50 + (int)sVar1);
          VDP_DATA = *(word *)((int)&DAT_0018ad52 + (int)sVar1);
          return 0xc014;
        }
      }
      else if (DAT_00ffddc4 == 0) {
        DAT_00ffdf90 = DAT_00ffdf90 + -1;
        if (DAT_00ffdf90 < 0) {
          DAT_00ffdf90 = 6;
          DAT_00ffdf92 = DAT_00ffdf92 + 1;
          if (5 < DAT_00ffdf92) {
            DAT_00ffdf92 = 0;
          }
          in_D0w = 0xc014;
          VDP_CTRL = 0xc014;
          VDP_CTRL = 0;
          VDP_DATA = *(word *)((int)&DAT_0018ad42 + (int)(short)(DAT_00ffdf92 << 1));
        }
      }
      else if (((DAT_00ffddc4 < 0x32) && (0x2b < DAT_00ffddc4)) &&
              (DAT_00ffdf90 = DAT_00ffdf90 + -1, DAT_00ffdf90 < 0)) {
        DAT_00ffdf90 = 9;
        DAT_00ffdf92 = DAT_00ffdf92 + 1;
        if (3 < DAT_00ffdf92) {
          DAT_00ffdf92 = 0;
        }
        sVar1 = DAT_00ffdf92 << 1;
        in_D0w = 0xc018;
        VDP_CTRL = 0xc018;
        VDP_CTRL = 0;
        VDP_DATA = *(word *)((int)&DAT_0018ad5a + (int)sVar1);
        VDP_DATA = *(word *)((int)&DAT_0018ad5c + (int)sVar1);
        VDP_DATA = *(word *)((int)&DAT_0018ad5e + (int)sVar1);
        VDP_DATA = *(word *)((int)&DAT_0018ad60 + (int)sVar1);
      }
    }
  }
  return in_D0w;
}


