#!/usr/bin/env python3
"""Extract conditional field dialogue outcomes selected by the US ROM.

The selector at ROM 0x59986 maps each of 56 field contexts to one of 16
groups, rolls twice with bound five, and indexes the 16x9 enable table at
0x1D70E4. Enabled cells indirect through the pointer table at 0x1D7174 to
one of the ROM's field-event handlers. Disabled cells take the separate
dynamic world-effect path. This offline extractor records the outcome and the
opening dialogue/menu text, then emits weighted semantic outcomes, event
content, and variable-length choice rows. Source handler addresses and
command words remain in the evidence table and are not used as runtime event
or choice IDs. The extractor does not place a character on the map.
"""

from __future__ import annotations

import argparse
import csv
import re
import sys
from collections import Counter
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[3]
DEFAULT_ROM = REPO_ROOT / "input" / "shadowrun_genesis_us.bin"
DEFAULT_OUTPUT = REPO_ROOT / "assets" / "field" / "world-activity-outcomes.tsv"
DEFAULT_RUNTIME_OUTPUT = (
    REPO_ROOT / "assets" / "field" / "world-activity-events.tsv"
)
DEFAULT_RUNTIME_CHOICES_OUTPUT = (
    REPO_ROOT / "assets" / "field" / "world-activity-event-choices.tsv"
)
DEFAULT_RUNTIME_EVENT_CONTENT_OUTPUT = (
    REPO_ROOT / "assets" / "field" / "world-activity-event-content.tsv"
)
DEFAULT_DYNAMIC_ACTORS = (
    REPO_ROOT / "assets" / "field" / "entities" /
    "dynamic-world-activity-actors.tsv"
)

CONTEXT_GROUP_TABLE = 0x3312
CONTEXT_COUNT = 56
OUTCOME_MASK_TABLE = 0x1D70E4
OUTCOME_POINTER_TABLE = 0x1D7174
OUTCOMES_PER_GROUP = 9
RANDOM_BOUND = 5
MODE_COMMAND_TABLE = 0x12CCB4
MODE_COMMAND_COUNT = 250
GENERIC_OUTCOME = "dynamic_world_effect_sequence"
ACTIVATION = (
    "Eligible field-loop pass at tick counter modulo 1024 == 10, after the "
    "ROM effect/entity guards and source random-threshold check; two random "
    "rolls with bound 5 select a sum from 0 through 8."
)

# Offline source handlers are translated to stable gameplay event identities.
# These addresses stay in the extraction/evidence data and are not Lua IDs.
EVENT_IDS_BY_HANDLER = {
    0x20EFA: "urban_disturbance",
    0x20F2A: "phantasmal_watcher",
    0x20F12: "lone_star_patrol_stop",
    0x20F8A: "flustered_woman",
    0x20F42: "faction_disturbance",
    0x2112C: "alley_grenade_seller",
    0x20F72: "injured_person",
    0x20FA2: "haggard_man_collapse",
    0x20F5A: "company_man_confrontation",
    0x2115C: "wilderness_disturbance",
    0x21144: "arrowhead_search",
    0x21114: "being_stalked",
    0x21004: "hidden_camera",
    0x2101E: "custodian_pass_check",
    0x210FC: "security_mage_stop",
    0x20FEC: "security_guard_stop",
    0x21036: "concealed_security_plate",
    0x20FBA: "open_security_panel",
    0x21082: "building_badge_check",
    0x20FD4: "scientist_encounter",
    0x2109A: "management_terminal",
    0x210B4: "company_man_toothpick",
    0x210CC: "renraku_strike_team",
    0x210E4: "renraku_security_stop",
    0x21050: "laser_detection_field",
    0x2106A: "lone_star_patrol",
}

