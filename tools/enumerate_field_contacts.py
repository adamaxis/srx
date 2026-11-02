#!/usr/bin/env python3
"""Enumerate field actor spawn records and import 0x21174 contact rows.

This is an offline ROM importer. The game consumes the generated named Lua
data; it never reads the ROM or reproduces the source pointer arithmetic.

Covered source record sets:
  * all 56 profile default-actor lists selected by 0x3232/profile+0x2A;
  * the named world-activity lists plus each scene alarm list;
  * all 69 one-of-three pedestrian candidate records at 0x1EDB88;
  * every disabled dynamic world-activity outcome list selected through
    0x3312/0x1D70E4/0x1D7174, and the two-row alarm sequence at 0x1E20A.

The dynamic outcome list records are included in the census but their source
template +0x9E bytes are not treated as live contact flags: the common source
spawn path initializes their runtime record separately. Explicit +0x9E list
overrides are honored. The 0x1E20A alarm rows are a separate pointer-only list
whose copied template carries the flags directly.
"""

from __future__ import annotations

import argparse
import csv
import json
import sys
from collections import defaultdict
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_ROM = ROOT / "decompilation/input/shadowrun_genesis_us.bin"
DEFAULT_LUA = ROOT / "decompilation/gameplay/scripts/data/field_contact_dialogue_rows.lua"
DEFAULT_REPORT = ROOT / "decompilation/gameplay/scripts/data/field_contact_enumeration.tsv"

SCENE_PROFILE_TABLE = 0x3232
SCENE_COUNT = 56
DEFAULT_ACTOR_LIST_OFFSET = 0x2A
PROFILE_EFFECT_POINTER_OFFSET = 0x58
EFFECT_ACTOR_LIST_POINTER_OFFSET = 0x10

CONTEXT_GROUP_TABLE = 0x3312
OUTCOME_ENABLE_TABLE = 0x1D70E4
OUTCOME_POINTER_TABLE = 0x1D7174
OUTCOMES_PER_GROUP = 9

CONDITIONAL_TABLE = 0x1EDB88
CONDITIONAL_FIRST_SCENE = 0x15
CONDITIONAL_LAST_SCENE = 0x2B
CONDITIONAL_CHOICES = 3

CONTACT_ROW_TABLE = 0xFE640
CONTACT_SCENE_GROUPS = 0xEEFE
CONTACT_GROUP_FIRST_SCENE = 10
CONTACT_GROUP_LAST_SCENE = 19
CONTACT_ROW_OFFSETS = (0, 8, 12)

WORLD_ACTIVITY_LISTS = (
    (10, 0, 0x17018, "scene_10_event_6_script"),
    (10, 1, 0x17034, "scene_10_default_script"),
    (11, 1, 0x17050, "scene_11_script"),
    (13, 3, 0x1706C, "scene_13_ilene_gate_script"),
    (14, 4, 0x17088, "scene_14_script"),
    (15, 5, 0x170A2, "scene_15_script"),
    (16, 6, 0x170BC, "scene_16_script"),
    (17, 7, 0x170D8, "scene_17_script"),
    (18, 8, 0x170F4, "scene_18_script"),
    (19, 9, 0x16FFC, "scene_19_script"),
    (20, -1, 0xA8CC, "scene_20_ilene_activity"),
    (34, -1, 0xA8AE, "scene_34_story_contact"),
    (47, -1, 0x1E552, "mode_13_entry"),
    (47, -1, 0x1E5C4, "pending_list_1e5c4"),
    (47, -1, 0x1E628, "pending_list_1e628"),
)

ALARM_SEQUENCE_LIST = 0x1E20A
ALARM_TEMPLATE = 0x1D6A1C
RUN_TARGET_GANG_LISTS = (
    (1, 9, 0x1D6D4A, "run_target_area_1_event_9"),
    (2, 9, 0x1D6D5E, "run_target_area_2_event_9"),
    (3, 4, 0x1D6D72, "run_target_area_3_event_4"),
)
BORIS_STRIKE_TEAM_LIST = 0x1D6FD0


@dataclass
class SpawnRecord:
    scene: int
    category: str
    source_list: int
    source_record: int
    source_order: int
    template: int
    actor_name: str
    activity_id: str | None
    flags: int | None
    flag_source: str
    contact_resolution: str = "not_contact_flagged"
    unresolved_reason: str = ""
    classification_reason: str = ""
    classification: str = ""
    actor_role: str = ""
    contact_variants: dict[int, dict[int, dict[str, Any]]] = field(
        default_factory=dict
    )

    @property
    def bit4(self) -> bool:
        return self.flags is not None and bool(self.flags & 0x10)

    @property
    def bit3(self) -> bool:
        return self.flags is not None and bool(self.flags & 0x08)


def checked_offset(rom: bytes, offset: int, size: int) -> int:
    if offset < 0 or offset + size > len(rom):
        raise ValueError(f"ROM range outside image: 0x{offset:X}+0x{size:X}")
    return offset


def read_word(rom: bytes, offset: int) -> int:
    checked_offset(rom, offset, 2)
    return int.from_bytes(rom[offset : offset + 2], "big")


def read_long(rom: bytes, offset: int) -> int:
    checked_offset(rom, offset, 4)
    return int.from_bytes(rom[offset : offset + 4], "big")


def actor_name(rom: bytes, template: int) -> str:
    checked_offset(rom, template, 0x17)
    raw = rom[template + 0x0A : template + 0x17]
    return raw.split(b"\0", 1)[0].decode("ascii", errors="replace").rstrip()


