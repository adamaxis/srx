#!/usr/bin/env python3
"""Inspect extracted field maps and author entity annotations.

The extracted mode-N bitmap is already a composed picture. This tool does not
pretend that pixels identify semantic objects: new regions start unreviewed,
and ROM priority, collision, and event data are shown as independent evidence.
Entity definitions and placements are saved as human-readable TSV files under
gameplay/world so they can be consumed by a native world-data loader later.
"""

from __future__ import annotations

import argparse
import csv
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path
import os
from typing import Iterable

from PyQt6.QtCore import QPointF, QRect, QRectF, Qt
from PyQt6.QtGui import QAction, QActionGroup, QBrush, QColor, QFont, QFontDatabase, QImage, QPainter, QPen, QPixmap
from PyQt6.QtWidgets import (
    QApplication,
    QComboBox,
    QDialog,
    QDialogButtonBox,
    QDockWidget,
    QFormLayout,
    QGraphicsItem,
    QGraphicsItemGroup,
    QGraphicsPixmapItem,
    QGraphicsRectItem,
    QGraphicsScene,
    QGraphicsView,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QListWidget,
    QListWidgetItem,
    QMainWindow,
    QMessageBox,
    QPushButton,
    QPlainTextEdit,
    QScrollArea,
    QSpinBox,
    QVBoxLayout,
    QWidget,
)


def find_project_root() -> Path:
    """Find the checkout when running from source or from a packaged exe."""
    starts = [Path(__file__).resolve()]
    if getattr(sys, "frozen", False):
        starts.insert(0, Path(sys.executable).resolve())
    for start in starts:
        for candidate in (start.parent, *start.parents):
            if (candidate / "decompilation" / "assets" / "field" / "maps" / "index.tsv").is_file():
                return candidate
    # Preserve source-mode defaults so the error can explain which assets are missing.
    return Path(__file__).resolve().parents[2]


ROOT = find_project_root()
DEFAULT_ASSETS = ROOT / "decompilation" / "assets"
DEFAULT_WORLD = ROOT / "decompilation" / "gameplay" / "world"
ENTITY_CATEGORIES = (
    "unreviewed",
    "building",
    "terrain_feature",
    "foliage",
    "decal",
    "prop",
    "structure",
    "foreground_part",
)
PART_ROLES = (
    "unreviewed",
    "body",
    "overhang",
    "shadow",
    "decal",
    "foreground",
    "background_attachment",
)
DRAW_LAYERS = ("ground", "behind_actor", "actor_depth", "foreground")
COLLISION_MODES = ("unreviewed", "blocking", "walkable", "nonblocking")


def read_tsv(path: Path) -> tuple[list[str], list[dict[str, str]]]:
    if not path.is_file():
        return [], []
    with path.open("r", newline="", encoding="utf-8-sig") as stream:
        reader = csv.DictReader(stream, delimiter="\t")
        return list(reader.fieldnames or []), [dict(row) for row in reader]


