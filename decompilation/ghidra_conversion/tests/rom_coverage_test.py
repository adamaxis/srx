#!/usr/bin/env python3
"""Regression coverage for the ROM map baseline contract."""

import csv
from pathlib import Path
import sys
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "decompilation/ghidra_conversion/tools"))
import rom_coverage  # noqa: E402


class RomCoverageBaselineTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="srx-rom-coverage-")
        self.root = Path(self.temp.name)
        docs = self.root / rom_coverage.STATUS_PATH.parent
        docs.mkdir(parents=True)
        functions = self.root / rom_coverage.FUNCTIONS_PATH
        functions.parent.mkdir(parents=True)
        source = self.root / "decompilation/gameplay/scripts/coverage_fixture.lua"
        source.parent.mkdir(parents=True)
        source.write_text("CoverageFixture = {}\n", encoding="utf-8")
        self.write_tsv(
            self.root / rom_coverage.STATUS_PATH,
            ["rom_entry", "subsystem", "native_owner", "test_name", "status", "visual_golden", "next"],
            [
                ["0x1000", "mapped fixture", "CoverageFixture", "", "partial", "", ""],
                ["0x1010", "existing backlog fixture", "", "", "pending", "", ""],
            ],
        )
        self.write_tsv(
            functions,
            ["entry", "name", "size", "callingConvention", "decompileStatus", "file"],
            [["00001000", "coverage_fixture", "8", "unknown", "ok", "fixture.cpp"]],
        )
        self.map_path = self.root / rom_coverage.MAP_PATH
        self.baseline_path = self.root / rom_coverage.BASELINE_PATH
        self.report_path = self.root / rom_coverage.REPORT_PATH
        self.map_path.parent.mkdir(parents=True, exist_ok=True)
        self.baseline_path.parent.mkdir(parents=True, exist_ok=True)
        self.write_tsv(
            self.map_path,
            rom_coverage.MAP_HEADER,
            [["0x1000", "port", "decompilation/gameplay/scripts/coverage_fixture.lua#CoverageFixture", "fixture source owner"]],
        )
        self.baseline_path.write_text("# mapped baseline\n0x1000\n", encoding="utf-8")
        coverage, export_count = rom_coverage.inspect(self.root, self.map_path, self.baseline_path)
        self.report_path.parent.mkdir(parents=True, exist_ok=True)
        self.report_path.write_text(rom_coverage.build_report(coverage, export_count), encoding="utf-8")

    def tearDown(self):
        self.temp.cleanup()

    @staticmethod
    def write_tsv(path, header, rows):
        with path.open("w", newline="", encoding="utf-8") as stream:
            writer = csv.writer(stream, delimiter="\t", lineterminator="\n")
            writer.writerow(header)
            writer.writerows(rows)

    def test_existing_unmapped_backlog_does_not_fail(self):
        coverage, export_count = rom_coverage.inspect(self.root, self.map_path, self.baseline_path)
        self.assertEqual(coverage.errors, [])
        self.assertEqual(export_count, 1)
        self.assertEqual([row.address for row in coverage.unmapped], ["0x1010"])

    def test_losing_a_baseline_mapping_fails(self):
        self.write_tsv(self.map_path, rom_coverage.MAP_HEADER, [])
        coverage, _ = rom_coverage.inspect(self.root, self.map_path, self.baseline_path)
        self.assertIn("baseline routine lost its mapping: 0x1000", coverage.errors)


if __name__ == "__main__":
    unittest.main()