def list_overrides(
    rom: bytes, cursor: int, flags: int, *, max_pairs: int = 128
) -> tuple[int, int]:
    """Apply ordered byte overrides following an optional list marker."""
    next_record = cursor + 8
    if read_long(rom, next_record) != 2:
        return next_record, flags
    pair_offset = next_record + 4
    for _ in range(max_pairs):
        pair = read_word(rom, pair_offset)
        pair_offset += 2
        if pair == 0:
            return pair_offset, flags
        if pair >> 8 == 0x9E:
            flags = pair & 0xFF
    raise ValueError(f"unterminated byte override list at 0x{next_record:X}")


def parse_paired_actor_list(
    rom: bytes,
    *,
    scene: int,
    list_offset: int,
    category: str,
    activity_id: str | None,
    compact_default_templates: bool = False,
    max_records: int = 32,
) -> list[SpawnRecord]:
    rows: list[SpawnRecord] = []
    cursor = list_offset & 0x7FFFFFFF
    found_terminator = False
    for order in range(max_records):
        record_offset = cursor
        tagged_template = read_long(rom, cursor)
        if tagged_template == 0:
            found_terminator = True
            break
        template = tagged_template & 0x7FFFFFFF
        position_pointer = read_long(rom, cursor + 4)
        checked_offset(rom, template, 0x4A)
        checked_offset(rom, position_pointer, 4)
        if compact_default_templates:
            # 0x44BC only copies compact default actor templates through +0x49.
            flags = 0
            flag_source = "compact default template has no +0x9E; ordered override"
        else:
            checked_offset(rom, template, 0x9F)
            flags = rom[template + 0x9E]
            flag_source = f"template+0x9E=0x{flags:02X}, then ordered overrides"
        next_record, flags = list_overrides(rom, cursor, flags)
        rows.append(
            SpawnRecord(
                scene=scene,
                category=category,
                source_list=list_offset,
                source_record=record_offset,
                source_order=order,
                template=template,
                actor_name=actor_name(rom, template),
                activity_id=activity_id,
                flags=flags,
                flag_source=flag_source,
            )
        )
        if next_record <= cursor:
            raise ValueError(f"actor list at 0x{list_offset:X} did not advance")
        cursor = next_record
    if not found_terminator:
        raise ValueError(f"actor list at 0x{list_offset:X} has no null sentinel")
    return rows


def parse_dynamic_outcome_list(
    rom: bytes, *, scene: int, list_offset: int, max_records: int = 32
) -> list[SpawnRecord]:
    """Walk candidate records from the scene's disabled world-event cells."""
    rows: list[SpawnRecord] = []
    cursor = list_offset
    found_terminator = False
    names: list[str] = []
    temporary: list[tuple[int, int, int, str, int]] = []
    for order in range(max_records):
        record_offset = cursor
        tagged_template = read_long(rom, cursor)
        if tagged_template == 0:
            found_terminator = True
            break
        template = tagged_template & 0x7FFFFFFF
        checked_offset(rom, template, 0x4A)
        name = actor_name(rom, template)
        # Dynamic event creation runs through the common source spawner. The
        # template's contact byte is not copied as the live value; 0x1E25A
        # sets its activity bit, and only an explicit ordered +0x9E override
        # can introduce bit 3/4 in this record's final source state.
        flags = 0
        next_record = cursor + 4
        if read_long(rom, next_record) == 2:
            pair_offset = next_record + 4
            for _ in range(128):
                pair = read_word(rom, pair_offset)
                pair_offset += 2
                if pair == 0:
                    break
                if pair >> 8 == 0x9E:
                    flags = pair & 0xFF
            else:
                raise ValueError(
                    f"unterminated dynamic override list at 0x{next_record:X}"
                )
            next_record = pair_offset
        temporary.append((record_offset, template, order, name, flags))
        names.append(name)
        if next_record <= cursor:
            raise ValueError(f"dynamic list at 0x{list_offset:X} did not advance")
        cursor = next_record
    if not found_terminator:
        raise ValueError(f"dynamic list at 0x{list_offset:X} has no null sentinel")
    activity_id = "scene" + str(scene) + ".world_effect." + ".then.".join(
        "_".join("".join(ch.lower() if ch.isalnum() else " " for ch in name).split())
        for name in names
    )
    for record_offset, template, order, name, flags in temporary:
        rows.append(
            SpawnRecord(
                scene=scene,
                category="dynamic_world_activity_candidate",
                source_list=list_offset,
                source_record=record_offset,
                source_order=order,
                template=template,
                actor_name=name,
                activity_id=activity_id,
                flags=flags,
                flag_source=(
                    "dynamic spawner 0x1E25A; only explicit +0x9E override"
                ),
            )
        )
    return rows


