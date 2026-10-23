#!/usr/bin/env python3
"""Extract the two catalogued 0x5D1F6 story-page illustrations.

This is an offline asset importer. The host runtime consumes the named BMPs
and does not decode ROM streams, VRAM commands, or 0x1728 layout records.
"""

from __future__ import annotations

import argparse
import struct
from dataclasses import dataclass
from pathlib import Path


ROM_SCREEN_WIDTH = 320
ROM_SCREEN_HEIGHT = 224
PLANE_COLUMNS = 64
TILE_SIZE = 8
TILE_BYTES = 32
VDP_PATTERN_TARGET = 0x3020
SHARED_VDP_SETUP = 0x1FE638


@dataclass(frozen=True)
class StoryProfile:
    name: str
    profile_offset: int
    output_name: str


STORY_PROFILES = (
    StoryProfile(
        name="salish.sinsearach.frosty_harlequin_exposition",
        profile_offset=0x1FE690,
        output_name="salish-sinsearach-frosty-harlequin.bmp",
    ),
    StoryProfile(
        name="salish.spirit_eyes.ritual_of_binding",
        profile_offset=0x1FE6C2,
        output_name="salish-spirit-eyes-ritual.bmp",
    ),
)


class RomReader:
    def __init__(self, data: bytes) -> None:
        self.data = data

    def require(self, offset: int, size: int, role: str) -> None:
        if offset < 0 or size < 0 or offset + size > len(self.data):
            raise ValueError(f"{role} is outside the US ROM at 0x{offset:X}")

    def u8(self, offset: int, role: str) -> int:
        self.require(offset, 1, role)
        return self.data[offset]

    def u16(self, offset: int, role: str) -> int:
        self.require(offset, 2, role)
        return (self.data[offset] << 8) | self.data[offset + 1]

    def u32(self, offset: int, role: str) -> int:
        self.require(offset, 4, role)
        return (
            (self.data[offset] << 24)
            | (self.data[offset + 1] << 16)
            | (self.data[offset + 2] << 8)
            | self.data[offset + 3]
        )


def decode_1530_stream(rom: RomReader, source_offset: int) -> tuple[int, bytes]:
    """Expand the 0x1530 literal, repeated-byte, and ring-copy stream."""
    counter = rom.u16(source_offset, "0x1530 output counter")
    cursor = source_offset + 2
    remaining = counter
    ring = bytearray(0x800)
    ring_index = 0
    control = 0
    control_bits_remaining = 1
    output = bytearray()

    def read_byte(role: str) -> int:
        nonlocal cursor
        value = rom.u8(cursor, role)
        cursor += 1
        return value

    def emit(value: int) -> None:
        nonlocal ring_index, remaining
        value &= 0xFF
        ring[ring_index] = value
        ring_index = (ring_index + 1) & 0x07FF
        output.append(value)
        remaining = (remaining - 1) & 0xFFFF

    while True:
        control_bits_remaining -= 1
        if control_bits_remaining == 0:
            control = read_byte("0x1530 control byte")
            control_bits_remaining = 8
        compressed = (control & 0x80) != 0
        control = (control << 1) & 0xFF

        if not compressed:
            emit(read_byte("0x1530 literal"))
        else:
            descriptor = read_byte("0x1530 run descriptor")
            if (descriptor & 0x80) == 0:
                count = (descriptor & 0x7F) + 3
                value = read_byte("0x1530 repeated value")
                for _ in range(count):
                    emit(value)
            else:
                offset_low = read_byte("0x1530 ring offset")
                distance = (
                    (offset_low << 3) | ((descriptor >> 4) & 0x07)
                ) & 0x07FF
                count = (descriptor & 0x0F) + 3
                source_index = (ring_index - distance) & 0x07FF
                for _ in range(count):
                    emit(ring[source_index])
                    source_index = (source_index + 1) & 0x07FF

        if remaining == 0xFFFF:
            expected_size = counter + 1
            if len(output) != expected_size:
                raise ValueError(
                    f"0x1530 stream at 0x{source_offset:X} produced "
                    f"{len(output)} bytes; expected {expected_size}"
                )
            return counter, bytes(output)
        if len(output) > counter + 64:
            raise ValueError(
                f"0x1530 stream at 0x{source_offset:X} exceeded its counter"
            )


def upload_pattern_bytes(
    vram: bytearray, target_offset: int, counter: int, output: bytes
) -> None:
    """Apply 0x1530's paired-byte write order to a host-side extraction buffer."""
    if target_offset < 0 or target_offset + len(output) > len(vram):
        raise ValueError("0x1530 pattern upload exceeds the 64 KiB VDP address space")

    data_word = 0
    remaining = counter
    word_index = 0
    for value in output:
        data_word = (data_word & 0xFF00) | value
        if remaining & 1:
            data_word = (data_word << 8) & 0xFFFF
        else:
            destination = target_offset + word_index * 2
            vram[destination] = (data_word >> 8) & 0xFF
            vram[destination + 1] = data_word & 0xFF
            word_index += 1
        remaining = (remaining - 1) & 0xFFFF


