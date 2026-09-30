// Ghidra headless export
// Entry: 0x0000075E
// Name: run_game_loop
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00000C2C clear_runtime_memory_and_vdp
//   0x000012F6 advance_title_palette_fade
//   0x0000131C step_palette_transition
//   0x0000195C sample_controller_input
//   0x00001A9E run_title_loop
//   0x0000281E initialize_field_scene_entry
//   0x000028C2 reinitialize_field_scene_entry
//   0x00002BD2 advance_field_scene_transition
//   0x00002CE0 advance_field_mode_transition
//   0x00002F32 initialize_field_scene_state
//   0x00002FA4 snapshot_entity_anchors
//   0x00003084 load_field_scene_profile_and_terrain
//   0x00003168 toggle_field_scene_variant
//   0x0000391A handle_mode_action_event
//   0x00003A1C update_field_camera
//   0x000042B0 initialize_joshua_archetype_selection
//   0x00004618 update_entity_render_pass
//   0x0000A7B4 initialize_entity_runtime
//   0x0000A8EA service_entity_behavior_frames
//   0x00010D30 run_command_overview
//   0x00016396 advance_palette_fade
//   0x000167AE initialize_world_activity_mode
//   0x0001DC22 select_next_entity
//   0x0001DD94 advance_world_frame_logic
//   0x0001E2EC begin_world_activity
//   0x0001E43C handle_world_activity_request
//   0x0001E68C reset_world_effect_globals
//   0x0001F5CA run_field_exit_orchestrator
//   0x00026938 copy_two_word_rows
//   0x00053126 prepare_mode_entry_service
//   0x00053156 service_audio_tick_wrapper
//   0x0005315E run_fixed_audio_transaction_wrapper
//   0x00053290 reset_audio_menu_arrays
//   0x000566A2 copy_action_selection_profile_table
//   0x00059092 run_start_options
//   0x00059318 initialize_save_roster_buffer
//   0x000594E0 restore_saved_field_state
//   0x00059C5E run_title_intro_stages
//   0x0005D746 run_pre_title_sequence


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void run_game_loop(void)