def parse_pointer_only_list(
    rom: bytes,
    *,
    scene: int,
    list_offset: int,
    category: str,
    activity_id: str,
    flag_source: str,
    max_records: int = 32,
) -> list[SpawnRecord]:
    """Walk actor lists that do not provide a live +0x9E contact byte."""
    rows: list[SpawnRecord] = []
    cursor = list_offset
    found_terminator = False
    for order in range(max_records):
        tagged_template = read_long(rom, cursor)
        if tagged_template == 0:
            found_terminator = True
            break
        template = tagged_template & 0x7FFFFFFF
        checked_offset(rom, template, 0x4A)
        rows.append(
            SpawnRecord(
                scene=scene,
                category=category,
                source_list=list_offset,
                source_record=cursor,
                source_order=order,
                template=template,
                actor_name=actor_name(rom, template),
                activity_id=activity_id,
                flags=None,
                flag_source=flag_source,
                contact_resolution="unresolved_flag_source",
                classification="ENEMY",
                actor_role="RUN_TARGET_ENCOUNTER",
                classification_reason=(
                    f"encounter actor selected from pointer-only list "
                    f"0x{list_offset:X} through "
                    f"{'0x1F086 run-target dispatch' if category == 'run_target_gang_actor' else '0x21BE4 -> 0x1E216 strike-team dispatch'}"
                ),
                unresolved_reason=(
                    "pointer-only actor list does not provide the final live "
                    "+0x9E byte; contact bits remain unresolved"
                ),
            )
        )
        cursor += 4
    if not found_terminator:
        raise ValueError(
            f"pointer-only actor list at 0x{list_offset:X} has no null sentinel"
        )
    return rows


def decode_dialogue(rom: bytes, pointer: int) -> str:
    checked_offset(rom, pointer, 1)
    segments = [""]
    for byte in rom[pointer : min(len(rom), pointer + 4096)]:
        if byte == 0xFF:
            return "|".join(segments).strip()
        if byte >= 0x80:
            segments.append("")
        elif 0x20 <= byte <= 0x7E:
            segments[-1] += chr(byte)
        else:
            raise ValueError(
                f"unsupported dialogue byte 0x{byte:02X} at 0x{pointer:X}"
            )
    raise ValueError(f"dialogue at 0x{pointer:X} has no 0xFF terminator")


def resolve_response(
    rom: bytes, *, variant: int, row_offset: int
) -> dict[str, Any]:
    # 0x21174 scales FFF0AE by 16 and adds the signed-selector byte offset.
    row_pointer_cell = CONTACT_ROW_TABLE + variant * 0x10 + row_offset
    row_pointer = read_long(rom, row_pointer_cell)
    if row_pointer < 0x10000 or row_pointer >= len(rom):
        raise ValueError(f"row pointer 0x{row_pointer:X} is not ROM data")
    text_pointer = read_long(rom, row_pointer)
    if text_pointer < 0x10000 or text_pointer >= len(rom):
        raise ValueError(
            f"script at 0x{row_pointer:X} has invalid first text pointer "
            f"0x{text_pointer:X}"
        )
    opening = decode_dialogue(rom, text_pointer)
    script_texts: list[dict[str, Any]] = []
    # The row resource is a source script table. Preserve its leading prompt
    # and response text pointers for evidence; non-text resource pointers are
    # ignored because the native runtime needs named text, not ROM controls.
    for index in range(32):
        pointer = read_long(rom, row_pointer + index * 4)
        if pointer < 0x10000 or pointer >= len(rom):
            continue
        try:
            text = decode_dialogue(rom, pointer)
        except ValueError:
            continue
        script_texts.append(
            {"index": index, "text_pointer": pointer, "text": text}
        )
    return {
        "row_pointer": row_pointer,
        "text_pointer": text_pointer,
        "opening_text": opening,
        "script_texts": script_texts,
    }


def generic_variants_for_record(
    rom: bytes, record: SpawnRecord
) -> list[int] | None:
    scene = record.scene
    if scene == 10:
        # 0xED0E..0xED2A selects 2 or 4 from FFFBF9. The two named actor
        # lists are gated by that same value: 0x17018 is the event-6/2 list,
        # while 0x17034 is the default/4 list. Dynamic alarms can use either.
        if record.source_list == 0x17018:
            return [2]
        if record.source_list == 0x17034:
            return [4]
        return [2, 4]
    if CONTACT_GROUP_FIRST_SCENE <= scene <= CONTACT_GROUP_LAST_SCENE:
        return [rom[CONTACT_SCENE_GROUPS + scene - CONTACT_GROUP_FIRST_SCENE]]
    return None


def row_offset_id(offset: int) -> str:
    return {0: "default", 8: "high", 12: "negative"}[offset]