def decode_1728_layout(
    rom: RomReader, source_offset: int, width: int, height: int, row_attribute: int
) -> tuple[list[int], int]:
    """Expand the source row layout into the profile's tile-word rectangle."""
    cursor = source_offset & ~1
    tile_base = rom.u16(cursor, "0x1728 tile base")
    cursor += 2
    cells: list[int] = []
    target_count = width * height

    def read_delta(word_sized: bool) -> int:
        nonlocal cursor
        if word_sized:
            value = rom.u16(cursor, "0x1728 word delta")
            cursor += 2
            return value
        value = rom.u8(cursor, "0x1728 byte delta")
        cursor += 1
        return value

    while len(cells) < target_count:
        descriptor = rom.u8(cursor, "0x1728 row descriptor")
        cursor += 1
        if descriptor == 0:
            break

        word_sized = (descriptor & 0x20) != 0
        encoded_count = descriptor & 0x1F
        if descriptor > 0xC0:
            # The ROM's BGT loop emits at least one tile, then exactly the
            # low-five-bit count for repeated-delta descriptors.
            for _ in range(max(1, encoded_count)):
                cells.append((tile_base + read_delta(word_sized) + row_attribute) & 0xFFFF)
                if len(cells) == target_count:
                    break
            continue

        # ROM 0x1728 selects -1 through 0x40, 0 through 0x80, then +1.
        step = -1 if descriptor <= 0x40 else 0 if descriptor <= 0x80 else 1
        tile_word = (tile_base + read_delta(word_sized) + row_attribute) & 0xFFFF
        for _ in range(encoded_count + 1):
            cells.append(tile_word)
            if len(cells) == target_count:
                break
            tile_word = (tile_word + step) & 0xFFFF

    cells.extend([row_attribute] * (target_count - len(cells)))
    return cells, cursor


def expand_cram_channel(value: int) -> int:
    return (value << 5) | (value << 2) | (value >> 1)


def read_palette_set(rom: RomReader, palette_offset: int) -> list[tuple[int, int, int]]:
    colors = []
    for index in range(64):
        value = rom.u16(palette_offset + index * 2, "story-page CRAM palette")
        colors.append(
            (
                expand_cram_channel((value >> 1) & 0x07),
                expand_cram_channel((value >> 5) & 0x07),
                expand_cram_channel((value >> 9) & 0x07),
            )
        )
    return colors


