#!/usr/bin/env python3
"""Extract typed Matrix system data from the US Genesis ROM.

The ROM stores 25 Matrix maps at 0x15DB48. Each entry points to a 14x10 map
and a linked node-record block. This is an offline content import; the Qt game
loads the generated Lua data and never reads ROM addresses at runtime.
"""

from __future__ import annotations

import argparse
from pathlib import Path


ROM_TABLE = 0x15DB48
MAP_CELLS = 14 * 10
NODE_BYTES = 0x14
SYSTEM_COUNT = 25


def read_u32(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset : offset + 4], "big")


def signed_byte(value: int) -> int:
    return value - 0x100 if value >= 0x80 else value


def require_range(data: bytes, start: int, size: int, label: str) -> None:
    if start < 0 or size < 0 or start + size > len(data):
        raise ValueError(
            f"{label} range 0x{start:X}..0x{start + size:X} is outside the ROM"
        )


def system_records(data: bytes) -> list[dict[str, object]]:
    systems: list[dict[str, object]] = []
    for system_id in range(SYSTEM_COUNT):
        entry = ROM_TABLE + system_id * 8
        map_offset = read_u32(data, entry)
        node_offset = read_u32(data, entry + 4)
        next_node_offset = (
            read_u32(data, entry + 12)
            if system_id + 1 < SYSTEM_COUNT
            else node_offset + 0x280
        )
        if not (0x100000 <= map_offset < len(data)):
            raise ValueError(
                f"system {system_id} has invalid map pointer 0x{map_offset:X}"
            )
        if not (0x100000 <= node_offset < len(data)):
            raise ValueError(
                f"system {system_id} has invalid node pointer 0x{node_offset:X}"
            )
        if next_node_offset < node_offset:
            raise ValueError(f"system {system_id} node range runs backwards")

        require_range(data, map_offset, MAP_CELLS, f"system {system_id} map")
        max_records = min(32, (next_node_offset - node_offset) // NODE_BYTES)
        require_range(
            data,
            node_offset,
            max_records * NODE_BYTES,
            f"system {system_id} nodes",
        )

        nodes: dict[int, dict[str, object]] = {}
        for node_id in range(max_records):
            record_offset = node_offset + node_id * NODE_BYTES
            record = data[record_offset : record_offset + NODE_BYTES]
            function_id = record[0]
            if function_id == 0xFF or function_id > 5:
                continue
            nodes[node_id] = {
                "function_id": function_id,
                "file_class_id": record[1],
                "x": record[3],
                "y": record[4],
                "security_color": record[5],
                "security": record[6],
                "links": {
                    link_id: signed_byte(value)
                    for link_id, value in enumerate(record[7:15])
                },
                "ic_condition": signed_byte(record[0x0F]),
                "ic_reaction": signed_byte(record[0x10]),
                "ic_rating": record[0x11],
                "ic_rating2": record[0x12],
                "ic_combo_id": signed_byte(record[0x13]),
            }

        systems.append(
            {
                "id": system_id,
                "map": list(data[map_offset : map_offset + MAP_CELLS]),
                "nodes": nodes,
            }
        )
    return systems


def lua_catalog(systems: list[dict[str, object]]) -> str:
    """Render compact named records: one row per map row and node record."""
    lines = ["return {"]
    for system in systems:
        system_id = int(system["id"])
        map_cells = system["map"]
        nodes = system["nodes"]
        lines.extend(
            [
                f"  [{system_id}] = {{",
                f"    id = {system_id},",
                "    map = {",
            ]
        )
        for row in range(10):
            start = row * 14
            values = map_cells[start : start + 14]
            lines.append("      " + ", ".join(map(str, values)) + ",")
        lines.extend(["    },", "    nodes = {"])
        for node_id, node in nodes.items():
            links = ", ".join(str(node["links"][link]) for link in range(8))
            lines.append(
                f"      [{node_id}] = {{ function_id = {node['function_id']}, "
                f"file_class_id = {node['file_class_id']}, x = {node['x']}, "
                f"y = {node['y']}, security_color = {node['security_color']}, "
                f"security = {node['security']}, links = {{{links}}}, "
                f"ic_condition = {node['ic_condition']}, "
                f"ic_reaction = {node['ic_reaction']}, "
                f"ic_rating = {node['ic_rating']}, "
                f"ic_rating2 = {node['ic_rating2']}, "
                f"ic_combo_id = {node['ic_combo_id']} }},"
            )
        lines.extend(["    },", "  },"])
    lines.extend(["}", ""])
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--rom",
        type=Path,
        default=Path("input/shadowrun_genesis_us.bin"),
        help="US Genesis ROM path (default: input/shadowrun_genesis_us.bin)",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("gameplay/scripts/data/matrix_systems.lua"),
        help="generated Lua module path",
    )
    args = parser.parse_args()

    rom = args.rom.read_bytes()
    systems = system_records(rom)
    node_count = sum(len(system["nodes"]) for system in systems)
    header = [
        "-- Generated by tools/matrix_systems_extract.py from the US Genesis ROM.",
        "-- System IDs and records follow the table at ROM 0x15DB48.",
        "-- The runtime consumes named fields and has no ROM-address dependency.",
    ]
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        "\n".join(header) + "\n" + lua_catalog(systems),
        encoding="utf-8",
        newline="\n",
    )
    print(f"systems={len(systems)} nodes={node_count} output={args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