# Player-facing choice text is converted to intent IDs during extraction. The
# event and choice pair is the native route key; Lua never receives the ROM's
# command words or dispatcher addresses.
CHOICE_IDS_BY_LABEL = {
    "more": "continue",
    "attack!!": "attack",
    "duck for cover.": "take_cover",
    "hack the database.": "hack_database",
    "help them.": "help_search",
    "re-wire the panel.": "rewire_security_panel",
    "run away!": "run_away",
    "talk to him.": "talk",
    "talk to them.": "talk",
    "wait for more information.": "wait_for_information",
    "walk away.": "leave",
    "i'll take the grenades.": "buy_grenades",
    "i'm looking for some information.": "ask_for_information",
    "slot off, frag-face!!!": "insult",
    "take me to him. i'll help.": "accept_watcher_request",
}

FIELDS = (
    "scene",
    "context_group",
    "random_sum",
    "roll_pairs",
    "roll_pair_count",
    "outcome_kind",
    "event_name",
    "handler_offset",
    "prompt_record_offset",
    "resource_bundle_offset",
    "opening_text",
    "choice_1",
    "choice_2",
    "choice_3",
    "choice_1_command",
    "choice_1_dispatch_target",
    "choice_2_command",
    "choice_2_dispatch_target",
    "choice_3_command",
    "choice_3_dispatch_target",
    "activation",
    "selection_cell_offset",
    "selection_pointer_offset",
    "selected_target_offset",
    "map_position",
)

RUNTIME_FIELDS = (
    "scene_id",
    "chance_weight",
    "outcome_kind",
    "outcome_id",
)

RUNTIME_EVENT_CONTENT_FIELDS = (
    "event_id",
    "opening_text",
)

RUNTIME_CHOICE_FIELDS = (
    "event_id",
    "choice_order",
    "choice_id",
    "label",
)


def read_u32(rom: bytes, offset: int) -> int:
    if offset < 0 or offset + 4 > len(rom):
        raise ValueError(f"longword outside ROM at 0x{offset:X}")
    return int.from_bytes(rom[offset : offset + 4], "big")


def decode_dialogue(rom: bytes, offset: int) -> str:
    """Decode the game's printable dialogue stream and paragraph controls."""
    if offset < 0 or offset >= len(rom):
        raise ValueError(f"dialogue pointer outside ROM: 0x{offset:X}")
    segments: list[str] = [""]
    for cursor in range(offset, min(len(rom), offset + 4096)):
        byte = rom[cursor]
        if byte == 0xFF:
            return "|".join(segments).strip()
        if byte >= 0x80:
            segments.append("")
        elif 0x20 <= byte <= 0x7E:
            segments[-1] += chr(byte)
        else:
            raise ValueError(
                f"unsupported dialogue byte 0x{byte:02X} at 0x{cursor:X}"
            )
    raise ValueError(f"dialogue at 0x{offset:X} has no ROM terminator")


def prompt_fields(
    rom: bytes, handler: int
) -> tuple[int, int, list[str], list[int]]:
    """Read the absolute prompt/resource operands used by a handler stub."""
    if rom[handler : handler + 2] != b"\x41\xF9":
        raise ValueError(f"handler 0x{handler:X} does not begin with LEA A0")
    prompt = read_u32(rom, handler + 2)
    cursor = handler + 6
    resource_bundle = 0
    if rom[cursor : cursor + 2] == b"\x4B\xF9":
        resource_bundle = read_u32(rom, cursor + 2)

    text_fields: list[str] = []
    for index in range(4):
        pointer = read_u32(rom, prompt + index * 4)
        if pointer in (0, 0xFFFFFFFF):
            text_fields.append("")
        else:
            text_fields.append(decode_dialogue(rom, pointer))
    commands = [read_u32(rom, prompt + 16 + index * 4) for index in range(3)]
    return prompt, resource_bundle, text_fields, commands


def command_dispatch_target(rom: bytes, command: int) -> str:
    """Resolve a prompt's small mode-command index to its ROM handler."""
    if command < MODE_COMMAND_COUNT:
        return f"0x{read_u32(rom, MODE_COMMAND_TABLE + command * 4):X}"
    if command < len(rom):
        # Commands at or above 0xFA return from the prompt loop. Some menus
        # use that return value as the next prompt-record pointer.
        return f"return:0x{command:X}"
    return ""