def render_story_art(
    vram: bytearray,
    palette: list[tuple[int, int, int]],
    cells: list[int],
    destination: int,
    width: int,
    height: int,
) -> tuple[bytearray, int, int]:
    pixels = bytearray(ROM_SCREEN_WIDTH * ROM_SCREEN_HEIGHT * 4)
    for pixel in range(ROM_SCREEN_WIDTH * ROM_SCREEN_HEIGHT):
        pixels[pixel * 4 : pixel * 4 + 4] = b"\0\0\0\xFF"

    plane_word = (destination & 0x3FFF) // 2
    origin_x = (plane_word % PLANE_COLUMNS) * TILE_SIZE
    origin_y = (plane_word // PLANE_COLUMNS) * TILE_SIZE

    for cell_index, word in enumerate(cells):
        tile_index = word & 0x07FF
        tile_offset = tile_index * TILE_BYTES
        if tile_offset + TILE_BYTES > len(vram):
            raise ValueError(f"0x1728 references tile {tile_index:#x} outside VRAM")
        flip_x = (word & 0x0800) != 0
        flip_y = (word & 0x1000) != 0
        palette_base = ((word >> 13) & 0x03) * 16
        cell_x = origin_x + (cell_index % width) * TILE_SIZE
        cell_y = origin_y + (cell_index // width) * TILE_SIZE

        for y in range(TILE_SIZE):
            for x in range(TILE_SIZE):
                source_x = 7 - x if flip_x else x
                source_y = 7 - y if flip_y else y
                packed = vram[tile_offset + source_y * 4 + source_x // 2]
                color_index = packed >> 4 if source_x % 2 == 0 else packed & 0x0F
                red, green, blue = palette[palette_base + color_index]
                screen_x, screen_y = cell_x + x, cell_y + y
                if screen_x >= ROM_SCREEN_WIDTH or screen_y >= ROM_SCREEN_HEIGHT:
                    continue
                pixel_offset = (screen_y * ROM_SCREEN_WIDTH + screen_x) * 4
                pixels[pixel_offset : pixel_offset + 4] = bytes(
                    (blue, green, red, 0xFF)
                )

    return pixels, origin_x, origin_y


def validate_layout_tile_range(
    cells: list[int], target_offset: int, uploaded_byte_count: int
) -> None:
    if target_offset % TILE_BYTES != 0 or uploaded_byte_count % TILE_BYTES != 0:
        raise ValueError("story-page pattern upload is not tile aligned")
    first_tile = target_offset // TILE_BYTES
    tile_limit = first_tile + uploaded_byte_count // TILE_BYTES
    referenced_tiles = {word & 0x07FF for word in cells}
    outside_upload = sorted(
        tile for tile in referenced_tiles
        if tile < first_tile or tile >= tile_limit
    )
    if outside_upload:
        sample = ", ".join(f"0x{tile:03X}" for tile in outside_upload[:8])
        raise ValueError(
            "0x1728 layout references tiles outside the profile upload: " +
            sample
        )


def write_bmp(path: Path, pixels: bytearray) -> None:
    row_bytes = ROM_SCREEN_WIDTH * 4
    image_bytes = bytearray()
    for y in range(ROM_SCREEN_HEIGHT - 1, -1, -1):
        start = y * row_bytes
        image_bytes.extend(pixels[start : start + row_bytes])

    dib_header = struct.pack(
        "<IiiHHIIiiII",
        40,
        ROM_SCREEN_WIDTH,
        ROM_SCREEN_HEIGHT,
        1,
        32,
        0,
        len(image_bytes),
        2835,
        2835,
        0,
        0,
    )
    pixel_offset = 14 + len(dib_header)
    file_header = struct.pack(
        "<2sIHHI",
        b"BM",
        pixel_offset + len(image_bytes),
        0,
        0,
        pixel_offset,
    )
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(file_header + dib_header + image_bytes)


def extract_profile(rom: RomReader, profile: StoryProfile, output_root: Path) -> None:
    offset = profile.profile_offset
    vdp_setup = rom.u32(offset, "story-page VDP setup pointer")
    palette_offset = rom.u32(offset + 4, "story-page CRAM pointer")
    mode_words = [rom.u16(offset + 8 + index * 2, "story-page mode word") for index in range(4)]
    pattern_pointer = rom.u32(offset + 0x10, "story-page pattern pointer")
    pattern_target = rom.u32(offset + 0x14, "story-page pattern target") & 0xFFFF
    pattern_auxiliary = rom.u32(offset + 0x18, "story-page pattern auxiliary word") & 0xFFFF
    pattern_terminator = rom.u32(offset + 0x1C, "story-page pattern-list terminator")
    layout_pointer = rom.u32(offset + 0x20, "story-page layout pointer")
    layout_destination = rom.u32(offset + 0x24, "story-page layout destination") & 0xFFFF
    width = rom.u16(offset + 0x28, "story-page layout width") + 1
    height = rom.u16(offset + 0x2A, "story-page layout height") + 1
    layout_attribute = rom.u16(offset + 0x2C, "story-page layout attribute")
    layout_terminator = rom.u32(offset + 0x2E, "story-page layout-list terminator")

    if vdp_setup != SHARED_VDP_SETUP:
        raise ValueError(f"unexpected VDP setup 0x{vdp_setup:X} at 0x{offset:X}")
    if any(mode_words):
        raise ValueError(f"unexpected story-page mode words at 0x{offset + 8:X}")
    if (pattern_pointer & 1) == 0 or (layout_pointer & 1) == 0:
        raise ValueError(f"story-page profile 0x{offset:X} has an untagged stream")
    if pattern_target != VDP_PATTERN_TARGET or pattern_auxiliary != 0:
        raise ValueError(f"unexpected pattern transfer in profile 0x{offset:X}")
    if pattern_terminator != 0 or layout_terminator != 0:
        raise ValueError(f"unexpected resource-list tail in profile 0x{offset:X}")
    if width > 40 or height > 28 or layout_attribute != 0:
        raise ValueError(f"unexpected 0x1728 dimensions/attributes in profile 0x{offset:X}")

    pattern_source = pattern_pointer & ~1
    counter, pattern_bytes = decode_1530_stream(rom, pattern_source)
    vram = bytearray(0x10000)
    upload_pattern_bytes(vram, pattern_target, counter, pattern_bytes)
    cells, stream_end = decode_1728_layout(
        rom,
        layout_pointer,
        width,
        height,
        layout_attribute,
    )
    validate_layout_tile_range(cells, pattern_target, len(pattern_bytes))
    palette = read_palette_set(rom, palette_offset)
    pixels, origin_x, origin_y = render_story_art(
        vram,
        palette,
        cells,
        layout_destination,
        width,
        height,
    )

    output_path = output_root / profile.output_name
    write_bmp(output_path, pixels)
    print(
        f"{profile.name}: pattern=0x{pattern_source:06X} "
        f"({len(pattern_bytes)} bytes to 0x{pattern_target:04X}), "
        f"layout=0x{layout_pointer & ~1:06X} "
        f"({width}x{height} tiles at {origin_x},{origin_y}), "
        f"palette=0x{palette_offset:06X}, text-ink={palette[48]}, "
        f"end=0x{stream_end:06X}, "
        f"output={output_path}"
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rom", type=Path, help="US Genesis ROM")
    parser.add_argument("output_directory", type=Path, help="named host asset directory")
    args = parser.parse_args()
    rom = RomReader(args.rom.read_bytes())
    for profile in STORY_PROFILES:
        extract_profile(rom, profile, args.output_directory)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
