// Ghidra headless export
// Entry: 0x00054662
// Name: FUN_00054662
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 FUN_00000bd2
//   0x00000BF2 FUN_00000bf2
//   0x00000D94 FUN_00000d94
//   0x00000DBA FUN_00000dba
//   0x00000F5C FUN_00000f5c
//   0x00001232 FUN_00001232
//   0x0000195C FUN_0000195c
//   0x0000C0DA FUN_0000c0da
//   0x0000C376 FUN_0000c376
//   0x00010952 FUN_00010952
//   0x000176EA FUN_000176ea
//   0x000177CC FUN_000177cc
//   0x00017F80 FUN_00017f80
//   0x0001822C FUN_0001822c
//   0x000532A8 FUN_000532a8
//   0x000549E4 FUN_000549e4
//   0x000549F6 FUN_000549f6
//   0x00054A28 FUN_00054a28
//   0x00054A8E FUN_00054a8e
//   0x00055F44 FUN_00055f44
//   0x000566F6 FUN_000566f6


/* WARNING: Removing unreachable block (ram,0x000548c4) */

undefined4 FUN_00054662(void)

{
  byte *pbVar1;
  char cVar2;
  byte bVar3;
  int iVar4;
  short sVar5;
  short sVar6;
  
  sVar6 = DAT_00fff102;
  DAT_00fff0b7 = 0xff;
  DAT_00fff0d4 = 0;
  *(byte *)((int)&DAT_00fffb30 + (int)DAT_00fff102) =
       *(byte *)((int)&DAT_00fffb30 + (int)DAT_00fff102) & 0xf;
  pbVar1 = (byte *)((int)&DAT_00fffb30 + (int)sVar6);
  *pbVar1 = *pbVar1 | 0x40;
  sVar5 = DAT_00fff118 * 0x14;
  pbVar1 = (byte *)((int)&DAT_00fffb30 + (int)sVar6);
  *pbVar1 = *(byte *)((int)&DAT_00ff1104 + sVar5 + 1) | *pbVar1;
  if (*(char *)((int)&DAT_00ff1100 + (int)sVar5) == '\x01') {
    FUN_000532a8();
  }
  DAT_00fff0a6 = 0xff;
  DAT_00fff0a8 = '\0';
  VDP_CTRL = 0x8b00;
LAB_000546cc:
  do {
    VDP_CTRL = 0x8134;
    FUN_00010952();
    FUN_000566f6();
    FUN_00000f5c();
    FUN_00000f5c();
    sVar6 = 5;
    do {
      FUN_00001232();
      sVar6 = sVar6 + -1;
    } while (sVar6 != -1);
    FUN_00001232();
    FUN_00001232();
    FUN_00001232();
    FUN_00001232();
    FUN_000549f6();
    FUN_00017f80();
    VDP_CTRL = 0x8174;
    FUN_00054a8e();
    if (DAT_00fff0b1 < '\0') {
      return 0;
    }
    FUN_0000c0da();
    FUN_0000c0da();
    FUN_00054a28();
    DAT_00fff0dc = 0;
LAB_0005488c:
    FUN_0000c376();
    do {
      FUN_00000bd2();
      FUN_00000bf2();
      FUN_0000195c();
      if (((DAT_00ffdd22._0_1_ & 1) != 0) || ((DAT_00ffdd22._0_1_ & 2) != 0)) goto LAB_0005488c;
      if (((DAT_00ffdd22._0_1_ & 0x40) != 0) ||
         (((DAT_00ffdd22._0_1_ & 0x10) != 0 || ((DAT_00ffdd22._0_1_ & 0x20) != 0)))) {
        FUN_00055f44();
        sVar6 = *(byte *)(DAT_00fffc04 + 0x88) - 1;
        if (sVar6 == 0) {
          sVar6 = 1;
        }
        cVar2 = *(char *)((int)&DAT_00ff1104 + (short)(DAT_00fff118 * 0x14) + 1) + -1;
        bVar3 = (char)sVar6 - cVar2;
        if ((bVar3 == 0 || SBORROW1((char)sVar6,cVar2) != (short)((ushort)bVar3 << 8) < 0) ||
           (FUN_00000dba(), DAT_00fff0dd < '\x01')) {
          FUN_000549e4();
          FUN_0000c0da();
          FUN_00000d94();
          DAT_00fff0a8 = DAT_00fff0a8 + '\x01';
        }
        else {
          iVar4 = (uint)(ushort)(short)*(char *)((int)&DAT_00ff1100 +
                                                (int)(short)(DAT_00fff118 * 0x14)) * 10;
          (**(code **)(*(int *)((int)&PTR_PTR_001664de + (int)(short)iVar4) +
                      (int)(short)((short)CONCAT31((int3)((uint)iVar4 >> 8),DAT_00fff0dc) << 2)))();
          DAT_00fff0a8 = DAT_00fff0a8 + '\x01';
        }
        goto LAB_000546cc;
      }
    } while ((DAT_00ffdd22._0_1_ & 0x80) == 0);
    VDP_CTRL = 0x8134;
    DAT_00fff0b7 = 0;
    FUN_000176ea();
    FUN_000177cc();
    VDP_CTRL = 0x8174;
    FUN_0001822c();
    sVar6 = 4;
    do {
      (&DAT_00fff0ed)[sVar6] = 6;
      (&DAT_00fff0e8)[sVar6] = 1;
      sVar6 = sVar6 + -1;
    } while (sVar6 != -1);
  } while( true );
}


