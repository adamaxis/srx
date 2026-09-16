#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace shadowrun {

struct PackedDecodeResult {
    std::vector<std::uint8_t> bytes;
    std::size_t consumed = 0;
};

// ROM 0x1610. The stream starts with a big-endian word containing
// (output_size - 1), followed by MSB-first flag bits:
//
//   flag 0: one literal byte
//   flag 1, token bit 7 clear: repeat the following byte
//   flag 1, token bit 7 set: copy an overlapping back-reference
//
// Token lengths are encoded with the same DBF convention as the original
// routine, hence the +3 below. No graphics assumptions are made here; callers
// can use the decoded bytes for tiles, maps, or text after identifying a ROM
// source table.
PackedDecodeResult decode_packed_stream(const std::uint8_t* packed,
                                       std::size_t packed_size);

} // namespace shadowrun
