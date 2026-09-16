#include "shadowrun/compression.hpp"

#include <stdexcept>

namespace shadowrun {
namespace {

class Reader {
public:
    Reader(const std::uint8_t* data, std::size_t size) : data_(data), size_(size) {}

    std::uint8_t read8() {
        if (offset_ >= size_) {
            throw std::runtime_error("packed stream ended while reading a byte");
        }
        return data_[offset_++];
    }

    std::uint16_t read16() {
        const auto high = read8();
        const auto low = read8();
        return static_cast<std::uint16_t>((static_cast<std::uint16_t>(high) << 8U) | low);
    }

    std::size_t consumed() const { return offset_; }

private:
    const std::uint8_t* data_;
    std::size_t size_;
    std::size_t offset_ = 0;
};

} // namespace

PackedDecodeResult decode_packed_stream(const std::uint8_t* packed,
                                       std::size_t packed_size) {
    if (packed == nullptr || packed_size < 2) {
        throw std::runtime_error("packed stream has no output-count header");
    }

    Reader reader(packed, packed_size);
    const auto output_size = static_cast<std::size_t>(reader.read16()) + 1U;
    std::vector<std::uint8_t> output;
    output.reserve(output_size);

    std::uint8_t flags = 0;
    unsigned flags_remaining = 0;
    while (output.size() < output_size) {
        if (flags_remaining == 0) {
            flags = reader.read8();
            flags_remaining = 8;
        }
        const bool encoded = (flags & 0x80U) != 0;
        flags = static_cast<std::uint8_t>(flags << 1U);
        --flags_remaining;

        if (!encoded) {
            output.push_back(reader.read8());
            continue;
        }

        const auto token = reader.read8();
        if ((token & 0x80U) == 0) {
            const auto length = static_cast<std::size_t>((token & 0x7FU) + 3U);
            const auto value = reader.read8();
            if (length > output_size - output.size()) {
                throw std::runtime_error("packed repeat exceeds output size");
            }
            output.insert(output.end(), length, value);
            continue;
        }

        const auto low_distance = reader.read8();
        const auto distance = static_cast<std::size_t>(
            (((static_cast<std::uint16_t>(low_distance) << 3U) |
              static_cast<std::uint16_t>((token >> 4U) & 0x07U))) & 0x07FFU);
        const auto length = static_cast<std::size_t>((token & 0x0FU) + 3U);
        if (distance == 0 || distance > output.size() ||
            length > output_size - output.size()) {
            throw std::runtime_error("packed back-reference is out of range");
        }
        for (std::size_t index = 0; index < length; ++index) {
            // Read the source index for each byte so overlap behaves exactly
            // like the ROM's advancing source and destination pointers.
            output.push_back(output[output.size() - distance]);
        }
    }

    return PackedDecodeResult{std::move(output), reader.consumed()};
}

} // namespace shadowrun