def mark_contact_rows(rom: bytes, records: list[SpawnRecord]) -> None:
    for record in records:
        if not (record.bit4 or record.bit3) and record.flags is not None:
            continue
        if record.bit3:
            if record.source_list == 0xA8CC:
                record.contact_resolution = "resolved_scripted_route"
                record.classification = "SCRIPTED"
                record.actor_role = "scene_20_ilene_activity"
                record.classification_reason = (
                    "bit 3 dispatches the Scene-20 pending world-effect "
                    "callback 0x54402; no 0x21174/0xFE640 row"
                )
            elif record.source_list == 0xA8AE:
                record.contact_resolution = "resolved_scripted_route"
                record.classification = "SCRIPTED"
                record.actor_role = "scene_34_story_contact"
                record.classification_reason = (
                    "bit 3 dispatches the Scene-34 active world-effect "
                    "callback 0x53C90; no 0x21174/0xFE640 row"
                )
            elif record.category == "conditional_pedestrian_candidate":
                record.actor_role = "PEDESTRIAN"
                if record.activity_id and record.activity_id.endswith(
                    "_unreachable"
                ):
                    record.contact_resolution = "non_talkable_source_gate"
                    record.classification = "NON_TALKABLE"
                    record.classification_reason = (
                        "choice 3 is not reachable: 0xA84C requires "
                        "FFF BED=5 and FFDF40=2; 0xA86E supplies random "
                        "bound 2 to 0x0D3A and 0xA878 adds 1, so only "
                        "choices 1/2 spawn"
                    )
                else:
                    record.contact_resolution = "resolved_other_route"
                    record.classification = "OTHER_ROUTE"
                    record.classification_reason = (
                        "reachable bit-3 pedestrian uses the ordinary "
                        "0x1F146 callback, not the generic 0x21174 row path"
                    )
            else:
                record.contact_resolution = "unresolved_alternate_route"
                record.classification = "OTHER_ROUTE"
                record.unresolved_reason = (
                    "bit 3 dispatches the 0xECCC one-shot activity path "
                    "(0x1F146 or selected world-effect callback), not 0x21174"
                )
            continue
        if not record.bit4:
            continue

        variants = generic_variants_for_record(rom, record)
        if variants is None:
            record.contact_resolution = "unresolved_generic_row"
            record.classification = "ENEMY"
            record.actor_role = "CORP_GUARD"
            record.classification_reason = (
                "ROM actor name is corp guard; the record is in an alarm "
                "reinforcement or 0x1E20A sequence; its generic row is not "
                "safely resolved for this scene"
            )
            record.unresolved_reason = (
                "0xECCC only has a documented row-group table for scenes "
                "10..19 (0xEEFE..0xEF07); this scene indexes outside that span"
            )
            continue
        if any(variant > 9 for variant in variants):
            record.contact_resolution = "unresolved_generic_row"
            record.classification = "ENEMY"
            record.actor_role = "CORP_GUARD"
            record.classification_reason = (
                "ROM actor name is corp guard; the record is in an alarm "
                "reinforcement or 0x1E20A sequence; its generic row is not "
                "safely resolved for this scene"
            )
            record.unresolved_reason = (
                "0xECCC supplies a scene byte outside the ten 0xFE640 row "
                "groups; no safe dialogue-row resolution"
            )
            continue
        try:
            resolved: dict[int, dict[int, dict[str, Any]]] = {}
            for variant in variants:
                resolved[variant] = {
                    row_offset: resolve_response(
                        rom, variant=variant, row_offset=row_offset
                    )
                    for row_offset in CONTACT_ROW_OFFSETS
                }
            record.contact_variants = resolved
            record.contact_resolution = "resolved_21174"
            record.classification = "GENERIC_CONTACT"
            record.actor_role = "FIELD_CONTACT"
            record.unresolved_reason = ""
        except ValueError as error:
            record.contact_resolution = "unresolved_generic_row"
            record.classification = "ENEMY"
            record.actor_role = "CORP_GUARD"
            record.classification_reason = (
                "ROM actor name is corp guard; the record is in an alarm "
                "reinforcement or 0x1E20A sequence; its generic row is not "
                "safely resolved for this scene"
            )
            record.unresolved_reason = str(error)


