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


class RomCoverageB2OwnerTest(unittest.TestCase):
    def test_historical_b2_set_is_fully_classified(self):
        expected_addresses = {
            "0x0200", "0x0208", "0x0210", "0x0218", "0x0220",
            "0x0228", "0x022E", "0x0234", "0x0236", "0x0330",
            "0x0436", "0x073A", "0x0BD2", "0x0C2C", "0x1342",
            "0x1728", "0x1D7A", "0x1DCA", "0x1DE0", "0x1DF6",
            "0x3084", "0x3B1E", "0x3BAC", "0x42E6", "0x44BC",
            "0x49A0", "0xB732", "0x104C8", "0x11136", "0x15354",
            "0x18450", "0x1BEBA", "0x1F1FC", "0x20A12", "0x21318",
            "0x21326", "0x21332", "0x215DA", "0x21CE4", "0x21E8C",
            "0x21FD0", "0x22018", "0x22072", "0x220EA", "0x22F90",
            "0x24A1E", "0x24A56", "0x530DE", "0x53290", "0x54A8E",
            "0x54B34", "0x54BD2", "0x54C5E", "0x55058", "0x5559E",
            "0x564D6", "0x59D12", "0x5BA4C", "0x5D172", "0x5D1B4",
            "0x5D996", "0x5DAE8", "0x5DAFC", "0x121FA0", "0x122420",
            "0x172CDC",
        }
        triage_path = ROOT / "decompilation/docs/ghidra/rom-coverage-triage.tsv"
        rows = rom_coverage.read_tsv(triage_path)
        triage_addresses = {
            rom_coverage.normalize_address(row["rom_entry"]): row
            for row in rows
        }
        self.assertEqual(set(triage_addresses), expected_addresses)

        coverage, _ = rom_coverage.inspect()
        missing_bug_addresses = {
            address
            for address, row in triage_addresses.items()
            if row["classification"] == "missing bug"
        }
        self.assertEqual(missing_bug_addresses, set())
        self.assertEqual(
            {routine.address for routine in coverage.unmapped},
            set(),
        )

        for address, row in triage_addresses.items():
            with self.subTest(address=address):
                category = row["classification"]
                if category == "missing bug":
                    self.assertEqual(row["mapping_kind"], "")
                    self.assertEqual(row["implementation"], "")
                    self.assertIn("missing bug", row["owner_or_replacement"])
                    self.assertNotIn(address, coverage.mappings)
                    continue

                expected_kind = {
                    "port owner found": "port",
                    "replaced-by": "replaced",
                }.get(category)
                self.assertIsNotNone(expected_kind, category)
                self.assertEqual(row["mapping_kind"], expected_kind)
                self.assertIn(address, coverage.mappings)
                mapping = coverage.mappings[address]
                self.assertEqual(mapping.kind, expected_kind)
                self.assertEqual(row["implementation"], mapping.implementation)
                self.assertTrue(row["evidence"].strip())

    def test_required_routines_have_explicit_port_owners(self):
        map_rows = rom_coverage.read_tsv(ROOT / rom_coverage.MAP_PATH)
        mappings = {
            rom_coverage.normalize_address(row["rom_entry"]): row
            for row in map_rows
        }
        expected = {
            "0x1728": (
                "decompilation/ghidra_conversion/src/core/remaining_field_conversion.cpp",
                "render_hud_sprite_stream",
            ),
            "0x3B1E": (
                "decompilation/ghidra_conversion/src/field/field_camera_conversion.cpp",
                "classify_field_camera_axis_step",
            ),
            "0x3BAC": (
                "decompilation/ghidra_conversion/src/field/field_camera_conversion.cpp",
                "follow_field_camera",
            ),
            "0x24A1E": (
                "decompilation/gameplay/scripts/dialogue_actions.lua",
                "register_core_building_purchases",
            ),
            "0x24A56": (
                "decompilation/gameplay/scripts/dialogue_actions.lua",
                "register_core_building_purchases",
            ),
            "0x44BC": (
                "decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp",
                "initialize_default_field_actors",
            ),
            "0x4FE2": (
                "decompilation/gameplay/scripts/actor/actions.lua",
                "advance_spell_effect",
            ),
            "0x508A": (
                "decompilation/gameplay/scripts/magic.lua",
                "actor_effect_profile",
            ),
            "0x5152": (
                "decompilation/gameplay/scripts/combat.lua",
                "resolve_spell_impact",
            ),
            "0x521E": (
                "decompilation/gameplay/scripts/actor/actions.lua",
                "advance_spell_effect",
            ),
            "0x52BE": (
                "decompilation/gameplay/scripts/actor/actions.lua",
                "advance_spell_effect",
            ),
            "0x5322": (
                "decompilation/gameplay/scripts/combat.lua",
                "resolve_spell_impact",
            ),
            "0x53C4": (
                "decompilation/ghidra_conversion/src/world/world_entity_conversion.cpp",
                "run_effect_variant_a",
            ),
        }
        for address, (path, owner) in expected.items():
            with self.subTest(address=address):
                mapping = mappings.get(address)
                self.assertIsNotNone(mapping, f"missing explicit owner for {address}")
                if mapping is None:
                    continue
                self.assertEqual(mapping["mapping_kind"], "port")
                self.assertIn(path, mapping["implementation"])
                self.assertIn(owner, mapping["implementation"])


if __name__ == "__main__":
    unittest.main()
