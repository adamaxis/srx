#include "shadowrun_ghidra/rendering/wrapped_encoded_text_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {

namespace {

void call_once(const std::function<void()>& callback) {
    if (callback) {
        callback();
    }
}

} // namespace

std::optional<std::uint16_t> wrapped_encoded_text_glyph_index(
    const std::uint8_t source_byte) {
    if ((source_byte & 0x80U) != 0U || source_byte < 0x20U) {
        return std::nullopt;
    }

    auto index = static_cast<std::uint16_t>(source_byte - 0x20U);
    if (index >= 0x3DU) {
        index = static_cast<std::uint16_t>(index - 4U);
    }
    return index;
}

std::size_t render_wrapped_encoded_text(
    const std::vector<std::uint8_t>& encoded,
    const std::size_t line_width,
    const WrappedEncodedTextCallbacks& callbacks) {
    if (line_width == 0U || encoded.empty()) {
        return 0U;
    }

    std::size_t written = 0U;
    std::size_t source_offset = 0U;
    std::size_t line = 0U;
    while (source_offset < encoded.size()) {
        call_once(callbacks.begin_transfer);

        // ROM 0xC168 scans through the next source space and keeps the last
        // fitting count in D7.  A high-bit marker ends the scan.  Normal ROM
        // strings always provide a fitting space or marker; the bounded host
        // fallback prevents malformed extracted input from reproducing a DBF
        // underflow.
        const auto line_start = source_offset;
        std::size_t scan_offset = source_offset;
        std::size_t scanned = 0U;
        std::size_t last_fitting_space = 0U;
        while (scan_offset < encoded.size()) {
            const auto value = encoded[scan_offset++];
            ++scanned;
            if ((value & 0x80U) != 0U) {
                break;
            }
            if (value == 0x20U) {
                if (scanned < line_width) {
                    last_fitting_space = scanned;
                    continue;
                }
                break;
            }
        }

        std::size_t line_length = scanned;
        if (scanned > line_width && last_fitting_space != 0U) {
            line_length = last_fitting_space;
        } else if (scanned > line_width && last_fitting_space == 0U) {
            // The ROM data keeps words within the width.  If an external host
            // asset violates that contract, consume one bounded line instead
            // of reproducing an unbounded 68000 DBF.
            line_length = line_width;
        }
        line_length = std::min(line_length, encoded.size() - line_start);
        if (line_length == 0U) {
            break;
        }

        bool line_break = false;
        bool stream_terminated = false;
        for (std::size_t column = 0U;
             column < line_length && source_offset < encoded.size();
             ++column, ++source_offset) {
            const auto value = encoded[source_offset];
            if ((value & 0x80U) != 0U) {
                // The source consumes the marker while deciding whether the
                // next transfer begins on another line or returns.
                ++source_offset;
                line_break = (value & 0x40U) == 0U;
                stream_terminated = !line_break;
                break;
            }

            const auto index = wrapped_encoded_text_glyph_index(value);
            if (!index.has_value()) {
                continue;
            }
            const auto glyph = callbacks.glyph_for_index
                                   ? callbacks.glyph_for_index(*index)
                                   : *index;
            if (callbacks.write_glyph_at) {
                callbacks.write_glyph_at(line, column, glyph);
            } else if (callbacks.write_glyph) {
                callbacks.write_glyph(glyph);
            }
            ++written;
        }

        if (stream_terminated) {
            break;
        }

        ++line;
        if (source_offset >= encoded.size()) {
            break;
        }

        // A marker already advanced source_offset.  A wrapped space span did
        // not encounter one and naturally starts the next source line.
        if (line_break) {
            continue;
        }
    }
    return written;
}

std::size_t render_wrapped_text(
    const std::vector<std::uint8_t>& encoded,
    const std::size_t line_width,
    const WrappedEncodedTextCallbacks& callbacks) {
    return render_wrapped_encoded_text(encoded, line_width, callbacks);
}

} // namespace shadowrun::ghidra_conversion
