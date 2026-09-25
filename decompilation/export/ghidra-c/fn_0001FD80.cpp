// Ghidra headless export
// Entry: 0x0001FD80
// Name: finalize_field_mode_exit
// Decompile status: source-shaped annotation
//
// Native conversion: ghidra_conversion::finalize_field_mode_exit.
// This is the final scene/submode policy reached by the field-exit
// orchestrator when the earlier specialized selectors did not claim the
// current route. The original A0/A5 pointers are host resource selections in
// the native conversion; they are not runtime ROM addresses.
//
// Calls recovered from direct call instructions:
//   0x00000D3A sample_source_random_bound
//   0x0001E1FC forward_effect_sequence
//   0x0001F246 run_mode_exit_services
//   0x00059BCC reset_runner_status_triplet

void finalize_field_mode_exit(void)
{
  // Scene 6/0 clears mode bit 9 before the service. Scene 7/0 performs the
  // first/pending/completed visit branch and may reset the three runner
  // status records. Scene 9/0 selects its visit message from world/status
  // flags. Each is a handled route.
  if ((DAT_00fffbf8 == 6) && (DAT_00fffbf9 == 0)) {
    DAT_00fffc30 &= ~0x00000200u;
    run_mode_exit_services();
    return;
  }
  if ((DAT_00fffbf8 == 7) && (DAT_00fffbf9 == 0)) {
    if ((DAT_00fffc1e & 0x01000000u) == 0) {
      DAT_00fffc1e |= 0x01000000u;
    } else if ((DAT_00fffc1e & 0x02000000u) != 0) {
      reset_runner_status_triplet();
    }
    run_mode_exit_services();
    return;
  }
  if ((DAT_00fffbf8 == 9) && (DAT_00fffbf9 == 0)) {
    run_mode_exit_services();
    return;
  }

  // The remaining routes select a named resource profile and a signed
  // alternate/default message. Default routes that set FFF0D6 to -1 retain
  // that status; all generic routes clear the alternate-result marker after
  // the shared service boundary.
  switch (DAT_00fffbf8) {
    case 0:
      // 1,2,4,5,6,7,8,9
      break;
    case 1:
      // 0,2,4,8,9,10
      break;
    case 2:
      // 0,3,4,5,7,8,9,10
      break;
    case 3:
      // 0,2,3,4,5,6,7,8,9
      break;
    case 4:
      // 1,2,5
      break;
    case 5:
      // 1,6
      break;
    default:
      return;
  }

  run_mode_exit_services();
  DAT_00fff0d5 = 0;
}