def semantic_choice_id(label: str) -> str:
    if not label.strip():
        return ""
    normalized = " ".join(label.rsplit("|", 1)[-1].split())
    if normalized.startswith("<") and normalized.endswith(">"):
        normalized = normalized[1:-1].strip()
    choice_id = CHOICE_IDS_BY_LABEL.get(normalized.casefold())
    if choice_id is None:
        raise ValueError(f"no semantic choice ID for ROM label {label!r}")
    return choice_id


def rolls_for_sum(total: int) -> list[str]:
    return [
        f"{left}+{right}"
        for left in range(RANDOM_BOUND)
        for right in range(RANDOM_BOUND)
        if left + right == total
    ]


def extract(rom: bytes) -> list[dict[str, str]]:
    if len(rom) < OUTCOME_POINTER_TABLE + 16 * OUTCOMES_PER_GROUP * 4:
        raise ValueError("ROM is too short for the world activity tables")
    rows: list[dict[str, str]] = []
    handler_cache: dict[int, tuple[int, int, list[str], list[int]]] = {}
    handler_names: Counter[int] = Counter()
    enabled_cells = 0
    for scene in range(CONTEXT_COUNT):
        group = rom[CONTEXT_GROUP_TABLE + scene]
        if group >= 16:
            raise ValueError(f"scene {scene} has invalid context group {group}")
        for total in range(OUTCOMES_PER_GROUP):
            cell_index = group * OUTCOMES_PER_GROUP + total
            cell_offset = OUTCOME_MASK_TABLE + cell_index
            pointer_offset = OUTCOME_POINTER_TABLE + cell_index * 4
            selected_pointer = read_u32(rom, pointer_offset)
            enabled = rom[cell_offset]
            if enabled not in (0, 1):
                raise ValueError(
                    f"selection cell at 0x{cell_offset:X} is {enabled}, expected 0 or 1"
                )

            if enabled:
                enabled_cells += 1
                handler = read_u32(rom, selected_pointer)
                if handler not in handler_cache:
                    handler_cache[handler] = prompt_fields(rom, handler)
                prompt, resource_bundle, text_fields, commands = handler_cache[handler]
                first_text = text_fields[0]
                event_name = first_text.split("|", 1)[0].strip(" <>\t")
                if not event_name:
                    event_name = f"ROM field event at {handler:06X}"
                outcome_kind = "dialogue_event"
                event_id = EVENT_IDS_BY_HANDLER.get(handler)
                if event_id is None:
                    raise ValueError(
                        f"no semantic event ID for handler 0x{handler:X}"
                    )
                handler_offset = f"0x{handler:X}"
                prompt_offset = f"0x{prompt:X}"
                resource_offset = (
                    f"0x{resource_bundle:X}" if resource_bundle else ""
                )
                target_offset = handler_offset
                opening, choice_a, choice_b, choice_c = text_fields
                choice_values = (choice_a, choice_b, choice_c)
                choice_ids = [semantic_choice_id(value)
                              for value in choice_values]
                choice_routes = []
                for choice_index, choice_text in enumerate(choice_values):
                    command = commands[choice_index]
                    if not choice_text.strip():
                        choice_routes.append(("", ""))
                    else:
                        choice_routes.append((
                            f"0x{command:X}",
                            command_dispatch_target(rom, command),
                        ))
                handler_names[handler] += 1
            else:
                outcome_kind = GENERIC_OUTCOME
                event_id = ""
                event_name = "Dynamic world actor/effect sequence"
                handler_offset = ""
                prompt_offset = ""
                resource_offset = ""
                target_offset = ""
                opening = choice_a = choice_b = choice_c = ""
                choice_routes = [("", ""), ("", ""), ("", "")]

            pairs = rolls_for_sum(total)
            rows.append({
                "scene": str(scene),
                "context_group": str(group),
                "random_sum": str(total),
                "roll_pairs": ", ".join(pairs),
                "roll_pair_count": str(len(pairs)),
                "outcome_kind": outcome_kind,
                "event_id": event_id,
                "event_name": event_name,
                "handler_offset": handler_offset,
                "prompt_record_offset": prompt_offset,
                "resource_bundle_offset": resource_offset,
                "opening_text": opening,
                "choice_1": choice_a,
                "choice_1_id": choice_ids[0] if enabled else "",
                "choice_2": choice_b,
                "choice_2_id": choice_ids[1] if enabled else "",
                "choice_3": choice_c,
                "choice_3_id": choice_ids[2] if enabled else "",
                "choice_1_command": choice_routes[0][0],
                "choice_1_dispatch_target": choice_routes[0][1],
                "choice_2_command": choice_routes[1][0],
                "choice_2_dispatch_target": choice_routes[1][1],
                "choice_3_command": choice_routes[2][0],
                "choice_3_dispatch_target": choice_routes[2][1],
                "map_position": "not fixed by this selector",
                "activation": ACTIVATION,
                "selection_cell_offset": f"0x{cell_offset:X}",
                "selection_pointer_offset": f"0x{pointer_offset:X}",
                "selected_target_offset": target_offset,
            })

    if len(rows) != CONTEXT_COUNT * OUTCOMES_PER_GROUP:
        raise ValueError(f"expected 504 scene/sum outcomes, extracted {len(rows)}")
    if len(handler_cache) != 26:
        raise ValueError(
            f"expected 26 unique handlers, found {len(handler_cache)}"
        )
    if enabled_cells != sum(handler_names.values()):
        raise ValueError("enabled outcome count does not match handler references")
    return rows


