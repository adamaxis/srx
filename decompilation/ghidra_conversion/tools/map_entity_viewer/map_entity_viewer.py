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

from PyQt6.QtCore import QPointF, QRectF, Qt
from PyQt6.QtGui import QAction, QActionGroup, QBrush, QColor, QFont, QFontDatabase, QImage, QPainter, QPen, QPixmap
from PyQt6.QtWidgets import (
    QApplication,
    QComboBox,
    QDialog,
    QDialogButtonBox,
    QDockWidget,
    QFormLayout,
    QGraphicsItem,
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
class EntityDefinition:
    entity_id: str
    name: str
    category: str = "unreviewed"
    evidence: str = ""
    parts: list[EntityPart] = field(default_factory=list)
    interactables: list[Interactable] = field(default_factory=list)

    @property
    def bounds(self) -> tuple[int, int, int, int]:
        if not self.parts:
            return (0, 0, 1, 1)
        x0 = min(part.local_x for part in self.parts)
        y0 = min(part.local_y for part in self.parts)
        x1 = max(part.local_x + part.width for part in self.parts)
        y1 = max(part.local_y + part.height for part in self.parts)
        return (x0, y0, max(1, x1 - x0), max(1, y1 - y0))


@dataclass
class EntityInstance:
    instance_id: str
    entity_id: str
    origin_x: int
    origin_y: int


class WorldContent:
    ENTITY_HEADER = ("entity_id", "name", "category", "evidence")
    PART_HEADER = (
        "part_id", "source_kind", "source_scene", "source_x", "source_y",
        "width", "height", "local_x", "local_y", "draw_layer", "role", "evidence",
    )
    INTERACTION_HEADER = (
        "interaction_id", "local_x", "local_y", "width", "height",
        "lua_handler", "source_event", "evidence",
    )
    AREA_HEADER = ("instance_id", "entity_id", "origin_x", "origin_y")

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

    @staticmethod
    def lua_quote(value: str) -> str:
        escaped = value.replace("\\", "\\\\").replace('"', '\\"')
        escaped = escaped.replace("\r", "\\r").replace("\n", "\\n").replace("\t", "\\t")
        return f'"{escaped}"'

    def export_lua(self, destination: Path) -> None:
        entity_lines: list[str] = []
        entity_root = self.root / "entities"
        if entity_root.is_dir():
            for directory in sorted(path for path in entity_root.iterdir() if path.is_dir()):
                entity_id = safe_id(directory.name)
                entity = self.load_entity(entity_id)
                if entity is None:
                    continue
                part_values = []
                for part in entity.parts:
                    part_values.append(
                        "{ "
                        f"id = {self.lua_quote(part.part_id)}, "
                        f"source_kind = {self.lua_quote(part.source_kind)}, "
                        f"source_scene = {part.source_scene}, source_x = {part.source_x}, source_y = {part.source_y}, "
                        f"width = {part.width}, height = {part.height}, local_x = {part.local_x}, local_y = {part.local_y}, "
                        f"draw_layer = {self.lua_quote(part.draw_layer)}, role = {self.lua_quote(part.role)}, "
                        f"evidence = {self.lua_quote(part.evidence)} "
                        "}"
                    )
                interaction_values = []
                for interaction in entity.interactables:
                    interaction_values.append(
                        "{ "
                        f"id = {self.lua_quote(interaction.interaction_id)}, "
                        f"local_x = {interaction.local_x}, local_y = {interaction.local_y}, "
                        f"width = {interaction.width}, height = {interaction.height}, "
                        f"lua_handler = {self.lua_quote(interaction.lua_handler)}, "
                        f"source_event = {self.lua_quote(interaction.source_event)}, "
                        f"evidence = {self.lua_quote(interaction.evidence)} "
                        "}"
                    )
                entity_lines.append(
                    f"    [{self.lua_quote(entity.entity_id)}] = {{ "
                    f"id = {self.lua_quote(entity.entity_id)}, name = {self.lua_quote(entity.name)}, "
                    f"category = {self.lua_quote(entity.category)}, evidence = {self.lua_quote(entity.evidence)}, "
                    f"parts = {{ {', '.join(part_values)} }}, "
                    f"interactables = {{ {', '.join(interaction_values)} }} "
                    "},"
                )

        area_lines: list[str] = []
        area_root = self.root / "areas"
        if area_root.is_dir():
            for path in sorted(area_root.glob("scene-*.tsv")):
                match = re.fullmatch(r"scene-(\d+)\.tsv", path.name)
                if not match:
                    continue
                scene_id = int(match.group(1))
                instances = self.load_scene(scene_id)
                values = ", ".join(
                    "{ "
                    f"instance_id = {self.lua_quote(item.instance_id)}, "
                    f"entity_id = {self.lua_quote(item.entity_id)}, "
                    f"origin_x = {item.origin_x}, origin_y = {item.origin_y} "
                    "}"
                    for item in instances
                )
                area_lines.append(f"    [{scene_id}] = {{ {values} }},")

        content = (
            "-- Generated by ghidra_conversion/tools/map_entity_viewer/map_entity_viewer.py from gameplay/world TSV files.\n"
            "return {\n"
            "  entities = {\n" + "\n".join(entity_lines) + "\n  },\n"
            "  areas = {\n" + "\n".join(area_lines) + "\n  },\n"
            "}\n"
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


def read_collision_rectangles(asset_root: Path, scene_id: int) -> list[tuple[int, int, int, int, int]]:
    path = asset_root / "records" / "field" / "scenes" / f"scene-{scene_id:02d}" / "terrain-collision-rectangles.bin"
    if not path.is_file():
        return []
    data = path.read_bytes()
    if len(data) % 10:
        return []
    result = []
    for offset in range(0, len(data), 10):
        words = [int.from_bytes(data[offset + i:offset + i + 2], "big") for i in range(0, 10, 2)]
        x0, x1, y0, y1, code = words
        if x1 >= x0 and y1 >= y0:
            result.append((x0, y0, x1 - x0 + 1, y1 - y0 + 1, code))
    return result


def read_event_rectangles(repo_root: Path, scene_id: int) -> list[tuple[int, int, int, int, str]]:
    path = repo_root / "decompilation" / "docs" / "field-map-trigger-records.tsv"
    _, rows = read_tsv(path)
    result = []
    for row in rows:
        if as_int(row, "scene_id", -1) != scene_id:
            continue
        x0 = as_int(row, "x_start_raw")
        x1 = as_int(row, "x_end_raw")
        y0 = as_int(row, "y_start_raw")
        y1 = as_int(row, "y_end_raw")
        if x1 >= x0 and y1 >= y0:
            result.append((x0, y0, x1 - x0 + 1, y1 - y0 + 1, row.get("event_id_low_5_bits", "")))
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
    def __init__(self, on_region, on_select, on_cursor):
        super().__init__()
        self.setScene(QGraphicsScene(self))
        self.setRenderHints(QPainter.RenderHint.Antialiasing)
        self.setTransformationAnchor(QGraphicsView.ViewportAnchor.AnchorUnderMouse)
        self.setResizeAnchor(QGraphicsView.ViewportAnchor.AnchorViewCenter)
        self.setDragMode(QGraphicsView.DragMode.NoDrag)
        self.on_region = on_region
        self.on_select = on_select
        self.on_cursor = on_cursor
        self.mode = "select"
        self.map_info: MapInfo | None = None
        self.background_item: QGraphicsPixmapItem | None = None
        self.grid: GridOverlay | None = None
        self.collision_rectangles: list[tuple[int, int, int, int, int]] = []
        self.event_rectangles: list[tuple[int, int, int, int, str]] = []
        self.instances: list[EntityInstance] = []
        self.entities: dict[str, EntityDefinition] = {}
        self.show_grid = True
        self.show_collision = False
        self.show_events = True
        self.show_priority = True
        self.assets_root = DEFAULT_ASSETS
        self.repo_root = ROOT
        self.drag_start: QPointF | None = None
        self.drag_rect: QGraphicsRectItem | None = None
        self.annotation_items: list[QGraphicsItem] = []
        self.setBackgroundBrush(QColor(28, 29, 33))

    def load_scene(self, info: MapInfo, assets: Path, repo_root: Path, instances: list[EntityInstance], entities: dict[str, EntityDefinition]) -> None:
        self.map_info = info
        self.assets_root = assets
        self.repo_root = repo_root
        self.instances = instances
        self.entities = entities
        self.collision_rectangles = read_collision_rectangles(assets, info.scene_id)
        self.event_rectangles = read_event_rectangles(repo_root, info.scene_id)
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
        self.resetTransform()
        self.fitInView(self.sceneRect(), Qt.AspectRatioMode.KeepAspectRatio)

    def set_layer_options(self, priority: bool, grid: bool, collision: bool, events: bool) -> None:
        changed_priority = self.show_priority != priority
        self.show_priority = priority
        self.show_grid = grid
        self.show_collision = collision
        self.show_events = events
        if changed_priority and self.map_info is not None:
            info = self.map_info
            self.load_scene(info, self.assets_root, self.repo_root, self.instances, self.entities)
            return
        if self.grid:
            self.grid.setVisible(grid)
        self.draw_evidence_overlays()

    def draw_evidence_overlays(self) -> None:
        for item in getattr(self, "evidence_items", []):
            self.scene().removeItem(item)
        self.evidence_items: list[QGraphicsItem] = []
        if self.show_collision:
            for x, y, width, height, code in self.collision_rectangles:
                rect = self.scene().addRect(QRectF(x, y, width, height), QPen(QColor(255, 55, 55, 210), 1), QColor(255, 35, 35, 22))
                rect.setZValue(8)
                rect.setToolTip(f"ROM terrain collision: code 0x{code:04X}; {x},{y} {width}x{height}")
                self.evidence_items.append(rect)
        if self.show_events:
            for x, y, width, height, event in self.event_rectangles:
                rect = self.scene().addRect(QRectF(x, y, width, height), QPen(QColor(255, 190, 35, 245), 2), QColor(255, 190, 35, 24))
                rect.setZValue(9)
                rect.setToolTip(f"ROM event trigger {event}: {x},{y} {width}x{height}")
                self.evidence_items.append(rect)

    def redraw_annotations(self) -> None:
        for item in self.annotation_items:
            self.scene().removeItem(item)
        self.annotation_items = []
        colors = {
            "unreviewed": QColor(255, 245, 90, 220),
            "building": QColor(80, 210, 255, 225),
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
            self.annotation_items.append(box)
            label = self.scene().addText(f"{entity.name} [{entity.category}]")
            label.setDefaultTextColor(color)
            label.setPos(instance.origin_x + x0 + 2, instance.origin_y + y0 + 2)
            label.setZValue(13)
            label.setData(0, instance.instance_id)
            self.annotation_items.append(label)
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
                self.annotation_items.append(part_item)
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
                self.annotation_items.append(hit)

    def wheelEvent(self, event) -> None:
        factor = 1.15 if event.angleDelta().y() > 0 else 1 / 1.15
        self.scale(factor, factor)

    def mousePressEvent(self, event) -> None:
        if event.button() == Qt.MouseButton.MiddleButton:
            self.setDragMode(QGraphicsView.DragMode.ScrollHandDrag)
            super().mousePressEvent(event)
            return
        if event.button() == Qt.MouseButton.LeftButton and self.mode in {"add_entity", "add_part", "add_interaction"}:
            self.drag_start = self.mapToScene(event.position().toPoint())
            self.drag_rect = self.scene().addRect(QRectF(self.drag_start, self.drag_start), QPen(QColor(255, 255, 255), 2, Qt.PenStyle.DashLine))
            self.drag_rect.setZValue(30)
            event.accept()
            return
        if event.button() == Qt.MouseButton.LeftButton:
            point = self.mapToScene(event.position().toPoint())
            self.on_select(point)
        super().mousePressEvent(event)

    def mouseMoveEvent(self, event) -> None:
        point = self.mapToScene(event.position().toPoint())
        self.on_cursor(int(point.x()), int(point.y()))
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
        self.selected_instance: EntityInstance | None = None
        self.dirty = False
        self.mode = "select"
        self._loading_fields = False
        self.setWindowTitle("Shadowrun Map and Entity Viewer")
        self.resize(1500, 980)
        self.setFont(QFont("Arial", 9))

        self.map_view = FieldMapView(self.region_created, self.select_at, self.cursor_moved)
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
        layout.addWidget(self.parts_list)
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
        self.info = QLabel("Select an annotation to inspect its parts and interactions.")
        self.info.setWordWrap(True)
        layout.addWidget(self.info)
        self.save_button = QPushButton("Save scene and definitions")
        self.save_button.clicked.connect(self.save_all)
        self.delete_button = QPushButton("Remove selected instance")
        self.delete_button.clicked.connect(self.remove_selected)
        layout.addWidget(self.save_button)
        layout.addWidget(self.delete_button)
        dock.setWidget(panel)
        dock.setMinimumWidth(350)
        dock.setMaximumWidth(460)
        self.addDockWidget(Qt.DockWidgetArea.RightDockWidgetArea, dock)

    def _build_toolbar(self) -> None:
        bar = self.addToolBar("Map tools")
        bar.setMovable(False)
        self.tool_buttons: dict[str, QPushButton] = {}
        scene_label = QLabel("  View layers: ")
        bar.addWidget(scene_label)
        self.priority_toggle = self._add_toggle(bar, "Priority pixels", True)
        self.grid_toggle = self._add_toggle(bar, "32 px grid", True)
        self.collision_toggle = self._add_toggle(bar, "ROM collision", False)
        self.event_toggle = self._add_toggle(bar, "ROM event triggers", True)
        bar.addSeparator()
        self._add_tool(bar, "Select", "select")
        self._add_tool(bar, "Add object region", "add_entity")
        self._add_tool(bar, "Add visual part", "add_part")
        self._add_tool(bar, "Add interactable", "add_interaction")
        bar.addSeparator()
        legend = QLabel("Unreviewed: yellow   Building: cyan   Decal: magenta   Prop: green")
        bar.addWidget(legend)
        bar.addSeparator()
        self.coordinate_label = QLabel(" x: —  y: — ")
        bar.addWidget(self.coordinate_label)

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
        if mode in {"add_interaction", "add_part"} and self.selected_instance is None:
            self.statusBar().showMessage("Select an entity instance before adding a visual part or interaction region.", 5000)
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
        }[mode])

    def layers_changed(self) -> None:
        self.map_view.set_layer_options(
            self.priority_toggle.isChecked(), self.grid_toggle.isChecked(),
            self.collision_toggle.isChecked(), self.event_toggle.isChecked(),
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
        self.entities = {}
        for instance in self.instances:
            entity = self.world.load_entity(instance.entity_id)
            if entity:
                self.entities[entity.entity_id] = entity
        self.selected_instance = None
        self._refresh_list()
        self.map_view.load_scene(self.map_by_id[scene_id], self.assets, self.repo_root, self.instances, self.entities)
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

    def _update_scene_summary(self) -> None:
        info = self.map_by_id[self.scene_id]
        unreviewed = sum(1 for instance in self.instances if self.entities.get(instance.entity_id, EntityDefinition("", "")).category == "unreviewed")
        self.statusBar().showMessage(
            f"Scene {self.scene_id:02d}: {info.width}×{info.height}; {len(self.instances)} annotations, {unreviewed} unreviewed. "
            f"Source encoding: {info.metatile_encoding}; collision regions: {len(self.map_view.collision_rectangles)}; "
            f"event regions: {len(self.map_view.event_rectangles)}."
        )

    def _show_properties(self, instance: EntityInstance | None) -> None:
        self._loading_fields = True
        enabled = instance is not None
        for widget in (
            self.name_edit, self.category_combo, self.x_spin, self.y_spin,
            self.evidence_edit, self.delete_button, self.parts_list,
            self.interactions_list, self.edit_part_button,
            self.remove_part_button, self.edit_interaction_button,
            self.remove_interaction_button,
        ):
            widget.setEnabled(enabled)
        if not enabled:
            self.name_edit.clear(); self.evidence_edit.clear()
            self.x_spin.setValue(0); self.y_spin.setValue(0)
            self.category_combo.setCurrentText("unreviewed")
            self.parts_list.clear()
            self.interactions_list.clear()
            self.info.setText("Select an annotation to inspect its source rectangle, visual-part roles, and Lua interaction regions.")
            self._loading_fields = False
            return
        entity = self.entities[instance.entity_id]
        selected_part = self.parts_list.currentItem().data(Qt.ItemDataRole.UserRole) if self.parts_list.currentItem() else None
        selected_interaction = self.interactions_list.currentItem().data(Qt.ItemDataRole.UserRole) if self.interactions_list.currentItem() else None
        self.parts_list.blockSignals(True)
        self.parts_list.clear()
        for part in entity.parts:
            item = QListWidgetItem(f"{part.part_id}: {part.role} / {part.draw_layer} @ {part.local_x},{part.local_y}")
            item.setData(Qt.ItemDataRole.UserRole, part.part_id)
            self.parts_list.addItem(item)
            if part.part_id == selected_part:
                self.parts_list.setCurrentItem(item)
        self.parts_list.blockSignals(False)
        self.interactions_list.blockSignals(True)
        self.interactions_list.clear()
        for interaction in entity.interactables:
            item = QListWidgetItem(f"{interaction.interaction_id}: {interaction.width}x{interaction.height}; {interaction.lua_handler or '(no Lua handler)'}")
            item.setData(Qt.ItemDataRole.UserRole, interaction.interaction_id)
            self.interactions_list.addItem(item)
            if interaction.interaction_id == selected_interaction:
                self.interactions_list.setCurrentItem(item)
        self.interactions_list.blockSignals(False)
        self.name_edit.setText(entity.name)
        self.category_combo.setCurrentText(entity.category if entity.category in ENTITY_CATEGORIES else "unreviewed")
        self.x_spin.setValue(instance.origin_x); self.y_spin.setValue(instance.origin_y)
        self.evidence_edit.setText(entity.evidence)
        part_summary = "\n".join(
            f"• {part.part_id}: {part.role}, source scene {part.source_scene:02d} at {part.source_x},{part.source_y} {part.width}×{part.height}; {part.draw_layer}"
            for part in entity.parts
        ) or "No visual parts"
        interaction_summary = "\n".join(
            f"• {item.interaction_id}: {item.local_x},{item.local_y} {item.width}×{item.height}; Lua: {item.lua_handler or '(unassigned)'}; event: {item.source_event or '(unmapped)'}"
            for item in entity.interactables
        ) or "No interaction regions"
        self.info.setText(f"ID: {entity.entity_id}\nVisual parts:\n{part_summary}\nInteractions:\n{interaction_summary}\n\nImage pixels remain in the extracted map; this editor changes annotations and scene placement metadata.")
        map_info = self.map_by_id[self.scene_id]
        bounds = QRectF(
            instance.origin_x + entity.bounds[0],
            instance.origin_y + entity.bounds[1],
            entity.bounds[2],
            entity.bounds[3],
        )
        collision_hits = sum(
            1 for x, y, width, height, _code in self.map_view.collision_rectangles
            if bounds.intersects(QRectF(x, y, width, height))
        )
        event_hits = sum(
            1 for x, y, width, height, _event in self.map_view.event_rectangles
            if bounds.intersects(QRectF(x, y, width, height))
        )
        evidence_summary = (
            f"\nMap evidence: {map_info.tilemap_width}×{map_info.tilemap_height} metatile cells, "
            f"{map_info.metatile_encoding}; primary {map_info.primary_tilemap}, "
            f"secondary {map_info.secondary_tilemap}. Region intersects {collision_hits} ROM collision rectangles "
            f"and {event_hits} ROM event triggers. The base bitmap is precomposed; the priority toggle shows its sparse overlay."
        )
        self.info.setText(self.info.text() + evidence_summary)
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

    def select_at(self, point: QPointF) -> None:
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

    def select_instance(self, instance: EntityInstance | None) -> None:
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

    def list_selection_changed(self, current, previous) -> None:
        if current is None:
            return
        target = current.data(Qt.ItemDataRole.UserRole)
        instance = next((item for item in self.instances if item.instance_id == target), None)
        self.select_instance(instance)

    def region_created(self, rect: QRectF) -> None:
        if rect.width() < 2 or rect.height() < 2:
            return
        x, y = round(rect.x()), round(rect.y())
        width, height = max(1, round(rect.width())), max(1, round(rect.height()))
        if self.mode == "add_interaction":
            self._add_interactable(x, y, width, height)
            return
        if self.mode == "add_part":
            self._add_visual_part(x, y, width, height)
            return
        self._add_entity_region(x, y, width, height)

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
            for entity in self.entities.values():
                self.world.save_entity(entity)
            self.world.save_scene(self.scene_id, self.instances)
            self.world.export_lua(self.world.root.parent / "scripts" / "world_content_data.lua")
        except (OSError, ValueError) as error:
            QMessageBox.critical(self, "Could not save map content", str(error))
            return
        self.dirty = False
        self.statusBar().showMessage(f"Saved scene {self.scene_id:02d} and {len(self.entities)} entity definitions under {self.world.root}", 6000)

    def keyPressEvent(self, event) -> None:
        if event.key() == Qt.Key.Key_S and event.modifiers() & Qt.KeyboardModifier.ControlModifier:
            self.save_all()
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
