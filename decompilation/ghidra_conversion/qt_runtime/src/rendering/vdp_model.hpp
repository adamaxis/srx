#pragma once

#include "vdp_frame_latch.hpp"

#include <QImage>

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>

class QPainter;

namespace shadowrun::ghidra_conversion::qt_runtime {

enum class VdpPlane : std::uint8_t {
    plane_a,
    plane_b,
    window,
};

struct VdpNameTableEntry {
    bool priority = false;
    std::uint8_t palette = 0;
    bool vertical_flip = false;
    bool horizontal_flip = false;
    std::uint16_t tile_index = 0;

    static VdpNameTableEntry decode(std::uint16_t word) noexcept;
    std::uint16_t encode() const noexcept;
};

// A typed host representation of the Genesis field VDP's indexed tile state.
// ROM provenance: 0x2C46 programs 64x32 planes and H40 output; name-word
// attributes are decoded from the VDP's documented 0x800-word format [C].
class VdpModel final {
public:
    static constexpr std::size_t kPatternCount = 0x800U;
    static constexpr std::size_t kPatternPixelCount = 8U * 8U;
    static constexpr std::size_t kNameTableWidth = 64U;
    static constexpr std::size_t kNameTableHeight = 32U;
    static constexpr std::size_t kNameTableWordCount =
        kNameTableWidth * kNameTableHeight;
    static constexpr std::size_t kCramPaletteCount = 4U;
    static constexpr std::size_t kCramColorsPerPalette = 16U;
    static constexpr std::size_t kCramWordCount =
        kCramPaletteCount * kCramColorsPerPalette;
    static constexpr int kDisplayWidth = 320;
    static constexpr int kDisplayHeight = 224;

    // Expanded 8x8 pixel indices; every byte is constrained to a 4bpp nibble.
    using Pattern = std::array<std::uint8_t, kPatternPixelCount>;
    using NameTable = std::array<std::uint16_t, kNameTableWordCount>;
    using SpritePaintCallback = std::function<void(QPainter&)>;

    void clear() noexcept;

    bool set_pattern(std::uint16_t tile_index, const Pattern& pattern) noexcept;
    const Pattern& pattern(std::uint16_t tile_index) const noexcept;

    bool set_cram_word(std::uint8_t palette, std::uint8_t color,
                       std::uint16_t word) noexcept;
    std::uint16_t cram_word(std::uint8_t palette,
                            std::uint8_t color) const noexcept;

    void set_name_word(VdpPlane plane, std::size_t x, std::size_t y,
                       std::uint16_t word) noexcept;
    std::uint16_t name_word(VdpPlane plane, std::size_t x,
                            std::size_t y) const noexcept;
    NameTable& name_table(VdpPlane plane) noexcept;
    const NameTable& name_table(VdpPlane plane) const noexcept;

    void set_scroll(const VdpScrollState& scroll) noexcept;
    const VdpScrollState& scroll() const noexcept { return scroll_; }

    // FFDD0E is plane B's vertical-scroll word. ROM 0x1AB0 copies FFE00A to
    // it; 0x26E2 increments it and sets FFEF14 bit 3 [C].
    void set_plane_b_vertical_scroll_offset(std::uint16_t ffdd0e) noexcept;

    void set_backdrop_cram_index(std::uint8_t index) noexcept;

    // The optional frame supplies the coherent VBlank-latched scroll used by
    // the already-modeled SAT/scroll latch. The caller retains sprite policy;
    // callbacks paint the low and high sprite passes at their VDP positions.
    void compose(QImage& output,
                 const SpritePaintCallback& low_sprites = {},
                 const SpritePaintCallback& high_sprites = {},
                 const VdpSatScrollFrame* latched_frame = nullptr) const;

private:
    std::array<Pattern, kPatternCount> patterns_{};
    std::array<std::uint16_t, kCramWordCount> cram_{};
    NameTable plane_a_{};
    NameTable plane_b_{};
    NameTable window_{};
    VdpScrollState scroll_{};
    std::uint8_t backdrop_cram_index_ = 0;

    void paint_plane(QImage& output, VdpPlane plane,
                     const VdpScrollState& scroll,
                     int priority_pass) const;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
