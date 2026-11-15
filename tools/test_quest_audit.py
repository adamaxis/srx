"""Self-checks quest_audit against three ROM effects with known outcomes."""

from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
import quest_audit


class QuestAuditSelfCheck(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.rom = quest_audit.DEFAULT_ROM.read_bytes()
        cls.audit = quest_audit.build_audit(
            quest_audit.DEFAULT_ROM, quest_audit.DEFAULT_DISASM)

    def test_plain_message_sentinel_has_no_choices(self):
        pages = quest_audit.walk_dialogue_pages(self.rom, 0xFF978)
        self.assertEqual(len(pages), 1)
        self.assertEqual(pages[0]["choices"], [])

    def test_invalid_choice_targets_are_not_followed(self):
        rom = bytearray(0x200)
        for offset, value in ((0x20, 0x100), (0x24, 0x120),
                              (0x30, 0x100000)):
            rom[offset:offset + 4] = value.to_bytes(4, "big")
        rom[0x100:0x10D] = b"Hello there!\xff"
        rom[0x120:0x12B] = b"Choice yes\xff"
        pages = quest_audit.walk_dialogue_pages(bytes(rom), 0x20)
        self.assertEqual(len(pages), 1)
        self.assertIsNone(pages[0]["choices"][0]["target"])
        self.assertIsNone(pages[0]["choices"][0]["next_page"])

    def test_zero_label_terminates_choice_list(self):
        rom = bytearray(0x200)
        for offset, value in ((0x20, 0x100), (0x28, 0x120), (0x30, 0x40)):
            rom[offset:offset + 4] = value.to_bytes(4, "big")
        rom[0x100:0x10D] = b"Hello there!\xff"
        rom[0x120:0x12B] = b"Choice yes\xff"
        page = quest_audit.parse_dialogue_page(bytes(rom), 0x20)
        self.assertEqual(page["choices"], [])

    def test_end_targets_and_small_command_targets(self):
        # ROM 0x1F530-0x1F58A: targets below 0xFA (including 0, command
        # 0x21326) dispatch via 0x12CCB4; 0xFA+ re-enter as a page record.
        for target, expected_command in ((0, 0), (0xF9, 0xF9),
                                         (0xFFFFFFFF, None), (0xFA, None)):
            rom = bytearray(0x200)
            for offset, value in ((0x20, 0x100), (0x24, 0x120), (0x30, target)):
                rom[offset:offset + 4] = value.to_bytes(4, "big")
            rom[0x100:0x10D] = b"Hello there!\xff"
            rom[0x120:0x12B] = b"Choice yes\xff"
            page = quest_audit.parse_dialogue_page(bytes(rom), 0x20)
            self.assertEqual(page["choices"][0]["command"], expected_command)
            self.assertEqual(page["choices"][0]["target"], expected_command)
            self.assertIsNone(page["choices"][0]["next_page"])

    def test_audit_has_no_out_of_rom_page_targets(self):
        for chain in self.audit["page_chains"]:
            for page in chain["pages"]:
                for choice in page["choices"]:
                    target = choice["target"]
                    if target is not None and target >= 0x100:
                        self.assertGreaterEqual(target, 0)
                        self.assertLess(target + 16, len(self.rom))
                        self.assertEqual(choice["next_page"], target)

    def test_page_summary_counts_page_status(self):
        page_rows = [page for chain in self.audit["page_chains"]
                     for page in chain["pages"]]
        self.assertEqual(sum(self.audit["summary"]["pages"].values()), len(page_rows))

    def test_flagged_no_title_root_has_labels_at_plus_zero(self):
        # 0x20898 loads A0=0xFE5EC and 0x2089E sets FFF10C. 0x1F3C4-0x1F3D4
        # then skips the +0 text read, so 0x1F40A reads label 1 at +0:
        # "Sure thing, Cabbie." -> command 0x0F (+12) and
        # "Sorry.  I changed my mind." -> page 0xFE600 (+16); +8 is zero.
        page = quest_audit.parse_dialogue_page(self.rom, 0xFE5EC, no_title=True)
        self.assertIsNotNone(page)
        self.assertIsNone(page["text_pointer"])
        self.assertEqual([c["label_pointer"] for c in page["choices"]],
                         [0xFE545, 0xFE559])
        self.assertEqual(page["choices"][0]["command"], 0x0F)
        self.assertEqual(page["choices"][1]["next_page"], 0xFE600)
        # 0xFF958 (0x21308/0x2130E): three labels, targets command 0,
        # command 1, page 0xFF970.
        page = quest_audit.parse_dialogue_page(self.rom, 0xFF958, no_title=True)
        self.assertEqual([c["target"] for c in page["choices"]], [0, 1, 0xFF970])

    def test_audit_marks_fff10c_roots_no_title(self):
        flagged = set()
        for chain in self.audit["page_chains"]:
            if chain["pages"] and chain["pages"][0].get("no_title"):
                flagged.add(chain["pages"][0]["record"])
        self.assertTrue({0xFE5EC, 0xFF958, 0x1058CC, 0x111978} <= flagged,
                        sorted(hex(value) for value in flagged))
        for chain in self.audit["page_chains"]:
            for page in chain["pages"][1:]:
                self.assertFalse(page.get("no_title"))

    def test_michael_alias_keyword_and_karma(self):
        flags = self.audit["flag_operations"]
        effects = self.audit["dialogue_effects"]
        self.assertTrue(any(row["address"] == 0x21B98 and row["bit"] == 10 and
                            row["operation"] == "set" for row in flags))
        self.assertTrue(any(row["address"] == 0x21BA6 and row["kind"] == "karma" and
                            row["amount"] == 1 for row in effects))
        self.assertTrue(any(row.get("text_root") == 0xF5414 and
                            row["kind"] == "keyword_unlock" for row in effects))

    def test_stoker_michael_payment_and_text_list(self):
        effects = self.audit["dialogue_effects"]
        self.assertTrue(any(row["address"] == 0x21406 and row["kind"] == "nuyen" and
                            row.get("delta") == -250 for row in effects))
        self.assertTrue(any("creds roll off your stick" in record["text"].lower()
                            for record in self.audit["text_record_roots"].get(0xF487C, [])))

    def test_boris_visa_item_flag(self):
        self.assertTrue(any(row["address"] == 0x21C2C and row["storage"] == 0xFFFC22 and
                            row["bit"] == 1 and row["operation"] == "set"
                            for row in self.audit["flag_operations"]))
        self.assertTrue(any(row["address"] == 0x21C64 and row["kind"] == "item" and
                            row.get("item_name") == "MAGLOCK_PASSKEY" and row.get("rating") == 2
                            for row in self.audit["dialogue_effects"]))

    def test_boris_karma_matches_questfix1_action(self):
        row = next(row for row in self.audit["dialogue_effects"]
                   if row["address"] == 0x21BA6 and row["kind"] == "karma")
        self.assertEqual(row["status"], "MATCH")
        self.assertEqual(row["port_action"], "boris.reveal_michael_information")

    def test_owlfeather_karma_matches_its_branch(self):
        row = next(row for row in self.audit["dialogue_effects"]
                   if row["address"] == 0x218CA and row["kind"] == "karma")
        self.assertEqual(row["status"], "MATCH")
        self.assertEqual(row["port_action"], "party.award_karma")

    def test_red_buffalo_compares_storage_identity(self):
        row = next(row for row in self.audit["flag_operations"]
                   if row["address"] == 0x5436C and row["storage"] == 0xFFFC22)
        self.assertEqual(row["bit"], 5)
        self.assertEqual(row["port_identity"], "GroupItemId.GIFT_LADY_GILLIAN")
        self.assertEqual(row["status"], "MATCH")
        self.assertEqual(row["port_write"]["identity"], "GroupItemId.GIFT_LADY_GILLIAN")

    def test_quinton_payment_and_item_result(self):
        row = next(row for row in self.audit["dialogue_effects"]
                   if row["address"] == 0x228E0 and row["kind"] == "nuyen")
        self.assertEqual(row["amount"], 1000)
        self.assertEqual(row["result_item_code"], 9)
        self.assertEqual(row["result_phase"], 0x18)
        self.assertEqual(row["status"], "MATCH")

    def test_bouncer_commands_are_missing(self):
        handlers = {row["entry"]: row for row in self.audit["command_handlers"]}
        self.assertEqual(handlers[0x2223E]["status"], "MISSING")
        self.assertEqual(handlers[0x22276]["status"], "MISSING")

    def test_halloweener_payment_amount_is_parsed(self):
        row = next(row for row in self.audit["dialogue_effects"]
                   if row["address"] == 0x23ED6 and row["kind"] == "nuyen")
        self.assertEqual(row["amount"], 10000)


if __name__ == "__main__":
    unittest.main()
