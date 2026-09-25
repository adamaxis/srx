// Ghidra headless export
// Entry: 0x00053200
// Name: select_audio_primary_stream
// Decompile status: source-shaped annotation
//
// Native conversion: ghidra_conversion::select_audio_primary_stream.
// The source indexes the immutable audio-resource table at 0x15DB48 using
// the current resource id and returns that entry's primary stream. The host
// returns a named Windows-playable asset alias instead of a ROM pointer.
//
// Calls recovered from direct call instructions: none.

AudioStreamSelection select_audio_primary_stream(std::uint8_t resource_id)
{
  return audio_resources[resource_id].primary;
}
