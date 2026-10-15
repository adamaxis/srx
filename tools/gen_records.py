#!/usr/bin/env python3
"""Emit item, spell, or accessory Lua rows from the US Genesis ROM."""

from __future__ import annotations

import argparse
import csv
import struct
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
ROM_DEFAULT = ROOT / "input" / "shadowrun_genesis_us.bin"
KIND_MAP_DEFAULT = ROOT / "docs" / "ghidra" / "source-records" / "items_full.tsv"


def read_u16(data: bytes, address: int) -> int:
    return struct.unpack_from(">H", data, address)[0]


def read_u32(data: bytes, address: int) -> int:
    return struct.unpack_from(">I", data, address)[0]


def source_text(raw: bytes) -> str:
    return " ".join(raw.split(b"\xff")[0].replace(b"\x80", b" ").decode("latin1").split())


def load_kind_map(path: Path) -> dict[int, str]:
    with path.open(encoding="utf-8", newline="") as source:
        return {
            int(row["code"], 16): row["kind_repo"]
            for row in csv.DictReader(source, delimiter="\t")
            if row.get("kind_repo")
        }


def item_rows(data: bytes, kinds: dict[int, str]) -> str:
    rows = []
    for code in range(1, 0x33):
        base = 0xD4540 + 0x30 * (code - 1)
        rows.append(
            f'    [0x{code:02X}] = Item{{ id = 0x{code:02X}, '
            f'name = "{source_text(data[base:base + 0x1A])}", '
            f'kind = ItemKind.{kinds[code].upper()}, '
            f'price = {read_u16(data, base + 0x22)}, '
            f'b24 = {data[base + 0x24]}, b25 = {data[base + 0x25]}, '
            f'b26 = {data[base + 0x26]}, b27 = {data[base + 0x27]}, '
            f'icon = 0x{read_u32(data, base + 0x1A):06X}, '
            f'detail = 0x{read_u32(data, base + 0x1E):08X} }},'
        )
    return "\n".join(rows)


def spell_rows(data: bytes) -> str:
    categories = data[0xC8AE0:0xC8AE0 + 14]
    rows = []
    for spell_type in range(14):
        code = 0x33 + spell_type
        base = 0xD4540 + 0x30 * (code - 1)
        raw_category = categories[spell_type]
        category = raw_category - 256 if raw_category >= 128 else raw_category
        rows.append(
            f'    [{spell_type}] = Spell{{ type = {spell_type}, '
            f'item_id = 0x{code:02X}, '
            f'name = "{source_text(data[base:base + 0x1A])}", '
            f'drain_level = {data[base + 0x24]}, '
            f'damage_class = {data[base + 0x25]}, '
            f'drain_modifier = {data[base + 0x26]}, '
            f'flag27 = {data[base + 0x27]}, '
            f'totem_category = {category}, '
            f'icon = 0x{read_u32(data, base + 0x1A):06X}, '
            f'description = 0x{read_u32(data, base + 0x1E):06X} }},'
        )
    return "\n".join(rows)


def accessory_rows(data: bytes) -> str:
    start = data.index(b" Smartgun", 0x1AAD00)
    names = data[start:0x1AAE8C].split(b"\xff")[:6]
    ids = (
        "SMARTGUN", "SILENCER", "SOUND_SUPPRESSOR", "LASER_SIGHT",
        "GAS_VENT_2", "GAS_VENT_3",
    )
    rows = []
    address = start
    for index, raw_name in enumerate(names):
        prices = [read_u16(data, 0x1AAE8C + shop * 0x0C + index * 2)
                  for shop in range(3)]
        rows.append(
            f'    [Accessory.{ids[index]}] = AccessoryRec{{ '
            f'id = Accessory.{ids[index]}, name = "{source_text(raw_name)}", '
            f'name_addr = 0x{address:06X}, price_by_shop = '
            f'{{ [0] = {prices[0]}, [1] = {prices[1]}, '
            f'[2] = {prices[2]} }} }},'
        )
        address += len(raw_name) + 1
    return "\n".join(rows)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("record", choices=("items", "spells", "accessories"))
    parser.add_argument("--rom", type=Path, default=ROM_DEFAULT)
    parser.add_argument("--kind-map", type=Path, default=KIND_MAP_DEFAULT)
    args = parser.parse_args()

    data = args.rom.read_bytes()
    if len(data) != 0x200000:
        raise SystemExit(f"expected a 2 MiB US Genesis ROM, got {len(data)} bytes")
    if args.record == "items":
        kinds = load_kind_map(args.kind_map)
        if any(code not in kinds for code in range(1, 0x33)):
            raise SystemExit("item kind map must classify all 50 item rows")
        result = item_rows(data, kinds)
    elif args.record == "spells":
        result = spell_rows(data)
    else:
        result = accessory_rows(data)
    print(result)


if __name__ == "__main__":
    main()