def write_tsv(path: Path, headers: Iterable[str], rows: Iterable[dict[str, object]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temp = path.with_suffix(path.suffix + ".tmp")
    with temp.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(headers), delimiter="\t", lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)
    temp.replace(path)


def as_int(row: dict[str, str], name: str, default: int = 0) -> int:
    value = row.get(name, "")
    try:
        return int(value, 0) if value else default
    except ValueError:
        return default


def safe_id(value: str) -> str:
    result = re.sub(r"[^a-zA-Z0-9_-]+", "_", value.strip()).strip("_").lower()
    if not result or result in {".", ".."}:
        raise ValueError("Entity ID must contain letters or numbers")
    return result


@dataclass
class MapInfo:
    scene_id: int
    asset: Path
    width: int
    height: int
    priority_asset: Path
    tilemap_width: int = 0
    tilemap_height: int = 0
    metatile_encoding: str = "unknown"
    primary_tilemap: str = ""
    secondary_tilemap: str = ""


@dataclass
class EntityPart:
    part_id: str
    source_kind: str
    source_scene: int
    source_x: int
    source_y: int
    width: int
    height: int
    local_x: int
    local_y: int
    draw_layer: str = "ground"
    role: str = "unreviewed"
    evidence: str = ""
    asset_path: str = ""


@dataclass
class Interactable:
    interaction_id: str
    local_x: int
    local_y: int
    width: int
    height: int
    lua_handler: str = ""
    source_event: str = ""
    evidence: str = ""


@dataclass
class EntityCollisionRegion:
    collision_id: str
    local_x: int
    local_y: int
    width: int
    height: int
    collision_mode: str = "unreviewed"
    evidence: str = ""
    review_status: str = "unreviewed"


@dataclass
class EntityDefinition:
    entity_id: str
    name: str
    category: str = "unreviewed"
    evidence: str = ""
    parts: list[EntityPart] = field(default_factory=list)
    interactables: list[Interactable] = field(default_factory=list)
    collision_regions: list[EntityCollisionRegion] = field(default_factory=list)

    @property
    def bounds(self) -> tuple[int, int, int, int]:
        rectangles = [
            (part.local_x, part.local_y, part.width, part.height)
            for part in self.parts
        ]
        rectangles.extend(
            (item.local_x, item.local_y, item.width, item.height)
            for item in self.interactables
        )
        rectangles.extend(
            (item.local_x, item.local_y, item.width, item.height)
            for item in self.collision_regions
        )
        if not rectangles:
            return (0, 0, 1, 1)
        x0 = min(rect[0] for rect in rectangles)
        y0 = min(rect[1] for rect in rectangles)
        x1 = max(rect[0] + rect[2] for rect in rectangles)
        y1 = max(rect[1] + rect[3] for rect in rectangles)
        return (x0, y0, max(1, x1 - x0), max(1, y1 - y0))


@dataclass
class EntityInstance:
    instance_id: str
    entity_id: str
    origin_x: int
    origin_y: int


@dataclass(frozen=True)
class MapEvidenceRegion:
    kind: str
    x: int
    y: int
    width: int
    height: int
    title: str
    details: str


@dataclass
class MapBehaviorRecord:
    record_id: str
    kind: str
    x: int
    y: int
    width: int = 1
    height: int = 1
    script_module: str = ""
    script_function: str = ""
    condition_module: str = ""
    condition_function: str = ""
    repeat_policy: str = ""
    actor_profile: str = ""
    heading: str = ""
    event_pool: str = ""
    source_record: str = ""
    source_table: str = ""
    evidence: str = ""
    review_status: str = "unreviewed"

    @property
    def title(self) -> str:
        labels = {
            "scripted_trigger": "Map script trigger",
            "actor_spawn": "Authored actor spawn",
            "random_encounter": "Random encounter region",
        }
        return f"{labels.get(self.kind, 'Map behavior')}: {self.record_id}"

    def describe(self, scene_id: int) -> str:
        lines = [
            self.title,
            f"Scene: {scene_id:02d}; region: {self.x},{self.y} {self.width}×{self.height}",
            f"Review: {self.review_status}",
        ]
        if self.kind == "scripted_trigger":
            lines.extend((
                f"Script: {self.script_module or '(unassigned)'}.{self.script_function or '(unassigned)'}",
                f"Condition: {self.condition_module or '(unassigned)'}"
                f".{self.condition_function or '(unassigned)'}",
                f"Repeat policy: {self.repeat_policy or '(unassigned)'}",
            ))
        elif self.kind == "actor_spawn":
            lines.extend((
                f"Actor Lua profile: {self.actor_profile or '(unassigned)'}",
                f"Heading: {self.heading or '(unassigned)'}",
                f"Condition: {self.condition_module or '(unassigned)'}"
                f".{self.condition_function or '(unassigned)'}",
                f"ROM source record: {self.source_record or '(new content / unverified)'}",
            ))
        elif self.kind == "random_encounter":
            lines.extend((
                f"Lua event pool: {self.event_pool or '(unassigned)'}",
                f"Condition: {self.condition_module or '(unassigned)'}"
                f".{self.condition_function or '(unassigned)'}",
                f"ROM source table: {self.source_table or '(new content / unverified)'}",
            ))
        lines.extend((
            f"Evidence: {self.evidence or '(not recorded)'}",
            "This is authored map content; it does not assert that the ROM uses this behavior.",
        ))
        return "\n".join(lines)


class WorldContent:
    ENTITY_HEADER = ("entity_id", "name", "category", "evidence")
    PART_HEADER = (
        "part_id", "source_kind", "source_scene", "source_x", "source_y",
        "width", "height", "local_x", "local_y", "draw_layer", "role", "evidence", "asset_path",
    )
    INTERACTION_HEADER = (
        "interaction_id", "local_x", "local_y", "width", "height",
        "lua_handler", "source_event", "evidence",
    )
    COLLISION_HEADER = (
        "collision_id", "local_x", "local_y", "width", "height",
        "collision_mode", "evidence", "review_status",
    )
    AREA_HEADER = ("instance_id", "entity_id", "origin_x", "origin_y")
    TRIGGER_HEADER = (
        "trigger_id", "x", "y", "width", "height", "script_module",
        "script_function", "condition_module", "condition_function",
        "repeat_policy", "evidence", "review_status",
    )
    SPAWN_HEADER = (
        "spawn_id", "actor_profile", "x", "y", "heading",
        "condition_module", "condition_function", "source_record",
        "evidence", "review_status",
    )
    ENCOUNTER_HEADER = (
        "region_id", "event_pool", "x", "y", "width", "height",
        "condition_module", "condition_function", "source_table",
        "evidence", "review_status",
    )

    def __init__(self, root: Path):
        self.root = root

    def load_entity(self, entity_id: str) -> EntityDefinition | None:
        try:
            entity_id = safe_id(entity_id)
        except ValueError:
            return None
        directory = self.root / "entities" / entity_id
        _, rows = read_tsv(directory / "entity.tsv")
        if not rows:
            return None
        row = rows[0]
        entity = EntityDefinition(
            entity_id=row.get("entity_id", entity_id),
            name=row.get("name", entity_id),
            category=row.get("category", "unreviewed"),
            evidence=row.get("evidence", ""),
        )
        _, part_rows = read_tsv(directory / "parts.tsv")
        entity.parts = [
            EntityPart(
                part_id=row.get("part_id", "part"),
                source_kind=row.get("source_kind", "map"),
                source_scene=as_int(row, "source_scene"),
                source_x=as_int(row, "source_x"),
                source_y=as_int(row, "source_y"),
                width=as_int(row, "width", 1),
                height=as_int(row, "height", 1),
                local_x=as_int(row, "local_x"),
                local_y=as_int(row, "local_y"),
                draw_layer=row.get("draw_layer", "ground"),
                role=row.get("role", "unreviewed"),
                evidence=row.get("evidence", ""),
                asset_path=row.get("asset_path", ""),
            )
            for row in part_rows
        ]
        _, interaction_rows = read_tsv(directory / "interactables.tsv")
        entity.interactables = [
            Interactable(
                interaction_id=row.get("interaction_id", "interaction"),
                local_x=as_int(row, "local_x"),
                local_y=as_int(row, "local_y"),
                width=as_int(row, "width", 1),
                height=as_int(row, "height", 1),
                lua_handler=row.get("lua_handler", ""),
                source_event=row.get("source_event", ""),
                evidence=row.get("evidence", ""),
            )
            for row in interaction_rows
        ]
        _, collision_rows = read_tsv(directory / "collisions.tsv")
        entity.collision_regions = [
            EntityCollisionRegion(
                collision_id=row.get("collision_id", "collision"),
                local_x=as_int(row, "local_x"),
                local_y=as_int(row, "local_y"),
                width=as_int(row, "width", 1),
                height=as_int(row, "height", 1),
                collision_mode=row.get("collision_mode", "unreviewed"),
                evidence=row.get("evidence", ""),
                review_status=row.get("review_status", "unreviewed"),
            )
            for row in collision_rows
        ]
        return entity

    def save_entity(self, entity: EntityDefinition) -> None:
        entity_id = safe_id(entity.entity_id)
        directory = self.root / "entities" / entity_id
        write_tsv(directory / "entity.tsv", self.ENTITY_HEADER, [{
            "entity_id": entity_id,
            "name": entity.name,
            "category": entity.category,
            "evidence": entity.evidence,
        }])
        write_tsv(directory / "parts.tsv", self.PART_HEADER, [part.__dict__ for part in entity.parts])
        write_tsv(
            directory / "interactables.tsv",
            self.INTERACTION_HEADER,
            [interaction.__dict__ for interaction in entity.interactables],
        )
        write_tsv(
            directory / "collisions.tsv",
            self.COLLISION_HEADER,
            [collision.__dict__ for collision in entity.collision_regions],
        )

    def export_part_assets(
        self, assets_root: Path, entities: Iterable[EntityDefinition]
    ) -> None:
        """Materialize each authored visual part as its own named PNG crop."""
        for entity in entities:
            entity_id = safe_id(entity.entity_id)
            for part in entity.parts:
                if part.source_kind == "map_composite":
                    source_path = assets_root / "field" / "maps" / f"mode-{part.source_scene}.bmp"
                elif part.source_kind == "priority_overlay":
                    source_path = assets_root / "field" / "maps" / f"mode-{part.source_scene}-priority-overlay.png"
                else:
                    raise ValueError(
                        f"Unsupported visual-part source kind {part.source_kind!r} "
                        f"for {entity_id}/{part.part_id}"
                    )
                source = QImage(str(source_path))
                if source.isNull():
                    raise ValueError(f"Could not read visual-part source image: {source_path}")
                if (
                    part.width <= 0 or part.height <= 0 or
                    part.source_x < 0 or part.source_y < 0 or
                    part.source_x + part.width > source.width() or
                    part.source_y + part.height > source.height()
                ):
                    raise ValueError(
                        f"Visual part {entity_id}/{part.part_id} exceeds source image bounds"
                    )
                crop = source.copy(QRect(
                    part.source_x, part.source_y, part.width, part.height
                ))
                relative_path = Path("field") / "entities" / entity_id / f"{safe_id(part.part_id)}.png"
                output_path = assets_root / relative_path
                output_path.parent.mkdir(parents=True, exist_ok=True)
                if not crop.save(str(output_path), "PNG"):
                    raise OSError(f"Could not write named entity-part asset: {output_path}")
                part.asset_path = relative_path.as_posix()

    def load_scene(self, scene_id: int) -> list[EntityInstance]:
        _, rows = read_tsv(self.root / "areas" / f"scene-{scene_id:02d}.tsv")
        return [
            EntityInstance(
                row.get("instance_id", "instance"),
                row.get("entity_id", ""),
                as_int(row, "origin_x"),
                as_int(row, "origin_y"),
            )
            for row in rows
        ]

    def save_scene(self, scene_id: int, instances: list[EntityInstance]) -> None:
        write_tsv(
            self.root / "areas" / f"scene-{scene_id:02d}.tsv",
            self.AREA_HEADER,
            [instance.__dict__ for instance in instances],
        )

    def load_behaviors(self, scene_id: int) -> list[MapBehaviorRecord]:
        directory = self.root / "areas"
        records: list[MapBehaviorRecord] = []
        sources = (
            ("scripted_trigger", f"scene-{scene_id:02d}-triggers.tsv", "trigger_id"),
            ("actor_spawn", f"scene-{scene_id:02d}-spawns.tsv", "spawn_id"),
            ("random_encounter", f"scene-{scene_id:02d}-encounters.tsv", "region_id"),
        )
        for kind, filename, id_column in sources:
            _, rows = read_tsv(directory / filename)
            for row in rows:
                records.append(MapBehaviorRecord(
                    record_id=row.get(id_column, ""),
                    kind=kind,
                    x=as_int(row, "x"),
                    y=as_int(row, "y"),
                    width=as_int(row, "width", 1),
                    height=as_int(row, "height", 1),
                    script_module=row.get("script_module", ""),
                    script_function=row.get("script_function", ""),
                    condition_module=row.get("condition_module", ""),
                    condition_function=row.get("condition_function", ""),
                    repeat_policy=row.get("repeat_policy", ""),
                    actor_profile=row.get("actor_profile", ""),
                    heading=row.get("heading", ""),
                    event_pool=row.get("event_pool", ""),
                    source_record=row.get("source_record", ""),
                    source_table=row.get("source_table", ""),
                    evidence=row.get("evidence", ""),
                    review_status=row.get("review_status", "unreviewed"),
                ))
        return records

    def save_behaviors(self, scene_id: int, records: list[MapBehaviorRecord]) -> None:
        directory = self.root / "areas"
        triggers = []
        spawns = []
        encounters = []
        for item in records:
            if item.kind == "scripted_trigger":
                triggers.append({
                    "trigger_id": item.record_id, "x": item.x, "y": item.y,
                    "width": item.width, "height": item.height,
                    "script_module": item.script_module,
                    "script_function": item.script_function,
                    "condition_module": item.condition_module,
                    "condition_function": item.condition_function,
                    "repeat_policy": item.repeat_policy,
                    "evidence": item.evidence, "review_status": item.review_status,
                })
            elif item.kind == "actor_spawn":
                spawns.append({
                    "spawn_id": item.record_id, "actor_profile": item.actor_profile,
                    "x": item.x, "y": item.y, "heading": item.heading,
                    "condition_module": item.condition_module,
                    "condition_function": item.condition_function,
                    "source_record": item.source_record, "evidence": item.evidence,
                    "review_status": item.review_status,
                })
            elif item.kind == "random_encounter":
                encounters.append({
                    "region_id": item.record_id, "event_pool": item.event_pool,
                    "x": item.x, "y": item.y, "width": item.width,
                    "height": item.height, "condition_module": item.condition_module,
                    "condition_function": item.condition_function,
                    "source_table": item.source_table, "evidence": item.evidence,
                    "review_status": item.review_status,
                })
        write_tsv(directory / f"scene-{scene_id:02d}-triggers.tsv", self.TRIGGER_HEADER, triggers)
        write_tsv(directory / f"scene-{scene_id:02d}-spawns.tsv", self.SPAWN_HEADER, spawns)
        write_tsv(directory / f"scene-{scene_id:02d}-encounters.tsv", self.ENCOUNTER_HEADER, encounters)

    @staticmethod
    def lua_quote(value: str) -> str:
        escaped = value.replace("\\", "\\\\").replace('"', '\\"')
        escaped = escaped.replace("\r", "\\r").replace("\n", "\\n").replace("\t", "\\t")
        return f'"{escaped}"'

    @classmethod
    def lua_value(cls, value, indent: int = 0) -> str:
        if isinstance(value, str):
            return cls.lua_quote(value)
        if value is None:
            return "nil"
        if isinstance(value, bool):
            return "true" if value else "false"
        if isinstance(value, (int, float)):
            return repr(value)
        pad = " " * indent
        child_pad = " " * (indent + 2)
        if isinstance(value, list):
            if not value:
                return "{}"
            rows = [f"{child_pad}{cls.lua_value(item, indent + 2)}," for item in value]
            return "{\n" + "\n".join(rows) + f"\n{pad}}}"
        if isinstance(value, dict):
            if not value:
                return "{}"
            rows = []
            for key, item in value.items():
                if isinstance(key, int):
                    rendered_key = f"[{key}]"
                elif isinstance(key, str) and re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", key):
                    rendered_key = key
                elif isinstance(key, str):
                    rendered_key = f"[{cls.lua_quote(key)}]"
                else:
                    raise TypeError(f"Unsupported Lua table key: {key!r}")
                rows.append(
                    f"{child_pad}{rendered_key} = {cls.lua_value(item, indent + 2)},"
                )
            return "{\n" + "\n".join(rows) + f"\n{pad}}}"
        raise TypeError(f"Unsupported Lua table value: {type(value).__name__}")

    def export_lua(self, destination: Path) -> None:
        data = {
            "entities": {},
            "areas": {},
            "map_triggers": {},
            "actor_spawns": {},
            "encounter_regions": {},
        }
        entity_root = self.root / "entities"
        if entity_root.is_dir():
            for directory in sorted(path for path in entity_root.iterdir() if path.is_dir()):
                entity_id = safe_id(directory.name)
                entity = self.load_entity(entity_id)
                if entity is None:
                    continue
                data["entities"][entity.entity_id] = {
                    "id": entity.entity_id,
                    "name": entity.name,
                    "category": entity.category,
                    "evidence": entity.evidence,
                    "parts": [
                        {
                            "id": part.part_id,
                            "source_kind": part.source_kind,
                            "source_scene": part.source_scene,
                            "source_x": part.source_x,
                            "source_y": part.source_y,
                            "width": part.width,
                            "height": part.height,
                            "local_x": part.local_x,
                            "local_y": part.local_y,
                            "draw_layer": part.draw_layer,
                            "role": part.role,
                            "evidence": part.evidence,
                            "asset_path": part.asset_path,
                        }
                        for part in entity.parts
                    ],
                    "interactables": [
                        {
                            "id": item.interaction_id,
                            "local_x": item.local_x,
                            "local_y": item.local_y,
                            "width": item.width,
                            "height": item.height,
                            "lua_handler": item.lua_handler,
                            "source_event": item.source_event,
                            "evidence": item.evidence,
                        }
                        for item in entity.interactables
                    ],
                    "collision_regions": [
                        {
                            "id": item.collision_id,
                            "local_x": item.local_x,
                            "local_y": item.local_y,
                            "width": item.width,
                            "height": item.height,
                            "mode": item.collision_mode,
                            "evidence": item.evidence,
                            "review_status": item.review_status,
                        }
                        for item in entity.collision_regions
                    ],
                }

        area_root = self.root / "areas"
        if area_root.is_dir():
            scene_ids: set[int] = set()
            for path in sorted(area_root.glob("scene-*.tsv")):
                match = re.fullmatch(r"scene-(\d+)(?:-(?:triggers|spawns|encounters))?\.tsv", path.name)
                if not match:
                    continue
                scene_ids.add(int(match.group(1)))
            for scene_id in sorted(scene_ids):
                instances = self.load_scene(scene_id)
                data["areas"][scene_id] = [
                    {
                        "instance_id": item.instance_id,
                        "entity_id": item.entity_id,
                        "origin_x": item.origin_x,
                        "origin_y": item.origin_y,
                    }
                    for item in instances
                ]
                trigger_values = []
                spawn_values = []
                encounter_values = []
                for item in self.load_behaviors(scene_id):
                    if item.kind == "scripted_trigger":
                        trigger_values.append(
                            {
                                "id": item.record_id,
                                "x": item.x,
                                "y": item.y,
                                "width": item.width,
                                "height": item.height,
                                "script_module": item.script_module,
                                "script_function": item.script_function,
                                "condition_module": item.condition_module,
                                "condition_function": item.condition_function,
                                "repeat_policy": item.repeat_policy,
                                "evidence": item.evidence,
                                "review_status": item.review_status,
                            }
                        )
                    elif item.kind == "actor_spawn":
                        spawn_values.append(
                            {
                                "id": item.record_id,
                                "actor_profile": item.actor_profile,
                                "x": item.x,
                                "y": item.y,
                                "heading": item.heading,
                                "condition_module": item.condition_module,
                                "condition_function": item.condition_function,
                                "source_record": item.source_record,
                                "evidence": item.evidence,
                                "review_status": item.review_status,
                            }
                        )
                    elif item.kind == "random_encounter":
                        encounter_values.append(
                            {
                                "id": item.record_id,
                                "event_pool": item.event_pool,
                                "x": item.x,
                                "y": item.y,
                                "width": item.width,
                                "height": item.height,
                                "condition_module": item.condition_module,
                                "condition_function": item.condition_function,
                                "source_table": item.source_table,
                                "evidence": item.evidence,
                                "review_status": item.review_status,
                            }
                        )
                data["map_triggers"][scene_id] = trigger_values
                data["actor_spawns"][scene_id] = spawn_values
                data["encounter_regions"][scene_id] = encounter_values

        content = (
            "-- Generated by ghidra_conversion/tools/map_entity_viewer/map_entity_viewer.py from gameplay/world TSV files.\n"
            "return " + self.lua_value(data) + "\n"
        )
        destination.parent.mkdir(parents=True, exist_ok=True)
        temp = destination.with_suffix(destination.suffix + ".tmp")
        temp.write_text(content, encoding="utf-8", newline="\n")
        temp.replace(destination)


def read_map_catalog(asset_root: Path) -> list[MapInfo]:
    index_path = asset_root / "field" / "maps" / "index.tsv"
    _, rows = read_tsv(index_path)
    maps: list[MapInfo] = []
    for row in rows:
        scene_id = as_int(row, "mode_id", -1)
        if scene_id < 0:
            continue
        asset = asset_root / row.get("asset", "")
        priority = asset_root / row.get("priority_overlay_asset", "")
        if not asset.is_file():
            continue
        maps.append(MapInfo(
            scene_id=scene_id,
            asset=asset,
            width=as_int(row, "image_width"),
            height=as_int(row, "image_height"),
            priority_asset=priority,
            tilemap_width=as_int(row, "tilemap_width"),
            tilemap_height=as_int(row, "tilemap_height"),
            metatile_encoding=row.get("metatile_encoding", "unknown"),
            primary_tilemap=row.get("primary_tilemap", ""),
            secondary_tilemap=row.get("secondary_tilemap", ""),
        ))
    return maps


def read_map_cell_evidence(
    asset_root: Path, info: MapInfo, x: int, y: int
) -> MapEvidenceRegion | None:
    """Describe the ROM tilemap words beneath one rendered 32 px cell."""
    cell_size = 32
    cell_x, cell_y = x // cell_size, y // cell_size
    if x < 0 or y < 0 or cell_x >= info.tilemap_width or cell_y >= info.tilemap_height:
        return None
    index = cell_y * info.tilemap_width + cell_x
    path = asset_root / "records" / "field" / "scenes" / f"scene-{info.scene_id:02d}"
    layers: list[str] = []
    for layer_name, filename in (
        ("Primary layer A", "terrain-layer-a.bin"),
        ("Secondary layer B", "terrain-layer-b.bin"),
    ):
        source = path / filename
        try:
            with source.open("rb") as stream:
                stream.seek(index * 2)
                raw = stream.read(2)
        except OSError:
            raw = b""
        if len(raw) != 2:
            layers.append(f"{layer_name}: unavailable ({source})")
        else:
            value = int.from_bytes(raw, "big")
            layers.append(
                f"{layer_name} metatile word: 0x{value:04X} ({value}); "
                f"record: {source}"
            )
    map_index = asset_root / "field" / "maps" / "index.tsv"
    return MapEvidenceRegion(
        kind="rom_map_cell",
        x=cell_x * cell_size,
        y=cell_y * cell_size,
        width=cell_size,
        height=cell_size,
        title=f"ROM map cell {cell_x},{cell_y}",
        details="\n".join((
            "ROM map cell; raw source tilemap values",
            f"Scene: {info.scene_id:02d}",
            f"Map pixel cell: ({cell_x}, {cell_y}); bounds "
            f"({cell_x * cell_size}, {cell_y * cell_size}) {cell_size}x{cell_size}",
            f"Tilemap dimensions: {info.tilemap_width}x{info.tilemap_height}; "
            f"row-major entry: {index}",
            f"{layers[0]}",
            f"{layers[1]}",
            f"Primary ROM tilemap offset: {info.primary_tilemap or '(not indexed)' }",
            f"Secondary ROM tilemap offset: {info.secondary_tilemap or '(not indexed)' }",
            "Words are source metatile indices, not semantic object classifications.",
            f"Extracted records: {path}",
            f"ROM map index: {map_index}",
        )),
    )


def read_collision_rectangles(asset_root: Path, scene_id: int) -> list[MapEvidenceRegion]:
    path = asset_root / "records" / "field" / "scenes" / f"scene-{scene_id:02d}" / "terrain-collision-rectangles.bin"
    if not path.is_file():
        return []
    data = path.read_bytes()
    if len(data) % 10:
        return []
    result: list[MapEvidenceRegion] = []
    for record_index, offset in enumerate(range(0, len(data), 10)):
        words = [int.from_bytes(data[offset + i:offset + i + 2], "big") for i in range(0, 10, 2)]
        x0, x1, y0, y1, code = words
        if x1 >= x0 and y1 >= y0:
            width, height = x1 - x0 + 1, y1 - y0 + 1
            result.append(MapEvidenceRegion(
                kind="rom_collision",
                x=x0,
                y=y0,
                width=width,
                height=height,
                title=f"ROM terrain collision 0x{code:04X}",
                details=(
                    f"ROM terrain collision\nScene: {scene_id:02d}\n"
                    f"Rectangle: {x0},{y0} {width}×{height}\n"
                    f"Raw bounds: X {x0:04X}..{x1:04X}, Y {y0:04X}..{y1:04X}\n"
                    f"Collision value: 0x{code:04X}\n"
                    f"Record: {record_index} (file offset 0x{offset:04X})\n"
                    f"Source: {path}"
                ),
            ))
    return result


def read_event_rectangles(repo_root: Path, scene_id: int) -> list[MapEvidenceRegion]:
    path = repo_root / "decompilation" / "docs" / "field-map-trigger-records.tsv"
    _, rows = read_tsv(path)
    result: list[MapEvidenceRegion] = []
    for row in rows:
        if as_int(row, "scene_id", -1) != scene_id:
            continue
        x0 = as_int(row, "x_start_raw")
        x1 = as_int(row, "x_end_raw")
        y0 = as_int(row, "y_start_raw")
        y1 = as_int(row, "y_end_raw")
        if x1 >= x0 and y1 >= y0:
            width, height = x1 - x0 + 1, y1 - y0 + 1
            event_id = row.get("event_id_low_5_bits", "")
            dispatcher = row.get("dispatcher_family", "unprofiled") or "unprofiled"
            route = row.get("event_table_route_or_bypass", "") or "unresolved route"
            record_index = row.get("record_index_zero_based", "?")
            record_address = row.get("record_address", "?")
            details = [
                "ROM field trigger",
                f"Scene: {scene_id:02d}; event: {event_id or '(unknown)'}",
                f"Rectangle: {x0},{y0} {width}×{height} pixels",
                f"Grid cells: X {row.get('x_start_grid_cell', '?')}..{row.get('x_end_grid_cell', '?')}, "
                f"Y {row.get('y_start_grid_cell', '?')}..{row.get('y_end_grid_cell', '?')}",
                f"Collision value: {row.get('raw_collision_value', '(unknown)')}",
                f"ROM record: {record_index} at {record_address}",
                f"Profile: {row.get('profile_address', '(unknown)')}; collision table: "
                f"{row.get('collision_records_address', '(unknown)')}",
                f"Route: {route}",
                f"Dispatch: {dispatcher}; target: {row.get('event_table_target_class', '(unknown)')}",
                f"ROM handlers: {row.get('rom_handler_addresses', '(unrecorded)')}",
                f"Converted owners: {row.get('converted_cpp_functions_or_runtime_owners', '(unrecorded)')}",
                f"Physical QA: {row.get('known_physical_qa_route', '(unrecorded)')}",
                f"Evidence table: {path}",
            ]
            result.append(MapEvidenceRegion(
                kind="rom_event",
                x=x0,
                y=y0,
                width=width,
                height=height,
                title=f"ROM event {event_id or '(unknown)'} — {dispatcher}",
                details="\n".join(details),
            ))
    return result


def read_default_actor_positions(asset_root: Path, scene_id: int) -> list[MapEvidenceRegion]:
    """Read ROM default-list coordinates without treating them as scenery."""
    path = asset_root / "field" / "entities" / "default-actors.tsv"
    _, rows = read_tsv(path)
    result: list[MapEvidenceRegion] = []
    for row in rows:
        if as_int(row, "scene", -1) != scene_id:
            continue
        try:
            raw_x = int(row.get("world_x", "0"), 0)
            raw_y = int(row.get("world_y", "0"), 0)
        except ValueError:
            continue
        # Actor world coordinates are 12.4 fixed point. The map editor uses
        # integer world pixels, matching the coordinate shift in the native
        # field projection path.
        x, y = raw_x >> 4, raw_y >> 4
        source_order = as_int(row, "source_order", len(result))
        profile_id = row.get("profile_id", "unprofiled")
        kind = as_int(row, "entity_kind", -1)
        details = "\n".join((
            "ROM default actor-list position",
            f"Scene: {scene_id:02d}; list order: {source_order}",
            f"Actor profile: {profile_id}; ROM kind: {kind}",
            f"World position: ({raw_x:04X}, {raw_y:04X}) fixed point",
            f"Map pixel anchor: ({x}, {y})",
            f"Initial action: {row.get('action_kind', 'unknown')} "
            f"{row.get('initial_action_code', '')}; heading: {row.get('initial_heading', 'unknown')}",
            f"Template: {row.get('template_offset', '(unknown)')}; position record: "
            f"{row.get('position_offset', '(unknown)')}; list: {row.get('list_offset', '(unknown)')}",
            f"Ordered overrides: {row.get('ordered_overrides', '(none)')}",
            "This is a ROM default-list position, not proof that the actor is active on every visit.",
            f"Extracted source record: {path}",
        ))
        result.append(MapEvidenceRegion(
            kind="rom_default_actor",
            x=x - 9,
            y=y - 9,
            width=18,
            height=18,
            title=f"ROM actor {source_order}: {profile_id}",
            details=details,
        ))
    return result


class GridOverlay(QGraphicsItem):
    def __init__(self, width: int, height: int, cell: int = 32):
        super().__init__()
        self.width = width
        self.height = height
        self.cell = cell
        self.setZValue(5)
        self.setAcceptedMouseButtons(Qt.MouseButton.NoButton)

    def boundingRect(self) -> QRectF:
        return QRectF(0, 0, self.width, self.height)

    def paint(self, painter: QPainter, option, widget=None) -> None:
        painter.setPen(QPen(QColor(255, 255, 255, 45), 1))
        for x in range(0, self.width + 1, self.cell):
            painter.drawLine(x, 0, x, self.height)
        for y in range(0, self.height + 1, self.cell):
            painter.drawLine(0, y, self.width, y)


class FieldMapView(QGraphicsView):
    def __init__(self, on_region, on_select, on_cursor, on_evidence, on_behavior, on_cell, on_move):
        super().__init__()
        self.setScene(QGraphicsScene(self))
        self.setRenderHints(QPainter.RenderHint.Antialiasing)
        self.setTransformationAnchor(QGraphicsView.ViewportAnchor.AnchorUnderMouse)
        self.setResizeAnchor(QGraphicsView.ViewportAnchor.AnchorViewCenter)
        self.setDragMode(QGraphicsView.DragMode.NoDrag)
        self.on_region = on_region
        self.on_select = on_select
        self.on_cursor = on_cursor
        self.on_evidence = on_evidence
        self.on_behavior = on_behavior
        self.on_cell = on_cell
        self.on_move = on_move
        self.mode = "select"
        self.map_info: MapInfo | None = None
        self.background_item: QGraphicsPixmapItem | None = None
        self.grid: GridOverlay | None = None
        self.collision_rectangles: list[MapEvidenceRegion] = []
        self.event_rectangles: list[MapEvidenceRegion] = []
        self.actor_positions: list[MapEvidenceRegion] = []
        self.behaviors: list[MapBehaviorRecord] = []
        self.instances: list[EntityInstance] = []
        self.entities: dict[str, EntityDefinition] = {}
        self.show_grid = True
        self.show_collision = False
        self.show_events = True
        self.show_actors = True
        self.show_behaviors = True
        self.show_priority = True
        self.assets_root = DEFAULT_ASSETS
        self.repo_root = ROOT
        self.drag_start: QPointF | None = None
        self.drag_rect: QGraphicsRectItem | None = None
        self.move_start: QPointF | None = None
        self.move_origin = QPointF()
        self.move_instance_id: str | None = None
        self.move_group: QGraphicsItemGroup | None = None
        self.annotation_items: list[QGraphicsItem] = []
        self.behavior_items: list[QGraphicsItem] = []
        self.entity_groups: dict[str, QGraphicsItemGroup] = {}
        self.setBackgroundBrush(QColor(28, 29, 33))

    def load_scene(
        self,
        info: MapInfo,
        assets: Path,
        repo_root: Path,
        instances: list[EntityInstance],
        entities: dict[str, EntityDefinition],
        behaviors: list[MapBehaviorRecord],
    ) -> None:
        self.map_info = info
        self.assets_root = assets
        self.repo_root = repo_root
        self.instances = instances
        self.entities = entities
        self.behaviors = behaviors
        self.collision_rectangles = read_collision_rectangles(assets, info.scene_id)
        self.event_rectangles = read_event_rectangles(repo_root, info.scene_id)
        self.actor_positions = read_default_actor_positions(assets, info.scene_id)
        self.scene().clear()
        base = QImage(str(info.asset)).convertToFormat(QImage.Format.Format_ARGB32)
        if self.show_priority and info.priority_asset.is_file():
            overlay = QImage(str(info.priority_asset)).convertToFormat(QImage.Format.Format_ARGB32)
            if overlay.size() == base.size():
                painter = QPainter(base)
                painter.drawImage(0, 0, overlay)
                painter.end()
        self.background_item = self.scene().addPixmap(QPixmap.fromImage(base))
        self.background_item.setZValue(0)
        self.scene().setSceneRect(0, 0, base.width(), base.height())
        self.annotation_items = []
        self.evidence_items = []
        self.grid = GridOverlay(base.width(), base.height())
        self.grid.setVisible(self.show_grid)
        self.scene().addItem(self.grid)
        self.draw_evidence_overlays()
        self.redraw_annotations()
        self.redraw_behaviors()
        self.resetTransform()
        self.fitInView(self.sceneRect(), Qt.AspectRatioMode.KeepAspectRatio)

    def set_layer_options(self, priority: bool, grid: bool, collision: bool, events: bool, actors: bool, behaviors: bool) -> None:
        changed_priority = self.show_priority != priority
        self.show_priority = priority
        self.show_grid = grid
        self.show_collision = collision
        self.show_events = events
        self.show_actors = actors
        self.show_behaviors = behaviors
        if changed_priority and self.map_info is not None:
            info = self.map_info
            self.load_scene(info, self.assets_root, self.repo_root, self.instances, self.entities, self.behaviors)
            return
        if self.grid:
            self.grid.setVisible(grid)
        self.draw_evidence_overlays()
        self.redraw_behaviors()

    def draw_evidence_overlays(self) -> None:
        for item in getattr(self, "evidence_items", []):
            self.scene().removeItem(item)
        self.evidence_items: list[QGraphicsItem] = []
        if self.show_collision:
            for index, region in enumerate(self.collision_rectangles):
                rect = self.scene().addRect(
                    QRectF(region.x, region.y, region.width, region.height),
                    QPen(QColor(255, 55, 55, 210), 1),
                    QColor(255, 35, 35, 22),
                )
                rect.setZValue(8)
                rect.setData(0, "map_evidence")
                rect.setData(1, "collision")
                rect.setData(2, index)
                rect.setToolTip(region.title)
                self.evidence_items.append(rect)
        if self.show_events:
            for index, region in enumerate(self.event_rectangles):
                rect = self.scene().addRect(
                    QRectF(region.x, region.y, region.width, region.height),
                    QPen(QColor(255, 190, 35, 245), 2),
                    QColor(255, 190, 35, 24),
                )
                rect.setZValue(9)
                rect.setData(0, "map_evidence")
                rect.setData(1, "event")
                rect.setData(2, index)
                rect.setToolTip(region.title)
                self.evidence_items.append(rect)
        if self.show_actors:
            for index, region in enumerate(self.actor_positions):
                marker = self.scene().addEllipse(
                    QRectF(region.x, region.y, region.width, region.height),
                    QPen(QColor(90, 255, 190, 245), 2),
                    QColor(20, 90, 75, 185),
                )
                marker.setZValue(10)
                marker.setData(0, "map_evidence")
                marker.setData(1, "actor")
                marker.setData(2, index)
                marker.setToolTip(region.title)
                self.evidence_items.append(marker)
                label = self.scene().addText(f"A{index}")
                label.setDefaultTextColor(QColor(145, 255, 210))
                label.setPos(region.x + region.width, region.y - 2)
                label.setZValue(10)
                label.setData(0, "map_evidence")
                label.setData(1, "actor")
                label.setData(2, index)
                self.evidence_items.append(label)

    def redraw_annotations(self) -> None:
        for item in self.annotation_items:
            self.scene().removeItem(item)
        self.annotation_items = []
        self.entity_groups = {}
        colors = {
            "unreviewed": QColor(255, 245, 90, 220),
            "building": QColor(80, 210, 255, 225),
            "terrain_feature": QColor(190, 190, 190, 230),
            "foliage": QColor(80, 220, 100, 230),
            "decal": QColor(240, 90, 255, 230),
            "prop": QColor(70, 255, 130, 225),
            "structure": QColor(255, 130, 60, 230),
            "foreground_part": QColor(255, 90, 90, 230),
        }
        role_colors = {
            "unreviewed": QColor(255, 245, 90, 230),
            "body": QColor(80, 210, 255, 225),
            "overhang": QColor(255, 135, 50, 235),
            "shadow": QColor(90, 90, 110, 235),
            "decal": QColor(240, 90, 255, 230),
            "foreground": QColor(255, 90, 90, 235),
            "background_attachment": QColor(90, 220, 130, 235),
        }
        for instance in self.instances:
            entity = self.entities.get(instance.entity_id)
            if not entity:
                continue
            group = QGraphicsItemGroup()
            group.setData(0, "entity_instance")
            group.setData(1, instance.instance_id)
            self.scene().addItem(group)
            self.entity_groups[instance.instance_id] = group
            color = colors.get(entity.category, colors["unreviewed"])
            x0, y0, width, height = entity.bounds
            box = self.scene().addRect(
                QRectF(instance.origin_x + x0, instance.origin_y + y0, width, height),
                QPen(color, 2),
                QColor(color.red(), color.green(), color.blue(), 28),
            )
            box.setZValue(12)
            box.setData(0, instance.instance_id)
            box.setToolTip(f"{entity.name} [{entity.category}]\n{instance.entity_id} @ {instance.origin_x},{instance.origin_y}")
            group.addToGroup(box)
            label = self.scene().addText(f"{entity.name} [{entity.category}]")
            label.setDefaultTextColor(color)
            label.setPos(instance.origin_x + x0 + 2, instance.origin_y + y0 + 2)
            label.setZValue(13)
            label.setData(0, instance.instance_id)
            group.addToGroup(label)
            for part in entity.parts:
                part_color = role_colors.get(part.role, role_colors["unreviewed"])
                part_item = self.scene().addRect(
                    QRectF(
                        instance.origin_x + part.local_x,
                        instance.origin_y + part.local_y,
                        part.width,
                        part.height,
                    ),
                    QPen(part_color, 1, Qt.PenStyle.DashLine),
                    QBrush(Qt.BrushStyle.NoBrush),
                )
                part_item.setZValue(13)
                part_item.setToolTip(
                    f"{entity.entity_id}/{part.part_id}: {part.role} ({part.draw_layer})\n"
                    f"Source scene {part.source_scene:02d} {part.source_kind} at "
                    f"{part.source_x},{part.source_y}, {part.width}x{part.height}\n"
                    f"Entity-local: {part.local_x},{part.local_y}"
                )
                group.addToGroup(part_item)
            for interaction in entity.interactables:
                hit = self.scene().addRect(
                    QRectF(
                        instance.origin_x + interaction.local_x,
                        instance.origin_y + interaction.local_y,
                        interaction.width,
                        interaction.height,
                    ),
                    QPen(QColor(55, 255, 235, 240), 2, Qt.PenStyle.DashLine),
                    QColor(55, 255, 235, 28),
                )
                hit.setZValue(14)
                hit.setToolTip(f"Lua handler: {interaction.lua_handler or '(unassigned)'}\nROM event: {interaction.source_event or '(not mapped)'}")
                hit.setData(0, "entity_interaction")
                hit.setData(1, instance.instance_id)
                hit.setData(2, interaction.interaction_id)
                group.addToGroup(hit)
            for collision in entity.collision_regions:
                region = self.scene().addRect(
                    QRectF(
                        instance.origin_x + collision.local_x,
                        instance.origin_y + collision.local_y,
                        collision.width,
                        collision.height,
                    ),
                    QPen(QColor(190, 255, 55, 245), 2, Qt.PenStyle.DashDotLine),
                    QBrush(Qt.BrushStyle.NoBrush),
                )
                region.setZValue(14)
                region.setToolTip(
                    f"Authored collision: {collision.collision_id}\n"
                    f"Mode: {collision.collision_mode}; review: {collision.review_status}\n"
                    f"Entity-local: {collision.local_x},{collision.local_y} "
                    f"{collision.width}x{collision.height}\n"
                    f"Evidence: {collision.evidence or '(not recorded)'}"
                )
                region.setData(0, "entity_collision")
                region.setData(1, instance.instance_id)
                region.setData(2, collision.collision_id)
                group.addToGroup(region)
            self.annotation_items.append(group)

    def evidence_at(self, point: QPointF) -> MapEvidenceRegion | None:
        """Return the topmost enabled ROM evidence region under the cursor."""
        for item in self.scene().items(point):
            if item.data(0) != "map_evidence":
                continue
            index = item.data(2)
            regions = {
                "event": self.event_rectangles,
                "collision": self.collision_rectangles,
                "actor": self.actor_positions,
            }.get(item.data(1), [])
            if isinstance(index, int) and 0 <= index < len(regions):
                return regions[index]
        return None

    def behavior_at(self, point: QPointF) -> MapBehaviorRecord | None:
        for item in self.scene().items(point):
            if item.data(0) == "map_behavior":
                index = item.data(1)
                if isinstance(index, int) and 0 <= index < len(self.behaviors):
                    return self.behaviors[index]
        return None

    def redraw_behaviors(self) -> None:
        for item in getattr(self, "behavior_items", []):
            self.scene().removeItem(item)
        self.behavior_items = []
        if not self.show_behaviors:
            return
        colors = {
            "scripted_trigger": QColor(215, 90, 255, 235),
            "actor_spawn": QColor(255, 150, 65, 245),
            "random_encounter": QColor(90, 150, 255, 235),
        }
        labels = {
            "scripted_trigger": "T",
            "actor_spawn": "S",
            "random_encounter": "R",
        }
        for index, record in enumerate(self.behaviors):
            color = colors.get(record.kind, QColor(230, 230, 230, 235))
            if record.kind == "actor_spawn":
                item = self.scene().addEllipse(
                    QRectF(record.x - 8, record.y - 8, 16, 16),
                    QPen(color, 2),
                    QColor(color.red(), color.green(), color.blue(), 90),
                )
            else:
                item = self.scene().addRect(
                    QRectF(record.x, record.y, record.width, record.height),
                    QPen(color, 2, Qt.PenStyle.DashLine),
                    QColor(color.red(), color.green(), color.blue(), 30),
                )
            item.setZValue(11)
            item.setData(0, "map_behavior")
            item.setData(1, index)
            item.setToolTip(record.title + " — authored, not ROM evidence")
            label = self.scene().addText(labels.get(record.kind, "?"))
            label.setDefaultTextColor(color)
            label.setPos(record.x + (8 if record.kind == "actor_spawn" else 2), record.y - 2)
            label.setZValue(11)
            label.setData(0, "map_behavior")
            label.setData(1, index)
            label.setToolTip(record.title + " — authored, not ROM evidence")
            self.behavior_items.extend((item, label))

    def entity_group_at(self, point: QPointF) -> tuple[str, QGraphicsItemGroup] | None:
        for item in self.scene().items(point):
            current = item
            while current is not None:
                if current.data(0) == "entity_instance":
                    instance_id = current.data(1)
                    if isinstance(instance_id, str):
                        return instance_id, current
                current = current.parentItem()
        return None

    def wheelEvent(self, event) -> None:
        factor = 1.15 if event.angleDelta().y() > 0 else 1 / 1.15
        self.scale(factor, factor)

    def mousePressEvent(self, event) -> None:
        if event.button() == Qt.MouseButton.MiddleButton:
            self.setDragMode(QGraphicsView.DragMode.ScrollHandDrag)
            super().mousePressEvent(event)
            return
        if event.button() == Qt.MouseButton.LeftButton and self.mode in {"add_entity", "add_part", "add_interaction", "add_collision"}:
            self.drag_start = self.mapToScene(event.position().toPoint())
            self.drag_rect = self.scene().addRect(QRectF(self.drag_start, self.drag_start), QPen(QColor(255, 255, 255), 2, Qt.PenStyle.DashLine))
            self.drag_rect.setZValue(30)
            event.accept()
            return
        if event.button() == Qt.MouseButton.LeftButton and self.mode == "add_actor_spawn":
            point = self.mapToScene(event.position().toPoint())
            self.on_region(QRectF(round(point.x()), round(point.y()), 1, 1))
            event.accept()
            return
        if event.button() == Qt.MouseButton.LeftButton:
            point = self.mapToScene(event.position().toPoint())
            behavior = self.behavior_at(point)
            if behavior is not None:
                self.on_behavior(behavior)
                event.accept()
                return
            evidence = self.evidence_at(point)
            if evidence is not None:
                self.on_evidence(evidence)
                event.accept()
                return
            selected = self.on_select(point)
            entity_group = self.entity_group_at(point)
            if selected is not None and entity_group is not None and entity_group[0] == selected.instance_id:
                self.move_start = point
                self.move_origin = entity_group[1].pos()
                self.move_instance_id = selected.instance_id
                self.move_group = entity_group[1]
                event.accept()
                return
            if selected is None:
                self.on_cell(point)
                event.accept()
                return
        super().mousePressEvent(event)

    def mouseMoveEvent(self, event) -> None:
        point = self.mapToScene(event.position().toPoint())
        self.on_cursor(int(point.x()), int(point.y()))
        if self.move_start is not None and self.move_group is not None:
            self.move_group.setPos(self.move_origin + point - self.move_start)
            event.accept()
            return
        if self.drag_start is not None and self.drag_rect is not None:
            self.drag_rect.setRect(QRectF(self.drag_start, point).normalized())
            event.accept()
            return
        super().mouseMoveEvent(event)

    def mouseReleaseEvent(self, event) -> None:
        if event.button() == Qt.MouseButton.MiddleButton:
            self.setDragMode(QGraphicsView.DragMode.NoDrag)
            event.accept()
            return
        if event.button() == Qt.MouseButton.LeftButton and self.drag_start is not None:
            end = self.mapToScene(event.position().toPoint())
            rect = QRectF(self.drag_start, end).normalized()
            if self.drag_rect is not None:
                self.scene().removeItem(self.drag_rect)
            self.drag_start = None
            self.drag_rect = None
            self.on_region(rect)
            event.accept()
            return
        if event.button() == Qt.MouseButton.LeftButton and self.move_start is not None:
            end = self.mapToScene(event.position().toPoint())
            instance_id = self.move_instance_id
            delta = end - self.move_start
            moved = abs(delta.x()) >= 2 or abs(delta.y()) >= 2
            if self.move_group is not None:
                self.move_group.setPos(self.move_origin)
            self.move_start = None
            self.move_instance_id = None
            self.move_group = None
            if moved and instance_id is not None:
                self.on_move(instance_id, round(delta.x()), round(delta.y()))
            event.accept()
            return
        super().mouseReleaseEvent(event)


class MapEntityViewer(QMainWindow):
    def __init__(self, assets: Path, world: Path, initial_scene: int):
        super().__init__()
        self.assets = assets
        self.world = WorldContent(world)
        self.repo_root = ROOT
        self.maps = read_map_catalog(assets)
        self.map_by_id = {item.scene_id: item for item in self.maps}
        self.scene_id = initial_scene
        self.instances: list[EntityInstance] = []
        self.entities: dict[str, EntityDefinition] = {}
        self.behaviors: list[MapBehaviorRecord] = []
        self.selected_instance: EntityInstance | None = None
        self.selected_behavior: MapBehaviorRecord | None = None
        self.dirty = False
        self.mode = "select"
        self.selected_evidence: MapEvidenceRegion | None = None
        self._loading_fields = False
        self.setWindowTitle("Shadowrun Map and Entity Viewer")
        self.resize(1500, 980)
        self.setFont(QFont("Arial", 9))

        self.map_view = FieldMapView(
            self.region_created,
            self.select_at,
            self.cursor_moved,
            self.show_evidence,
            self.show_behavior,
            self.show_map_cell,
            self.move_instance,
        )
        self.setCentralWidget(self.map_view)
        self._build_menus()
        self._build_dock()
        self._build_toolbar()
        scene_index = self.scene_combo.findData(initial_scene)
        if scene_index >= 0:
            self.scene_combo.setCurrentIndex(scene_index)
        self.load_scene(initial_scene)
        self.statusBar().showMessage("No automatic object classifier is active. New annotations start unreviewed.")

    def _build_menus(self) -> None:
        file_menu = self.menuBar().addMenu("&File")
        save_action = QAction("Save scene and definitions", self)
        save_action.setShortcut("Ctrl+S")
        save_action.triggered.connect(self.save_all)
        file_menu.addAction(save_action)
        file_menu.addSeparator()
        exit_action = QAction("Exit", self)
        exit_action.setShortcut("Ctrl+Q")
        exit_action.triggered.connect(self.close)
        file_menu.addAction(exit_action)

        scene_menu = self.menuBar().addMenu("&Scene")
        self.scene_action_group = QActionGroup(self)
        self.scene_action_group.setExclusive(True)
        self.scene_actions: dict[int, QAction] = {}
        for info in self.maps:
            action = QAction(f"Scene {info.scene_id:02d}  ({info.width} × {info.height})", self)
            action.setCheckable(True)
            action.setData(info.scene_id)
            action.triggered.connect(
                lambda checked=False, scene_id=info.scene_id: self.select_scene(scene_id)
            )
            self.scene_action_group.addAction(action)
            scene_menu.addAction(action)
            self.scene_actions[info.scene_id] = action

    def _build_dock(self) -> None:
        dock = QDockWidget("Map content", self)
        dock.setFeatures(QDockWidget.DockWidgetFeature.NoDockWidgetFeatures)
        panel = QWidget()
        layout = QVBoxLayout(panel)
        self.scene_combo = QComboBox()
        for info in self.maps:
            self.scene_combo.addItem(f"Scene {info.scene_id:02d} — {info.width}×{info.height}", info.scene_id)
        self.scene_combo.currentIndexChanged.connect(self.scene_changed)
        layout.addWidget(QLabel("Preassembled map"))
        layout.addWidget(self.scene_combo)
        self.entity_list = QListWidget()
        self.entity_list.currentItemChanged.connect(self.list_selection_changed)
        layout.addWidget(QLabel("Entity instances on this scene"))
        layout.addWidget(self.entity_list, 1)

        layout.addWidget(QLabel("Owned visual parts"))
        self.parts_list = QListWidget()
        self.parts_list.setMaximumHeight(95)
        self.parts_list.currentItemChanged.connect(self.part_selection_changed)
        layout.addWidget(self.parts_list)
        self.part_preview = QLabel("Select a visual part to preview its named asset.")
        self.part_preview.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.part_preview.setMinimumHeight(78)
        self.part_preview.setMaximumHeight(112)
        layout.addWidget(self.part_preview)
        part_buttons = QHBoxLayout()
        self.edit_part_button = QPushButton("Edit part")
        self.edit_part_button.clicked.connect(self.edit_selected_part)
        self.remove_part_button = QPushButton("Remove part")
        self.remove_part_button.clicked.connect(self.remove_selected_part)
        part_buttons.addWidget(self.edit_part_button)
        part_buttons.addWidget(self.remove_part_button)
        layout.addLayout(part_buttons)

        layout.addWidget(QLabel("Interaction regions"))
        self.interactions_list = QListWidget()
        self.interactions_list.setMaximumHeight(80)
        layout.addWidget(self.interactions_list)
        interaction_buttons = QHBoxLayout()
        self.edit_interaction_button = QPushButton("Edit interaction")
        self.edit_interaction_button.clicked.connect(self.edit_selected_interaction)
        self.remove_interaction_button = QPushButton("Remove interaction")
        self.remove_interaction_button.clicked.connect(self.remove_selected_interaction)
        interaction_buttons.addWidget(self.edit_interaction_button)
        interaction_buttons.addWidget(self.remove_interaction_button)
        layout.addLayout(interaction_buttons)

        layout.addWidget(QLabel("Entity collision regions (green dash-dot)"))
        self.collisions_list = QListWidget()
        self.collisions_list.setMaximumHeight(72)
        self.collisions_list.currentItemChanged.connect(self.collision_selection_changed)
        layout.addWidget(self.collisions_list)
        collision_buttons = QHBoxLayout()
        self.edit_collision_button = QPushButton("Edit collision")
        self.edit_collision_button.clicked.connect(self.edit_selected_collision)
        self.remove_collision_button = QPushButton("Remove collision")
        self.remove_collision_button.clicked.connect(self.remove_selected_collision)
        collision_buttons.addWidget(self.edit_collision_button)
        collision_buttons.addWidget(self.remove_collision_button)
        layout.addLayout(collision_buttons)

        layout.addWidget(QLabel("Authored map logic (purple T / orange S / blue R)"))
        behavior_buttons = QHBoxLayout()
        self.edit_behavior_button = QPushButton("Edit map record")
        self.edit_behavior_button.clicked.connect(self.edit_selected_behavior)
        self.delete_behavior_button = QPushButton("Remove map record")
        self.delete_behavior_button.clicked.connect(self.remove_selected_behavior)
        behavior_buttons.addWidget(self.edit_behavior_button)
        behavior_buttons.addWidget(self.delete_behavior_button)
        layout.addLayout(behavior_buttons)

        form = QFormLayout()
        self.name_edit = QLineEdit()
        self.name_edit.editingFinished.connect(self.properties_changed)
        self.category_combo = QComboBox()
        self.category_combo.addItems(ENTITY_CATEGORIES)
        self.category_combo.currentTextChanged.connect(self.properties_changed)
        self.x_spin = QSpinBox(); self.x_spin.setRange(-32768, 32767); self.x_spin.valueChanged.connect(self.properties_changed)
        self.y_spin = QSpinBox(); self.y_spin.setRange(-32768, 32767); self.y_spin.valueChanged.connect(self.properties_changed)
        self.evidence_edit = QLineEdit()
        self.evidence_edit.setPlaceholderText("Source or visual evidence; leave blank if unknown")
        self.evidence_edit.editingFinished.connect(self.properties_changed)
        form.addRow("Name", self.name_edit)
        form.addRow("Classification", self.category_combo)
        form.addRow("Origin X", self.x_spin)
        form.addRow("Origin Y", self.y_spin)
        form.addRow("Evidence", self.evidence_edit)
        layout.addLayout(form)
        self.info = QPlainTextEdit()
        self.info.setReadOnly(True)
        self.info.setLineWrapMode(QPlainTextEdit.LineWrapMode.WidgetWidth)
        self.info.setMaximumHeight(220)
        self.info.setPlainText(
            "Click a map cell for its ROM layer words. Click a yellow ROM trigger or red collision region for its source record. "
            "Click a ROM actor marker to inspect its list entry. Click an authored T/S/R mark to inspect its Lua data. "
            "Drag entity outlines to move grouped annotations."
        )
        layout.addWidget(self.info)
        self.save_button = QPushButton("Save scene and definitions")
        self.save_button.clicked.connect(self.save_all)
        self.delete_button = QPushButton("Remove selected instance")
        self.delete_button.clicked.connect(self.remove_selected)
        layout.addWidget(self.save_button)
        layout.addWidget(self.delete_button)
        scroll = QScrollArea()
        scroll.setWidgetResizable(True)
        scroll.setFrameShape(QScrollArea.Shape.NoFrame)
        scroll.setWidget(panel)
        dock.setWidget(scroll)
        dock.setMinimumWidth(350)
        dock.setMaximumWidth(460)
        self.addDockWidget(Qt.DockWidgetArea.RightDockWidgetArea, dock)

    def _build_toolbar(self) -> None:
        bar = self.addToolBar("Map evidence layers")
        bar.setMovable(False)
        self.tool_buttons: dict[str, QPushButton] = {}
        scene_label = QLabel("  View layers: ")
        bar.addWidget(scene_label)
        self.priority_toggle = self._add_toggle(bar, "Priority pixels", True)
        self.grid_toggle = self._add_toggle(bar, "32 px grid", True)
        self.collision_toggle = self._add_toggle(bar, "ROM collision", False)
        self.event_toggle = self._add_toggle(bar, "ROM event triggers", True)
        self.actor_toggle = self._add_toggle(bar, "ROM default actors", True)
        self.behavior_toggle = self._add_toggle(bar, "Authored behavior", True)
        bar.addSeparator()
        legend = QLabel("Unreviewed: yellow   Building: cyan   Terrain: gray   Foliage: green   Entity collision: green dash-dot")
        bar.addWidget(legend)
        self.addToolBarBreak(Qt.ToolBarArea.TopToolBarArea)
        tools = self.addToolBar("Map editing")
        tools.setMovable(False)
        self._add_tool(tools, "Select / move", "select")
        self._add_tool(tools, "Add object", "add_entity")
        self._add_tool(tools, "Add visual part", "add_part")
        self._add_tool(tools, "Add interactable", "add_interaction")
        self._add_tool(tools, "Add collision", "add_collision")
        self._add_tool(tools, "Add trigger", "add_scripted_trigger")
        self._add_tool(tools, "Add spawn", "add_actor_spawn")
        self._add_tool(tools, "Add encounter", "add_encounter_region")
        tools.addSeparator()
        self.coordinate_label = QLabel(" x: —  y: — ")
        tools.addWidget(self.coordinate_label)

    def _add_toggle(self, toolbar, text: str, checked: bool) -> QPushButton:
        button = QPushButton(text)
        button.setCheckable(True)
        button.setChecked(checked)
        button.toggled.connect(self.layers_changed)
        toolbar.addWidget(button)
        return button

    def _add_tool(self, toolbar, label: str, mode: str) -> None:
        button = QPushButton(label)
        button.setCheckable(True)
        button.setChecked(mode == "select")
        button.clicked.connect(lambda _checked=False, selected=mode: self.set_mode(selected))
        toolbar.addWidget(button)
        self.tool_buttons[mode] = button

    def set_mode(self, mode: str) -> None:
        if mode in {"add_interaction", "add_part", "add_collision"} and self.selected_instance is None:
            self.statusBar().showMessage("Select an entity instance before adding a visual part, interaction, or collision region.", 5000)
            mode = "select"
        self.mode = mode
        self.map_view.mode = mode
        for button_mode, button in self.tool_buttons.items():
            button.setChecked(button_mode == mode)
        self.statusBar().showMessage({
            "select": "Select entities. Middle mouse pans; wheel zooms.",
            "add_entity": "Drag a rectangle around one object or decal. It will be saved as unreviewed.",
            "add_part": "Drag another source-map region owned by the selected entity; assign its visual role and depth.",
            "add_interaction": "Drag an interaction rectangle inside the selected entity; enter a Lua handler only when verified.",
            "add_collision": "Drag an authored collision rectangle inside the selected entity; choose its mode only when supported.",
            "add_scripted_trigger": "Drag a map region for a Lua trigger. It is authored content, separate from read-only ROM event evidence.",
            "add_actor_spawn": "Click a world position to author an actor spawn point. Link a Lua actor profile and conditions only when supported.",
            "add_encounter_region": "Drag a region for an expandable Lua encounter pool; leave it unreviewed until sourced.",
        }[mode])

    def layers_changed(self) -> None:
        self.map_view.set_layer_options(
            self.priority_toggle.isChecked(), self.grid_toggle.isChecked(),
            self.collision_toggle.isChecked(), self.event_toggle.isChecked(),
            self.actor_toggle.isChecked(), self.behavior_toggle.isChecked(),
        )

    def load_scene(self, scene_id: int) -> None:
        if scene_id not in self.map_by_id:
            return
        self.scene_id = scene_id
        if hasattr(self, "scene_combo"):
            index = self.scene_combo.findData(scene_id)
            if index >= 0 and self.scene_combo.currentIndex() != index:
                self.scene_combo.blockSignals(True)
                self.scene_combo.setCurrentIndex(index)
                self.scene_combo.blockSignals(False)
        if hasattr(self, "scene_actions") and scene_id in self.scene_actions:
            self.scene_actions[scene_id].setChecked(True)
        self.dirty = False
        self.instances = self.world.load_scene(scene_id)
        self.behaviors = self.world.load_behaviors(scene_id)
        self.entities = {}
        for instance in self.instances:
            entity = self.world.load_entity(instance.entity_id)
            if entity:
                self.entities[entity.entity_id] = entity
        self.selected_instance = None
        self.selected_behavior = None
        self._refresh_list()
        self.map_view.load_scene(
            self.map_by_id[scene_id], self.assets, self.repo_root,
            self.instances, self.entities, self.behaviors,
        )
        self._show_properties(None)
        self._update_scene_summary()

    def scene_changed(self, index: int) -> None:
        if index >= 0:
            scene_id = self.scene_combo.itemData(index)
            if scene_id is not None and scene_id != self.scene_id:
                self.select_scene(int(scene_id))

    def select_scene(self, scene_id: int) -> None:
        if scene_id not in self.map_by_id or scene_id == self.scene_id:
            return
        if self.dirty:
            self.save_all()
        self.load_scene(scene_id)

    def _refresh_list(self) -> None:
        # Keep the graphics scene on the current authoritative editor records.
        # Delete/scene-switch paths can replace the list object rather than
        # mutating the list originally handed to FieldMapView.
        self.map_view.instances = self.instances
        self.map_view.entities = self.entities
        self.map_view.behaviors = self.behaviors
        self.entity_list.blockSignals(True)
        self.entity_list.clear()
        for instance in self.instances:
            entity = self.entities.get(instance.entity_id)
            if not entity:
                continue
            item = QListWidgetItem(f"{entity.name} [{entity.category}]   @ {instance.origin_x},{instance.origin_y}")
            item.setData(Qt.ItemDataRole.UserRole, instance.instance_id)
            self.entity_list.addItem(item)
        self.entity_list.blockSignals(False)
        self.map_view.redraw_annotations()
        self.map_view.redraw_behaviors()

    def _update_scene_summary(self) -> None:
        info = self.map_by_id[self.scene_id]
        unreviewed = sum(1 for instance in self.instances if self.entities.get(instance.entity_id, EntityDefinition("", "")).category == "unreviewed")
        self.statusBar().showMessage(
            f"Scene {self.scene_id:02d}: {info.width}×{info.height}; {len(self.instances)} annotations, {unreviewed} unreviewed. "
            f"Source encoding: {info.metatile_encoding}; collision regions: {len(self.map_view.collision_rectangles)}; "
            f"event regions: {len(self.map_view.event_rectangles)}; "
            f"ROM default-list actor positions: {len(self.map_view.actor_positions)}; "
            f"authored logic records: {len(self.behaviors)}."
        )

    def _show_properties(self, instance: EntityInstance | None) -> None:
        self._loading_fields = True
        enabled = instance is not None
        for widget in (
            self.name_edit, self.category_combo, self.x_spin, self.y_spin,
            self.evidence_edit, self.delete_button, self.parts_list,
            self.interactions_list, self.edit_part_button,
            self.remove_part_button, self.edit_interaction_button,
            self.remove_interaction_button, self.collisions_list,
        ):
            widget.setEnabled(enabled)
        self.edit_collision_button.setEnabled(False)
        self.remove_collision_button.setEnabled(False)
        self.edit_behavior_button.setEnabled(False)
        self.delete_behavior_button.setEnabled(False)
        if not enabled:
            self.name_edit.clear(); self.evidence_edit.clear()
            self.x_spin.setValue(0); self.y_spin.setValue(0)
            self.category_combo.setCurrentText("unreviewed")
            self.parts_list.clear()
            self.part_preview.setPixmap(QPixmap())
            self.part_preview.setText("Select a visual part to preview its named asset.")
            self.interactions_list.clear()
            self.collisions_list.clear()
            self.info.setPlainText(
                "Click a map cell for its ROM layer words. Click a yellow ROM trigger or red collision region for its source record. "
                "Click a ROM actor marker to inspect its list entry. Click an authored T/S/R mark to inspect its Lua data. "
                "Drag entity outlines to move grouped annotations."
            )
            self._loading_fields = False
            return
        entity = self.entities[instance.entity_id]
        selected_part = self.parts_list.currentItem().data(Qt.ItemDataRole.UserRole) if self.parts_list.currentItem() else None
        selected_interaction = self.interactions_list.currentItem().data(Qt.ItemDataRole.UserRole) if self.interactions_list.currentItem() else None
        self.parts_list.blockSignals(True)
        self.parts_list.clear()
        for part in entity.parts:
            item = QListWidgetItem(
                f"{part.part_id} @ {part.local_x},{part.local_y}"
            )
            item.setData(Qt.ItemDataRole.UserRole, part.part_id)
            self.parts_list.addItem(item)
            if part.part_id == selected_part:
                self.parts_list.setCurrentItem(item)
        self.parts_list.blockSignals(False)
        self.part_selection_changed(self.parts_list.currentItem(), None)
        self.interactions_list.blockSignals(True)
        self.interactions_list.clear()
        for interaction in entity.interactables:
            item = QListWidgetItem(
                f"{interaction.interaction_id}: {interaction.source_event or '(unmapped)'} @ "
                f"{interaction.local_x},{interaction.local_y} "
                f"{interaction.width}x{interaction.height}"
            )
            item.setData(Qt.ItemDataRole.UserRole, interaction.interaction_id)
            self.interactions_list.addItem(item)
            if interaction.interaction_id == selected_interaction:
                self.interactions_list.setCurrentItem(item)
        self.interactions_list.blockSignals(False)
        selected_collision = (
            self.collisions_list.currentItem().data(Qt.ItemDataRole.UserRole)
            if self.collisions_list.currentItem() else None
        )
        self.collisions_list.blockSignals(True)
        self.collisions_list.clear()
        for collision in entity.collision_regions:
            item = QListWidgetItem(
                f"{collision.collision_id}: {collision.collision_mode} @ "
                f"{collision.local_x},{collision.local_y} "
                f"{collision.width}x{collision.height}"
            )
            item.setData(Qt.ItemDataRole.UserRole, collision.collision_id)
            self.collisions_list.addItem(item)
            if collision.collision_id == selected_collision:
                self.collisions_list.setCurrentItem(item)
        self.collisions_list.blockSignals(False)
        has_selected_collision = self.collisions_list.currentItem() is not None
        self.edit_collision_button.setEnabled(has_selected_collision)
        self.remove_collision_button.setEnabled(has_selected_collision)
        self.name_edit.setText(entity.name)
        self.category_combo.setCurrentText(entity.category if entity.category in ENTITY_CATEGORIES else "unreviewed")
        self.x_spin.setValue(instance.origin_x); self.y_spin.setValue(instance.origin_y)
        self.evidence_edit.setText(entity.evidence)
        part_summary = "\n".join(
            f"- {part.part_id}: {part.role}, source scene {part.source_scene:02d} at "
            f"{part.source_x},{part.source_y} {part.width}x{part.height}; "
            f"{part.draw_layer}; asset {part.asset_path or '(created on save)'}"
            for part in entity.parts
        ) or "No visual parts"
        interaction_summary = "\n".join(
            f"• {item.interaction_id}: {item.local_x},{item.local_y} {item.width}×{item.height}; Lua: {item.lua_handler or '(unassigned)'}; event: {item.source_event or '(unmapped)'}"
            for item in entity.interactables
        ) or "No interaction regions"
        collision_summary = "\n".join(
            f"- {item.collision_id}: {item.collision_mode} at "
            f"{item.local_x},{item.local_y} {item.width}x{item.height}; "
            f"{item.review_status}; evidence: {item.evidence or '(not recorded)'}"
            for item in entity.collision_regions
        ) or "No authored collision regions"
        self.info.setPlainText(
            f"ID: {entity.entity_id}\nVisual parts:\n{part_summary}"
            f"\nInteractions:\n{interaction_summary}"
            f"\nAuthored collision:\n{collision_summary}\n\n"
            "ROM collision remains separate evidence; these entity records do not "
            "modify the preassembled map or live field collision."
        )
        map_info = self.map_by_id[self.scene_id]
        bounds = QRectF(
            instance.origin_x + entity.bounds[0],
            instance.origin_y + entity.bounds[1],
            entity.bounds[2],
            entity.bounds[3],
        )
        collision_hits = sum(
            1 for region in self.map_view.collision_rectangles
            if bounds.intersects(QRectF(region.x, region.y, region.width, region.height))
        )
        event_hits = sum(
            1 for region in self.map_view.event_rectangles
            if bounds.intersects(QRectF(region.x, region.y, region.width, region.height))
        )
        evidence_summary = (
            f"\nMap evidence: {map_info.tilemap_width}×{map_info.tilemap_height} metatile cells, "
            f"{map_info.metatile_encoding}; primary {map_info.primary_tilemap}, "
            f"secondary {map_info.secondary_tilemap}. Region intersects {collision_hits} ROM collision rectangles "
            f"and {event_hits} ROM event triggers. The base bitmap is precomposed; the priority toggle shows its sparse overlay."
        )
        self.info.setPlainText(self.info.toPlainText() + evidence_summary)
        self._loading_fields = False

    def edit_selected_part(self) -> None:
        instance = self.selected_instance
        item = self.parts_list.currentItem()
        if instance is None or item is None:
            return
        entity = self.entities[instance.entity_id]
        part = next((part for part in entity.parts if part.part_id == item.data(Qt.ItemDataRole.UserRole)), None)
        if part is None:
            return
        dialog = QDialog(self)
        dialog.setWindowTitle(f"Edit visual part: {part.part_id}")
        form = QFormLayout(dialog)
        part_id = QLineEdit(part.part_id)
        source_kind = QComboBox(); source_kind.addItems(["map_composite", "priority_overlay"]); source_kind.setCurrentText(part.source_kind)
        fields: dict[str, QSpinBox] = {}
        for key, label, value, minimum, maximum in (
            ("source_scene", "Source scene", part.source_scene, 0, 55),
            ("source_x", "Source X", part.source_x, -32768, 32767),
            ("source_y", "Source Y", part.source_y, -32768, 32767),
            ("width", "Width", part.width, 1, 8192),
            ("height", "Height", part.height, 1, 8192),
            ("local_x", "Entity-local X", part.local_x, -32768, 32767),
            ("local_y", "Entity-local Y", part.local_y, -32768, 32767),
        ):
            spin = QSpinBox(); spin.setRange(minimum, maximum); spin.setValue(value)
            fields[key] = spin
            form.addRow(label, spin)
        role = QComboBox(); role.addItems(PART_ROLES); role.setCurrentText(part.role)
        depth = QComboBox(); depth.addItems(DRAW_LAYERS); depth.setCurrentText(part.draw_layer)
        evidence = QLineEdit(part.evidence)
        form.insertRow(0, "Part ID", part_id)
        form.insertRow(1, "Source kind", source_kind)
        form.addRow("Role", role); form.addRow("Draw depth", depth); form.addRow("Evidence", evidence)
        buttons = QDialogButtonBox(QDialogButtonBox.StandardButton.Save | QDialogButtonBox.StandardButton.Cancel)
        buttons.accepted.connect(dialog.accept); buttons.rejected.connect(dialog.reject)
        form.addRow(buttons)
        if dialog.exec() != QDialog.DialogCode.Accepted:
            return
        part.part_id = safe_id(part_id.text())
        part.source_kind = source_kind.currentText()
        for key, spin in fields.items():
            setattr(part, key, spin.value())
        part.role = role.currentText(); part.draw_layer = depth.currentText(); part.evidence = evidence.text().strip()
        self.dirty = True
        self._show_properties(instance)
        self.map_view.redraw_annotations()

    def remove_selected_part(self) -> None:
        instance = self.selected_instance
        item = self.parts_list.currentItem()
        if instance is None or item is None:
            return
        entity = self.entities[instance.entity_id]
        if len(entity.parts) <= 1:
            self.statusBar().showMessage("An entity needs at least one visual part.", 4000)
            return
        part_id = item.data(Qt.ItemDataRole.UserRole)
        entity.parts = [part for part in entity.parts if part.part_id != part_id]
        self.dirty = True
        self._show_properties(instance)
        self.map_view.redraw_annotations()

    def edit_selected_interaction(self) -> None:
        from PyQt6.QtWidgets import QDialogButtonBox
        instance = self.selected_instance
        item = self.interactions_list.currentItem()
        if instance is None or item is None:
            return
        entity = self.entities[instance.entity_id]
        interaction = next((record for record in entity.interactables if record.interaction_id == item.data(Qt.ItemDataRole.UserRole)), None)
        if interaction is None:
            return
        dialog = QDialog(self)
        dialog.setWindowTitle(f"Edit interaction: {interaction.interaction_id}")
        form = QFormLayout(dialog)
        interaction_id = QLineEdit(interaction.interaction_id)
        fields: dict[str, QSpinBox] = {}
        for key, label, value, minimum, maximum in (
            ("local_x", "Entity-local X", interaction.local_x, -32768, 32767),
            ("local_y", "Entity-local Y", interaction.local_y, -32768, 32767),
            ("width", "Width", interaction.width, 1, 8192),
            ("height", "Height", interaction.height, 1, 8192),
        ):
            spin = QSpinBox(); spin.setRange(minimum, maximum); spin.setValue(value)
            fields[key] = spin
            form.addRow(label, spin)
        handler = QLineEdit(interaction.lua_handler)
        source_event = QLineEdit(interaction.source_event)
        evidence = QLineEdit(interaction.evidence)
        form.insertRow(0, "Interaction ID", interaction_id)
        form.addRow("Lua handler", handler); form.addRow("ROM event", source_event); form.addRow("Evidence", evidence)
        buttons = QDialogButtonBox(QDialogButtonBox.StandardButton.Save | QDialogButtonBox.StandardButton.Cancel)
        buttons.accepted.connect(dialog.accept); buttons.rejected.connect(dialog.reject)
        form.addRow(buttons)
        if dialog.exec() != QDialog.DialogCode.Accepted:
            return
        interaction.interaction_id = safe_id(interaction_id.text())
        for key, spin in fields.items():
            setattr(interaction, key, spin.value())
        interaction.lua_handler = handler.text().strip()
        interaction.source_event = source_event.text().strip()
        interaction.evidence = evidence.text().strip()
        self.dirty = True
        self._show_properties(instance)
        self.map_view.redraw_annotations()

    def remove_selected_interaction(self) -> None:
        instance = self.selected_instance
        item = self.interactions_list.currentItem()
        if instance is None or item is None:
            return
        entity = self.entities[instance.entity_id]
        interaction_id = item.data(Qt.ItemDataRole.UserRole)
        entity.interactables = [record for record in entity.interactables if record.interaction_id != interaction_id]
        self.dirty = True
        self._show_properties(instance)
        self.map_view.redraw_annotations()

    def _add_collision_region(self, x: int, y: int, width: int, height: int) -> None:
        instance = self.selected_instance
        if instance is None:
            return
        entity = self.entities[instance.entity_id]
        serial = 1
        existing = {item.collision_id for item in entity.collision_regions}
        while f"collision_{serial:02d}" in existing:
            serial += 1
        entity.collision_regions.append(EntityCollisionRegion(
            collision_id=f"collision_{serial:02d}",
            local_x=x - instance.origin_x,
            local_y=y - instance.origin_y,
            width=width,
            height=height,
        ))
        self.dirty = True
        self._show_properties(instance)
        self.map_view.redraw_annotations()
        self.statusBar().showMessage(
            "Added an unreviewed entity collision region. It is separate from ROM collision evidence.",
            6000,
        )

    def collision_selection_changed(self, current, _previous) -> None:
        enabled = current is not None and self.selected_instance is not None
        self.edit_collision_button.setEnabled(enabled)
        self.remove_collision_button.setEnabled(enabled)

    def edit_selected_collision(self) -> None:
        instance = self.selected_instance
        item = self.collisions_list.currentItem()
        if instance is None or item is None:
            return
        entity = self.entities[instance.entity_id]
        collision = next(
            (record for record in entity.collision_regions
             if record.collision_id == item.data(Qt.ItemDataRole.UserRole)),
            None,
        )
        if collision is None:
            return
        dialog = QDialog(self)
        dialog.setWindowTitle(f"Edit collision region: {collision.collision_id}")
        form = QFormLayout(dialog)
        collision_id = QLineEdit(collision.collision_id)
        form.addRow("Stable ID", collision_id)
        fields: dict[str, QSpinBox] = {}
        for key, label, value, minimum, maximum in (
            ("local_x", "Entity-local X", collision.local_x, -32768, 32767),
            ("local_y", "Entity-local Y", collision.local_y, -32768, 32767),
            ("width", "Width", collision.width, 1, 8192),
            ("height", "Height", collision.height, 1, 8192),
        ):
            spin = QSpinBox()
            spin.setRange(minimum, maximum)
            spin.setValue(value)
            fields[key] = spin
            form.addRow(label, spin)
        collision_mode = QComboBox()
        collision_mode.addItems(COLLISION_MODES)
        collision_mode.setCurrentText(
            collision.collision_mode
            if collision.collision_mode in COLLISION_MODES else "unreviewed"
        )
        form.addRow("Collision mode", collision_mode)
        evidence = QLineEdit(collision.evidence)
        evidence.setPlaceholderText("ROM trace or explicit new-content rationale")
        form.addRow("Evidence", evidence)
        review_status = QComboBox()
        review_status.addItems(["unreviewed", "source_verified", "new_content"])
        review_status.setCurrentText(
            collision.review_status
            if collision.review_status in {"unreviewed", "source_verified", "new_content"}
            else "unreviewed"
        )
        form.addRow("Review status", review_status)
        buttons = QDialogButtonBox(
            QDialogButtonBox.StandardButton.Save | QDialogButtonBox.StandardButton.Cancel
        )
        buttons.accepted.connect(dialog.accept)
        buttons.rejected.connect(dialog.reject)
        form.addRow(buttons)
        if dialog.exec() != QDialog.DialogCode.Accepted:
            return
        try:
            new_id = safe_id(collision_id.text())
        except ValueError:
            QMessageBox.warning(self, "Invalid collision ID", "Enter an ID with at least one letter or number.")
            return
        if any(
            other is not collision and other.collision_id == new_id
            for other in entity.collision_regions
        ):
            QMessageBox.warning(self, "Duplicate collision ID", f"{new_id} is already used by this entity.")
            return
        collision.collision_id = new_id
        for key, spin in fields.items():
            setattr(collision, key, spin.value())
        collision.collision_mode = collision_mode.currentText()
        collision.evidence = evidence.text().strip()
        collision.review_status = review_status.currentText()
        self.dirty = True
        self._show_properties(instance)
        self.map_view.redraw_annotations()

    def remove_selected_collision(self) -> None:
        instance = self.selected_instance
        item = self.collisions_list.currentItem()
        if instance is None or item is None:
            return
        collision_id = item.data(Qt.ItemDataRole.UserRole)
        entity = self.entities[instance.entity_id]
        entity.collision_regions = [
            collision for collision in entity.collision_regions
            if collision.collision_id != collision_id
        ]
        self.dirty = True
        self._show_properties(instance)
        self.map_view.redraw_annotations()

    def select_at(self, point: QPointF) -> EntityInstance | None:
        matches = []
        for instance in reversed(self.instances):
            entity = self.entities.get(instance.entity_id)
            if not entity:
                continue
            x, y, width, height = entity.bounds
            if instance.origin_x + x <= point.x() <= instance.origin_x + x + width and instance.origin_y + y <= point.y() <= instance.origin_y + y + height:
                matches.append(instance)
        if matches:
            self.select_instance(matches[0])
            return matches[0]
        self.select_instance(None)
        return None

    def show_evidence(self, evidence: MapEvidenceRegion) -> None:
        self.select_instance(None)
        self.selected_evidence = evidence
        self.selected_behavior = None
        self.edit_behavior_button.setEnabled(False)
        self.delete_behavior_button.setEnabled(False)
        self.info.setPlainText(evidence.details)
        self.statusBar().showMessage(
            f"Inspecting {evidence.title}; source-derived evidence is read-only.",
            7000,
        )

    def show_map_cell(self, point: QPointF) -> None:
        info = self.map_by_id.get(self.scene_id)
        if info is None:
            return
        x, y = int(point.x()), int(point.y())
        evidence = read_map_cell_evidence(self.assets, info, x, y)
        if evidence is not None:
            self.show_evidence(evidence)

    def move_instance(self, instance_id: str, delta_x: int, delta_y: int) -> None:
        instance = next(
            (item for item in self.instances if item.instance_id == instance_id),
            None,
        )
        if instance is None or (delta_x == 0 and delta_y == 0):
            return
        instance.origin_x += delta_x
        instance.origin_y += delta_y
        self.dirty = True
        self._refresh_list()
        self.select_instance(instance)
        self._update_scene_summary()
        self.statusBar().showMessage(
            f"Moved grouped annotation {instance_id} by {delta_x},{delta_y} pixels. "
            "The extracted map bitmap itself is unchanged.",
            6000,
        )

    def select_instance(self, instance: EntityInstance | None) -> None:
        self.selected_evidence = None
        self.selected_behavior = None
        self.edit_behavior_button.setEnabled(False)
        self.delete_behavior_button.setEnabled(False)
        self.selected_instance = instance
        selected_row = None
        if instance:
            for index in range(self.entity_list.count()):
                item = self.entity_list.item(index)
                if item.data(Qt.ItemDataRole.UserRole) == instance.instance_id:
                    selected_row = item
                    break
        self.entity_list.blockSignals(True)
        self.entity_list.setCurrentItem(selected_row)
        self.entity_list.blockSignals(False)
        self._show_properties(instance)

    def show_behavior(self, record: MapBehaviorRecord) -> None:
        self.selected_evidence = None
        self.selected_behavior = record
        self.selected_instance = None
        self.entity_list.blockSignals(True)
        self.entity_list.setCurrentItem(None)
        self.entity_list.blockSignals(False)
        self._show_properties(None)
        self.edit_behavior_button.setEnabled(True)
        self.delete_behavior_button.setEnabled(True)
        self.info.setPlainText(record.describe(self.scene_id))
        self.statusBar().showMessage(
            f"Selected authored {record.kind}; this record is separate from ROM event evidence.",
            6000,
        )

    def list_selection_changed(self, current, previous) -> None:
        if current is None:
            return
        target = current.data(Qt.ItemDataRole.UserRole)
        instance = next((item for item in self.instances if item.instance_id == target), None)
        self.select_instance(instance)

    def part_selection_changed(self, current, _previous) -> None:
        self.part_preview.setPixmap(QPixmap())
        if current is None or self.selected_instance is None:
            self.part_preview.setText("Select a visual part to preview its named asset.")
            return
        entity = self.entities.get(self.selected_instance.entity_id)
        if entity is None:
            self.part_preview.setText("Entity definition is unavailable.")
            return
        part_id = current.data(Qt.ItemDataRole.UserRole)
        part = next((item for item in entity.parts if item.part_id == part_id), None)
        if part is None or not part.asset_path:
            self.part_preview.setText("Save the scene to generate this part asset.")
            return
        asset_path = self.assets / Path(part.asset_path)
        pixmap = QPixmap(str(asset_path))
        if pixmap.isNull():
            self.part_preview.setText(f"Asset not found: {part.asset_path}")
            return
        self.part_preview.setText("")
        self.part_preview.setPixmap(pixmap.scaled(
            self.part_preview.size(),
            Qt.AspectRatioMode.KeepAspectRatio,
            Qt.TransformationMode.FastTransformation,
        ))
        self.part_preview.setToolTip(
            f"{part.asset_path}\n{part.width}x{part.height}; "
            f"{part.role} / {part.draw_layer}\n{part.evidence}"
        )

    def region_created(self, rect: QRectF) -> None:
        x, y = round(rect.x()), round(rect.y())
        width, height = max(1, round(rect.width())), max(1, round(rect.height()))
        if self.mode == "add_actor_spawn":
            self.add_behavior_record("actor_spawn", x, y, 1, 1)
            return
        if rect.width() < 2 or rect.height() < 2:
            return
        if self.mode in {"add_scripted_trigger", "add_encounter_region"}:
            right = round((rect.x() + rect.width()) / 32) * 32
            bottom = round((rect.y() + rect.height()) / 32) * 32
            x = round(rect.x() / 32) * 32
            y = round(rect.y() / 32) * 32
            width, height = max(32, right - x), max(32, bottom - y)
            kind = "scripted_trigger" if self.mode == "add_scripted_trigger" else "random_encounter"
            self.add_behavior_record(kind, x, y, width, height)
            return
        if self.mode == "add_interaction":
            self._add_interactable(x, y, width, height)
            return
        if self.mode == "add_collision":
            self._add_collision_region(x, y, width, height)
            return
        if self.mode == "add_part":
            self._add_visual_part(x, y, width, height)
            return
        self._add_entity_region(x, y, width, height)

    def add_behavior_record(self, kind: str, x: int, y: int, width: int, height: int) -> None:
        prefixes = {
            "scripted_trigger": "trigger",
            "actor_spawn": "spawn",
            "random_encounter": "encounter",
        }
        prefix = prefixes[kind]
        serial = 1
        existing = {item.record_id for item in self.behaviors}
        while f"scene{self.scene_id:02d}_{prefix}{serial:03d}" in existing:
            serial += 1
        record = MapBehaviorRecord(
            record_id=f"scene{self.scene_id:02d}_{prefix}{serial:03d}",
            kind=kind,
            x=x,
            y=y,
            width=width,
            height=height,
        )
        self.behaviors.append(record)
        self.dirty = True
        self._refresh_list()
        self.show_behavior(record)
        self.edit_selected_behavior()
        self._update_scene_summary()

    def edit_selected_behavior(self) -> None:
        record = self.selected_behavior
        if record is None:
            return
        dialog = QDialog(self)
        dialog.setWindowTitle(f"Edit {record.title}")
        form = QFormLayout(dialog)
        record_id = QLineEdit(record.record_id)
        form.addRow("Stable ID", record_id)
        coordinate_fields: dict[str, QSpinBox] = {}
        for name, label, value in (
            ("x", "Map X", record.x),
            ("y", "Map Y", record.y),
        ):
            spin = QSpinBox()
            spin.setRange(-32768, 32767)
            spin.setValue(value)
            coordinate_fields[name] = spin
            form.addRow(label, spin)
        if record.kind != "actor_spawn":
            for name, label, value in (
                ("width", "Width", record.width),
                ("height", "Height", record.height),
            ):
                spin = QSpinBox()
                spin.setRange(1, 8192)
                spin.setValue(value)
                coordinate_fields[name] = spin
                form.addRow(label, spin)

        fields: dict[str, QLineEdit] = {}
        if record.kind == "scripted_trigger":
            field_specs = (
                ("script_module", "Lua script module"),
                ("script_function", "Lua event function"),
                ("condition_module", "Condition module"),
                ("condition_function", "Condition function"),
                ("repeat_policy", "Repeat policy"),
            )
        elif record.kind == "actor_spawn":
            field_specs = (
                ("actor_profile", "Lua actor profile"),
                ("heading", "Initial heading"),
                ("condition_module", "Condition module"),
                ("condition_function", "Condition function"),
                ("source_record", "ROM source record"),
            )
        else:
            field_specs = (
                ("event_pool", "Lua event pool"),
                ("condition_module", "Condition module"),
                ("condition_function", "Condition function"),
                ("source_table", "ROM source table"),
            )
        for name, label in field_specs:
            edit = QLineEdit(getattr(record, name))
            edit.setPlaceholderText("Leave blank until known")
            fields[name] = edit
            form.addRow(label, edit)
        evidence = QLineEdit(record.evidence)
        evidence.setPlaceholderText("ROM trace or explicit new-content rationale")
        form.addRow("Evidence", evidence)
        review_status = QComboBox()
        review_status.addItems(["unreviewed", "source_verified", "new_content"])
        review_status.setCurrentText(
            record.review_status if record.review_status in {"unreviewed", "source_verified", "new_content"} else "unreviewed"
        )
        form.addRow("Review status", review_status)
        buttons = QDialogButtonBox(QDialogButtonBox.StandardButton.Save | QDialogButtonBox.StandardButton.Cancel)
        buttons.accepted.connect(dialog.accept)
        buttons.rejected.connect(dialog.reject)
        form.addRow(buttons)
        if dialog.exec() != QDialog.DialogCode.Accepted:
            return
        try:
            new_id = safe_id(record_id.text())
        except ValueError:
            QMessageBox.warning(self, "Invalid map record ID", "Enter a stable ID with at least one letter or number.")
            return
        if any(item is not record and item.record_id == new_id for item in self.behaviors):
            QMessageBox.warning(self, "Duplicate map record ID", f"{new_id} is already used in this scene.")
            return
        record.record_id = new_id
        for name, widget in coordinate_fields.items():
            setattr(record, name, widget.value())
        for name, widget in fields.items():
            setattr(record, name, widget.text().strip())
        record.evidence = evidence.text().strip()
        record.review_status = review_status.currentText()
        self.dirty = True
        self._refresh_list()
        self.show_behavior(record)
        self._update_scene_summary()

    def remove_selected_behavior(self) -> None:
        record = self.selected_behavior
        if record is None:
            return
        self.behaviors = [item for item in self.behaviors if item is not record]
        self.selected_behavior = None
        self.dirty = True
        self._refresh_list()
        self._show_properties(None)
        self._update_scene_summary()

    def _add_entity_region(self, x: int, y: int, width: int, height: int) -> None:
        existing = {entity_id for entity_id in self.entities}
        serial = 1
        while f"scene{self.scene_id:02d}_object{serial:03d}" in existing or (self.world.root / "entities" / f"scene{self.scene_id:02d}_object{serial:03d}").exists():
            serial += 1
        entity_id = f"scene{self.scene_id:02d}_object{serial:03d}"
        entity = EntityDefinition(
            entity_id=entity_id,
            name=f"Scene {self.scene_id:02d} object {serial:03d}",
            category="unreviewed",
            evidence="",
            parts=[EntityPart("main", "map_composite", self.scene_id, x, y, width, height, 0, 0, "ground", "unreviewed", "")],
        )
        instance = EntityInstance(entity_id, entity_id, x, y)
        self.entities[entity_id] = entity
        self.instances.append(instance)
        self.dirty = True
        self.select_instance(instance)
        self._refresh_list()
        self.select_instance(instance)
        self._update_scene_summary()
        self.statusBar().showMessage(f"Created {entity_id} as unreviewed. Inspect source layers and ROM regions before assigning its class.", 7000)

    def _add_interactable(self, x: int, y: int, width: int, height: int) -> None:
        instance = self.selected_instance
        if instance is None:
            return
        entity = self.entities[instance.entity_id]
        local_x, local_y = x - instance.origin_x, y - instance.origin_y
        from PyQt6.QtWidgets import QInputDialog
        handler_name, ok = QInputDialog.getText(self, "Lua interaction handler", "Lua function identifier (leave blank until verified):")
        if not ok:
            return
        source_event, ok = QInputDialog.getText(self, "ROM event", "Verified ROM event ID (leave blank if this is a new scripted interaction):")
        if not ok:
            return
        evidence, ok = QInputDialog.getText(self, "Interaction evidence", "ROM trace, map reference, or visual evidence (optional):")
        if not ok:
            return
        interaction_id = f"interaction_{len(entity.interactables) + 1:02d}"
        entity.interactables.append(Interactable(interaction_id, local_x, local_y, width, height, handler_name.strip(), source_event.strip(), evidence.strip()))
        self.dirty = True
        self._show_properties(instance)
        self.map_view.redraw_annotations()

    def _add_visual_part(self, x: int, y: int, width: int, height: int) -> None:
        from PyQt6.QtWidgets import QInputDialog
        instance = self.selected_instance
        if instance is None:
            return
        entity = self.entities[instance.entity_id]
        role, ok = QInputDialog.getItem(
            self, "Visual part role", "What does this part represent?", list(PART_ROLES), 0, False
        )
        if not ok:
            return
        draw_layer, ok = QInputDialog.getItem(
            self, "Visual depth", "Where does this part draw relative to actors?", list(DRAW_LAYERS), 0, False
        )
        if not ok:
            return
        evidence, ok = QInputDialog.getText(
            self, "Visual part evidence", "ROM or visual evidence for this role (optional):"
        )
        if not ok:
            return
        part_id = f"part_{len(entity.parts) + 1:02d}"
        entity.parts.append(EntityPart(
            part_id, "map_composite", self.scene_id, x, y, width, height,
            x - instance.origin_x, y - instance.origin_y,
            draw_layer, role, evidence.strip(),
        ))
        self.dirty = True
        self._show_properties(instance)
        self.map_view.redraw_annotations()

    def properties_changed(self, *_args) -> None:
        if self._loading_fields or self.selected_instance is None:
            return
        instance = self.selected_instance
        entity = self.entities[instance.entity_id]
        entity.name = self.name_edit.text().strip() or entity.entity_id
        entity.category = self.category_combo.currentText()
        entity.evidence = self.evidence_edit.text().strip()
        if entity.parts and entity.parts[0].role == "unreviewed":
            entity.parts[0].role = {
                "building": "body",
                "decal": "decal",
                "prop": "body",
                "structure": "body",
                "foreground_part": "foreground",
            "terrain_feature": "body",
            "foliage": "decal",
            }.get(entity.category, "unreviewed")
        instance.origin_x = self.x_spin.value()
        instance.origin_y = self.y_spin.value()
        self.dirty = True
        self._refresh_list()
        self.select_instance(instance)
        self._update_scene_summary()

    def cursor_moved(self, x: int, y: int) -> None:
        if self.map_by_id.get(self.scene_id):
            self.coordinate_label.setText(f" x: {x:4d}  y: {y:4d}  cell: {x // 32:02d},{y // 32:02d} ")

    def remove_selected(self) -> None:
        instance = self.selected_instance
        if instance is None:
            return
        self.instances = [item for item in self.instances if item.instance_id != instance.instance_id]
        self.selected_instance = None
        self.dirty = True
        self._refresh_list()
        self._show_properties(None)
        self._update_scene_summary()

    def save_all(self) -> None:
        try:
            self.world.export_part_assets(self.assets, self.entities.values())
            for entity in self.entities.values():
                self.world.save_entity(entity)
            self.world.save_scene(self.scene_id, self.instances)
            self.world.save_behaviors(self.scene_id, self.behaviors)
            self.world.export_lua(self.world.root.parent / "scripts" / "world_content_data.lua")
        except (OSError, ValueError) as error:
            QMessageBox.critical(self, "Could not save map content", str(error))
            return
        self.dirty = False
        self.part_selection_changed(self.parts_list.currentItem(), None)
        self.statusBar().showMessage(f"Saved scene {self.scene_id:02d} and {len(self.entities)} entity definitions under {self.world.root}", 6000)

    def keyPressEvent(self, event) -> None:
        if event.key() == Qt.Key.Key_S and event.modifiers() & Qt.KeyboardModifier.ControlModifier:
            self.save_all()
            event.accept()
            return
        if event.key() == Qt.Key.Key_Delete and self.selected_behavior is not None:
            self.remove_selected_behavior()
            event.accept()
            return
        if event.key() == Qt.Key.Key_Delete and self.selected_instance is not None:
            self.remove_selected()
            event.accept()
            return
        super().keyPressEvent(event)


def render_snapshot(viewer: MapEntityViewer, output: Path) -> None:
    viewer.show()
    QApplication.processEvents()
    image = QImage(viewer.size(), QImage.Format.Format_ARGB32)
    image.fill(QColor(22, 23, 26))
    painter = QPainter(image)
    viewer.render(painter)
    painter.end()
    output.parent.mkdir(parents=True, exist_ok=True)
    if not image.save(str(output)):
        raise RuntimeError(f"Could not write viewer snapshot: {output}")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Inspect extracted Shadowrun field maps and edit entity annotations.")
    parser.add_argument("--assets", type=Path, default=DEFAULT_ASSETS, help="decompilation/assets directory")
    parser.add_argument("--world", type=Path, default=DEFAULT_WORLD, help="gameplay/world authoring directory")
    parser.add_argument("--scene", type=int, default=2, help="scene/mode id to open")
    parser.add_argument("--snapshot", type=Path, help="render the viewer window to an image and exit (for headless inspection)")
    args = parser.parse_args(argv)
    maps = read_map_catalog(args.assets)
    if not maps:
        parser.error(f"No extracted field maps found under {args.assets}")
    if args.scene not in {item.scene_id for item in maps}:
        parser.error(f"Scene {args.scene} has no extracted field map")
    app = QApplication(sys.argv[:1])
    windows_font = Path(os.environ.get("WINDIR", "C:/Windows")) / "Fonts" / "arial.ttf"
    if windows_font.is_file():
        QFontDatabase.addApplicationFont(str(windows_font))
    app.setFont(QFont("Arial", 9))
    viewer = MapEntityViewer(args.assets, args.world, args.scene)
    if args.snapshot:
        render_snapshot(viewer, args.snapshot)
        return 0
    viewer.show()
    return app.exec()


if __name__ == "__main__":
    raise SystemExit(main())