def collect_records(rom: bytes) -> tuple[list[SpawnRecord], dict[str, Any]]:
    records: list[SpawnRecord] = []
    source_totals: dict[str, int] = defaultdict(int)
    for scene in range(SCENE_COUNT):
        profile = read_long(rom, SCENE_PROFILE_TABLE + scene * 4) & 0x7FFFFFFF
        list_offset = read_long(rom, profile + DEFAULT_ACTOR_LIST_OFFSET)
        default_rows = parse_paired_actor_list(
            rom,
            scene=scene,
            list_offset=list_offset,
            category="profile_default",
            activity_id=None,
            compact_default_templates=True,
        )
        records.extend(default_rows)
        source_totals["profile_default"] += len(default_rows)

    for scene, _script_index, list_offset, activity_id in WORLD_ACTIVITY_LISTS:
        rows = parse_paired_actor_list(
            rom,
            scene=scene,
            list_offset=list_offset,
            category="world_activity_script",
            activity_id=activity_id,
        )
        records.extend(rows)
        source_totals["world_activity_script"] += len(rows)

    for scene in range(21, 44):
        profile = read_long(rom, SCENE_PROFILE_TABLE + scene * 4) & 0x7FFFFFFF
        effect = read_long(rom, profile + PROFILE_EFFECT_POINTER_OFFSET)
        if effect == 0:
            raise ValueError(f"scene {scene} has no field effect profile")
        list_offset = read_long(
            rom, effect + EFFECT_ACTOR_LIST_POINTER_OFFSET
        )
        rows = parse_paired_actor_list(
            rom,
            scene=scene,
            list_offset=list_offset,
            category="field_alarm_reinforcement",
            activity_id=f"field_alarm_scene_{scene}",
        )
        records.extend(rows)
        source_totals["field_alarm_reinforcement"] += len(rows)

    for scene in range(CONDITIONAL_FIRST_SCENE, CONDITIONAL_LAST_SCENE + 1):
        scene_table_pointer = (
            read_long(
                rom,
                CONDITIONAL_TABLE
                + (scene - CONDITIONAL_FIRST_SCENE) * 4,
            )
            & 0x7FFFFFFF
        )
        for choice in range(1, CONDITIONAL_CHOICES + 1):
            list_offset = read_long(
                rom, scene_table_pointer + (choice - 1) * 4
            )
            candidate_id = (
                f"scene_{scene}_pedestrian_"
                + ("first" if choice == 1 else "second" if choice == 2 else "unreachable")
            )
            rows = parse_paired_actor_list(
                rom,
                scene=scene,
                list_offset=list_offset,
                category="conditional_pedestrian_candidate",
                activity_id=candidate_id,
            )
            if len(rows) != 1:
                raise ValueError(
                    f"conditional scene {scene} choice {choice} has {len(rows)} actors"
                )
            records.extend(rows)
            source_totals["conditional_pedestrian_candidate"] += len(rows)

    for scene in range(SCENE_COUNT):
        context_group = rom[CONTEXT_GROUP_TABLE + scene]
        if context_group >= 16:
            raise ValueError(
                f"scene {scene} has invalid world-activity group {context_group}"
            )
        for total in range(OUTCOMES_PER_GROUP):
            cell = context_group * OUTCOMES_PER_GROUP + total
            enabled = rom[OUTCOME_ENABLE_TABLE + cell]
            if enabled not in (0, 1):
                raise ValueError(
                    f"world-activity cell 0x{OUTCOME_ENABLE_TABLE + cell:X} "
                    f"is {enabled}, not boolean"
                )
            if enabled:
                continue
            list_offset = read_long(rom, OUTCOME_POINTER_TABLE + cell * 4)
            rows = parse_dynamic_outcome_list(
                rom, scene=scene, list_offset=list_offset
            )
            records.extend(rows)
            source_totals["dynamic_world_activity_candidate"] += len(rows)

    # 0x1E1DA selects the fixed two-template sequence at 0x1E20A in every
    # field context. Its source template's +0x9E byte is copied to live actors.
    for scene in range(SCENE_COUNT):
        names: list[str] = []
        tagged_templates: list[tuple[int, int]] = []
        for order in range(32):
            record_offset = ALARM_SEQUENCE_LIST + order * 4
            tagged = read_long(rom, record_offset)
            if tagged == 0:
                break
            template = tagged & 0x7FFFFFFF
            checked_offset(rom, template, 0x9F)
            names.append(actor_name(rom, template))
            tagged_templates.append((record_offset, template))
        else:
            raise ValueError("alarm sequence has no null sentinel")
        activity_id = "scene" + str(scene) + ".world_effect." + ".then.".join(
            "_".join("".join(ch.lower() if ch.isalnum() else " " for ch in name).split())
            for name in names
        )
        for order, (record_offset, template) in enumerate(tagged_templates):
            flags = rom[template + 0x9E]
            records.append(
                SpawnRecord(
                    scene=scene,
                    category="dynamic_alarm_sequence",
                    source_list=ALARM_SEQUENCE_LIST,
                    source_record=record_offset,
                    source_order=order,
                    template=template,
                    actor_name=actor_name(rom, template),
                    activity_id=activity_id,
                    flags=flags,
                    flag_source=f"template 0x{template:X}+0x9E=0x{flags:02X}",
                )
            )
            source_totals["dynamic_alarm_sequence"] += 1

    # ROM 0x1F086 selects these pointer-only lists for a gang-contract target.
    # Their typed source records import the actor profile but no live +0x9E
    # contact byte, so retain the actors and report that flag source as open.
    for scene, event_id, list_offset, list_id in RUN_TARGET_GANG_LISTS:
        rows = parse_pointer_only_list(
            rom,
            scene=scene,
            list_offset=list_offset,
            category="run_target_gang_actor",
            activity_id=f"run_target.scene_{scene}.event_{event_id}",
            flag_source=(
                f"pointer-only 0x{list_offset:X} list selected by 0x1F086; "
                "+0x9E live copy is unresolved"
            ),
        )
        records.extend(rows)
        source_totals["run_target_gang_actor"] += len(rows)

    # ROM 0x21BE4 passes the two-row 0x1D6FD0 list to 0x1E216 with
    # map-derived placement; its source record is also pointer-only.
    rows = parse_pointer_only_list(
        rom,
        scene=2,
        list_offset=BORIS_STRIKE_TEAM_LIST,
        category="boris_strike_team_actor",
        activity_id="scene_2.boris_strike_team",
        flag_source=(
            "pointer-only 0x1D6FD0 list passed by 0x21BE4 to 0x1E216; "
            "+0x9E live copy is unresolved"
        ),
    )
    records.extend(rows)
    source_totals["boris_strike_team_actor"] += len(rows)

    mark_contact_rows(rom, records)
    summary = {
        "source_totals": dict(sorted(source_totals.items())),
        "total_spawn_records": len(records),
        "source_sets": {
            "profile_default": "0x3232 scene profiles, profile+0x2A",
            "world_activity_script": "named 0xA8AE/0xA8CC/0x16FFC..0x1E628 lists",
            "field_alarm_reinforcement": "scene profile+0x58, effect profile+0x10",
            "conditional_pedestrian_candidate": "0x1EDB88 scene/choice records",
            "dynamic_world_activity_candidate": "0x3312/0x1D70E4/0x1D7174 disabled cells",
            "dynamic_alarm_sequence": "0x1E1DA -> 0x1E20A in each of 56 field contexts",
            "run_target_gang_actor": "0x1F086 -> pointer lists 0x1D6D4A/0x1D6D5E/0x1D6D72",
            "boris_strike_team_actor": "0x21BE4 -> 0x1E216 -> pointer list 0x1D6FD0",
        },
    }
    return records, summary


def response_id(scene: int, variant: int, offset: int) -> str:
    if scene == 10 and variant == 4 and offset == 0:
        return "scene_10_default_neutral"
    return f"scene_{scene}_group_{variant}_{row_offset_id(offset)}"


