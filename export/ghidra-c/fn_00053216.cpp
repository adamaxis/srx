// Ghidra headless export
// Entry: 0x00053216
// Name: select_secondary_runner_record_table
// Decompile status: source-shaped annotation
//
// Native conversion: ghidra_conversion::select_secondary_runner_record_table.
// This is the paired secondary-bank lookup for the same immutable compact
// runner-record table. It is used when the caller copies the selected record
// state; the host exposes the named data asset directly.
//
// Calls recovered from direct call instructions: none.

RunnerRecordTableSelection select_secondary_runner_record_table(std::uint8_t resource_id)
{
  return audio_resources[resource_id].secondary;
}
