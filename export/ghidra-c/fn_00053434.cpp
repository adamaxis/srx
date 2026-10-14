// Ghidra headless export
// Entry: 0x00053434
// Name: expand_runner_animation_grid
// Decompile status: source-shaped annotation
//
// Native conversion: ghidra_conversion::expand_runner_animation_grid.
// This routine is the recursive four-direction worker. It tests the source
// guard bit for each direction, rejects row-wrap/out-of-range cells, marks a
// newly accepted cell, and recurses with the reverse-direction guard bit.
// Source type-table membership is represented by an explicit native callback
// until each of the four ROM tables has a verified semantic name.
//
// Calls recovered from direct call instructions:
//   0x00053434 expand_runner_animation_grid (recursive, 4 sites)

void expand_runner_animation_grid(short current_cell,
                                  unsigned short recursion_guard)
{
  for (const auto& step : runner_animation_grid_directions) {
    if ((recursion_guard & step.blocked_bit) != 0) {
      continue;
    }
    const auto next_cell = current_cell + step.delta;
    if (!runner_animation_cell_is_in_bounds(current_cell, next_cell, step)) {
      continue;
    }
    const auto decision = classify_runner_animation_step(
        current_cell, next_cell, step.direction);
    if (decision == blocked) {
      continue;
    }
    mark_runner_animation_cell(next_cell);
    if (decision == recurse) {
      expand_runner_animation_grid(
          next_cell, recursion_guard | step.reverse_bit);
    }
  }
}
