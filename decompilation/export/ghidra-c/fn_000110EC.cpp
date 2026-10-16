// Ghidra headless export
// Entry: 0x000110EC
// Name: draw_active_runner_hud_panel
// Decompile status: ok
//
// Behavior annotation: the active runner/entity record is in A0. The source
// reads +0x3E, subtracts one, and selects tile origin -0x796C for values <=0,
// -0x7956 for value 1, and -0x7940 for values >=2. It reads +0x2C into D2,
// sets D7 and D1 to 3, and submits a 4x4 HUD panel through 0x03648. The
// native conversion represents those inputs as ActiveRunnerHudPanelInput and
// produces an ActiveRunnerHudPanelPlan.
// Calls recovered from direct call instructions:
//   0x00003648 draw_hud_panel


void draw_active_runner_hud_panel(void)

{
  draw_hud_panel();
  return;
}