def build_lua_model(records: list[SpawnRecord], summary: dict[str, Any]) -> dict[str, Any]:
    by_activity: dict[str, dict[str, Any]] = {}
    responses: dict[str, dict[str, Any]] = {}
    report_records: list[dict[str, Any]] = []

    for record in records:
        if not (record.bit4 or record.bit3) and record.flags is not None:
            continue
        entry: dict[str, Any] = {
            "scene_id": record.scene,
            "category": record.category,
            "source_list": f"0x{record.source_list:X}",
            "source_record": f"0x{record.source_record:X}",
            "source_order": record.source_order,
            "template_offset": f"0x{record.template:X}",
            "actor_name": record.actor_name,
            "activity_id": record.activity_id or "",
            "source_spawn_state_flags": record.flags,
            "flag_source": record.flag_source,
            "bit4": record.bit4,
            "bit3": record.bit3,
            "classification": record.classification,
            "actor_role": record.actor_role,
            "classification_reason": record.classification_reason,
            "contact_resolution": record.contact_resolution,
            "unresolved_reason": record.unresolved_reason,
        }
        report_records.append(entry)
        if not record.activity_id:
            continue
        if record.contact_resolution == "resolved_21174":
            route = {
                "scene_id": record.scene,
                "handler": "ordinary",
                "action": "generic_dialogue",
                "classification": record.classification,
                "callback_address": "0x21174",
                "source_records": [],
                "rows_by_variant": {},
            }
        elif record.contact_resolution == "resolved_other_route":
            route = {
                "scene_id": record.scene,
                "handler": "ordinary",
                "action": "random_pedestrian_contact",
                "classification": record.classification,
                "callback_address": "0x1F146",
                "source_records": [],
            }
        elif record.contact_resolution == "resolved_scripted_route":
            is_ilene = record.source_list == 0xA8CC
            route = {
                "scene_id": record.scene,
                "handler": "pending_world_effect" if is_ilene else "world_effect",
                "action": "ilene_activity" if is_ilene else "aragorn_contact",
                "classification": record.classification,
                "callback_address": "0x54402" if is_ilene else "0x53C90",
                "mark_escort": True,
                "source_records": [],
            }
        else:
            continue
        activity = by_activity.setdefault(record.activity_id, route)
        if activity["scene_id"] != record.scene:
            raise ValueError(
                f"activity ID {record.activity_id!r} is reused in multiple scenes"
            )
        activity["source_records"].append(
            {
                "category": record.category,
                "source_list": f"0x{record.source_list:X}",
                "source_record": f"0x{record.source_record:X}",
                "template_offset": f"0x{record.template:X}",
                "actor_name": record.actor_name,
                "classification": record.classification,
                "contact_resolution": record.contact_resolution,
            }
        )
        if record.contact_resolution != "resolved_21174":
            continue
        for variant, rows in record.contact_variants.items():
            variant_rows = activity["rows_by_variant"].setdefault(variant, {})
            for offset, response in rows.items():
                content_id = response_id(record.scene, variant, offset)
                variant_rows[offset] = content_id
                responses[content_id] = {
                    "scene_id": record.scene,
                    "variant": variant,
                    "row_offset": offset,
                    "row_pointer": f"0x{response['row_pointer']:X}",
                    "text_pointer": f"0x{response['text_pointer']:X}",
                    "opening_text": response["opening_text"],
                    "script_texts": [
                        {
                            "index": text["index"],
                            "text_pointer": f"0x{text['text_pointer']:X}",
                            "text": text["text"],
                        }
                        for text in response["script_texts"]
                    ],
                }

    target_records = [
        record for record in records
        if (record.flags is None or record.bit4 or record.bit3)
        and record.contact_resolution != "resolved_21174"
    ]
    classification_counts: dict[str, int] = {
        name: 0
        for name in ("ENEMY", "PEDESTRIAN", "SCRIPTED", "OTHER_ROUTE", "NON_TALKABLE")
    }
    actor_role_counts: dict[str, int] = defaultdict(int)
    for record in target_records:
        if record.classification:
            classification_counts[record.classification] += 1
        if record.actor_role:
            actor_role_counts[record.actor_role] += 1
    resolved_target_records = sum(
        record.contact_resolution in {
            "resolved_other_route", "resolved_scripted_route"
        }
        for record in target_records
    )
    unresolved_target_records = sum(
        record.contact_resolution.startswith("unresolved_")
        for record in target_records
    )
    non_talkable_target_records = sum(
        record.contact_resolution == "non_talkable_source_gate"
        for record in target_records
    )
    counts = {
        "total_spawn_records": summary["total_spawn_records"],
        "bit4_records": sum(record.bit4 for record in records),
        "bit3_records": sum(record.bit3 for record in records),
        "resolved_talkable_records": sum(
            record.contact_resolution in {
                "resolved_21174", "resolved_other_route", "resolved_scripted_route"
            }
            for record in records
        ),
        "unresolved_records": sum(
            record.contact_resolution.startswith("unresolved_")
            and (record.flags is None or record.bit4 or record.bit3)
            for record in records
        ),
        "generated_activity_routes": len(by_activity),
        "original_unresolved_census_records": len(target_records),
        "resolved_from_unresolved_records": resolved_target_records,
        "still_unresolved_with_reason": unresolved_target_records,
        "non_talkable_records": non_talkable_target_records,
        "classification_counts": classification_counts,
        "actor_role_counts": dict(sorted(actor_role_counts.items())),
    }
    per_scene: dict[int, dict[str, int]] = {
        scene: {
            "total_spawn_records": 0,
            "bit4_records": 0,
            "bit3_records": 0,
            "resolved_talkable_records": 0,
            "unresolved_records": 0,
        }
        for scene in range(SCENE_COUNT)
    }
    for record in records:
        scene = per_scene[record.scene]
        scene["total_spawn_records"] += 1
        scene["bit4_records"] += int(record.bit4)
        scene["bit3_records"] += int(record.bit3)
        if record.contact_resolution == "resolved_21174":
            scene["resolved_talkable_records"] += 1
        elif record.contact_resolution.startswith("unresolved_") and (
            record.flags is None or record.bit4 or record.bit3
        ):
            scene["unresolved_records"] += 1

    return {
        "counts": counts,
        "per_scene": per_scene,
        "source_totals": summary["source_totals"],
        "source_sets": summary["source_sets"],
        "records": report_records,
        "by_activity_id": by_activity,
        "responses": responses,
    }


