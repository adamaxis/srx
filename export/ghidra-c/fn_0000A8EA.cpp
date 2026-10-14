// Ghidra headless export
// Entry: 0x0000A8EA
// Name: service_entity_behavior_frames
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000391A handle_mode_action_event
//   0x0000AB02 complete_entity_behavior_transition
//   0x0000AC5E clear_entity_effect_if_forced
//   0x0000AC84 dispatch_entity_collision_behavior
//   0x0000ACAE advance_entity_behavior_action
//   0x0000B330 enter_entity_behavior_transition
//   0x0000B49C resolve_entity_fallback_movement
//   0x0000BCAE apply_mode_five_boundary_motion
//   0x0000EB92 resolve_entity_proximity_interactions
//   0x00013874 dispatch_entity_action_candidates
//   0x00015BB4 noop_entity_action_hook
//   0x0001E514 clear_effect_slot_state


void service_entity_behavior_frames(void)

{
  short sVar1;
  byte bVar2;
  undefined1 uVar3;
  short sVar4;
  undefined2 *puVar5;
  undefined2 *extraout_A0;
  undefined2 *extraout_A0_00;
  undefined2 *extraout_A0_01;
  undefined2 *extraout_A0_02;
  undefined2 *extraout_A0_03;
  undefined2 *extraout_A0_04;
  undefined2 *extraout_A0_05;
  undefined2 *extraout_A0_06;
  undefined2 *extraout_A0_07;
  undefined2 *extraout_A0_08;
  undefined2 *extraout_A0_09;
  
  dispatch_entity_action_candidates();
  puVar5 = &DAT_00ff0100;
  sVar4 = 0xf;
  do {
    if ((*(byte *)(puVar5 + 8) & 1) != 0) {
      if (-1 < (short)puVar5[0x6e]) {
        puVar5[0x6e] = puVar5[0x6e] + -1;
      }
      if ((*(byte *)(puVar5 + 8) & 4) == 0) {
LAB_0000aa54:
        if ((*(byte *)(puVar5 + 8) & 2) == 0) {
          if ((*(byte *)(puVar5 + 8) & 4) != 0) {
            dispatch_entity_collision_behavior();
            puVar5 = extraout_A0_07;
          }
        }
        else {
          advance_entity_behavior_action();
          puVar5 = extraout_A0_06;
        }
      }
      else if ((*(byte *)(puVar5 + 8) & 0x40) == 0) {
        if ((*(byte *)((int)puVar5 + 0x11) & 0x10) != 0) {
          uVar3 = noop_entity_action_hook();
          *(undefined1 *)((int)extraout_A0 + 0xb3) = uVar3;
          puVar5 = extraout_A0;
        }
        if ((short)puVar5[0xb] < 0) {
          if ((*(undefined1 **)(puVar5 + 0x1d) == &LAB_0000b0d4) ||
             (*(undefined1 **)(puVar5 + 0x1d) == &LAB_0000b10c)) {
LAB_0000a9be:
            complete_entity_behavior_transition();
            clear_effect_slot_state();
            puVar5 = extraout_A0_02;
          }
          else {
            if (*(char *)(puVar5 + 0x4e) == '\0') {
              bVar2 = *(byte *)(puVar5 + 8);
              *(byte *)(puVar5 + 8) = bVar2 | 0x10;
              if ((bVar2 & 0x10) != 0) {
                if (puVar5[7] != 0xe) {
                  complete_entity_behavior_transition();
                  puVar5 = extraout_A0_03;
                  if ((*(byte *)(extraout_A0_03 + 8) & 2) != 0) {
                    handle_mode_action_event();
                  }
                  goto LAB_0000aa74;
                }
                goto LAB_0000a9be;
              }
            }
            else {
              if (*(char *)(puVar5 + 0x26) != '\0') goto LAB_0000aa54;
              bVar2 = *(byte *)(puVar5 + 8);
              *(byte *)(puVar5 + 8) = bVar2 | 0x10;
              if ((bVar2 & 0x10) != 0) {
                if (puVar5[7] != 0xe) {
                  if (((puVar5[7] != 0xc) || (puVar5[0xb] != 0x80)) &&
                     (complete_entity_behavior_transition(), puVar5 = extraout_A0_04,
                     (*(byte *)(extraout_A0_04 + 8) & 2) != 0)) {
                    handle_mode_action_event();
                  }
                  goto LAB_0000aa74;
                }
                goto LAB_0000a9be;
              }
            }
LAB_0000aa4c:
            enter_entity_behavior_transition();
            puVar5 = extraout_A0_05;
          }
        }
        else if ((short)puVar5[0xb] < 0x3d) {
          if (*(char *)(puVar5 + 0x4e) == '\0') {
            bVar2 = *(byte *)(puVar5 + 8);
            *(byte *)(puVar5 + 8) = bVar2 | 0x10;
            if ((bVar2 & 0x10) == 0) goto LAB_0000aa4c;
            complete_entity_behavior_transition();
            puVar5 = extraout_A0_00;
            if ((*(byte *)(extraout_A0_00 + 8) & 2) != 0) {
              handle_mode_action_event();
            }
          }
          else {
            if (*(char *)(puVar5 + 0x26) != '\0') goto LAB_0000aa54;
            bVar2 = *(byte *)(puVar5 + 8);
            *(byte *)(puVar5 + 8) = bVar2 | 0x10;
            if ((bVar2 & 0x10) == 0) goto LAB_0000aa4c;
            complete_entity_behavior_transition();
            puVar5 = extraout_A0_01;
            if ((*(byte *)(extraout_A0_01 + 8) & 2) != 0) {
              handle_mode_action_event();
            }
          }
        }
      }
LAB_0000aa74:
      if ((*(byte *)(puVar5 + 8) & 4) != 0) {
        apply_mode_five_boundary_motion();
        resolve_entity_proximity_interactions();
        bVar2 = *(byte *)((int)extraout_A0_08 + 0x11);
        *(byte *)((int)extraout_A0_08 + 0x11) = bVar2 & 0xfd;
        puVar5 = extraout_A0_08;
        if ((bVar2 & 2) != 0) {
          *(undefined4 *)(extraout_A0_08 + 0x6a) = *(undefined4 *)(extraout_A0_08 + 0x1d);
          *(code **)(extraout_A0_08 + 0x1d) = resolve_entity_fallback_movement;
        }
      }
      *(byte *)((int)puVar5 + 0x11) = *(byte *)((int)puVar5 + 0x11) & 0xfb;
      sVar1 = puVar5[0xb];
      (**(code **)(puVar5 + 0x1d))();
      clear_entity_effect_if_forced();
      puVar5 = extraout_A0_09;
      if (sVar1 != extraout_A0_09[0xb]) {
        *(byte *)((int)extraout_A0_09 + 0x11) = *(byte *)((int)extraout_A0_09 + 0x11) | 1;
      }
    }
    puVar5 = puVar5 + 0x80;
    sVar4 = sVar4 + -1;
    if (sVar4 == -1) {
      return;
    }
  } while( true );
}


