#!/usr/bin/env python3
"""Import Contacts and Shadowrunners text from the US Genesis ROM.

This is an offline asset step. The Qt runtime consumes the resulting keyed
text in assets/dialogue/script-text.lua; it never reads ROM pointers.
"""

from __future__ import annotations

import argparse
from pathlib import Path


TABLES = (
    ("contact", 13, 0xC66AC, 0xC66E0),
    ("shadowrunner", 10, 0xC665C, 0xC6684),
)
PAGE_LABELS = (
    ("pocket_secretary.notebook.record_page.option.next", 0xC661D),
    ("pocket_secretary.notebook.record_page.option.prior", 0xC662B),
)
BEGIN = "    -- BEGIN ROM-extracted Notebook Contacts and Shadowrunners\n"
END = "    -- END ROM-extracted Notebook Contacts and Shadowrunners\n"


def read_long(rom: bytes, offset: int) -> int:
    if offset < 0 or offset + 4 > len(rom):
        raise ValueError(f"pointer table access outside ROM: {offset:#x}")
    return int.from_bytes(rom[offset : offset + 4], "big")


def read_text(rom: bytes, offset: int) -> str:
    if offset < 0 or offset >= len(rom):
        raise ValueError(f"text pointer outside ROM: {offset:#x}")
    segments: list[str] = []
    current = bytearray()
    for byte in rom[offset:]:
        if byte == 0xFF:
            if current:
                segments.append(current.decode("ascii"))
            return "\n".join(segments)
        if byte & 0x80:
            segments.append(current.decode("ascii"))
            current.clear()
            continue
        if not 0x20 <= byte <= 0x7E:
            raise ValueError(f"unsupported text byte {byte:#04x} at {offset:#x}")
        current.append(byte)
    raise ValueError(f"unterminated text at {offset:#x}")


def lua_string(value: str) -> str:
    return '"' + value.replace("\\", "\\\\").replace('"', '\\"').replace(
        "\n", "\\n"
    ) + '"'


def extract(rom: bytes) -> list[tuple[str, str]]:
    entries: list[tuple[str, str]] = []
    for kind, count, name_table, detail_table in TABLES:
        for index in range(count):
            record = f"record_{index}"
            entries.append(
                (
                    f"pocket_secretary.notebook.{kind}.name.option.{record}",
                    read_text(rom, read_long(rom, name_table + index * 4)),
                )
            )
            entries.append(
                (
                    f"pocket_secretary.notebook.{kind}.detail.option.{record}",
                    read_text(rom, read_long(rom, detail_table + index * 4)),
                )
            )
    entries.extend((key, read_text(rom, offset)) for key, offset in PAGE_LABELS)
    return entries


def update_script_text(path: Path, entries: list[tuple[str, str]]) -> None:
    raw = path.read_bytes()
    newline = b"\r\n" if b"\r\n" in raw else b"\n"
    text = raw.decode("utf-8").replace("\r\n", "\n")
    if BEGIN in text:
        start = text.index(BEGIN)
        end = text.index(END, start) + len(END)
        text = text[:start] + text[end:]
    closing = "    },\n}\n"
    if not text.endswith(closing):
        raise ValueError(f"unexpected generated script-text.lua layout: {path}")
    block = BEGIN + "".join(
        f"        [{lua_string(key)}] = {lua_string(value)},\n"
        for key, value in entries
    ) + END
    updated = text[: -len(closing)] + block + closing
    path.write_bytes(updated.replace("\n", newline.decode("ascii")).encode("utf-8"))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--rom", type=Path, required=True)
    parser.add_argument("--script-text", type=Path, required=True)
    args = parser.parse_args()
    entries = extract(args.rom.read_bytes())
    update_script_text(args.script_text, entries)
    print(f"notebook_records={len(entries)} output={args.script_text}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