def lua_literal(value: Any, *, level: int = 0) -> str:
    indent = "    " * level
    child = "    " * (level + 1)
    if value is None:
        return "nil"
    if value is True:
        return "true"
    if value is False:
        return "false"
    if isinstance(value, (int, float)):
        return str(value)
    if isinstance(value, str):
        return json.dumps(value, ensure_ascii=True)
    if isinstance(value, list):
        if not value:
            return "{}"
        return "{\n" + "\n".join(
            f"{child}{lua_literal(item, level=level + 1)}," for item in value
        ) + f"\n{indent}}}"
    if isinstance(value, dict):
        if not value:
            return "{}"
        parts = []
        for key, item in value.items():
            if isinstance(key, int):
                lua_key = f"[{key}]"
            elif isinstance(key, str) and key.isidentifier():
                lua_key = key
            else:
                lua_key = f"[{json.dumps(str(key))}]"
            parts.append(f"{child}{lua_key} = {lua_literal(item, level=level + 1)},")
        return "{\n" + "\n".join(parts) + f"\n{indent}}}"
    raise TypeError(f"cannot render Lua value {type(value).__name__}")


def render_lua(model: dict[str, Any]) -> str:
    lines = [
        "-- GENERATED by tools/enumerate_field_contacts.py; do not edit.",
        "-- Offline source import: 0x3232/profile+0x2A, 0xA8AE/0xA8CC,",
        "-- 0x1EDB88, 0x3312/0x1D70E4/0x1D7174, and alarm list 0x1E20A.",
        "-- Generic dialogue rows follow 0xECCC -> 0x21174 -> 0xFE640.",
        "local E = require(\"enums\")",
        "local M = {",
        f"    counts = {lua_literal(model['counts'], level=1)},",
        f"    source_totals = {lua_literal(model['source_totals'], level=1)},",
        f"    source_sets = {lua_literal(model['source_sets'], level=1)},",
        f"    per_scene = {lua_literal(model['per_scene'], level=1)},",
        f"    records = {lua_literal(model['records'], level=1)},",
        "    by_activity_id = {",
    ]
    for activity_id, route in sorted(model["by_activity_id"].items()):
        handler_enum = {
            "ordinary": "ORDINARY",
            "world_effect": "WORLD_EFFECT",
            "pending_world_effect": "PENDING_WORLD_EFFECT",
        }[route["handler"]]
        action_enum = {
            "generic_dialogue": "GENERIC_DIALOGUE",
            "random_pedestrian_contact": "RANDOM_PEDESTRIAN_CONTACT",
            "aragorn_contact": "ARAGORN_CONTACT",
            "ilene_activity": "ILENE_ACTIVITY",
        }[route["action"]]
        lines.extend(
            [
                f"        [{json.dumps(activity_id)}] = {{",
                f"            scene_id = {route['scene_id']},",
                f"            handler = E.FieldActorContactHandler.{handler_enum},",
                f"            action = E.FieldActorContactAction.{action_enum},",
                f"            classification = {json.dumps(route['classification'])},",
                f"            callback_address = {json.dumps(route['callback_address'])},",
                "            source_records = "
                + lua_literal(route["source_records"], level=3)
                + ",",
            ]
        )
        if route.get("mark_escort"):
            lines.append("            mark_escort = true,")
        if route["action"] == "generic_dialogue":
            lines.extend(
                [
                    "            generic_dialogue = {",
                    "                signed_selector = E.GenericFieldContact.SIGNED_SELECTOR_NEUTRAL,",
                    "                rows_by_variant = {",
                ]
            )
            for variant, rows in sorted(route["rows_by_variant"].items()):
                lines.append(
                    f"                    [E.GenericFieldContactVariant.GROUP_{variant}] = {{"
                )
                for offset, content_id in sorted(rows.items()):
                    row_constant = {
                        0: "ROW_OFFSET_DEFAULT",
                        8: "ROW_OFFSET_HIGH",
                        12: "ROW_OFFSET_NEGATIVE",
                    }[offset]
                    lines.append(
                        f"                        [E.GenericFieldContact.{row_constant}] = "
                        f"{json.dumps(content_id)},"
                    )
                lines.append("                    },")
            lines.extend(["                },", "            },"])
        lines.append("        },")
    lines.extend(["    },", "    responses = {"])
    for content_id, response in sorted(model["responses"].items()):
        lines.extend(
            [
                f"        [{json.dumps(content_id)}] = {{",
                f"            scene_id = {response['scene_id']},",
                f"            variant = {response['variant']},",
                f"            row_offset = {response['row_offset']},",
                f"            row_pointer = {json.dumps(response['row_pointer'])},",
                f"            text_pointer = {json.dumps(response['text_pointer'])},",
                f"            opening_text = {json.dumps(response['opening_text'], ensure_ascii=True)},",
                "            script_texts = "
                + lua_literal(response["script_texts"], level=3)
                + ",",
                "        },",
            ]
        )
    lines.extend(["    },", "}", "return M", ""])
    return "\n".join(lines)


