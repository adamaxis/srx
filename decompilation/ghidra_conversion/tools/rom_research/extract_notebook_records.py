#!/usr/bin/env python3
"""Import Contacts and Shadowrunners text from the US Genesis ROM.

This is an offline asset step. The Qt runtime consumes the resulting keyed
text in assets/dialogue/script-text.json; it never reads ROM pointers.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path


TABLES = (
    ("contact", 13, 0xC66AC, 0xC66E0),
    ("shadowrunner", 10, 0xC665C, 0xC6684),
)
PAGE_LABELS = (
    ("pocket_secretary.notebook.record_page.option.next", 0xC661D),
    ("pocket_secretary.notebook.record_page.option.prior", 0xC662B),
)


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


def update_text_catalog(path: Path, entries: list[tuple[str, str]]) -> None:
    if path.exists():
        document = json.loads(path.read_text(encoding="utf-8"))
    else:
        document = {"schemaVersion": 1, "texts": {}}
    if not isinstance(document, dict) or document.get("schemaVersion") != 1:
        raise ValueError(f"unsupported dialogue JSON schema: {path}")
    texts = document.get("texts")
    if not isinstance(texts, dict) or any(
        not isinstance(key, str) or not isinstance(value, str)
        for key, value in texts.items()
    ):
        raise ValueError(f"dialogue JSON texts must be a string map: {path}")
    for key, value in entries:
        texts[key] = value
    document["texts"] = texts
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(
        json.dumps(document, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
        newline="\n",
    )
    temporary.replace(path)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--rom", type=Path, required=True)
    parser.add_argument("--text-catalog", "--script-text", dest="text_catalog",
                        type=Path, required=True)
    args = parser.parse_args()
    entries = extract(args.rom.read_bytes())
    update_text_catalog(args.text_catalog, entries)
    print(f"notebook_records={len(entries)} output={args.text_catalog}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
