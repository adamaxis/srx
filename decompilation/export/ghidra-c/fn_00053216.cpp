// Ghidra headless export
// Entry: 0x00053216
// Name: select_audio_secondary_stream
// Decompile status: source-shaped annotation
//
// Native conversion: ghidra_conversion::select_audio_secondary_stream.
// This is the paired secondary-stream lookup for the same immutable resource
// table. It is used when the caller copies the selected audio state; the host
// exposes the named asset directly rather than retaining a source address.
//
// Calls recovered from direct call instructions: none.

AudioStreamSelection select_audio_secondary_stream(std::uint8_t resource_id)
{
  return audio_resources[resource_id].secondary;
}
