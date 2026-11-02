#!/usr/bin/env python3
"""Check that the ROM asset importer emits the three live tail-fan phases."""

from __future__ import annotations

import pathlib
import json
import subprocess
import sys
import tempfile
import zlib


AGIRA_PURCHASE_TEXT = {
    "vid_phone.agira_tetsumi.little_chiba_offer": (
        'There is a place called Little Chiba in the Redmond Barrens.||'
        'They are under our "protection" and will give you extraordinary '
        "service at a discounted rate."
    ),
    "vid_phone.agira_tetsumi.combat_armor_offer": (
        "I also have acquired a supply of military-issue combat armor.||"
        "I spent a lot of time and money to get them, but I will sell you one "
        "for 30,000$."
    ),
    "vid_phone.agira_tetsumi.purchase_offer": (
        "Lastly, I have many powerful mages in my employ.||"
        "They have created an item that will increase the magical abilities "
        "of the one who holds it.||"
        "I will sell this power focus for 80,000$.||"
        "Do you have anything to say?"
    ),
    "vid_phone.agira_tetsumi.option.heavy_combat_armor":
        "I'd like to buy the Combat Armor.",
    "vid_phone.agira_tetsumi.option.power_focus_phase4":
        "I'd like to buy the Power Focus.",
    "vid_phone.agira_tetsumi.option.decline_purchase": "Neg on that, chum.",
    "vid_phone.agira_tetsumi.purchase_success": (
        "The item you have requested is on its way.||"
        "May the Yakuza guide you, Joshua-san."
    ),
    "vid_phone.agira_tetsumi.purchase_insufficient": (
        "You cannot buy what you desire with something you do not have.||"
        "The Yakuza will guide you, Joshua-san, but you must build your own "
        "empire."
    ),
    "vid_phone.agira_tetsumi.purchase_inventory_full": (
        "I am so sorry, Joshua-san, but this transaction must wait.||"
        "May the Yakuza guide you, Joshua-san."
    ),
    "vid_phone.agira_tetsumi.power_focus_ineligible": (
        "This little item isn't going to do you any good.||"
        "Maybe one of your magically inclined chums could call me back."
    ),
    "vid_phone.agira_tetsumi.purchase_declined":
        "May the Yakuza guide you, Joshua-san.",
}


def decode_rgba_png(path: pathlib.Path) -> tuple[int, int, bytes]:
    data = path.read_bytes()
    if not data.startswith(b"\x89PNG\r\n\x1a\n"):
        raise AssertionError(f"not a PNG: {path}")

    offset = 8
    width = height = bit_depth = color_type = interlace = None
    compressed = bytearray()
    while offset < len(data):
        length = int.from_bytes(data[offset : offset + 4], "big")
        kind = data[offset + 4 : offset + 8]
        payload = data[offset + 8 : offset + 8 + length]
        offset += length + 12
        if kind == b"IHDR":
            width = int.from_bytes(payload[0:4], "big")
            height = int.from_bytes(payload[4:8], "big")
            bit_depth, color_type = payload[8], payload[9]
            interlace = payload[12]
        elif kind == b"IDAT":
            compressed.extend(payload)
        elif kind == b"IEND":
            break

    if (width is None or height is None or bit_depth != 8 or color_type != 6 or
            interlace != 0):
        raise AssertionError(f"expected a non-interlaced RGBA8 PNG: {path}")

    raw = zlib.decompress(compressed)
    stride = width * 4
    if len(raw) != height * (stride + 1):
        raise AssertionError(f"invalid RGBA PNG data length: {path}")

    pixels = bytearray(height * stride)
    prior = bytearray(stride)
    source_offset = 0
    for row in range(height):
        filter_type = raw[source_offset]
        source_offset += 1
        encoded = raw[source_offset : source_offset + stride]
        source_offset += stride
        decoded = bytearray(stride)
        for index, byte in enumerate(encoded):
            left = decoded[index - 4] if index >= 4 else 0
            above = prior[index]
            upper_left = prior[index - 4] if index >= 4 else 0
            if filter_type == 0:
                predictor = 0
            elif filter_type == 1:
                predictor = left
            elif filter_type == 2:
                predictor = above
            elif filter_type == 3:
                predictor = (left + above) // 2
            elif filter_type == 4:
                estimate = left + above - upper_left
                left_distance = abs(estimate - left)
                above_distance = abs(estimate - above)
                corner_distance = abs(estimate - upper_left)
                if left_distance <= above_distance and left_distance <= corner_distance:
                    predictor = left
                elif above_distance <= corner_distance:
                    predictor = above
                else:
                    predictor = upper_left
            else:
                raise AssertionError(f"unsupported PNG row filter {filter_type}: {path}")
            decoded[index] = (byte + predictor) & 0xFF
        pixels[row * stride : (row + 1) * stride] = decoded
        prior = decoded
    return width, height, bytes(pixels)


def main() -> int:
    if len(sys.argv) != 4:
        raise SystemExit(
            "usage: graphics_extract_aircraft_tail_rotor_test.py "
            "<extractor> <rom> <assets>"
        )
    extractor, rom, assets = map(pathlib.Path, sys.argv[1:])
    phase_rows = (
        ("intro.aircraft.tail_rotor.phase.0", "intro/aircraft-tail-rotor-phase-0.png"),
        ("intro.aircraft.tail_rotor.phase.1", "intro/aircraft-tail-rotor-phase-1.png"),
        ("intro.aircraft.tail_rotor.phase.2", "intro/aircraft-tail-rotor-phase-2.png"),
    )

    with tempfile.TemporaryDirectory(prefix="srx-tail-rotor-extract-") as temporary:
        output_root = pathlib.Path(temporary)
        completed = subprocess.run(
            [str(extractor), str(rom), str(output_root), "--all-assets"],
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        if completed.returncode != 0:
            raise AssertionError(
                "asset extraction failed:\n"
                f"stdout:\n{completed.stdout}\nstderr:\n{completed.stderr}"
            )

        manifest_lines = (output_root / "resources.tsv").read_text().splitlines()
        manifest = {
            fields[0]: fields
            for line in manifest_lines[1:]
            if (fields := line.split("\t")) and len(fields) == 5
        }
        for alias, relative in phase_rows:
            generated = output_root / relative
            expected = assets / relative
            if not generated.is_file():
                raise AssertionError(f"extractor omitted {relative}")
            if alias not in manifest or manifest[alias] != [alias, relative, "image", "0", "0"]:
                raise AssertionError(f"runtime resource alias is missing or wrong: {alias}")
            generated_pixels = decode_rgba_png(generated)
            expected_pixels = decode_rgba_png(expected)
            if generated_pixels != expected_pixels:
                raise AssertionError(f"ROM extraction differs from committed pixels: {relative}")

        generated_text_path = output_root / "dialogue" / "script-text.json"
        generated_text = json.loads(generated_text_path.read_text())["texts"]
        for key, expected in AGIRA_PURCHASE_TEXT.items():
            if generated_text.get(key) != expected:
                raise AssertionError(
                    f"extractor dialogue text mismatch for {key}: "
                    f"{generated_text.get(key)!r}"
                )

    print("graphics_extract_aircraft_tail_rotor: 3 ROM phases pixel-identical")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
