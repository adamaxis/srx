#!/usr/bin/env python3
"""Focused control-flow and byte-range checks for rom_reachability.py."""

from __future__ import annotations

import csv
from pathlib import Path
import sys
import tempfile
import unittest


TOOLS = Path(__file__).resolve().parents[1] / "tools"
sys.path.insert(0, str(TOOLS))
import rom_reachability  # noqa: E402


def write_tsv(path: Path, header: list[str], rows: list[list[str]]) -> None:
    with path.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.writer(stream, delimiter="\t", lineterminator="\n")
        writer.writerow(header)
        writer.writerows(rows)


class RomReachabilityTest(unittest.TestCase):
    def test_vectors_calls_branches_and_known_function_masks(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            rom = bytearray(0x200)
            rom[4:8] = (0x100).to_bytes(4, "big")

            # Reset entry: JSR absolute to 0x110, then BRA to 0x10C.
            rom[0x100:0x108] = bytes.fromhex("4eb9000001106004")
            rom[0x108:0x10C] = bytes.fromhex("4e722700")
            rom[0x10C:0x10E] = bytes.fromhex("4e75")
            # The called routine has one table-resolved call, one unresolved
            # call, then returns. The resolved call target returns to 0x112.
            rom[0x110:0x116] = bytes.fromhex("4e904e914e75")
            rom[0x120:0x124] = bytes.fromhex("70014e75")
            rom_path = root / "rom.bin"
            rom_path.write_bytes(rom)

            functions_path = root / "functions.tsv"
            write_tsv(
                functions_path,
                ["entry", "name", "size"],
                [["00000110", "callee", "6"], ["00000120", "table_target", "4"]],
            )
            status_path = root / "status.tsv"
            write_tsv(status_path, ["rom_entry"], [["00000100"], ["00000110"], ["00000120"]])
            call_sites_path = root / "call-sites.tsv"
            write_tsv(
                call_sites_path,
                ["caller", "caller_name", "instruction", "text", "target", "callee", "status"],
                [
                    ["00000110", "callee", "00000110", "jsr (A0)", "00000120", "table_target", "resolved"],
                    ["00000110", "callee", "00000112", "jsr (A1)", "", "", "indirect/unresolved"],
                ],
            )
            disassembly_path = root / "disassembly.s"
            disassembly_path.write_text("00000130: jmp (A0)\n", encoding="utf-8")

            analysis = rom_reachability.analyze_rom(
                bytes(rom), functions_path, status_path, call_sites_path, disassembly_path
            )

            self.assertEqual([root.target for root in analysis.vector_roots], [0x100])
            self.assertEqual(
                set(analysis.instructions),
                {0x100, 0x106, 0x10C, 0x110, 0x112, 0x114, 0x120, 0x122},
            )
            self.assertEqual(analysis.reached_bytes, 20)
            self.assertEqual(analysis.outside_ranges, [(0x106, 0x108), (0x10C, 0x10E)])
            self.assertEqual(analysis.outside_bytes, 4)
            self.assertEqual(len(analysis.unresolved_calls), 1)
            self.assertEqual(len(analysis.resolved_indirect), 1)
            self.assertEqual(analysis.resolved_indirect[0].target, 0x120)
            self.assertEqual(len(analysis.unresolved_jumps), 1)
            self.assertFalse(analysis.unresolved_jumps[0].reached)
            self.assertEqual(analysis.known_function_bytes_total, 10)
            self.assertEqual(analysis.known_function_bytes_reached, 10)
            self.assertEqual(analysis.unreached_functions, [])

    def test_absolute_pointer_jump_table_seeds_all_candidates(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            rom = bytearray(0x200)
            rom[4:8] = (0x100).to_bytes(4, "big")

            # LEA table,A0; load selected pointer; JMP (A0).
            rom[0x100:0x10A] = bytes.fromhex("41f90000014020504ed0")
            rom[0x140:0x148] = (0x160).to_bytes(4, "big") + (0x170).to_bytes(4, "big")
            rom[0x148:0x14C] = bytes(4)  # Invalid pointer terminates the table.
            rom[0x160:0x162] = bytes.fromhex("4e75")
            rom[0x170:0x172] = bytes.fromhex("4e75")

            functions_path = root / "functions.tsv"
            write_tsv(
                functions_path,
                ["entry", "name", "size"],
                [["00000160", "first_target", "2"], ["00000170", "second_target", "2"]],
            )
            status_path = root / "status.tsv"
            write_tsv(status_path, ["rom_entry"], [["00000100"], ["00000160"], ["00000170"]])
            call_sites_path = root / "call-sites.tsv"
            write_tsv(
                call_sites_path,
                ["caller", "caller_name", "instruction", "text", "target", "callee", "status"],
                [],
            )
            disassembly_path = root / "disassembly.s"
            disassembly_path.write_text(
                "00000100: lea (0x140).l,A0\n"
                "00000106: movea.l (A0),A0\n"
                "00000108: jmp (A0)\n"
                "00000140: data 00000160 00000170\n"
                "00000160: rts\n"
                "00000170: rts\n",
                encoding="utf-8",
            )

            analysis = rom_reachability.analyze_rom(
                bytes(rom), functions_path, status_path, call_sites_path, disassembly_path
            )

            self.assertTrue({0x160, 0x170}.issubset(analysis.instructions))
            self.assertEqual(len(analysis.dispatch_tables), 1)
            self.assertEqual(analysis.dispatch_tables[0].site, 0x108)
            self.assertEqual(analysis.dispatch_tables[0].base, 0x140)
            self.assertEqual(analysis.dispatch_tables[0].targets, (0x160, 0x170))
            self.assertEqual(analysis.unresolved_jumps, [])

    def test_guarded_table_uses_rom_index_bound_for_unlisted_targets(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            rom = bytearray(0x200)
            rom[4:8] = (0x100).to_bytes(4, "big")

            # CMPI.W #2,D0 / BGT past dispatch gives valid indexes 0..2.
            rom[0x100:0x110] = bytes.fromhex(
                "0c4000026e0c41f90000014020504ed0"
            )
            rom[0x112:0x114] = bytes.fromhex("4e75")
            targets = (0x160, 0x170, 0x180)
            rom[0x140:0x14C] = b"".join(target.to_bytes(4, "big") for target in targets)
            for target in targets:
                rom[target:target + 2] = bytes.fromhex("4e75")

            functions_path = root / "functions.tsv"
            write_tsv(functions_path, ["entry", "name", "size"], [["00000100", "dispatch", "18"]])
            status_path = root / "status.tsv"
            write_tsv(status_path, ["rom_entry"], [["00000100"]])
            call_sites_path = root / "call-sites.tsv"
            write_tsv(
                call_sites_path,
                ["caller", "caller_name", "instruction", "text", "target", "callee", "status"],
                [],
            )
            disassembly_path = root / "disassembly.s"
            disassembly_path.write_text(
                "00000100: cmpi.w #0x2,D0w\n"
                "00000104: bgt.s 0x00000112\n"
                "00000106: lea (0x140).l,A0\n"
                "0000010C: movea.l (A0),A0\n"
                "0000010E: jmp (A0)\n"
                "00000140: data\n",
                encoding="utf-8",
            )

            analysis = rom_reachability.analyze_rom(
                bytes(rom), functions_path, status_path, call_sites_path, disassembly_path
            )

            self.assertEqual(analysis.dispatch_tables[0].targets, targets)
            self.assertTrue(set(targets).issubset(analysis.instructions))
            self.assertEqual(analysis.unresolved_jumps, [])

    def test_runtime_indexed_struct_callback_field_uses_stride_and_offset(self) -> None:
        rom = bytearray(0x200)
        for index, target in enumerate((0x180, 0x190, 0x1A0)):
            rom[0x140 + 8 * index + 4:0x140 + 8 * index + 8] = target.to_bytes(4, "big")
            rom[target:target + 2] = bytes.fromhex("4e75")

        # This block is present in raw ROM but absent from the Ghidra listing.
        # It indexes the callback at +4 in eight-byte records.
        instructions = {
            0x100: rom_reachability.Instruction(0x100, 6, "lea.l", "$140.l, a0"),
            0x106: rom_reachability.Instruction(0x106, 2, "lsl.w", "#$3, d0"),
            0x108: rom_reachability.Instruction(0x108, 2, "addq.l", "#$4, d0"),
            0x10A: rom_reachability.Instruction(0x10A, 4, "movea.l", "(a0, d0.w), a0"),
            0x10E: rom_reachability.Instruction(0x10E, 2, "jsr", "(a0)"),
        }
        listing = [
            rom_reachability.ListingInstruction(0x100, "lea.l", "(0x140).l, a0"),
            rom_reachability.ListingInstruction(0x112, "lea.l", "(0x158).l, a1"),
        ]

        dispatch = rom_reachability.discover_runtime_pointer_table(
            bytes(rom), instructions, 0x10E, listing
        )

        self.assertIsNotNone(dispatch)
        assert dispatch is not None
        self.assertEqual(dispatch.base, 0x140)
        self.assertEqual(dispatch.kind, "jsr runtime record-field dispatch (stride 8, offset 4)")
        self.assertEqual(dispatch.targets, (0x180, 0x190, 0x1A0))
        self.assertTrue(dispatch.bounded)

    def test_us_rom_dialogue_and_computed_dispatch_tables_are_reported(self) -> None:
        analysis = rom_reachability.analyze_rom(
            rom_reachability.DEFAULT_ROM.read_bytes(),
            rom_reachability.DEFAULT_FUNCTIONS,
            rom_reachability.DEFAULT_STATUS,
            rom_reachability.DEFAULT_CALL_SITES,
            rom_reachability.DEFAULT_DISASSEMBLY,
        )

        dialogue = next(
            table for table in analysis.dispatch_tables
            if table.site == 0x1F58A and table.base == 0x12CCB4
        )
        self.assertIn(0x24A1E, dialogue.targets)
        self.assertIn(0x24A56, dialogue.targets)
        self.assertEqual(len(dialogue.targets), 250)

        shared_dispatch_tables = {
            table.base for table in analysis.dispatch_tables if table.site == 0xC444
        }
        self.assertEqual(shared_dispatch_tables, {0xC1400, 0xC14C8, 0xC1558, 0x1D90C0})

        word_dispatch = next(
            table for table in analysis.dispatch_tables
            if table.site == 0x24E32 and table.base == 0x24E3A
        )
        self.assertEqual(word_dispatch.targets, (0x24E44, 0x24E84, 0x24EAC, 0x24F28, 0x24F16))

        inline = next(table for table in analysis.dispatch_tables if table.site == 0x2510A)
        self.assertEqual(len(inline.targets), 160)
        self.assertFalse(inline.bounded)
        self.assertTrue(any(site.address == 0x2510A for site in analysis.unresolved_jumps))
        callback_field = next(table for table in analysis.dispatch_tables if table.site == 0x214EC)
        self.assertEqual(callback_field.base, 0xF3E1E)
        self.assertEqual(callback_field.targets, (0x1EB32, 0x1E9AC))
        self.assertNotIn(0x214EC, {call.address for call in analysis.unresolved_calls})
        self.assertEqual(analysis.known_function_bytes_total, 111_098)
        self.assertGreaterEqual(analysis.known_function_bytes_reached, 109_300)
        self.assertEqual(len(analysis.unreached_functions), 26)
        self.assertEqual(analysis.unreached_functions[0].address, 0xBB6)


if __name__ == "__main__":
    unittest.main()