def render(rows: list[dict[str, str]]) -> str:
    from io import StringIO

    stream = StringIO(newline="")
    writer = csv.DictWriter(
        stream, fieldnames=FIELDS, delimiter="\t", lineterminator="\n",
        extrasaction="ignore",
    )
    writer.writeheader()
    writer.writerows(rows)
    return stream.getvalue()


def semantic_actor_sequence_id(scene: str, actor_names: list[str]) -> str:
    if not actor_names or any(not name for name in actor_names):
        raise ValueError(
            f"dynamic outcome in scene {scene} has no actor identity"
        )
    actor_ids = [
        re.sub(r"[^a-z0-9]+", "_", name.lower()).strip("_")
        for name in actor_names
    ]
    return f"scene{scene}.world_effect." + ".then.".join(actor_ids)


def read_weighted_actor_sequences(path: Path) -> dict[tuple[str, str, str], str]:
    sequences: dict[tuple[str, str, str], list[tuple[int, str]]] = {}
    try:
        with path.open(encoding="utf-8-sig", newline="") as source:
            for row in csv.DictReader(source, delimiter="\t"):
                if row.get("random_sum") == "255":
                    continue
                key = (row["scene"], row["context_group"], row["random_sum"])
                sequences.setdefault(key, []).append(
                    (int(row["source_order"]), row["actor_name"])
                )
    except (OSError, KeyError, ValueError) as error:
        raise ValueError(f"cannot read dynamic actor catalog {path}: {error}")
    return {
        key: semantic_actor_sequence_id(
            key[0], [actor_name for _, actor_name in sorted(profiles)]
        )
        for key, profiles in sequences.items()
    }


