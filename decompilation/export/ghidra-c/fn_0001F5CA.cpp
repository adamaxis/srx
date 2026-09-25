// Ghidra headless export
// Entry: 0x0001F5CA
// Name: run_field_exit_orchestrator
// Decompile status: source-shaped annotation
//
// The native conversion is ghidra_conversion::run_field_exit_orchestrator.
// It preserves the source stage order and D6/FFF0CC abort boundaries while
// making each converted scene policy an explicit native callback.
//
// Calls recovered from direct call instructions:
//   0x0000C64C route_to_graphics_service
//   0x0001EEDC dispatch_world_effect
//   0x0001F65A select_field_exit_prompt
//   0x0001F830 prepare_field_mode_exit
//   0x0001FA78 select_field_mode_service
//   0x0001FD80 finalize_field_mode_exit
//   0x00020634 prepare_world_exit
//   0x00053A78 run_indexed_title_services

void run_field_exit_orchestrator(void)
{
  if ((DAT_00ff0110 & 0x40) != 0 || (DAT_00ff0110 & 0x10) != 0) {
    route_to_graphics_service();
    return;
  }

  prepare_world_exit();
  DAT_00fff10c = 0;
  DAT_00fff0cc = 0;

  dispatch_world_effect();
  if (D6 < 0 || (signed char)DAT_00fff0cc < 0) {
    DAT_00fff0cc = 0;
    return;
  }
  run_indexed_title_services();
  if (D6 < 0 || (signed char)DAT_00fff0cc < 0) {
    DAT_00fff0cc = 0;
    return;
  }

  const void (*const routes[])(void) = {
    select_field_exit_prompt,
    prepare_field_mode_exit,
    select_field_mode_service,
    finalize_field_mode_exit,
  };
  for (const auto route : routes) {
    route();
    if (D6 < 0) {
      DAT_00fff0cc = 0;
      return;
    }
  }
  DAT_00fff0cc = 0;
}
