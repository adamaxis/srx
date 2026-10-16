#!/usr/bin/env python3
"""Emit weapon Lua rows from the US Genesis ROM weapon tables."""

from __future__ import annotations

import argparse
import struct
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[1]
ROM_DEFAULT = PROJECT_ROOT.parent / "input" / "shadowrun_genesis_us.bin"
WEAPON_IDS = (
    "STREETLINE_SPECIAL", "MODEL_101T", "AMERICAN_L36", "SECURITY_500",
    "WARHAWK", "MAX_POWER", "PREDATOR", "AK97", "HK227S", "MACH22",
    "ALLEGIANCE", "ROOMSWEEPER", "FRAG_GRENADE", "SCATTER_GRENADE",
    "CONCUSSION_GRENADE",
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


def weapon_rows(data: bytes) -> str:
    fire_sounds = struct.unpack_from(">15H", data, 0x15318)
    silenced_sounds = struct.unpack_from(">15H", data, 0x15336)
    rows = []
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
        rows.append(
            f'    [ItemId.{item_id}] = Weapon{{\n'
            f'        id = ItemId.{item_id}, name = "{source_text(data, base)}", '
            f'class = WeaponClass.{weapon_class},\n'
            f'        price = {read_u16(data, base + 0x22)}, '
            f'clip_size = {data[base + 0x24]}, '
            f'damage_class = {data[base + 0x25]}, '
            f'power = {data[base + 0x26]}, band = {data[base + 0x27]},\n'
            f'        fire_sound = 0x{fire_sounds[code - 1]:02X}, '
            f'silenced_sound = 0x{silenced_sounds[code - 1]:02X},\n'
            f'        automatic = {str(automatic).lower()}, '
            f'rounds_per_shot = {3 if automatic else 1},\n'
            f'        reload_equip_mode = {2 if code in (5, 11, 12) else 0},\n'
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