def render_runtime(
    rows: list[dict[str, str]], actor_catalog: Path
) -> str:
    from io import StringIO

    actor_sequences = read_weighted_actor_sequences(actor_catalog)
    # Convert the ROM's two bounded draws into ordinary content weights.
    # Coalesce only adjacent equal outcomes: their positions in the weighted
    # list must still correspond to the source sum buckets when the native
    # adapter maps a bounded pair to a normalized selection fraction.
    outcomes: list[dict[str, str]] = []
    ordered_rows = sorted(
        rows, key=lambda row: (int(row["scene"]), int(row["random_sum"]))
    )
    for row in ordered_rows:
        scene = row["scene"]
        if row["outcome_kind"] == "dialogue_event":
            outcome_id = f"event.{row['event_id']}"
        else:
            actor_key = (scene, row["context_group"], row["random_sum"])
            outcome_id = actor_sequences.get(actor_key, "")
            if not outcome_id:
                raise ValueError(
                    "dynamic field outcome has no semantic actor sequence "
                    f"for scene={scene}, context={actor_key[1]}, "
                    f"sum={actor_key[2]}"
                )
        weight = int(row["roll_pair_count"])
        if (
            outcomes
            and outcomes[-1]["scene_id"] == scene
            and outcomes[-1]["outcome_kind"] == row["outcome_kind"]
            and outcomes[-1]["outcome_id"] == outcome_id
        ):
            outcomes[-1]["chance_weight"] = str(
                int(outcomes[-1]["chance_weight"]) + weight
            )
        else:
            outcomes.append({
                "scene_id": scene,
                "chance_weight": str(weight),
                "outcome_kind": row["outcome_kind"],
                "outcome_id": outcome_id,
            })

    totals: dict[str, int] = {}
    for outcome in outcomes:
        scene = outcome["scene_id"]
        totals[scene] = totals.get(scene, 0) + int(outcome["chance_weight"])
    if len(totals) != CONTEXT_COUNT or any(total != 25 for total in totals.values()):
        raise ValueError("semantic outcome weights must total 25 in every scene")

    stream = StringIO(newline="")
    writer = csv.DictWriter(
        stream, fieldnames=RUNTIME_FIELDS, delimiter="\t", lineterminator="\n",
        extrasaction="ignore",
    )
    writer.writeheader()
    writer.writerows(outcomes)
    return stream.getvalue()


def render_runtime_event_content(rows: list[dict[str, str]]) -> str:
    from io import StringIO

    event_text: dict[str, str] = {}
    for row in rows:
        if row["outcome_kind"] != "dialogue_event":
            continue
        event_id = row["event_id"]
        opening_text = row["opening_text"]
        if not opening_text:
            raise ValueError(f"event {event_id} has no opening text")
        previous = event_text.setdefault(event_id, opening_text)
        if previous != opening_text:
            raise ValueError(f"event {event_id} has conflicting opening text")

    content_rows = [
        {"event_id": event_id, "opening_text": opening_text}
        for event_id, opening_text in sorted(event_text.items())
    ]
    stream = StringIO(newline="")
    writer = csv.DictWriter(
        stream, fieldnames=RUNTIME_EVENT_CONTENT_FIELDS, delimiter="\t",
        lineterminator="\n", extrasaction="ignore",
    )
    writer.writeheader()
    writer.writerows(content_rows)
    return stream.getvalue()