REPORT_FIELDS = (
    "scene",
    "category",
    "activity_id",
    "source_list",
    "source_record",
    "source_order",
    "actor_name",
    "template_offset",
    "source_spawn_state_flags",
    "flag_source",
    "bit4",
    "bit3",
    "classification",
    "actor_role",
    "classification_reason",
    "contact_resolution",
    "unresolved_reason",
    "resolved_variants",
)


def render_report(records: list[SpawnRecord]) -> str:
    rows: list[dict[str, str]] = []
    for record in records:
        if not (record.bit4 or record.bit3) and record.flags is not None:
            continue
        variants = ",".join(
            f"{variant}:" + ",".join(
                f"{offset}=0x{response['text_pointer']:X}"
                for offset, response in sorted(responses.items())
            )
            for variant, responses in sorted(record.contact_variants.items())
        )
        rows.append(
            {
                "scene": str(record.scene),
                "category": record.category,
                "activity_id": record.activity_id or "",
                "source_list": f"0x{record.source_list:X}",
                "source_record": f"0x{record.source_record:X}",
                "source_order": str(record.source_order),
                "actor_name": record.actor_name,
                "template_offset": f"0x{record.template:X}",
                "source_spawn_state_flags": (
                    "" if record.flags is None else f"0x{record.flags:02X}"
                ),
                "flag_source": record.flag_source,
                "bit4": "" if record.flags is None else str(int(record.bit4)),
                "bit3": "" if record.flags is None else str(int(record.bit3)),
                "classification": record.classification,
                "actor_role": record.actor_role,
                "classification_reason": record.classification_reason,
                "contact_resolution": record.contact_resolution,
                "unresolved_reason": record.unresolved_reason,
                "resolved_variants": variants or "none",
            }
        )
    from io import StringIO

    stream = StringIO(newline="")
    writer = csv.DictWriter(
        stream,
        fieldnames=REPORT_FIELDS,
        delimiter="\t",
        lineterminator="\n",
        extrasaction="ignore",
    )
    writer.writeheader()
    writer.writerows(rows)
    return stream.getvalue()


def summary_text(model: dict[str, Any]) -> str:
    counts = model["counts"]
    lines = [
        "field contact enumeration (US ROM)",
        "counts: "
        + ", ".join(f"{name}={value}" for name, value in counts.items()),
        "classification counts for the original 253 unresolved records: "
        + ", ".join(
            f"{name}={value}"
            for name, value in counts["classification_counts"].items()
        ),
        "source actor roles in that census: "
        + ", ".join(
            f"{name}={value}"
            for name, value in counts["actor_role_counts"].items()
        ),
        "per scene: scene total bit4 bit3 resolved unresolved",
    ]
    for scene, row in model["per_scene"].items():
        lines.append(
            f"{scene:02d} {row['total_spawn_records']:4d} "
            f"{row['bit4_records']:4d} {row['bit3_records']:4d} "
            f"{row['resolved_talkable_records']:4d} "
            f"{row['unresolved_records']:4d}"
        )
    lines.append("flagged source records:")
    for row in model["records"]:
        variants = ",".join(
            f"{v}:{','.join(str(k) for k in sorted(rows))}"
            for v, rows in sorted(row["contact_variants"].items())
        ) if "contact_variants" in row else ""
        lines.append(
            f"scene={row['scene_id']:02d} flags="
            f"{('unknown' if row['source_spawn_state_flags'] is None else '0x' + format(row['source_spawn_state_flags'], '02X'))} "
            f"actor={row['actor_name']} activity={row['activity_id']} "
            f"list={row['source_list']} record={row['source_record']} "
            f"{row['contact_resolution']} {row['unresolved_reason']}"
        )
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--rom", type=Path, default=DEFAULT_ROM)
    parser.add_argument("--output", type=Path, default=DEFAULT_LUA)
    parser.add_argument("--report-tsv", type=Path, default=DEFAULT_REPORT)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    try:
        rom = args.rom.read_bytes()
        records, summary = collect_records(rom)
        model = build_lua_model(records, summary)
        lua = render_lua(model)
        report = render_report(records)
    except (OSError, ValueError, TypeError) as error:
        print(f"field contact enumeration failed: {error}", file=sys.stderr)
        return 1

    print(summary_text(model))
    if args.check:
        try:
            existing_lua = args.output.read_text(encoding="utf-8")
            existing_report = args.report_tsv.read_text(encoding="utf-8")
        except OSError as error:
            print(f"cannot read generated output: {error}", file=sys.stderr)
            return 1
        if existing_lua != lua or existing_report != report:
            print("ROM enumeration differs from generated Lua/TSV", file=sys.stderr)
            return 1
        print(
            f"verified {model['counts']['total_spawn_records']} spawn records; "
            f"{model['counts']['resolved_talkable_records']} talkable contact routes"
        )
        return 0

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.report_tsv.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(lua, encoding="utf-8", newline="\n")
    args.report_tsv.write_text(report, encoding="utf-8", newline="\n")
    print(f"wrote {args.output}")
    print(f"wrote {args.report_tsv}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
