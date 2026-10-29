#!/usr/bin/env python3
"""Emit weapon Lua rows from the US Genesis ROM weapon tables."""

from __future__ import annotations

import argparse
import struct
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[1]
ROM_DEFAULT = PROJECT_ROOT.parent / "input" / "shadowrun_genesis_us.bin"
WEAPON_IDS = (
    "streetline", "model_101t", "american_l36", "security_500",
    "warhawk", "max_power", "predator", "ak_97", "hk227_s", "mach_22",
    "allegiance", "roomsweeper", "frag", "scatter", "concussion",
)
PISTOL_ACCEPTS = (
    "{ [Accessory.SMARTGUN] = true, [Accessory.SILENCER] = true, "
    "[Accessory.LASER_SIGHT] = true }"
)
OTHER_ACCEPTS = (
    "{ [Accessory.SMARTGUN] = true, [Accessory.SOUND_SUPPRESSOR] = true, "
    "[Accessory.LASER_SIGHT] = true, [Accessory.GAS_VENT_2] = true, "
    "[Accessory.GAS_VENT_3] = true }"
)


def read_u16(data: bytes, address: int) -> int:
    return struct.unpack_from(">H", data, address)[0]


def source_text(data: bytes, address: int) -> str:
    raw = data[address:address + 0x1A].split(b"\xff")[0]
    return " ".join(raw.replace(b"\x80", b" ").decode("latin1").split())


def signed_byte(value: int) -> int:
    return value - 256 if value >= 128 else value


def weapon_rows(data: bytes) -> str:
    fire_sounds = struct.unpack_from(">15H", data, 0x15318)
    silenced_sounds = struct.unpack_from(">15H", data, 0x15336)
    rows = []
    sound_names = {
        0x1C: "SHOT_LIGHT_PISTOL",
        0x51: "SHOT_HEAVY_PISTOL",
        0x1E: "SHOT_SMG_OR_THROWN",
        0x1D: "SHOT_SHOTGUN",
    }
    silenced_sound_names = {
        0x3F: "SHOT_SILENCED",
        0x4A: "SHOT_SILENCED_SMG_OR_THROWN",
    }
    for code, item_id in enumerate(WEAPON_IDS, start=1):
        base = 0xD4540 + 0x30 * (code - 1)
        weapon_class = (
            "THROWN" if code >= 13 else
            "SMG" if 8 <= code <= 10 else
            "SHOTGUN" if code >= 11 else
            "HEAVY_PISTOL" if code >= 5 else "LIGHT_PISTOL"
        )
        automatic = 8 <= code <= 10
        accepts = PISTOL_ACCEPTS if code < 8 else OTHER_ACCEPTS
        damage_selector = data[base + 0x25]
        damage_effect = -signed_byte(data[0x166AD4 + damage_selector])
        source_action = (
            f'        source_action = {{ effect_value = {damage_effect}, '
            f'difficulty = {data[base + 0x26]} }},\n'
            if code <= 12 else ""
        )
        rows.append(
            f'    {item_id} = Weapon{{\n'
            f'        id = "{item_id}", name = "{source_text(data, base)}", '
            f'class = WeaponClass.{weapon_class},\n'
            f'        purchase_base = {read_u16(data, base + 0x22)}, '
            f'magazine_capacity = {data[base + 0x24]}, '
            f'damage_effect = {damage_effect}, '
            f'power_rating = {data[base + 0x26]},\n'
            f'{source_action}'
            f'        fire_sound = SoundId.{sound_names[fire_sounds[code - 1]]}, '
            f'silenced_sound = SoundId.{silenced_sound_names[silenced_sounds[code - 1]]},\n'
            f'        automatic = {str(automatic).lower()}, '
            f'rounds_per_shot = {3 if automatic else 1},\n'
            f'        accepts = {accepts},\n'
            f'    }},'
        )
    return "\n".join(rows)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--rom", type=Path, default=ROM_DEFAULT)
    args = parser.parse_args()
    data = args.rom.read_bytes()
    if len(data) != 0x200000:
        raise SystemExit(f"expected a 2 MiB US Genesis ROM, got {len(data)} bytes")
    print(weapon_rows(data))


if __name__ == "__main__":
    main()
