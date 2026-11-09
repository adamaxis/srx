#!/usr/bin/env python3
"""Emit item, spell, or accessory Lua rows from the US Genesis ROM."""

from __future__ import annotations

import argparse
import csv
import struct
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[1]
DECOMP_ROOT = PROJECT_ROOT.parent
ROM_DEFAULT = DECOMP_ROOT / "input" / "shadowrun_genesis_us.bin"
KIND_MAP_DEFAULT = DECOMP_ROOT / "docs" / "ghidra" / "source-records" / "items_full.tsv"
ITEM_RUNTIME_IDS = (
    "streetline", "model_101t", "american_l36", "security_500",
    "warhawk", "max_power", "predator", "ak_97", "hk227_s", "mach_22",
    "allegiance", "roomsweeper", "frag", "scatter", "concussion",
    "medkit", "stim_patch", "trauma_patch", "fetish", "maglock_passkey",
    "electronic_kit", "smart_goggles", "enchanted_dagger", "power_focus",
    "flame_dart_focus", "flame_bolt_focus", "hellblast_focus",
    "invisibility_focus", "sleep_focus", "stink_focus",
    "supr_barrier_focus", "mana_zap_focus", "mana_blast_focus",
    "mana_storm_focus", "rockskin_focus", "heal_wounds_focus",
    "confusion_focus", "barrier_focus", "protection_talisman",
    "combat_sense_spell_lock", "rat_totem", "gator_totem", "bear_totem",
    "armor_vest", "armor_clothing", "armor_jacket", "vest_with_plates",
    "lined_duster", "light_combat_armor", "heavy_combat_armor",
)
SPELL_IDS = (
    "flame_dart", "flame_bolt", "hellblast", "invisibility", "sleep",
    "stink", "super_barrier", "mana_zap", "mana_blast", "mana_storm",
    "rockskin", "heal_wounds", "confusion", "barrier",
)
# Source table 0x13E7C is converted to ordinary world-distance bounds here.
# Runtime content receives the range, never the selector byte.
ACTION_MOTION_RANGES = (
    (0x0500, 0x0640),
    (0x05A0, 0x0780),
    (0x0640, 0x08C0),
    (0x08C0, 0x0C80),
)


def read_u16(data: bytes, address: int) -> int:
    return struct.unpack_from(">H", data, address)[0]


def read_u32(data: bytes, address: int) -> int:
    return struct.unpack_from(">I", data, address)[0]


def source_text(raw: bytes) -> str:
    return " ".join(raw.split(b"\xff")[0].replace(b"\x80", b" ").decode("latin1").split())


def signed_byte(value: int) -> int:
    return value - 256 if value >= 128 else value


def signed_word(value: int) -> int:
    return value - 0x10000 if value >= 0x8000 else value


def load_kind_map(path: Path) -> dict[int, str]:
    with path.open(encoding="utf-8", newline="") as source:
        return {
            int(row["code"], 16): row["kind_repo"]
            for row in csv.DictReader(source, delimiter="\t")
            if row.get("kind_repo")
        }


def item_rows(data: bytes, kinds: dict[int, str]) -> str:
    rows = []
    # 0x12A8C uses live actor +0x6E detail values for item actions 0x0D+
    # instead of the row's +0x24. Keep those bytes out of static item data.
    spell_categories = tuple(signed_byte(value) for value in
                             data[0xC8AE0:0xC8AE0 + 14])
    for code in range(1, 0x33):
        base = 0xD4540 + 0x30 * (code - 1)
        kind = kinds[code]
        runtime_id = ITEM_RUNTIME_IDS[code - 1]
        range_selector = data[base + 0x27]
        if range_selector >= len(ACTION_MOTION_RANGES):
            raise ValueError(
                f"item {code:02X} has unsupported target-motion range "
                f"{range_selector:02X}"
            )
        minimum_distance, maximum_distance = ACTION_MOTION_RANGES[range_selector]
        semantic_fields = []
        if kind == "totem":
            modifiers = []
            for spell_index, selector in enumerate(spell_categories):
                if selector < 0 or selector > 2:
                    continue
                modifier = signed_word(read_u16(
                    data, base + 0x22 + selector * 2))
                modifiers.append(
                    f'{SPELL_IDS[spell_index]} = {modifier}')
            semantic_fields.append(
                "totem_spell_modifiers = { " + ", ".join(modifiers) + " }"
            )
        else:
            semantic_fields.append(
                f'purchase_base = {read_u16(data, base + 0x22)}'
            )
        if kind == "armor":
            semantic_fields.append(
                'armor = { '
                f'primary_modifier = {data[base + 0x25]}, '
                f'secondary_modifier = {data[base + 0x26]} }}'
            )
        if kind == "grenade":
            # ROM 0x55900 supplies each grenade's base effect while the
            # D4540 +0x26 field supplies its action difficulty. The shared
            # blast caller at 0x15554 uses a narrower rectangular window for
            # item 0x0E; 0x5590A marks item 0x0F as mental damage. Publish
            # those as ordinary gameplay properties, not source record bytes.
            # The shared launch/flight timing comes from 0x15A4C, 0x156B8,
            # 0x15E56/0x1575A, and the 0x15390/0x15402 behavior path.
            base_effect = 6 if code == 0x0D else 3
            blast_extent = 320 if code == 0x0E else 640
            damage_track = "mental" if code == 0x0F else "physical"
            semantic_fields.append(
                'throwable_action = { '
                f'base_effect = {base_effect}, '
                f'difficulty = {data[base + 0x26]}, '
                f'blast_extent = {blast_extent}, '
                'launch_distance = 176, flight_speed = 32, '
                'arming_delay_updates = 19, flight_duration_updates = 180, '
                f'damage_track = "{damage_track}", '
                'accuracy_skill = "throwing", armor_channel = "secondary" }'
            )
        semantic_fields.append(
            f'target_motion_range = {{ minimum_distance = {minimum_distance}, '
            f'maximum_distance = {maximum_distance} }}'
        )
        semantic_fields.append(
            f'has_detail_art = '
            f'{str(read_u32(data, base + 0x1E) != 0xFFFFFFFF).lower()}'
        )
        rows.append(
            f'    ["{runtime_id}"] = Item{{ id = "{runtime_id}", '
            f'name = "{source_text(data[base:base + 0x1A])}", '
            f'kind = ItemKind.{kind.upper()}, '
            f'{", ".join(semantic_fields)} }},'
        )
    return "\n".join(rows)


def spell_rows(data: bytes) -> str:
    rows = []
    for spell_type in range(14):
        code = 0x33 + spell_type
        base = 0xD4540 + 0x30 * (code - 1)
        drain_selector = data[base + 0x24]
        base_drain = -signed_byte(data[0x166AD4 + drain_selector])
        force_drain_modifier = data[base + 0x26]
        range_selector = data[base + 0x27]
        if range_selector < len(ACTION_MOTION_RANGES):
            minimum_distance, maximum_distance = ACTION_MOTION_RANGES[
                range_selector
            ]
            target_range = (
                f"{{ minimum_distance = {minimum_distance}, "
                f"maximum_distance = {maximum_distance} }}"
            )
        else:
            target_range = "nil"
        rows.append(
            f'    ["{SPELL_IDS[spell_type]}"] = Spell{{ '
            f'id = "{SPELL_IDS[spell_type]}", '
            f'name = "{source_text(data[base:base + 0x1A])}", '
            f'base_drain = {base_drain}, '
            f'force_drain_modifier = {force_drain_modifier}, '
            f'target_motion_range = {target_range}'
            f' }},'
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
