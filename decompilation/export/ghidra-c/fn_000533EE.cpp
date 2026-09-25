// Ghidra headless export
// Entry: 0x000533EE
// Name: initialize_runner_animation_grid
// Decompile status: source-shaped annotation
//
// Native conversion: ghidra_conversion::initialize_runner_animation_grid.
// The selected compact record contributes its +3/+4 grid coordinates. The
// source multiplies the row by the fixed 14-cell width, marks the starting
// cell in the 140-byte working map, and enters the guarded neighbor walker.
// The runner-record bank and transition predicate are host-owned boundaries.
//
// Calls recovered from direct call instructions:
//   0x00053200 select_primary_runner_record_table

void initialize_runner_animation_grid(void)
{
  select_primary_runner_record_table();
  const auto position = resolve_selected_runner_position();
  const auto start_cell = position.y * 14 + position.x;
  mark_runner_animation_cell(start_cell);
  expand_runner_animation_grid(start_cell, 0);
}
