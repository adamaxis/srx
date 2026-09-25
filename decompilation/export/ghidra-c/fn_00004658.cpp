// Ghidra headless export
// Entry: 0x00004658
// Name: FUN_00004658
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000481E FUN_0000481e
//   0x00004ADC FUN_00004adc
//   0x00004E38 advance_entity_primary_timer
//   0x00004E64 advance_entity_secondary_timer
//   0x00004E86 FUN_00004e86
//   0x000053C4 FUN_000053c4
//   0x0000557E FUN_0000557e
//   0x00005936 FUN_00005936
//   0x000059EA advance_title_sprite_timer
//   0x00005B1E sort_entity_slots_by_depth
//   0x00005B98 emit_title_sprite_burst
//   0x00005C7E FUN_00005c7e
//   0x0000622C expand_entity_sprite_descriptors
//   0x0000AEBA advance_title_sprite_wave
//   0x0005D29C FUN_0005d29c


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00004658(void)

{
  undefined4 uVar1;
  undefined2 uVar2;
  short sVar3;
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A0_01;
  int extraout_A0_02;
  int extraout_A0_03;
  int extraout_A0_04;
  int extraout_A0_05;
  int extraout_A0_06;
  int extraout_A0_07;
  int iVar4;
  int extraout_A0_08;
  int *piVar5;
  bool bVar6;
  
  sort_entity_slots_by_depth();
  DAT_00ffdd02 = 1;
  sVar3 = 0xf;
  piVar5 = &DAT_00ffde8a;
  do {
    iVar4 = *piVar5;
    if ((*(byte *)(iVar4 + 0x10) & 1) != 0) {
      if (*(short *)(iVar4 + 0xc) == 0) {
        if (*(char *)(iVar4 + 0xb2) == '\x06') {
          FUN_0000557e();
        }
        else if (*(char *)(iVar4 + 0xb2) == '\r') {
          FUN_000053c4();
        }
      }
      else {
        if (*(undefined1 **)(iVar4 + 0x3a) == &LAB_0000afe2) {
          FUN_00005936();
          iVar4 = extraout_A0;
        }
        else if ((*(short *)(iVar4 + 0xe) == 0xf) &&
                (*(undefined **)(iVar4 + 0x3a) != &DAT_0000afe0)) {
          advance_title_sprite_timer();
          iVar4 = extraout_A0_00;
        }
        if (*(char *)(iVar4 + 0xb2) != '\0') {
          (**(code **)(&DAT_0000490a + (short)((short)*(char *)(iVar4 + 0xb2) << 2)))();
          iVar4 = extraout_A0_01;
        }
        if ((*(byte *)(iVar4 + 0x11) & 0x20) != 0) {
          advance_entity_secondary_timer();
          iVar4 = extraout_A0_02;
        }
        if ((*(byte *)(iVar4 + 0x11) & 0x80) != 0) {
          advance_entity_primary_timer();
          iVar4 = extraout_A0_03;
        }
        if ((*(byte *)(iVar4 + 0x11) & 0x10) != 0) {
          FUN_00005c7e();
          iVar4 = extraout_A0_04;
        }
        if ((*(byte *)(iVar4 + 0x11) & 0x40) != 0) {
          FUN_00004e86();
          iVar4 = extraout_A0_05;
        }
        if ((*(byte *)(iVar4 + 0x10) & 0x80) != 0) {
          emit_title_sprite_burst();
          iVar4 = extraout_A0_06;
        }
        if ((*(undefined1 **)(iVar4 + 0x3a) != &LAB_00015390) || (*(short *)(iVar4 + 0x46) < 0)) {
          if (((*(byte *)(iVar4 + 0x10) & 2) == 0) || (_DAT_00ffdfda == 0)) {
LAB_0000479c:
            expand_entity_sprite_descriptors();
            iVar4 = extraout_A0_07;
          }
          else {
            bVar6 = (DAT_00ffdf88 & 0xf) < 7;
            if ((DAT_00ffdf88 & 0xf) == 7) {
              bVar6 = DAT_00ffdfdc == 0;
              DAT_00ffdfdc = DAT_00ffdfdc + -1;
              if (DAT_00ffdfdc < 0) {
                _DAT_00ffdfda = 0;
                goto LAB_0000479c;
              }
            }
            if (bVar6) goto LAB_0000479c;
          }
          if ((*(undefined1 **)(iVar4 + 0x3a) == &LAB_00015390) && (*(short *)(iVar4 + 0x34) != 0))
          {
            uVar2 = *(undefined2 *)(iVar4 + 0xc);
            uVar1 = *(undefined4 *)(iVar4 + 0x12);
            *(short *)(iVar4 + 0xc) = *(short *)(iVar4 + 0xc) - *(short *)(iVar4 + 8);
            *(undefined **)(iVar4 + 0x12) = &DAT_0000959a;
            expand_entity_sprite_descriptors();
            *(undefined4 *)(extraout_A0_08 + 0x12) = uVar1;
            *(undefined2 *)(extraout_A0_08 + 0xc) = uVar2;
          }
        }
        _DAT_00ffd980 = 1;
      }
    }
    sVar3 = sVar3 + -1;
    piVar5 = piVar5 + 1;
    if (sVar3 == -1) {
      if (DAT_00ffdca0 != 0) {
        FUN_00004adc();
      }
      if (_DAT_00ffdfea != 0) {
        advance_title_sprite_wave();
      }
      FUN_0000481e();
      FUN_0005d29c();
      _DAT_00ffd980 = _DAT_00ffd980 & 0xff00;
      return;
    }
  } while( true );
}


