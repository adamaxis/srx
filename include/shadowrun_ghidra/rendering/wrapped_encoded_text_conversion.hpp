#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct WrappedEncodedTextCallbacks {
    std::function<void()> begin_transfer;
    // The ROM subtracts 0x20 from a source byte, skips four unused glyph
    // entries after index 0x3C, then indexes its glyph table.  The host
    // callback receives that typed table index rather than a raw ROM byte.
    std::function<std::uint16_t(std::uint16_t)> glyph_for_index;
    std::function<void(std::uint16_t)> write_glyph;
    // Optional host presentation boundary retaining the source line/column
    // that the Genesis transfer address represented.
    std::function<void(std::size_t line, std::size_t column,
                       std::uint16_t)> write_glyph_at;
};

// ROM 0xC168 glyph-table index: printable source bytes begin at 0x20 and the
// source table omits four entries at the 0x3D boundary.  High-bit values are
// line/stream markers, not glyphs, and malformed bytes below the printable
// range are rejected by the host conversion instead of indexing out of range.
std::optional<std::uint16_t> wrapped_encoded_text_glyph_index(
    std::uint8_t source_byte);

// ROM 0xC168: render the selected normalized text buffer into successive
// destination lines.  0x80-0xBF ends the current line; 0xC0-0xFF ends the
// stream.  Wrapping chooses the last source space that fits the requested
// width, matching the source scan rather than hard-wrapping bytes.
std::size_t render_wrapped_encoded_text(
    const std::vector<std::uint8_t>& encoded,
    std::size_t line_width,
    const WrappedEncodedTextCallbacks& callbacks);

// ROM 0xC160: host-facing compatibility boundary for callers that selected
// the normal text buffer.  The source wrapper sets FFF0B2 before falling
// through to 0xC168; native callers select their typed buffer before calling
// the shared renderer, so this function deliberately has no global flag.
std::size_t render_wrapped_text(
    const std::vector<std::uint8_t>& encoded,
    std::size_t line_width,
    const WrappedEncodedTextCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
