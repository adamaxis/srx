// Ghidra headless export
// Entry: 0x0005794E
// Name: apply_selected_field_action
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D94 wait_for_controller_event
//   0x0000C0DA render_encoded_title_text
//   0x00057A44 calculate_field_action_record_cost
//   0x00057A6E apply_field_action_cost_discounts
//   0x00057B8C run_field_action_quantity_chooser
//   0x00057C5E refresh_field_action_detail_panel
//
// A0 is the selected action-record table base and D0 is the source record
// group. The routine selects group*0x18 + primary_cursor*3 before applying
// one record to the active runner.

void apply_selected_field_action(void)
{
  A0 += (D0 & 0xffff) * 0x18;
  A0 += (uint)DAT_00fff0d0 * 3;
  if ((char)*A0 < 0) {
    DAT_00fff112 = 0xff;
    return;
  }

  refresh_field_action_detail_panel();
  const bool special_action = *A0 == 'E';
  short inventory_slot = 0;
  if (!special_action) {
    while (inventory_slot < 8 &&
           *(char *)(DAT_00fffc04 + 0x66 + inventory_slot) != 0) {
      ++inventory_slot;
    }
    if (inventory_slot == 8) {
      render_encoded_title_text(); // no empty runner action slot
      wait_for_controller_event();
      return;
    }
  }

  if (special_action) {
    D1 = (uint)(unsigned char)A0[2];
    apply_field_action_cost_discounts();
  }
  else {
    calculate_field_action_record_cost();
  }

  if (DAT_00fffbfe < D1) {
    render_encoded_title_text(); // insufficient funds
    wait_for_controller_event();
    return;
  }

  run_field_action_quantity_chooser();
  if (DAT_00fff0dc != 0) {
    return; // cancel path leaves runner state and funds untouched
  }

  if (special_action) {
    ++*(char *)(DAT_00fffc04 + 0x4f);
    if (*(char *)(DAT_00fffc04 + 0x4f) >= 0x15) {
      *(char *)(DAT_00fffc04 + 0x4f) = 0x14;
      refresh_field_action_detail_panel();
      render_encoded_title_text(); // special-action capacity reached
      return;
    }
  }
  else {
    // The source writes the selected marker and immediately increments the
    // stored byte; the companion record byte is copied without modification.
    *(char *)(DAT_00fffc04 + 0x66 + inventory_slot) = *A0 + 1;
    *(char *)(DAT_00fffc04 + 0x6e + inventory_slot) = A0[1];
  }
  DAT_00fffbfe -= D1;
}
