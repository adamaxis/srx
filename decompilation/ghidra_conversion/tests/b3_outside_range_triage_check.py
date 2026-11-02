#!/usr/bin/env python3
"""Validate the B3 outside-function range triage against the current ROM walk."""

from __future__ import annotations

import argparse
from pathlib import Path
import re
import sys


ROOT = Path(__file__).resolve().parents[3]
TOOLS = Path(__file__).resolve().parents[1] / "tools"
REPORT = ROOT / "decompilation/docs/ghidra/b3-outside-range-triage.md"
sys.path.insert(0, str(TOOLS))

import rom_reachability as reachability  # noqa: E402


RANGE_ROW = re.compile(
    r"^\|\s*(\d+)\s*\|\s*`0x([0-9A-Fa-f]+)\.\.0x([0-9A-Fa-f]+)`"
    r"\s*\|\s*([0-9,]+)\s*\|\s*([^|]+)\|\s*([A-Z][0-9]{2})\s*\|\s*([^|]+)\|$"
)


def check() -> list[str]:
    errors: list[str] = []
    if not REPORT.is_file():
        return [f"triage report is missing: {REPORT}"]

    analysis = reachability.analyze_rom(
        reachability.DEFAULT_ROM.read_bytes(),
        reachability.DEFAULT_FUNCTIONS,
        reachability.DEFAULT_STATUS,
        reachability.DEFAULT_CALL_SITES,
        reachability.DEFAULT_DISASSEMBLY,
    )
    expected = reachability.range_rows(analysis.outside_ranges)
    text = REPORT.read_text(encoding="utf-8")
    if "## Range-by-range classification" not in text:
        return ["triage report has no range-by-range classification section"]

    rows = []
    in_rows = False
    for line in text.splitlines():
        if line == "| Rank | ROM interval (inclusive) | Bytes | Classification | Group | ROM edge / owner anchor |":
            in_rows = True
            continue
        if in_rows and line.startswith("|---"):
            continue
        if in_rows and line.startswith("|"):
            match = RANGE_ROW.match(line)
            if not match:
                errors.append(f"malformed triage range row: {line[:160]}")
                continue
            rank, start, end, byte_count, classification, group, owner = match.groups()
            rows.append(
                (
                    int(rank),
                    int(start, 16),
                    int(end, 16) + 1,
                    int(byte_count.replace(",", "")),
                    classification.strip(),
                    group,
                    owner.strip(),
                )
            )
            continue
        if in_rows:
            break

    if len(rows) != len(expected):
        errors.append(f"triage has {len(rows)} rows; ROM walk has {len(expected)}")

    group_ids = set(re.findall(r"^\|\s*(G[0-9]{2})\s*\|", text, re.MULTILINE))
    seen: set[tuple[int, int]] = set()
    false_data = []
    for index, row in enumerate(rows):
        rank, start, end, byte_count, classification, group, owner = row
        if (start, end) in seen:
            errors.append(f"duplicate range 0x{start:06X}..0x{end - 1:06X}")
        seen.add((start, end))
        if rank != index + 1:
            errors.append(f"range rank {rank} is out of order at row {index + 1}")
        if group not in group_ids:
            errors.append(f"range 0x{start:06X} uses undefined group {group}")
        if not owner or (
            "owner anchor" not in owner.lower()
            and "no nearby" not in owner.lower()
            and "0x" not in owner.lower()
        ):
            errors.append(f"range 0x{start:06X} has no source/owner anchor")
        if index < len(expected):
            expected_size, expected_start, expected_end = expected[index]
            if (start, end, byte_count) != (expected_start, expected_end, expected_size):
                errors.append(
                    f"range row {rank} differs from ROM walk: "
                    f"0x{start:06X}..0x{end - 1:06X}/{byte_count} vs "
                    f"0x{expected_start:06X}..0x{expected_end - 1:06X}/{expected_size}"
                )
        if start == 0x166514:
            false_data.append((end - start, classification))
            if "[C] DATA" not in classification:
                errors.append("0x166514 must remain classified as confirmed text data")
        elif "[C] CODE" not in classification:
            errors.append(f"0x{start:06X} is not classified as confirmed code")

    if sum(row[3] for row in rows) != analysis.outside_bytes:
        errors.append("triage byte total does not match the ROM walk")
    # Six ROM-confirmed effect callback entry instructions are now recorded in
    # the status ledger. Their entry bytes are excluded from the outside-range
    # walk, splitting two former spans into four additional intervals.
    if len(rows) != 175 or analysis.outside_bytes != 33_562:
        errors.append(
            f"B3 snapshot changed: expected 175 ranges / 33,562 bytes, "
            f"found {len(rows)} / {analysis.outside_bytes}"
        )
    if false_data != [(2, "[C] DATA (string; false code candidate)")]:
        errors.append(f"expected the single 2-byte text-data candidate at 0x166514; found {false_data}")
    if "[H] Whether any runtime record can ever select that text pointer" not in text:
        errors.append("the unresolved runtime domain for 0x054960 is not documented")
    if "No new gameplay-relevant missing behavior was established" not in text:
        errors.append("triage disposition is missing")
    return errors


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", required=True)
    parser.parse_args()
    errors = check()
    if errors:
        for error in errors:
            print(f"B3 range triage check failed: {error}", file=sys.stderr)
        return 1
    print(f"Checked {REPORT}: 175 ranges / 33,562 bytes; 174 code ranges, one data false positive.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
