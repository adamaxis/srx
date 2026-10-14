# Interactable testing methodology

An interactable is not covered by proving that its first screen appears. A
source-backed interactable must be tested as a stateful contract: input edges,
selection state, derived values, mutation rules, compositing, and re-entry all
need explicit checks. This applies to inventory cells, command pages, dialogue
choices, clips, cyberware, field actions, terminals, and world objects.

## Required test stages

| Stage | What must be itemized | Minimum evidence |
|---|---|---|
| Source contract | ROM entry, typed state fields, valid ranges, draw path, exit path, and unresolved indirect calls | Source trace plus named conversion function |
| Entry | Fresh state, initial cursor/selection, available assets, and the first frame before input | State assertion and 320x224 dump |
| Selection | Every valid direction/action, bounds, clamp or wrap rule, disabled/empty entries, and simultaneous-input priority | Per-edge state assertions and selected-focus pixel check |
| Re-selection | Close or confirm, return to the parent, then select the same and a different item again | Re-entry sequence with no stale cursor or text |
| Un-selection | B/Cancel/Start, prompt No, and any source backout edge | Parent remains usable; no unintended mutation |
| Dispatch | Each source action branch, including protected/rejected branches and confirmation gates | Typed event/branch assertion; explicit boundary for anything not converted |
| Mutation | Possession, equipment, phase/ammunition, health/status, recipient, and flags before/after | Before/after invariant checks against the converted function |
| Derived math | Values that control bars, meters, costs, quantities, difficulty, damage, or animation cadence | Source-derived numeric result plus a visual asset/layout assertion using that result |
| Presentation | Layer order, coordinates, sprite opacity, text rows, clear behavior, and changed pixels when state changes | Region-level pixel checks and named frame artifacts |
| Repeatability | Open/close/reopen across multiple cycles and after a mutation | Repeated executable walk, not just a unit test |
| Negative paths | Empty cell, invalid item, full recipient, protected item, failed roll, and unresolved source branch | Failure/result text or explicit source boundary; no invented success |
| Independent review | Comparison against ROM trace and an independent Genesis capture/video | Separate review notes; same-source composites are regression checks only |

## Current inventory-action coverage

`shadowrun_ghidra_conversion_qt_field_action_qa` currently covers the
following sequence in the actual Qt executable:

1. Enter the command overview through the converted Start path and reach the
   source inventory column.
2. Open the four-choice row and verify the direct focus block at the source
   action-row coordinate.
3. Verify Left wraps Equip/Use to Cancel, Right wraps back, and the rendered
   focus block moves to the corresponding direct asset.
4. Press B to un-select without changing possession, then re-select the same
   cell and verify the same frame is restored.
5. Dispatch Equip/Unequip. The test predicts the typed mutation with the
   converted equipment function and compares the live runner record to it.
6. Traverse to the combat page and verify the typed attack/defense results
   select the exact direct bar assets at their source-derived positions.
7. Return to inventory, re-open the action row, enter Discard, verify No, toggle
   Yes, confirm, and verify the source cell is cleared and the subloop closes.

The resulting ten BMPs are executable regression artifacts outside the asset
tree. They validate state-to-presentation wiring and layout. They do not by
themselves prove Genesis visual fidelity; that still requires the ROM trace and
the independent longplay/Genesis visual source.

## Rule for new interactables

Do not add a new interactable implementation with only an initial smoke test.
Create its source trace, typed state contract, test matrix rows, repeatable CLI
walk, and named visual checkpoints together. If the required resource or
source branch cannot be located, stop at an explicit boundary and report the
missing conversion evidence instead of inventing a substitute behavior.
