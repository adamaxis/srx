// Ghidra headless export
// Entry: 0x000167AE
// Name: FUN_000167ae
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D3A sample_source_random_bound
//   0x00000D44 advance_source_prng
//   0x00003074 FUN_00003074
//   0x00016A26 FUN_00016a26
//   0x00016AFA upload_mode_word_block
//   0x00016B7C FUN_00016b7c
//   0x00016E30 snapshot_scene_state
//   0x0001DBAA schedule_world_activity_timeout


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000167ae(void)

{
  int iVar1;
  uint uVar2;
  char cVar3;
  byte bVar4;
  short sVar5;
  undefined2 *puVar6;
  
  DAT_00ffe179 = 0;
  DAT_00ffe17a = 0;
  DAT_00ffe17b = 0;
  DAT_00ffe17c = 0;
  DAT_00ffe17d = 0;
  DAT_00ffe176 = 0;
  DAT_00ffe16f = 0;
  DAT_00ffe170 = 0;
  sVar5 = CONCAT11(DAT_00fffbf9 >> 7,*(undefined1 *)(DAT_00ffde14 + (short)DAT_00fffbf9));
  if (DAT_00ffe186 != '\0') {
    schedule_world_activity_timeout();
    sVar5 = 0x36;
    if (DAT_00ffe186 != '\x01') {
      sVar5 = 0x37;
    }
  }
  if (DAT_00ffe162 != 0) {
    FUN_00016a26();
    sVar5 = 0xf;
  }
  snapshot_scene_state();
  _DAT_00ffdf8e = 0xffff;
  DAT_00ffddc4 = sVar5;
  FUN_00003074();
  if (0x14 < DAT_00ffddc4) {
    if (DAT_00ffddc4 < 0x2c) {
      DAT_00ffdf40 = 2;
      DAT_00fff0c8 = 0;
      _DAT_00fff0c6 = *(undefined2 *)*DAT_00ffde1e;
      DAT_00ffe12c = sample_source_random_bound();
      bVar4 = advance_source_prng();
      DAT_00ffe185 = bVar4 & 7;
      if (DAT_00ffe184 != '\0') {
        DAT_00ffe12c = 5;
        DAT_00ffe185 = 0;
      }
      if (DAT_00ffe187 != '\0') {
        DAT_00ffe12c = 2;
        DAT_00ffe185 = 3;
      }
      if (DAT_00ffe180 != '\0') {
        DAT_00ffe12c = 4;
      }
      DAT_00ffe114 = DAT_00ffde1e[1];
      FUN_00016b7c();
      upload_mode_word_block();
    }
    else if (DAT_00ffddc4 < 0x38) {
      DAT_00ffdf40 = 3;
    }
    else {
      stop();
    }
    sVar5 = 0x1f;
    puVar6 = &DAT_00ffb700;
    do {
      *puVar6 = 0;
      sVar5 = sVar5 + -1;
      puVar6 = puVar6 + 1;
    } while (sVar5 != -1);
    return;
  }
  DAT_00ffdf40 = 1;
  if (DAT_00ffe120 != (undefined *)0x0) {
    DAT_00ffdf40 = 1;
    return;
  }
  if ((short)(DAT_00ffddc4 + -10) == 0) {
    cVar3 = '\x04';
    if (DAT_00fffbf9 == '\x06') {
      cVar3 = '\x02';
    }
  }
  else {
    cVar3 = (&DAT_0000eefe)[(short)(DAT_00ffddc4 + -10)];
  }
  if (DAT_00ffddc4 == 0x14) {
    if ((DAT_00fffc2c & 0x4000000) == 0) {
      DAT_00ffdf40 = 1;
      return;
    }
    DAT_00fffc2c = DAT_00fffc2c & 0xfbffffff;
    schedule_world_activity_timeout();
    return;
  }
  if (DAT_00ffddc4 == 0xd) {
    uVar2 = DAT_00fffc1e & 0x80000;
  }
  else {
    if (DAT_00ffddc4 != 0xf) goto LAB_000168ee;
    uVar2 = DAT_00fffc1e & 0x20000000;
  }
  if (uVar2 == 0) {
    DAT_00ffdf40 = 1;
    return;
  }
LAB_000168ee:
  iVar1 = (int)DAT_00ffe120;
  if (((char)(cVar3 + '\x01') != DAT_00fffc0c) && ((char)(cVar3 + '\x01') != DAT_00fffc0d)) {
    if ((short)(DAT_00ffddc4 + -10) == 0) {
      DAT_00ffe120 = &DAT_00017034;
      if (DAT_00fffbf9 == '\x06') {
        DAT_00ffe120 = &UNK_00017018;
        DAT_00ffdf40 = 1;
        return;
      }
      DAT_00ffdf40 = 1;
      return;
    }
    iVar1 = *(int *)((int)&PTR_DAT_00016fd4 + (int)(short)((DAT_00ffddc4 + -10) * 4));
    if (iVar1 == 0) {
      DAT_00ffdf40 = 1;
      return;
    }
  }
  DAT_00ffe120 = (undefined *)iVar1;
  DAT_00ffdf40 = 1;
  return;
}