def render_runtime_choices(rows: list[dict[str, str]]) -> str:
    from io import StringIO

    choices_by_event: dict[str, tuple[tuple[str, str], ...]] = {}
    for row in rows:
        if row["outcome_kind"] != "dialogue_event":
            continue
        choices = tuple(
            (row[f"choice_{index}_id"], row[f"choice_{index}"])
            for index in range(1, 4)
            if row[f"choice_{index}"].strip()
        )
        if any(not choice_id for choice_id, _ in choices):
            raise ValueError(
                f"event {row['event_id']} has a player choice without an ID"
            )
        previous = choices_by_event.setdefault(row["event_id"], choices)
        if previous != choices:
            raise ValueError(
                f"event {row['event_id']} has inconsistent choice lists"
            )

    choice_rows = [
        {
            "event_id": event_id,
            "choice_order": str(index),
            "choice_id": choice_id,
            "label": label,
        }
        for event_id, choices in sorted(choices_by_event.items())
        for index, (choice_id, label) in enumerate(choices, start=1)
    ]
    stream = StringIO(newline="")
    writer = csv.DictWriter(
        stream, fieldnames=RUNTIME_CHOICE_FIELDS, delimiter="\t",
        lineterminator="\n", extrasaction="ignore",
    )
    writer.writeheader()
    writer.writerows(choice_rows)
    return stream.getvalue()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--rom", type=Path, default=DEFAULT_ROM)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument(
        "--runtime-output", type=Path, default=DEFAULT_RUNTIME_OUTPUT,
        help="weighted semantic outcomes consumed by gameplay",
    )
    parser.add_argument(
        "--runtime-choices-output", type=Path,
        default=DEFAULT_RUNTIME_CHOICES_OUTPUT,
        help="variable-length semantic choice rows consumed by gameplay",
    )
    parser.add_argument(
        "--runtime-event-content-output", type=Path,
        default=DEFAULT_RUNTIME_EVENT_CONTENT_OUTPUT,
        help="semantic event text rows consumed by gameplay",
    )
    parser.add_argument(
        "--dynamic-actors", type=Path, default=DEFAULT_DYNAMIC_ACTORS,
        help="semantic actor catalog used to name weighted world-effect groups",
    )
    parser.add_argument(
        "--check", action="store_true",
        help="verify the checked-in TSV against a fresh ROM extraction",
    )
    args = parser.parse_args()
    try:
        rows = extract(args.rom.read_bytes())
        content = render(rows)
        runtime_content = render_runtime(rows, args.dynamic_actors)
        runtime_choices_content = render_runtime_choices(rows)
        runtime_event_content = render_runtime_event_content(rows)
    except (OSError, ValueError) as error:
        print(f"world activity extraction failed: {error}", file=sys.stderr)
        return 1
    if args.check:
        try:
            existing = args.output.read_text(encoding="utf-8")
        except OSError as error:
            print(f"cannot read {args.output}: {error}", file=sys.stderr)
            return 1
        if existing != content:
            print(f"ROM extraction differs from {args.output}", file=sys.stderr)
            return 1
        try:
            existing_runtime = args.runtime_output.read_text(encoding="utf-8")
        except OSError as error:
            print(f"cannot read {args.runtime_output}: {error}", file=sys.stderr)
            return 1
        if existing_runtime != runtime_content:
            print(
                f"semantic runtime extraction differs from {args.runtime_output}",
                file=sys.stderr,
            )
            return 1
        try:
            existing_runtime_choices = args.runtime_choices_output.read_text(
                encoding="utf-8"
            )
        except OSError as error:
            print(
                f"cannot read {args.runtime_choices_output}: {error}",
                file=sys.stderr,
            )
            return 1
        if existing_runtime_choices != runtime_choices_content:
            print(
                "semantic runtime choices differ from "
                f"{args.runtime_choices_output}",
                file=sys.stderr,
            )
            return 1
        try:
            existing_runtime_event_content = (
                args.runtime_event_content_output.read_text(encoding="utf-8")
            )
        except OSError as error:
            print(
                f"cannot read {args.runtime_event_content_output}: {error}",
                file=sys.stderr,
            )
            return 1
        if existing_runtime_event_content != runtime_event_content:
            print(
                "semantic runtime event content differs from "
                f"{args.runtime_event_content_output}",
                file=sys.stderr,
            )
            return 1
        print("verified 504 source outcomes, weighted runtime events, and 26 ROM handlers")
        return 0
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(content, encoding="utf-8", newline="\n")
    args.runtime_output.parent.mkdir(parents=True, exist_ok=True)
    args.runtime_output.write_text(
        runtime_content, encoding="utf-8", newline="\n"
    )
    args.runtime_choices_output.parent.mkdir(parents=True, exist_ok=True)
    args.runtime_choices_output.write_text(
        runtime_choices_content, encoding="utf-8", newline="\n"
    )
    args.runtime_event_content_output.parent.mkdir(parents=True, exist_ok=True)
    args.runtime_event_content_output.write_text(
        runtime_event_content, encoding="utf-8", newline="\n"
    )
    print(f"wrote 504 scene/sum outcomes to {args.output}")
    print(f"wrote semantic runtime events to {args.runtime_output}")
    print(f"wrote variable-length event choices to {args.runtime_choices_output}")
    print(f"wrote semantic event text to {args.runtime_event_content_output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
