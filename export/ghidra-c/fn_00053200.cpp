// Ghidra headless export
// Entry: 0x00053200
// Name: select_primary_runner_record_table
// Decompile status: source-shaped annotation
//
// Native conversion: ghidra_conversion::select_primary_runner_record_table.
// The source indexes the immutable compact runner-record table at 0x15DB48
// using the current scene/resource id and returns that entry's primary bank.
// The host returns a named extracted data alias instead of a ROM pointer.
//
// Calls recovered from direct call instructions: none.

RunnerRecordTableSelection select_primary_runner_record_table(std::uint8_t resource_id)
{
  return audio_resources[resource_id].primary;
}