{
  char cVar1;
  byte bVar2;
  word wVar3;
  undefined *puVar4;
  undefined *puVar5;
  undefined *puVar6;
  undefined *puVar7;
  short sVar8;
  byte bVar9;
  uint uVar10;
  uint uVar11;
  short sVar12;
  short unaff_D6w;
  short sVar13;
  char *pcVar14;
  byte *pbVar15;
  char *extraout_A1;
  undefined2 *puVar17;
  undefined4 *puVar18;
  undefined1 *puVar20;
  bool bVar21;
  byte *pbVar16;
  undefined4 *puVar19;
  
  clear_runtime_memory_and_vdp();
  run_pre_title_sequence();
  if (DAT_00fff9e8 != 0x12345678) {
    DAT_00fff9e8 = 0x12345678;
  }
  DAT_00fff9f8 = (short)DAT_00fff9ec + DAT_00fff9f8;
  _DAT_00fffc08 = 1;
  _DAT_00fffc0a = 1;
  _DAT_00ffdfe8 = 0xffff;
  run_title_loop();
  prepare_mode_entry_service();
  if (_DAT_00ffddc0 != 0) {
    initialize_save_roster_buffer();
    run_start_options();
    step_palette_transition();
    if (_DAT_00ffe108 == 0) {
      run_title_intro_stages();
    }
LAB_000007f0:
    prepare_mode_entry_service();
    reset_audio_menu_arrays();
    DAT_00fffbfe = 0x14;
    _DAT_00fffc12 = 0;
    _DAT_00fffc16 = 0;
    _DAT_00fffc1a = 0;
    DAT_00fffc1e = 0;
    DAT_00fffc22 = 0;
    DAT_00fffc26 = 0;
    DAT_00fffc27 = 0;
    _DAT_00fffc28 = 0;
    DAT_00fffc2a = 0;
    DAT_00fffc2c = 0;
    DAT_00fffc34 = 0;
    DAT_00fffc38 = 0;
    DAT_00fffc30 = 1;
    DAT_00ffddc4 = 2;
    DAT_00fffbf8 = 2;
    load_field_scene_profile_and_terrain();
    _DAT_00ffdfe8 = 0xffff;
    DAT_00fff0a0 = 0xff;
    initialize_entity_runtime();
    reset_world_effect_globals();
    copy_action_selection_profile_table();
    copy_two_word_rows();
    initialize_field_scene_entry();
    if (_DAT_00ffe108 != 0) {
      initialize_joshua_archetype_selection();
      restore_saved_field_state();
      reinitialize_field_scene_entry();
      _DAT_00ffe108 = 0;
    }
LAB_000008c0:
    do {
      sVar13 = 0x1e;
      _DAT_00ffdfe8 = 0;
      DAT_00ffe18a = 1;
      DAT_00ffdd22 = 0;
      _DAT_00ffdd26 = 0;
      DAT_00ffdd04 = 0xffff;
      _DAT_00ffdd06 = 0xffff;
      do {
        service_entity_behavior_frames();
        update_field_camera();
        update_entity_render_pass();
        DAT_00ffef14 = DAT_00ffef14 | 0xe;
        sVar13 = sVar13 + -1;
      } while (sVar13 != -1);
      wait_for_vblank();
      _DAT_00ffdfe8 = 0xffff;
      advance_title_palette_fade();
      DAT_00ffe18a = 0;
      sample_controller_input();
      sample_controller_input();
      _DAT_00ffdfe6 = 0;
      _DAT_00ffdfe8 = 0;
      do {
        do {
          wait_for_vblank();
          if (_DAT_00ffdf3e != 0) {
            DAT_00ffe172 = 0xff;
            _DAT_00ffdf3e = 0;
            prepare_mode_entry_service();
            if (DAT_00ffe186 == '\0') {
              bVar21 = false;
              if (DAT_00ffdf40 != 0) goto LAB_00000a72;
              toggle_field_scene_variant();
              if (!bVar21) {
                run_field_exit_orchestrator();
                DAT_00ffe18b = '\0';
                if (unaff_D6w != 0) {
                  _DAT_00ff012e = 0x8000;
                  _DAT_00ff0134 = 0x8000;
                  DAT_00ff022e = 0x8000;
                  _DAT_00ff0234 = 0x8000;
                  _DAT_00ff032e = 0x8000;
                  _DAT_00ff0334 = 0x8000;
                  _DAT_00ffdfe8 = 0xffff;
                  snapshot_entity_anchors();
                  goto LAB_00000b9c;
                }
                if ((DAT_00ffddc4 != 4) || (DAT_00fffbf9 != 6)) {
                  if ((_DAT_00fffc0e == 0) || (DAT_00fffbf9 < 8)) goto LAB_00000a32;
                  advance_field_mode_transition();
                }
              }
            }
            else {
              DAT_00ffe186 = '\x02';
LAB_00000a32:
              initialize_world_activity_mode();
              if (DAT_00ffddc4 == 0x37) {
                _DAT_00ffdd2c = &UNK_00020938;
                begin_world_activity();
              }
            }
            _DAT_00ffdfe8 = 0xffff;
            _DAT_00ffdf8e = 0xffff;
            goto LAB_00000b9c;
          }
          if (_DAT_00ffdf42 != 0) {
LAB_00000a72:
            if (DAT_00ffe18f != '\0') {
              DAT_00ffe18f = '\0';
              _DAT_00fffc1a = _DAT_00fffc1a | 0x8000;
            }
            _DAT_00ffdfe8 = 0xffff;
            handle_world_activity_request();
            initialize_field_scene_state();
            _DAT_00ffdf42 = 0;
            goto LAB_000008c0;
          }
          latch_controller_events();
          sample_controller_input();
          service_entity_behavior_frames();
          update_field_camera();
          update_entity_render_pass();
          advance_palette_fade();
          advance_world_frame_logic();
          DAT_00ffef14 = DAT_00ffef14 | 0xe;
        } while (((DAT_00ffe18b != '\0') || (DAT_00ffe166 != 0)) || (_DAT_00ffdfe8 != 0));
        if ((DAT_00ffdd22 & 0x2000) != 0) {
          handle_mode_action_event();
        }
        if ((DAT_00ffdd22 & 0x8000) != 0) {
          run_fixed_audio_transaction_wrapper();
          _DAT_00ffdfe8 = 0xffff;
          run_command_overview();
          service_audio_tick_wrapper();
          select_next_entity();
          if (_DAT_00ffe108 == 0) goto LAB_00000b9c;
          initialize_joshua_archetype_selection();
          goto LAB_000007f0;
        }
      } while (DAT_00ffe104 == (code *)0x0);
      _DAT_00ffdfe8 = 0xffff;
      (*DAT_00ffe104)();
      if (DAT_00ffe104 == (code *)&UNK_0005bac4) {
        DAT_00ffe104 = (code *)0x0;
        break;
      }
      DAT_00ffe104 = (code *)0x0;
LAB_00000b9c:
      advance_field_scene_transition();
    } while( true );
  }
  step_palette_transition();
  puVar7 = PTR_VDP_CTRL_000002da;
  puVar6 = PTR_VDP_DATA_000002d6;
  puVar5 = PTR_IO_Z80RES_000002d2;
  puVar4 = PTR_IO_Z80BUS_000002ce;
  sVar13 = DAT_000002c8;
  wVar3 = VDP_CTRL;
  wVar3 = VDP_CTRL;
  bVar9 = IO_PCBVER._1_1_;
  cVar1 = *(char *)((short)(ushort)(bVar9 >> 6) + 0x46c);
  if (cVar1 != '\0') {
    pcVar14 = GameHeader_00000100.Notes + 0x34;
    sVar8 = 0xf;
    do {
      if (cVar1 == *pcVar14) {
        uVar10 = (uint)DAT_000002c4;
        sVar8 = 0x17;
        puVar19 = (undefined4 *)&DAT_000002de;
        do {
          puVar18 = (undefined4 *)((int)puVar19 + 1);
          sVar12 = (short)CONCAT31((int3)(uVar10 >> 8),*(undefined1 *)puVar19);
          *(short *)puVar7 = sVar12;
          uVar10 = (uint)(ushort)(sVar13 + sVar12);
          sVar8 = sVar8 + -1;
          puVar19 = puVar18;
        } while (sVar8 != -1);
        *(undefined4 *)puVar7 = *puVar18;
        *(undefined2 *)puVar6 = 0;
        sVar8 = 0x3ffe;
        do {
          sVar8 = sVar8 + -1;
        } while (sVar8 != -1);
        *(short *)puVar4 = sVar13;
        *(short *)puVar5 = sVar13;
        *(undefined4 *)puVar7 = DAT_00000320;
        *(undefined4 *)puVar7 = DAT_00000324;
        sVar13 = 0x1f;
        do {
          *(undefined4 *)puVar6 = 0;
          sVar13 = sVar13 + -1;
        } while (sVar13 != -1);
        *(undefined4 *)puVar7 = DAT_00000328;
        sVar13 = 0x13;
        do {
          *(undefined4 *)puVar6 = 0;
          sVar13 = sVar13 + -1;
        } while (sVar13 != -1);
        sVar13 = 3;
        puVar20 = &DAT_0000032c;
        do {
          puVar6[0x11] = *puVar20;
          sVar13 = sVar13 + -1;
          puVar20 = puVar20 + 1;
        } while (sVar13 != -1);
        wait_for_vdp_ready();
        run_game_loop();
        return;
      }
      pcVar14 = pcVar14 + 1;
      sVar8 = sVar8 + -1;
    } while (sVar8 != -1);
    VDP_CTRL = 0x8164;
    VDP_CTRL = 0x8230;
    VDP_CTRL = 0x8c81;
    VDP_CTRL = 0x8f02;
    VDP_CTRL = 0x9001;
    Ram00c00004 = 0xc0020000;
    VDP_DATA = 0xeee;
    Ram00c00004 = 0x40000000;
    pbVar15 = &DAT_000004ee;
    sVar13 = 0x3a;
    uVar10 = 0x10000000;
    do {
      sVar8 = 7;
      pbVar16 = pbVar15;
      do {
        pbVar15 = pbVar16 + 1;
        bVar9 = *pbVar16;
        uVar11 = 0;
        sVar12 = 7;
        do {
          uVar10 = uVar10 << 4 | uVar10 >> 0x1c;
          bVar2 = bVar9 << 7;
          bVar9 = bVar2 | bVar9 >> 1;
          if (bVar2 != 0) {
            uVar11 = uVar10 | uVar11;
          }
          sVar12 = sVar12 + -1;
        } while (sVar12 != -1);
        Ram00c00000 = uVar11;
        sVar8 = sVar8 + -1;
        pbVar16 = pbVar15;
      } while (sVar8 != -1);
      sVar13 = sVar13 + -1;
    } while (sVar13 != -1);
    write_startup_text_to_vdp();
    for (pcVar14 = GameHeader_00000100.Notes + 0x34; *pcVar14 != ' '; pcVar14 = pcVar14 + 1) {
      for (puVar17 = &DAT_00000470; (char)*puVar17 != '\0'; puVar17 = puVar17 + 3) {
        if ((char)*puVar17 == *pcVar14) {
          if ((pcVar14[1] == ' ') && (pcVar14 != GameHeader_00000100.Notes + 0x34)) {
            write_startup_text_to_vdp();
          }
          write_startup_text_to_vdp();
          pcVar14 = extraout_A1;
          break;
        }
      }
    }
    write_startup_text_to_vdp();
  }
  do {
                    /* WARNING: Do nothing block with infinite loop */
  } while( true );
}

