#!/usr/bin/env python3
"""Inspect extracted field maps and author entity annotations.

The extracted mode-N bitmap is already a composed picture. This tool does not
pretend that pixels identify semantic objects: new regions start unreviewed,
and ROM priority, collision, and event data are shown as independent evidence.
Entity definitions and placements are saved as human-readable TSV files under
gameplay/world. Map-event rows can also be compiled into the current typed Lua
event registry without duplicating their placement or state-gate data.
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
from PyQt6.QtGui import QAction, QActionGroup, QBrush, QColor, QFont, QFontDatabase, QImage, QPainter, QPainterPath, QPen, QPixmap, QPolygonF
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
    QTabWidget,
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


def read_lua_string_table(path: Path) -> dict[str, str]:
    """Read the quoted key/value strings from the imported Lua text catalog."""
    if not path.is_file():
        return {}
    source = path.read_text(encoding="utf-8")
    pair_pattern = re.compile(
        r'\[\s*"((?:\\.|[^"\\])*)"\s*\]\s*=\s*"((?:\\.|[^"\\])*)"',
        re.DOTALL,
    )

    def unescape(value: str) -> str:
        replacements = {"n": "\n", "r": "\r", "t": "\t", '"': '"', "\\": "\\"}
        return re.sub(
            r"\\([nrt\"\\])",
            lambda match: replacements[match.group(1)],
            value,
        )

    return {
        unescape(match.group(1)): unescape(match.group(2))
        for match in pair_pattern.finditer(source)
    }


def extract_lua_block(source: str, opening: int, left: str = "{", right: str = "}") -> str:
    """Return a balanced Lua block, ignoring braces inside strings/comments."""
    if opening < 0 or opening >= len(source) or source[opening] != left:
        return ""
    depth = 0
    quote = ""
    escaped = False
    line_comment = False
    for index in range(opening, len(source)):
        char = source[index]
        next_char = source[index + 1] if index + 1 < len(source) else ""
        if line_comment:
            if char == "\n":
                line_comment = False
            continue
        if quote:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == quote:
                quote = ""
            continue
        if char == "-" and next_char == "-":
            line_comment = True
            continue
        if char in ("'", '"'):
            quote = char
        elif char == left:
            depth += 1
        elif char == right:
            depth -= 1
            if depth == 0:
                return source[opening + 1:index]
    return ""


def read_shop_profile_details(
    repo_root: Path,
    profile: str,
    text_catalog: dict[str, str],
    include_source_details: bool = True,
) -> list[str]:
    """Summarize the selected Lua shop profile without executing Lua code."""
    shop_path = repo_root / "decompilation" / "gameplay" / "scripts" / "shop.lua"
    items_path = repo_root / "decompilation" / "gameplay" / "scripts" / "items.lua"
    try:
        shop_source = shop_path.read_text(encoding="utf-8")
    except OSError:
        return [f"Shop source is unavailable: {shop_path}"]
    catalog_match = re.search(r"\bShop\.catalog\s*=\s*\{", shop_source)
    if not catalog_match:
        return [f"Shop catalog declaration is unavailable in {shop_path}"]
    entry_match = re.search(
        rf"(?m)^\s{{4}}{re.escape(profile)}\s*=\s*\{{",
        shop_source[catalog_match.end():],
    )
    if not entry_match:
        return [f"Shop profile '{profile}' is not declared in {shop_path}"]
    entry_start = catalog_match.end() + entry_match.start()
    opening = shop_source.find("{", entry_start)
    body = extract_lua_block(shop_source, opening)
    if not body:
        return [f"Shop profile '{profile}' could not be read as a balanced Lua table."]

    lines = [f"Shop profile source: {shop_path}"] if include_source_details else []
    title_key = f"shop.service.option.{profile}"
    title = text_catalog.get(title_key, title_key)
    lines.append(
        f"Shop title: {title} [{title_key}]" if include_source_details
        else f"Shop: {title}"
    )

    item_names: dict[int, str] = {}
    item_ids: dict[int, str] = {}
    item_prices: dict[int, tuple[int, int]] = {}
    try:
        item_source = items_path.read_text(encoding="utf-8")
    except OSError:
        item_source = ""
    for match in re.finditer(
        r'define_item\(\s*"([^"]+)"\s*,\s*(\d+)\s*,\s*"[^"]+"\s*,\s*(\d+)\s*,',
        item_source,
    ):
        code = int(match.group(2))
        purchase_base = int(match.group(3))
        item_ids[code] = match.group(1)
        item_prices[code] = (purchase_base, purchase_base // 2)
        item_names[code] = re.sub(
            r"\s+",
            " ",
            text_catalog.get(
                f"items.name.{code}", match.group(1).replace("_", " ")
            ).replace("||", " "),
        ).strip()
    duplicate_names = {
        name for name in item_names.values()
        if list(item_names.values()).count(name) > 1
    }
    for code, name in tuple(item_names.items()):
        if name in duplicate_names and item_ids.get(code, "").startswith("armor_"):
            subtype = item_ids[code][len("armor_"):]
            item_names[code] = f"{name} ({friendly_identifier(subtype).lower()})"
    # The Ares offer type 0x45 is the special universal-clips purchase row,
    # named by the shop's own extracted title and description rather than an
    # ordinary item-template entry.
    universal_clips_name = text_catalog.get(
        "shop.universal_clips.title", "Universal Gun Clips"
    )
    if universal_clips_name:
        item_names[0x45] = re.sub(
            r"\s+", " ", universal_clips_name.replace("||", " ")
        ).strip()

    group_summaries: list[str] = []
    shop_actions: list[str] = []
    cursor = 0
    while True:
        match = re.search(r"\bgroup\s*\(", body[cursor:])
        if not match:
            break
        group_open = cursor + match.end() - 1
        group_body = extract_lua_block(body, group_open, "(", ")")
        if not group_body:
            break
        group_id_match = re.match(r'\s*"([^"]+)"', group_body)
        group_id = group_id_match.group(1) if group_id_match else "unnamed group"
        heading_ids = re.findall(r'action_heading\(\s*"([^"]+)"\s*\)', group_body)
        headings = [
            re.sub(
                r"\s+", " ",
                text_catalog.get(
                    f"shop.action_heading.option.{heading_id}", ""
                ).replace("||", " "),
            ).strip()
            for heading_id in heading_ids
        ]
        headings = [heading for heading in headings if heading]
        offer_ids = [
            int(value, 0)
            for value in re.findall(r"\boffer\s*\(\s*(0x[0-9A-Fa-f]+|\d+)", group_body)
        ]
        if offer_ids:
            stocks = []
            for code in offer_ids:
                name = item_names.get(
                    code,
                    f"Unidentified item (source type 0x{code:02X})"
                    if include_source_details else "Unidentified item",
                )
                price = item_prices.get(code)
                if price is not None:
                    purchase_base, resale_base = price
                    name += (
                        f" (base buy ${purchase_base}; base resale ${resale_base})"
                    )
                stocks.append(name)
            group_name = {
                "weapons": "Weapons",
                "other_items": "Other items",
                "armor": "Armor",
                "sell": "Items you can sell",
            }.get(group_id, friendly_identifier(group_id))
            group_summaries.append(f"{group_name}: {', '.join(stocks)}")
        elif group_id == "sell":
            shop_actions.append("Sell items from your inventory")
        elif headings:
            shop_actions.extend(
                " ".join(headings[index:index + 2])
                for index in range(0, len(headings), 2)
            )
        cursor = group_open + len(group_body) + 2
    if group_summaries:
        lines.append("Items offered:")
        lines.extend(f"  {summary}" for summary in group_summaries)
        if item_prices:
            lines.append(
                "Base prices are before the runner- and transaction-specific adjustments shown by the shop."
            )
    if shop_actions:
        lines.append("Other shop options:")
        lines.extend(f"  {action}" for action in dict.fromkeys(shop_actions))

    text_keys = sorted(set(re.findall(r'"(shop\.[^"]+)"', body)))
    text_keys.extend(
        f"shop.action_heading.option.{key}"
        for key in re.findall(r'action_heading\("([^"]+)"\)', body)
    )
    text_keys.append(title_key)
    resolved_keys = list(dict.fromkeys(text_keys))
    if resolved_keys:
        lines.append(
            "Shop text and headings from the Lua profile:" if include_source_details
            else "Shop instructions and headings:"
        )
        lines.extend(
            f"  {key}: {text_catalog.get(key, '(text key is not imported)')}"
            if include_source_details else
            f"  {text_catalog.get(key, '(text not imported)')}"
            for key in resolved_keys
        )
    return lines


def friendly_identifier(value: str) -> str:
    """Turn an internal content label into readable words for the viewer."""
    words = re.sub(r"[_\-.]+", " ", value.strip())
    words = re.sub(r"\s+", " ", words)
    return words[:1].upper() + words[1:] if words else "Unspecified"


def lua_named_table_body(source: str, declaration: str, name: str) -> str:
    """Read one named entry from a declared Lua table without executing Lua."""
    table_match = re.search(declaration, source)
    if not table_match:
        return ""
    table_open = source.find("{", table_match.start())
    table_body = extract_lua_block(source, table_open)
    entry_match = re.search(
        rf'(?m)^\s*\[\s*"{re.escape(name)}"\s*\]\s*=\s*\{{',
        table_body,
    )
    if not entry_match:
        entry_match = re.search(
            rf"(?m)^\s*{re.escape(name)}\s*=\s*\{{", table_body
        )
    if not entry_match:
        return ""
    entry_open = table_body.find("{", entry_match.start())
    return extract_lua_block(table_body, entry_open)


def read_dialogue_profile_details(
    repo_root: Path,
    content_profile: str,
    speaker_id: str,
    text_catalog: dict[str, str],
    include_source_details: bool = True,
) -> list[str]:
    """Resolve authored dialogue pages, portraits, menus, and choice labels."""
    dialogue_path = repo_root / "decompilation" / "gameplay" / "scripts" / "dialogue.lua"
    try:
        source = dialogue_path.read_text(encoding="utf-8")
    except OSError:
        return [f"Dialogue source is unavailable: {dialogue_path}"]

    lines: list[str] = []
    if speaker_id:
        speaker_body = lua_named_table_body(
            source, r"\blocal\s+speakers\s*=\s*\{", speaker_id
        )
        if speaker_body:
            name = re.search(r'\bname\s*=\s*"([^"]+)"', speaker_body)
            profile = re.search(r'\btext_profile\s*=\s*"([^"]+)"', speaker_body)
            portrait = re.search(r'\bportrait_alias\s*=\s*"([^"]+)"', speaker_body)
            speaker_name = name.group(1) if name else friendly_identifier(speaker_id)
            if include_source_details:
                lines.append(
                    f"Speaker from dialogue.lua: {speaker_name}; "
                    f"portrait resource alias: {portrait.group(1) if portrait else '(not declared)'}"
                )
            else:
                lines.append(f"Speaker: {speaker_name}")
            if profile and include_source_details:
                lines.append(f"Speaker text profile: {profile.group(1)}")
            events_match = re.search(r"\bevents\s*=\s*\{", speaker_body)
            if events_match:
                events_open = speaker_body.find("{", events_match.start())
                events_body = extract_lua_block(speaker_body, events_open)
                event_pattern = re.compile(
                    r'(?m)^ {12}(?:\[\s*"([^"]+)"\s*\]|([A-Za-z_][A-Za-z0-9_-]*))\s*=\s*\{'
                )
                pages: list[str] = []
                for match in event_pattern.finditer(events_body):
                    event_name = match.group(1) or match.group(2)
                    page_open = events_body.find("{", match.start())
                    page_body = extract_lua_block(events_body, page_open)
                    title = re.search(r'\btitle_key\s*=\s*"([^"]*)"', page_body)
                    continuation = re.search(
                        r'\bcontinuation_key\s*=\s*"([^"]*)"', page_body
                    )
                    if not title:
                        continue
                    title_key = title.group(1)
                    text = text_catalog.get(title_key, "(text key is not imported)")
                    page = (
                        f"{event_name}: {text} [{title_key}]" if include_source_details
                        else f"{friendly_identifier(event_name)}: {text}"
                    )
                    if continuation and continuation.group(1):
                        key = continuation.group(1)
                        continuation_text = text_catalog.get(key, key)
                        page += (
                            f"; continuation: {continuation_text} [{key}]"
                            if include_source_details else
                            f"; then: {continuation_text}"
                        )
                    pages.append(page)
                if pages:
                    lines.append(
                        "Speaker dialogue pages in dialogue.lua:" if include_source_details
                        else "Dialogue:"
                    )
                    lines.extend(f"  {page}" for page in pages)

    if content_profile:
        profile_body = lua_named_table_body(
            source, r"\blocal\s+menu_profiles\s*=\s*\{", content_profile
        )
        if profile_body:
            page_pattern = re.compile(
                r"(?m)^ {8}([A-Za-z_][A-Za-z0-9_]*)\s*=\s*\{"
            )
            menu_pages: list[str] = []
            for match in page_pattern.finditer(profile_body):
                page_id = match.group(1)
                page_open = profile_body.find("{", match.start())
                page_body = extract_lua_block(profile_body, page_open)
                title = re.search(r'\btitle_key\s*=\s*"([^"]+)"', page_body)
                title_text_value = (
                    text_catalog.get(title.group(1), '(text key is not imported)')
                    if title else "(no title key declared)"
                )
                title_text = (
                    f"{title_text_value} [{title.group(1)}]"
                    if title and include_source_details else title_text_value
                )
                choice_match = re.search(r"\bchoice_label_keys\s*=\s*\{", page_body)
                choices: list[str] = []
                if choice_match:
                    choice_open = page_body.find("{", choice_match.start())
                    choice_body = extract_lua_block(page_body, choice_open)
                    for choice in re.finditer(
                        r'(?:\[\s*"([^"]+)"\s*\]|([A-Za-z_][A-Za-z0-9_]*))\s*=\s*"([^"]+)"',
                        choice_body,
                    ):
                        choice_id = choice.group(1) or choice.group(2)
                        text_key = choice.group(3)
                        label = text_catalog.get(text_key, "(text key is not imported)")
                        choices.append(
                            f"{choice_id}={label} [{text_key}]"
                            if include_source_details else label
                        )
                menu_pages.append(
                    f"{page_id}: {title_text}; choices: " +
                    (", ".join(choices) if choices else "none declared")
                    if include_source_details else
                    f"{friendly_identifier(page_id)}: {title_text}; choices: " +
                    (", ".join(choices) if choices else "none declared")
                )
            if menu_pages:
                lines.append(
                    f"Menu pages from Lua profile {content_profile}:"
                    if include_source_details else "Menu pages:"
                )
                lines.extend(f"  {page}" for page in menu_pages)
        elif include_source_details:
            lines.append(
                f"Menu profile '{content_profile}' is not declared in dialogue.lua."
            )
    return lines


def read_lua_quest_module_details(repo_root: Path, site: dict[str, str]) -> list[str]:
    """Expose quest/script entry points and state facts linked to a map route."""
    script_root = repo_root / "decompilation" / "gameplay" / "scripts"
    module_names = [
        name.strip() for name in site.get("lua_modules", "").split(";")
        if name.strip().endswith(".lua")
    ]
    if not module_names:
        return []
    lines = ["Connected game scripts and story-state checks:"]
    for module_name in module_names:
        path = script_root / module_name
        try:
            source = path.read_text(encoding="utf-8")
        except OSError:
            lines.append(f"  {module_name}: source file is unavailable")
            continue
        functions = list(dict.fromkeys(re.findall(
            r"(?m)^function\s+([A-Za-z_][A-Za-z0-9_.:]*)\s*\(", source
        )))
        state_keys = sorted(set(re.findall(
            r"\bgame\.(?:state|quests|story)\.([A-Za-z_][A-Za-z0-9_]*)", source
        )))
        service_calls = sorted(set(re.findall(
            r'game\.services\.call\(\s*"([^"]+)"\s*,\s*"([^"]+)"', source
        )))
        facts = []
        if functions:
            facts.append("functions " + ", ".join(functions[:12]))
        if state_keys:
            facts.append("state facts " + ", ".join(state_keys))
        if service_calls:
            facts.append("host actions " + ", ".join(
                f"{module}.{action}" for module, action in service_calls
            ))
        lines.append(f"  {module_name}: " + ("; ".join(facts) or "no public facts found"))
    return lines


def read_site_content_details(
    repo_root: Path, site: dict[str, str], text_catalog: dict[str, str],
    scene_id: int, include_source_details: bool = True,
) -> list[str]:
    lines: list[str] = []

    def readable_text(key: str) -> str:
        return text_catalog.get(key, "(text key is not imported)").replace(
            "\\n", "\n"
        ).replace("\\t", "\t")

    contract_location = site.get("rom_contract_location_index", "").strip()
    if contract_location and include_source_details:
        try:
            location_index = int(contract_location, 0)
        except ValueError:
            location_index = -1
        profiles = read_rom_contract_area_profiles(repo_root)
        area = profiles.get(scene_id, {})
        locations = area.get("locations", {})
        if location_index >= 0 and location_index in locations:
            lines.append(
                "ROM run-location name: "
                f"{locations[location_index]} (option {location_index} in "
                f"gameplay/scripts/run_support.lua). This names a contract "
                "destination; the field event route is shown separately."
            )
        else:
            lines.append(
                f"ROM run-location option {contract_location} has no named "
                "entry in the current area table."
            )
    text_keys = [
        key.strip()
        for key in site.get("text_keys", "").split(";")
        if key.strip()
    ]
    if text_keys:
        lines.append("Dialogue, choices, and shop text shown by this route:")
        lines.extend(
            f"  {key}: {readable_text(key)}" if include_source_details
            else f"  {readable_text(key)}"
            for key in text_keys
        )
    reference_text_keys = [
        key.strip()
        for key in site.get("reference_text_keys", "").split(";")
        if key.strip()
    ]
    if reference_text_keys:
        lines.append("Related in-game text (not confirmed on this map route):")
        lines.extend(
            f"  {key}: {readable_text(key)}" if include_source_details
            else f"  {readable_text(key)}"
            for key in reference_text_keys
        )
    guide_reference = site.get("guide_reference", "").strip()
    if guide_reference:
        lines.append("Guide cross-check only (not ROM event text):")
        lines.append(f"  {guide_reference}")
    menu_profiles = [
        profile.strip()
        for profile in site.get("menu_profiles", "").split(";")
        if profile.strip()
    ]
    if menu_profiles:
        if include_source_details:
            lines.append(
                "Menu pages and choices: " + ", ".join(menu_profiles) +
                " (the linked scripts determine which choices appear for each story state)."
            )
        elif not site.get("content_profile", "").strip():
            lines.append(
                "A menu is linked to this place, but its page text and choices "
                "are not available in the current content profile."
            )
    content_profile = site.get("content_profile", "").strip()
    speaker_id = site.get("speaker_id", "").strip()
    if content_profile or speaker_id:
        lines.extend(read_dialogue_profile_details(
            repo_root, content_profile, speaker_id, text_catalog,
            include_source_details=include_source_details,
        ))
    if include_source_details:
        lines.extend(read_lua_quest_module_details(repo_root, site))
    shop_profile = site.get("shop_profile", "").strip()
    if shop_profile:
        lines.extend(read_shop_profile_details(
            repo_root, shop_profile, text_catalog,
            include_source_details=include_source_details,
        ))
    return lines


def read_map_audio_details(repo_root: Path, assets: Path, info: MapInfo) -> str:
    """Report map-audio evidence without assigning menu audio to a map."""
    lines = [
        f"Scene {info.scene_id:02d} map audio",
        "",
        "Map-scoped background music: unresolved.",
        "The extracted scene descriptor and the traced scene-profile loader "
        "(ROM 0x03084) do not provide a music resource ID. The world-frame "
        "service (0x02900) advances pending audio commands but does not select "
        "a scene track. This does not establish that the map is silent; a "
        "scene-entry or persistent audio path still needs to be traced.",
        "",
        "Separate menu-mode audio found in the ROM:",
        "  0x173F2 enters prepare_menu_mode_resources, which calls 0x1745E. "
        "That routine submits event 0x5B as a start/stop pair and event 0x03 "
        "at 0x17652 under the FFFC08 audio gate. This path is menu-mode setup "
        "and is not assigned to the selected map.",
    ]
    aliases_path = assets / "resources.tsv"
    _, aliases = read_tsv(aliases_path)
    alias_by_key = {row.get("alias", ""): row for row in aliases}
    for event_id, description in (
        (3, "menu-mode track request"),
        (0x5B, "menu-mode start/stop request"),
    ):
        resource_id = event_id
        alias = (
            f"audio.music.resource.{resource_id:03d}"
            if event_id == 3 else f"audio.effects.resource.{resource_id:03d}"
        )
        resource = alias_by_key.get(alias)
        lines.append(
            f"  Event 0x{event_id:02X} ({description}) -> {alias}: "
            f"{resource.get('path', '(asset alias not found)') if resource else '(asset alias not found)'}"
        )
    _, loop_rows = read_tsv(assets / "audio" / "loop-points.tsv")
    loop = next(
        (row for row in loop_rows
         if row.get("alias") == "audio.music.resource.003"),
        None,
    )
    if loop:
        lines.append(
            "  Resource 003 loop metadata: decoded frames "
            f"{loop.get('loop_start_frame')}..{loop.get('loop_end_frame')} at "
            f"{loop.get('sample_rate')} Hz. This metadata describes that audio "
            "asset; the menu-mode call path does not identify it as map music."
        )
    lines.extend((
        "",
        "No music is inferred from the map artwork, guide label, or nearby "
        "audio filename. The map-level track remains open pending a complete "
        "ROM entry/exit audio trace.",
    ))
    return "\n".join(lines)


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


def parse_outline_paths(value: str) -> list[list[QPointF]]:
    """Parse entity-local polygons encoded as x,y;x,y|x,y;x,y."""
    paths: list[list[QPointF]] = []
    for path_text in value.split("|"):
        if not path_text.strip():
            continue
        points: list[QPointF] = []
        for point_text in path_text.split(";"):
            components = point_text.split(",")
            if len(components) != 2:
                raise ValueError(f"Invalid entity outline point: {point_text!r}")
            try:
                x, y = (int(component.strip(), 10) for component in components)
            except ValueError as error:
                raise ValueError(f"Invalid entity outline point: {point_text!r}") from error
            points.append(QPointF(x, y))
        if len(points) < 3:
            raise ValueError("An entity outline needs at least three points")
        paths.append(points)
    return paths


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
    record_offset: str = ""
    camera_x_fixed: int = 0
    camera_y_fixed: int = 0
    bound_x: int = 0
    bound_y: int = 0
    min_x: int = 0
    min_y: int = 0
    terrain_record: str = ""
    event_table: str = ""
    default_actor_list_offset: str = ""
    default_actor_count: int = 0
    default_actor_types: str = ""


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
    outline_points: str = ""


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
    content_profile: str = ""
    speaker_id: str = ""
    shop_profile: str = ""
    parts: list[EntityPart] = field(default_factory=list)
    interactables: list[Interactable] = field(default_factory=list)
    collision_regions: list[EntityCollisionRegion] = field(default_factory=list)

    @property
    def bounds(self) -> tuple[int, int, int, int]:
        # This is the visual selection frame. Collision and interaction
        # rectangles are separate authoring/evidence layers and must not swell
        # a building's artwork outline or imply extra pixels belong to it.
        rectangles = []
        for part in self.parts:
            paths = parse_outline_paths(part.outline_points)
            if paths:
                points = [point for path in paths for point in path]
                x0 = min(int(point.x()) for point in points)
                y0 = min(int(point.y()) for point in points)
                x1 = max(int(point.x()) for point in points)
                y1 = max(int(point.y()) for point in points)
                rectangles.append((x0, y0, x1 - x0, y1 - y0))
            else:
                rectangles.append((part.local_x, part.local_y, part.width, part.height))
        if not rectangles:
            rectangles = [
                (item.local_x, item.local_y, item.width, item.height)
                for item in self.interactables
            ]
        if not rectangles:
            return (0, 0, 1, 1)
        x0 = min(rect[0] for rect in rectangles)
        y0 = min(rect[1] for rect in rectangles)
        x1 = max(rect[0] + rect[2] for rect in rectangles)
        y1 = max(rect[1] + rect[3] for rect in rectangles)
        return (x0, y0, max(1, x1 - x0), max(1, y1 - y0))

    def contains_local_point(self, x: float, y: float) -> bool:
        if self.parts:
            for part in self.parts:
                paths = parse_outline_paths(part.outline_points)
                if paths:
                    if any(
                        QPolygonF(path).containsPoint(
                            QPointF(x, y), Qt.FillRule.WindingFill
                        )
                        for path in paths
                    ):
                        return True
                elif (
                    part.local_x <= x <= part.local_x + part.width and
                    part.local_y <= y <= part.local_y + part.height
                ):
                    return True
            return False
        regions = self.interactables or self.collision_regions
        return any(
            region.local_x <= x <= region.local_x + region.width and
            region.local_y <= y <= region.local_y + region.height
            for region in regions
        )


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
    sprite_path: Path | None = None
    sprite_origin_x: int = 0
    sprite_origin_y: int = 0
    conditional: bool = False
    route_details: str = ""
    map_label: str = ""
    path_points: tuple[tuple[int, int], ...] = ()
    population_class: str = "default"
    activity_group: str = ""


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
    event_id: int = 0
    trigger_type: str = "actor_within_scope"
    coordinate_space: str = "map_pixels"
    scope_min_x: int = -1
    scope_max_x: int = -1
    scope_min_y: int = -1
    scope_max_y: int = -1
    required_flags: str = ""
    action: str = "run_lua_handler"

    @property
    def title(self) -> str:
        labels = {
            "scripted_trigger": "Runtime map event" if self.event_id > 0 else "Map script trigger",
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
                f"Event ID: {self.event_id if self.event_id > 0 else '(editor-only)'}",
                f"Trigger type: {self.trigger_type}",
                f"Coordinate space: {self.coordinate_space}",
                f"Required state flags: {self.required_flags or '(none)'}",
                f"Action: {self.action}",
                f"Script: {self.script_module or '(unassigned)'}.{self.script_function or '(unassigned)'}",
                f"Condition: {self.condition_module or '(unassigned)'}"
                f".{self.condition_function or '(unassigned)'}",
                f"Repeat policy: {self.repeat_policy or '(unassigned)'}",
            ))
            if self.coordinate_space == "field_fixed_4":
                lines.append(
                    "Exact field scope: "
                    f"X {self.scope_min_x}..{self.scope_max_x}, "
                    f"Y {self.scope_min_y}..{self.scope_max_y}"
                )
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
            "This map marker is an editor view; only records accepted by the runtime event loader execute.",
        ))
        return "\n".join(lines)


class WorldContent:
    ENTITY_HEADER = (
        "entity_id", "name", "category", "evidence",
        "content_profile", "speaker_id", "shop_profile",
    )
    PART_HEADER = (
        "part_id", "source_kind", "source_scene", "source_x", "source_y",
        "width", "height", "local_x", "local_y", "draw_layer", "role", "evidence", "asset_path", "outline_points",
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
        "trigger_id", "event_id", "trigger_type", "coordinate_space",
        "scope_min_x", "scope_max_x", "scope_min_y", "scope_max_y",
        "required_flags", "action", "x", "y", "width", "height",
        "script_module", "script_function", "condition_module",
        "condition_function", "repeat_policy", "evidence", "review_status",
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
        def optional_value(name: str) -> str:
            value = row.get(name, "")
            return "" if value in ("", "-") else value

        entity = EntityDefinition(
            entity_id=row.get("entity_id", entity_id),
            name=row.get("name", entity_id),
            category=row.get("category", "unreviewed"),
            evidence=row.get("evidence", ""),
            content_profile=optional_value("content_profile"),
            speaker_id=optional_value("speaker_id"),
            shop_profile=optional_value("shop_profile"),
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
                outline_points=row.get("outline_points", ""),
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
        optional_values = {
            "content_profile": entity.content_profile or "-",
            "speaker_id": entity.speaker_id or "-",
            "shop_profile": entity.shop_profile or "-",
        }
        write_tsv(directory / "entity.tsv", self.ENTITY_HEADER, [{
            "entity_id": entity_id,
            "name": entity.name,
            "category": entity.category,
            "evidence": entity.evidence,
            **optional_values,
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
                outline_paths = parse_outline_paths(part.outline_points)
                if outline_paths:
                    clip_path = QPainterPath()
                    for outline in outline_paths:
                        crop_points = [
                            QPointF(point.x() - part.local_x, point.y() - part.local_y)
                            for point in outline
                        ]
                        clip_path.addPolygon(QPolygonF(crop_points))
                    masked = QImage(crop.size(), QImage.Format.Format_ARGB32)
                    masked.fill(Qt.GlobalColor.transparent)
                    painter = QPainter(masked)
                    painter.setClipPath(clip_path)
                    painter.drawImage(0, 0, crop)
                    painter.end()
                    crop = masked
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
                    event_id=as_int(row, "event_id"),
                    trigger_type=row.get("trigger_type", "actor_within_scope"),
                    coordinate_space=row.get("coordinate_space", "map_pixels"),
                    scope_min_x=as_int(row, "scope_min_x", -1),
                    scope_max_x=as_int(row, "scope_max_x", -1),
                    scope_min_y=as_int(row, "scope_min_y", -1),
                    scope_max_y=as_int(row, "scope_max_y", -1),
                    required_flags=row.get("required_flags", ""),
                    action=row.get("action", "run_lua_handler"),
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
                    "trigger_id": item.record_id, "event_id": item.event_id,
                    "trigger_type": item.trigger_type,
                    "coordinate_space": item.coordinate_space,
                    "scope_min_x": item.scope_min_x,
                    "scope_max_x": item.scope_max_x,
                    "scope_min_y": item.scope_min_y,
                    "scope_max_y": item.scope_max_y,
                    "required_flags": item.required_flags,
                    "action": item.action,
                    "x": item.x, "y": item.y,
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
                elif isinstance(key, str) and re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", key) and key not in {
                    "and", "break", "do", "else", "elseif", "end", "false",
                    "for", "function", "goto", "if", "in", "local", "nil",
                    "not", "or", "repeat", "return", "then", "true", "until", "while",
                }:
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
            "map_events": [],
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
                    "content_profile": entity.content_profile,
                    "speaker_id": entity.speaker_id,
                    "shop_profile": entity.shop_profile,
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
                            "outline_points": part.outline_points,
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
            map_event_ids: set[int] = set()
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
                        if item.event_id > 0:
                            if item.event_id > 0xFFFFFFFF:
                                raise ValueError(
                                    f"Runtime map event ID for {item.record_id} exceeds 32 bits"
                                )
                            if item.event_id in map_event_ids:
                                raise ValueError(
                                    f"Duplicate Lua map event ID {item.event_id}"
                                )
                            map_event_ids.add(item.event_id)
                            if item.script_module != "field_events":
                                raise ValueError(
                                    f"Runtime map event {item.record_id} must use the field_events Lua module"
                                )
                            if item.repeat_policy not in {
                                "every_match", "once_per_scene_visit", "once_per_session"
                            }:
                                raise ValueError(
                                    f"Runtime map event {item.record_id} has unsupported repeat policy {item.repeat_policy!r}"
                                )
                            if item.coordinate_space == "field_fixed_4":
                                bounds = (
                                    item.scope_min_x, item.scope_max_x,
                                    item.scope_min_y, item.scope_max_y,
                                )
                                if any(value < 0 or value > 0xFFFF for value in bounds):
                                    raise ValueError(
                                        f"Runtime map event {item.record_id} has an invalid source-coordinate scope"
                                    )
                                min_x, max_x, min_y, max_y = bounds
                            elif item.coordinate_space == "map_pixels":
                                min_x = item.x * 16
                                max_x = (item.x + item.width) * 16 - 1
                                min_y = item.y * 16
                                max_y = (item.y + item.height) * 16 - 1
                                if min_x < 0 or min_y < 0 or max_x > 0xFFFF or max_y > 0xFFFF:
                                    raise ValueError(
                                        f"Runtime map event {item.record_id} exceeds the field-coordinate range"
                                    )
                            else:
                                raise ValueError(
                                    f"Runtime map event {item.record_id} has unsupported coordinate space {item.coordinate_space!r}"
                                )
                            if min_x > max_x or min_y > max_y:
                                raise ValueError(
                                    f"Runtime map event {item.record_id} has an inverted coordinate scope"
                                )
                            if item.trigger_type not in {"scene_entry", "actor_within_scope"}:
                                raise ValueError(
                                    f"Runtime map event {item.record_id} has unsupported trigger type {item.trigger_type!r}"
                                )
                            if item.action not in {"run_lua_handler", "dispatch_native_script"}:
                                raise ValueError(
                                    f"Runtime map event {item.record_id} has unsupported action {item.action!r}"
                                )
                            required_flags = [
                                value.strip() for value in item.required_flags.split(",")
                                if value.strip()
                            ]
                            if any(flag != "new_game_opening_completed" for flag in required_flags):
                                raise ValueError(
                                    f"Runtime map event {item.record_id} has an unsupported state flag"
                                )
                            event_record = {
                                "id": item.event_id,
                                "scene": scene_id,
                                "trigger": item.trigger_type,
                                "scope": {
                                    "min_x": min_x,
                                    "max_x": max_x,
                                    "min_y": min_y,
                                    "max_y": max_y,
                                },
                                "requires": required_flags,
                                "repeat": item.repeat_policy,
                                "action": item.action,
                            }
                            if item.action == "run_lua_handler":
                                if not item.script_function:
                                    raise ValueError(
                                        f"Runtime map event {item.record_id} has no Lua handler"
                                    )
                                event_record["handler"] = item.script_function
                            data["map_events"].append(event_record)
                        trigger_values.append(
                            {
                                "id": item.record_id,
                                "event_id": item.event_id,
                                "trigger_type": item.trigger_type,
                                "coordinate_space": item.coordinate_space,
                                "scope_min_x": item.scope_min_x,
                                "scope_max_x": item.scope_max_x,
                                "scope_min_y": item.scope_min_y,
                                "scope_max_y": item.scope_max_y,
                                "required_flags": item.required_flags,
                                "action": item.action,
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
    _, actor_list_rows = read_tsv(
        asset_root / "field" / "entities" / "default-actor-lists.tsv")
    actor_lists = {
        as_int(row, "scene", -1): row for row in actor_list_rows
    }
    maps: list[MapInfo] = []
    for row in rows:
        scene_id = as_int(row, "mode_id", -1)
        if scene_id < 0:
            continue
        asset = asset_root / row.get("asset", "")
        priority = asset_root / row.get("priority_overlay_asset", "")
        actor_list = actor_lists.get(scene_id, {})
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
            record_offset=row.get("record_offset", ""),
            camera_x_fixed=as_int(row, "camera_x_fixed"),
            camera_y_fixed=as_int(row, "camera_y_fixed"),
            bound_x=as_int(row, "bound_x"),
            bound_y=as_int(row, "bound_y"),
            min_x=as_int(row, "min_x"),
            min_y=as_int(row, "min_y"),
            terrain_record=row.get("terrain_record", ""),
            event_table=row.get("event_table", ""),
            default_actor_list_offset=actor_list.get("list_offset", ""),
            default_actor_count=as_int(actor_list, "actor_count"),
            default_actor_types=(
                "" if actor_list.get("actor_types", "-") == "-"
                else actor_list.get("actor_types", "")
            ),
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


def read_profile_entry_positions(
    asset_root: Path, info: MapInfo
) -> list[MapEvidenceRegion]:
    """Expose the default three-member party placement from the scene profile.

    ROM 0x2B34 copies these three profile coordinates into the three ordinary
    actor records when there is no pending coordinate-list override. The
    scene-entry loader can replace the points for a transition, so these are
    the default party placement, not every route's arrival position.
    """
    descriptor = (
        asset_root / "records" / "field" / "scenes" /
        f"scene-{info.scene_id:02d}" / "descriptor.bin"
    )
    try:
        data = descriptor.read_bytes()
    except OSError:
        return []
    result: list[MapEvidenceRegion] = []
    party_roles = (
        "lead actor",
        "party member 2",
        "party member 3",
    )
    for slot, offset in enumerate((0x42, 0x46, 0x4A), start=1):
        if len(data) < offset + 4:
            break
        raw_x = int.from_bytes(data[offset:offset + 2], "big")
        raw_y = int.from_bytes(data[offset + 2:offset + 4], "big")
        if raw_x == 0 and raw_y == 0:
            continue
        is_new_game_entry = info.scene_id == 2 and slot == 1
        title = f"Default party entry {slot}: {party_roles[slot - 1]}"
        details = [
            title,
            f"Scene: {info.scene_id:02d}; descriptor: {descriptor}",
            f"Profile slot offset: +0x{offset:02X}",
            f"Raw X/Y: 0x{raw_x:04X},0x{raw_y:04X} (12.4 fixed point)",
            f"Map coordinate: ({raw_x >> 4},{raw_y >> 4}) pixels",
            "ROM 0x03084 loads these three profile positions. If no incoming "
            "coordinate list is pending, 0x02B34 copies them to the three "
            "ordinary actor records.",
            "A map transition can override these defaults with its own "
            "coordinate list or transition position; this marker is the "
            "profile's default party placement.",
        ]
        if is_new_game_entry:
            details.append(
                "The New Game route uses slot 1 as the scene-2 lead actor "
                "position, then runs the automatic walk toward Stoker's event."
            )
        result.append(MapEvidenceRegion(
            kind="rom_profile_entry",
            x=(raw_x >> 4) - 6,
            y=(raw_y >> 4) - 6,
            width=13,
            height=13,
            title=title,
            details="\n".join(details),
        ))
    return result


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


def read_site_route_index(repo_root: Path, scene_id: int) -> dict[str, dict[str, str]]:
    return {
        row.get("rom_record_address", "").strip().upper(): row
        for row in read_site_route_rows(repo_root, scene_id)
        if row.get("rom_record_address", "").strip()
    }


def read_site_route_rows(repo_root: Path, scene_id: int) -> list[dict[str, str]]:
    path = repo_root / "decompilation" / "gameplay" / "world" / "areas" / f"scene-{scene_id:02d}-site-routes.tsv"
    _, rows = read_tsv(path)
    return rows


def read_rom_contract_area_profiles(repo_root: Path) -> dict[int, dict[str, object]]:
    """Read the ROM-derived district and run-location names from Lua data."""
    path = repo_root / "decompilation" / "gameplay" / "scripts" / "run_support.lua"
    try:
        source = path.read_text(encoding="utf-8")
    except OSError:
        return {}

    area_match = re.search(r"\bSupport\.area_suffixes\s*=\s*\{", source)
    location_match = re.search(r"\bSupport\.location_names\s*=\s*\{", source)
    rolls_match = re.search(r"\bSupport\.location_rolls_by_area\s*=\s*\{", source)
    if not area_match or not location_match or not rolls_match:
        return {}
    area_body = extract_lua_block(source, source.find("{", area_match.start()))
    location_body = extract_lua_block(
        source, source.find("{", location_match.start())
    )
    rolls_body = extract_lua_block(source, source.find("{", rolls_match.start()))
    if not area_body or not location_body or not rolls_body:
        return {}

    area_names: dict[int, str] = {}
    for match in re.finditer(r'\[(\d+)\]\s*=\s*"([^"]+)"', area_body):
        phrase = match.group(2).strip().rstrip(".")
        for prefix in ("in ", "on "):
            if phrase.startswith(prefix):
                phrase = phrase[len(prefix):]
                break
        if phrase.startswith("the "):
            phrase = phrase[len("the "):]
        area_names[int(match.group(1))] = phrase

    locations: dict[int, dict[int, str]] = {}
    for match in re.finditer(r"\[(\d+)\]\s*=\s*\{", location_body):
        opening = location_body.find("{", match.start())
        body = extract_lua_block(location_body, opening)
        if not body:
            continue
        area_id = int(match.group(1))
        locations[area_id] = {
            int(row.group(1)): row.group(2)
            for row in re.finditer(r'\[(\d+)\]\s*=\s*"([^"]+)"', body)
        }

    used_locations: dict[int, set[int]] = {}
    for match in re.finditer(r"\[(\d+)\]\s*=\s*\{", rolls_body):
        opening = rolls_body.find("{", match.start())
        body = extract_lua_block(rolls_body, opening)
        if body:
            used_locations[int(match.group(1))] = {
                int(value) for value in re.findall(r"\b(\d+)\b", body)
            }

    return {
        area_id: {
            "name": name,
            "locations": {
                index: location_name
                for index, location_name in locations.get(area_id, {}).items()
                if index in used_locations.get(area_id, set(locations.get(area_id, {})))
            },
        }
        for area_id, name in area_names.items()
    }


def describe_event_table_target(target_class: str) -> str:
    """Turn the ROM route classification into a readable numeric destination."""
    value = target_class.strip()
    match = re.match(
        r"^(field scene|resource activity scene|scripted activity scene)\s+"
        r"(0x[0-9a-fA-F]+)(.*)$",
        value,
    )
    if match:
        route_kind = {
            "field scene": "Field map",
            "resource activity scene": "Resource activity",
            "scripted activity scene": "Scripted activity",
        }[match.group(1)]
        scene_number = int(match.group(2), 16)
        condition = match.group(3).strip()
        condition = f" ({condition.lstrip('; ').strip()})" if condition else ""
        return f"{route_kind} {scene_number}{condition}"
    if "table byte is diagnostic only" in value:
        return "Special story/service handler; the table byte does not select the destination"
    if "direct event continuation" in value:
        return "Direct ROM continuation; final destination is not established"
    if "no event table" in value:
        return "No event-table destination; generic field handler continues"
    if "unresolved" in value:
        return "Destination not established by the current ROM trace"
    return value or "Destination not established by the current ROM trace"


def describe_site_destination(value: str, current_scene: int) -> str:
    """Turn crosswalk scene IDs into readable map/activity destinations."""
    value = value.strip()
    if not value:
        return "Destination not established"
    if re.search(r"handler\s+0x[0-9a-f]+.*destination (?:is )?unresolved", value, re.I):
        return "Special game event; final destination not established"
    conditional = re.match(
        r"^story handler may consume; otherwise (.+)$", value, re.I
    )
    if conditional:
        return "Story event may take over; otherwise " + describe_site_destination(
            conditional.group(1), current_scene
        )

    same_scene = re.match(r"^scene\s+(0x[0-9a-f]+|\d+)(.*)$", value, re.I)
    if same_scene:
        scene_token, suffix = same_scene.groups()
        if int(scene_token, 0) == current_scene:
            if "no separate" in suffix.lower():
                return "Stay on this map; there is no separate interior"
            if "opens shop" in suffix.lower():
                return "Stay on this map; A opens the shop"
            if "same-scene" in suffix.lower():
                return "Stay on this map; the guide's external connection is not established by this route"

    scene_match = re.match(
        r"^(field|resource activity|scripted activity)?\s*scene\s+"
        r"(0x[0-9a-f]+|\d+)(.*)$",
        value,
        re.I,
    )
    if scene_match:
        kind, scene_token, suffix = scene_match.groups()
        scene_number = int(scene_token, 0)
        suffix_lower = suffix.lower()
        if scene_number == current_scene and "same-scene" in suffix_lower:
            return "Stay on this map; the guide's external connection is not established by this route"
        if kind and kind.lower().startswith("resource"):
            label = f"Resource activity {scene_number}"
        elif kind and kind.lower().startswith("scripted"):
            label = f"Scripted activity {scene_number}"
        elif "resource activity" in suffix_lower:
            label = f"Resource activity {scene_number}"
        elif "scripted activity" in suffix_lower:
            label = f"Scripted activity {scene_number}"
        else:
            label = f"Field map {scene_number}"
        suffix = re.sub(
            r"\s*\((?:field|resource activity|scripted activity)\)",
            "",
            suffix,
            flags=re.I,
        )
        suffix = re.sub(
            r"with source flag/timeout gate",
            "when its story gate allows it",
            suffix.strip(),
            flags=re.I,
        )
        suffix = re.sub(
            r"if generic route continues",
            "if the standard route continues",
            suffix,
            flags=re.I,
        )
        return label + (f" {suffix}" if suffix else "")

    return value


def describe_site_behavior(value: str) -> str:
    """Translate route-profile labels into concise map-editor language."""
    descriptions = {
        "entry_description_then_field_route":
            "Shows the location's entry description, then follows its field route.",
        "generic_area_route":
            "Uses the map's ordinary area route.",
        "resource_activity_route":
            "Starts the resource activity selected by the game.",
        "scripted_activity_route":
            "Starts a scripted activity selected by the game.",
        "conditional_area_route":
            "Checks the story condition before choosing the destination.",
        "same_map_dialogue_and_motel_service":
            "Opens Stoker's dialogue and motel services here; there is no separate motel map.",
        "service_prompt_then_shop":
            "Shows the service prompt; choosing it opens the shop on this map.",
        "indexed_story_or_generic_route":
            "Checks for a story event first; otherwise it continues through the standard route.",
        "conditional_maglock_then_activity":
            "Checks the electromagnetic lock before continuing to the activity route.",
        "unresolved":
            "The location or route is not identified by the current evidence.",
    }
    return descriptions.get(
        value.strip(), "The interaction behavior is not yet described in plain language."
    )


def describe_site_conditions(site: dict[str, str]) -> str:
    """Return only player-readable conditions, never source implementation notes."""
    player_conditions = site.get("player_conditions", "").strip()
    if player_conditions:
        return player_conditions
    route = site.get("rom_route", "").strip()
    notes = site.get("state_or_branch_notes", "").strip()
    if route == "same_map_dialogue_and_motel_service" and notes:
        return re.sub(
            r"\s*Exact branch logic is in [^.;]+\.lua\.?\s*$",
            "",
            notes,
            flags=re.IGNORECASE,
        ).strip()
    descriptions = {
        "entry_description_then_field_route":
            "A scene-entry condition selects the destination after the entry description.",
        "conditional_area_route":
            "The destination can change with the current story condition.",
        "indexed_story_or_generic_route":
            "A story event may run first; otherwise the game follows the standard activity route.",
        "conditional_maglock_then_activity":
            "The activity route continues only after the electromagnetic-lock check.",
    }
    return descriptions.get(route, "")


def describe_trigger_action(route: str) -> str:
    """Describe how the ROM reaches a route-table row or direct handler."""
    value = route.strip().lower()
    row_match = re.search(r"table_index_0x([0-9a-f]+)", value)
    row_number = int(row_match.group(1), 16) if row_match else None
    if "conditional_maglock" in value and row_number is not None:
        return (
            "Try the electromagnetic lock; if it stays closed, use map "
            f"route entry {row_number}"
        )
    if "bypass" in value:
        return "Run the special story or service action"
    if "event_table_index" in value and row_number is not None:
        return f"Use this map's route entry {row_number}"
    if "direct event continuation" in value:
        return "Continue into a special game event; destination not established"
    if "no event table" in value:
        return "Continue through the map interaction system; destination not established"
    if "unresolved generic route" in value:
        return "Continue the map interaction; destination not established"
    return re.sub(r"[_\s]+", " ", route.strip()).strip().capitalize() or "Route not established"


def describe_site_crosswalk(status: str) -> str:
    return {
        "rom_guide_corroborated": "ROM text and the guide map support this name",
        "guide_name_crosswalk": "name comes from the guide map; the ROM uses a numeric event",
        "nearby_candidate": "nearby guide marker; exact match is tentative",
        "unresolved": "place name not established",
    }.get(status.strip(), "name evidence not recorded")


def read_event_rectangles(repo_root: Path, scene_id: int) -> list[MapEvidenceRegion]:
    path = repo_root / "decompilation" / "docs" / "field-map-trigger-records.tsv"
    _, rows = read_tsv(path)
    result: list[MapEvidenceRegion] = []
    site_routes = read_site_route_index(repo_root, scene_id)
    text_catalog = read_lua_string_table(
        repo_root / "decompilation" / "assets" / "dialogue" / "script-text.lua"
    )
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
            site = site_routes.get(str(record_address).strip().upper(), {})
            site_name = site.get("site_name", "").strip()
            route_label = site.get("rom_route", "").strip() or route
            route_label = re.sub(
                r"event-table entry",
                "map route entry",
                route_label,
                flags=re.IGNORECASE,
            )
            route_label = re.sub(r"[_\s]+", " ", route_label).strip().capitalize()
            table_target = describe_event_table_target(
                row.get("event_table_target_class", "")
            )
            destination = describe_site_destination(
                site.get("rom_destination", "").strip() or table_target,
                scene_id,
            )
            table_byte = row.get("event_table_byte_at_event_id", "").strip()
            action = describe_trigger_action(route)
            if table_byte:
                row_match = re.search(r"table_index_0x([0-9a-f]+)", route, re.IGNORECASE)
                if "bypass" in route.lower():
                    table_detail = f"{table_byte} is diagnostic; the special handler bypasses the table"
                elif row_match:
                    table_detail = f"route row {int(row_match.group(1), 16)} -> {table_byte}"
                else:
                    table_detail = f"{table_byte} ({table_target})"
            else:
                table_detail = "No event-table byte is recorded for this route"
            details = [
                "Map interaction",
                f"Location: {site_name or 'Not identified'}",
                f"Action on contact: {action if not site_name else route_label}",
                f"Destination or result: {destination}",
                f"Map route-table result: {table_detail}",
                f"Trigger area: pixels {x0},{y0}, size {width}×{height}; "
                f"map cells X {row.get('x_start_grid_cell', '?')}..{row.get('x_end_grid_cell', '?')}, "
                f"Y {row.get('y_start_grid_cell', '?')}..{row.get('y_end_grid_cell', '?')}",
                f"Expected dialogue, choices, or shop: "
                f"{site.get('text_keys', '').strip() or '(not identified for this trigger)' }",
                f"Story conditions: {site.get('state_or_branch_notes', '').strip() or '(not profiled)' }",
                "Source trail",
                f"ROM event {event_id or '(unknown)'}; record {record_index} at {record_address}; "
                f"collision value {row.get('raw_collision_value', '(unknown)')}",
                f"ROM profile {row.get('profile_address', '(unknown)')}; collision list "
                f"{row.get('collision_records_address', '(unknown)')}; handler addresses "
                f"{row.get('rom_handler_addresses', '(unrecorded)')}",
                f"Current implementation path: {row.get('converted_cpp_functions_or_runtime_owners', '(unrecorded)')}",
                f"Physical route checked: {row.get('known_physical_qa_route', '(not recorded)')}",
                f"Evidence index: {path}",
            ]
            route_lines: list[str] = []
            map_label = ""
            if site:
                marker = site.get("guide_marker", "").strip()
                if site_name:
                    match = describe_site_crosswalk(
                        site.get("crosswalk_status", "unresolved")
                    )
                    crosswalk = f"Guide marker {marker}: {match}" if marker else match
                    confidence_prefix = "" if match == "rom_guide_corroborated" else "? "
                    map_label = f"{confidence_prefix}{marker + ': ' if marker else ''}{site_name}"
                    details.extend((
                        f"Name evidence: {crosswalk}; {site.get('evidence', '').strip() or '(not recorded)' }",
                        f"Scripts involved: {site.get('lua_modules', '').strip() or '(not linked / not profiled)' }",
                        f"Speaker: {site.get('speaker_id', '').strip() or '(none identified)' }",
                        f"Choice/menu pages: {site.get('menu_profiles', '').strip() or '(not identified)' }",
                        f"Shop catalog: {site.get('shop_profile', '').strip() or '(none identified)' }",
                    ))
                    player_summary = site.get("player_summary", "").strip()
                    player_destination = site.get("player_destination", "").strip()
                    player_conditions = describe_site_conditions(site)
                    route_lines.extend((
                        f"Location: {site_name}",
                        "How this point is used: " + (
                            player_summary or describe_site_behavior(
                                site.get("rom_route", "")
                            )
                        ),
                        f"Destination or result: {player_destination or destination}",
                    ))
                    if player_conditions:
                        route_lines.append(f"Story conditions: {player_conditions}")
                    display_content_lines = read_site_content_details(
                        repo_root, site, text_catalog, scene_id,
                        include_source_details=False,
                    )
                    source_content_lines = read_site_content_details(
                        repo_root, site, text_catalog, scene_id
                    )
                    if source_content_lines:
                        details.append("")
                        details.extend(source_content_lines)
                    if display_content_lines:
                        route_lines.append("")
                        route_lines.extend(display_content_lines)
                    if not display_content_lines:
                        route_lines.append(
                            "Dialogue, choices, or shop: not linked to this map point yet."
                        )
                    title = f"Interaction: {site_name}"
                else:
                    title = "Map interaction: place not identified"
            else:
                title = "Map interaction: place not identified"
            route_lower = route.lower()
            is_direct_game_event = (
                dispatcher == "world_activity_event31" and
                (
                    "direct event continuation" in route_lower or
                    "direct_b7b8_continuation" in route_lower
                )
            )
            if not site_name and is_direct_game_event:
                # Keep the player/editor description separate from the raw
                # handler and converted owner list shown in the Source tab.
                title = "Game event handoff"
                map_label = "Game flow"
                route_lines.extend((
                    "What happens: the game continues into a special event handler.",
                    "Where it goes: this record skips the map's normal route list, and the final screen or location is not established yet.",
                    "Dialogue, choices, or shop: none is linked to this trigger in the current evidence.",
                ))
            if not route_lines:
                route_lines.extend((
                    "Place: no building or doorway name is confirmed for this trigger yet.",
                    f"What happens on contact: {action}",
                    f"Next map or activity: {destination}",
                    f"Text, choices, or shop: {site.get('text_keys', '').strip() or 'not linked to this trigger yet'}",
                    "The ROM identifies a numeric destination here, but does not name the interior or its dialogue.",
                ))
                if not site_name:
                    title = f"Map interaction: {destination}"
            result.append(MapEvidenceRegion(
                kind="rom_event",
                x=x0,
                y=y0,
                width=width,
                height=height,
                title=title,
                details="\n".join(details),
                route_details="\n".join(route_lines),
                map_label=map_label,
            ))
    return result


def actor_direction_from_heading(heading: int) -> int:
    """Use the source 0xB36A rotate/round operation for the C052 direction."""
    rotated = ((heading << 3) | (heading >> 13)) & 0xFFFF
    if rotated & 0x8000:
        rotated = (rotated + 1) & 0xFFFF
    return rotated & 0x7


def read_actor_sprite_catalog(asset_root: Path) -> dict[str, tuple[int, int, int, int]]:
    """Return extracted sprite offsets/sizes from the named asset catalog."""
    _, rows = read_tsv(asset_root / "catalog.tsv")
    result: dict[str, tuple[int, int, int, int]] = {}
    for row in rows:
        path = row.get("asset", "")
        if not path.startswith("field/entities/actors/"):
            continue
        result[path] = (
            as_int(row, "x"), as_int(row, "y"),
            as_int(row, "width"), as_int(row, "height"),
        )
    return result


def read_out_of_bounds_actor_details(
    asset_root: Path, info: MapInfo
) -> list[str]:
    """Report source actor records outside the extracted map image bounds."""
    scene_id = info.scene_id
    result: list[str] = []
    tables = (
        ("default-actors.tsv", "default list order", "source_order"),
        ("conditional-actor-candidates.tsv", "conditional candidate choice", "choice"),
        ("world-activity-actors.tsv", "world activity list order", "source_order"),
    )
    for filename, label, ordinal_field in tables:
        path = asset_root / "field" / "entities" / filename
        _, rows = read_tsv(path)
        for row in rows:
            if as_int(row, "scene", -1) != scene_id:
                continue
            raw_x = as_int(row, "world_x", -1)
            raw_y = as_int(row, "world_y", -1)
            x, y = raw_x >> 4, raw_y >> 4
            if raw_x < 0 or raw_y < 0 or (0 <= x < info.width and 0 <= y < info.height):
                continue
            result.append(
                f"{label} {row.get(ordinal_field, '?')}: "
                f"{row.get('actor_name', 'actor')} at ({x},{y}) pixels "
                f"(0x{raw_x:04X},0x{raw_y:04X}); image bounds are "
                f"0..{info.width - 1}, 0..{info.height - 1}. Source "
                f"template/list/position: {row.get('template_offset', '?')}/"
                f"{row.get('list_offset', '?')}/{row.get('position_offset', '?')}. "
                "Kept in the ROM census, omitted from map overlay pending "
                "coordinate/route interpretation."
            )
    return result


def read_actor_positions(asset_root: Path, info: MapInfo) -> list[MapEvidenceRegion]:
    """Read exact ROM actor placements and draw their source kind/direction art.

    Default-list rows are listed separately from conditional candidates. The
    latter are three alternatives for one selected pedestrian, not three
    simultaneously active actors.
    """
    scene_id = info.scene_id
    sprite_catalog = read_actor_sprite_catalog(asset_root)
    result: list[MapEvidenceRegion] = []
    default_path = asset_root / "field" / "entities" / "default-actors.tsv"
    _, default_rows = read_tsv(default_path)
    waypoint_path = asset_root / "field" / "entities" / "default-actor-waypoints.tsv"
    _, waypoint_rows = read_tsv(waypoint_path)
    waypoint_lists: dict[tuple[int, int], list[tuple[int, int]]] = {}
    for row in waypoint_rows:
        waypoint_key = (
            as_int(row, "scene", -1),
            as_int(row, "source_order", -1),
        )
        waypoint_lists.setdefault(waypoint_key, []).append((
            as_int(row, "world_x"), as_int(row, "world_y")
        ))
    actor_names: dict[tuple[int, int], str] = {}
    actor_stems: dict[tuple[int, int], str] = {}
    for row in default_rows:
        kind = as_int(row, "entity_kind", -1)
        palette = as_int(row, "palette_line", -1)
        if kind >= 0 and palette >= 0:
            actor_names[(kind, palette)] = row.get(
                "actor_name", f"ROM actor kind {kind}")
            stem = row.get("sprite_stem", "").strip()
            if stem:
                actor_stems[(kind, palette)] = stem

    def add_actor(
        *,
        actor_kind: int,
        palette_line: int,
        raw_x: int,
        raw_y: int,
        heading: int,
        title: str,
        details: list[str],
        conditional: bool,
        path_points: tuple[tuple[int, int], ...] = (),
        position_list_address: str = "",
        population_class: str = "default",
    ) -> None:
        x, y = raw_x >> 4, raw_y >> 4
        if not (0 <= x < info.width and 0 <= y < info.height):
            return
        direction = actor_direction_from_heading(heading)
        visual_key = (actor_kind, palette_line)
        stem = actor_stems.get(visual_key)
        actor_name = actor_names.get(
            visual_key, f"ROM actor kind {actor_kind}")
        filename = (
            f"{stem}-idle.bmp" if direction == 0
            else f"{stem}-idle-dir-{direction}.bmp"
        ) if stem else ""
        sprite_relative = f"field/entities/actors/{filename}" if filename else ""
        sprite_path = asset_root / sprite_relative if sprite_relative else None
        if sprite_path is not None and not sprite_path.is_file():
            sprite_path = None
        if actor_kind == 15 and direction != 0 and stem:
            # ROM 0x62AC/0xC052 resolves all eight kind-15 idle selectors to
            # the same seven-piece sprite list. Its named asset is therefore
            # direction-independent, not a missing directional extraction.
            shared_relative = f"field/entities/actors/{stem}-idle.bmp"
            shared_path = asset_root / shared_relative
            if shared_relative in sprite_catalog and shared_path.is_file():
                sprite_relative = shared_relative
                sprite_path = shared_path
        offset_x, offset_y, width, height = sprite_catalog.get(
            sprite_relative, (0, -16, 1, 1)
        )
        details.extend((
            f"Actor: {actor_name} (ROM entity kind {actor_kind})",
            f"Heading: 0x{heading:04X}; source direction: {direction}",
            f"World coordinate: (0x{raw_x:04X}, 0x{raw_y:04X}) 12.4 fixed point",
            f"Map anchor: ({x}, {y}) pixels",
        ))
        details.append(
            f"Sprite asset: {sprite_relative or '(unsupported ROM kind)'}"
        )
        if actor_kind == 15:
            details.append(
                "ROM state table uses the same seven-piece sprite list for all "
                "eight directional selectors."
            )
        if path_points:
            details.extend((
                f"ROM waypoint list: {len(path_points)} ordered point(s); "
                f"source list {position_list_address or '(unknown)' }.",
                "These are source route coordinates. The actor service, action, "
                "collision, and state gates decide whether and when an actor "
                "follows them; the line overlay is not proof of current movement.",
                "Waypoints (12.4 fixed point): " + " -> ".join(
                    f"({x:04X},{y:04X})" for x, y in path_points
                ),
            ))
        result.append(MapEvidenceRegion(
            kind="rom_actor",
            x=x + offset_x,
            y=y + offset_y,
            width=max(1, width),
            height=max(1, height),
            title=title,
            details="\n".join(details),
            sprite_path=sprite_path,
            sprite_origin_x=offset_x,
            sprite_origin_y=offset_y,
            conditional=conditional,
            path_points=path_points,
            population_class=population_class,
        ))

    for row in default_rows:
        if as_int(row, "scene", -1) != scene_id:
            continue
        raw_x = as_int(row, "world_x", -1)
        raw_y = as_int(row, "world_y", -1)
        if raw_x < 0 or raw_y < 0:
            continue
        source_order = as_int(row, "source_order", len(result))
        actor_kind = as_int(row, "entity_kind", -1)
        palette_line = as_int(row, "palette_line", -1)
        heading = as_int(row, "initial_heading")
        actor_name = row.get(
            "actor_name", actor_names.get((actor_kind, palette_line), "Actor"))
        details = [
            "ROM profile-selected default actor record",
            f"Scene: {scene_id:02d}; source-list order: {source_order}",
            f"Actor type: {actor_name}; kind {actor_kind}; palette line {palette_line}",
            f"Profile: {row.get('profile_id', '(unprofiled)')}",
            f"Initial action: {row.get('action_kind', 'unknown')} "
            f"{row.get('initial_action_code', '')}",
            f"Template: {row.get('template_offset', '(unknown)')}; "
            f"position list: {row.get('position_offset', '(unknown)')}; "
            f"actor list: {row.get('list_offset', '(unknown)')}",
            f"Ordered template overrides: {row.get('ordered_overrides', '(none)')}",
            "Activation: selected by the scene profile; this source row does not prove "
            "it is active on every transition or gameplay mode.",
        ]
        route_points = tuple(waypoint_lists.get((scene_id, source_order), ()))
        add_actor(
            actor_kind=actor_kind,
            palette_line=palette_line,
            raw_x=raw_x,
            raw_y=raw_y,
            heading=heading,
            title=f"{actor_name} - default list record {source_order}",
            details=details,
            conditional=False,
            path_points=route_points,
            position_list_address=row.get("position_offset", ""),
        )

    candidate_path = asset_root / "field" / "entities" / "conditional-actor-candidates.tsv"
    _, candidate_rows = read_tsv(candidate_path)
    candidate_groups: dict[tuple[int, int, int, int, int], list[dict[str, str]]] = {}
    for row in candidate_rows:
        if as_int(row, "scene", -1) != scene_id:
            continue
        raw_x = as_int(row, "world_x", -1)
        raw_y = as_int(row, "world_y", -1)
        if raw_x < 0 or raw_y < 0:
            continue
        key = (
            raw_x, raw_y,
            as_int(row, "entity_kind", -1),
            as_int(row, "palette_line", -1),
            as_int(row, "heading_word"),
        )
        candidate_groups.setdefault(key, []).append(row)

    for (raw_x, raw_y, actor_kind, palette_line, heading), group_rows in candidate_groups.items():
        choices = [as_int(row, "choice", 0) for row in group_rows]
        first = group_rows[0]
        details = [
            "ROM conditional actor candidate; one choice is selected, not all three.",
            f"Scene: {scene_id:02d}; candidate choice(s): {', '.join(map(str, choices))} of 3",
            f"Candidate group: {first.get('selection_group', '(unknown)')}",
            f"Activation gate: {first.get('activation_gate', '(unknown)')}",
            "The ROM selector retains its one-based choice until the source clears it.",
        ]
        for row in group_rows:
            choice = as_int(row, "choice", 0)
            details.extend((
                f"Choice {choice}: template {row.get('template_offset', '(unknown)')}; "
                f"candidate list {row.get('list_offset', '(unknown)')}; "
                f"position {row.get('position_offset', '(unknown)')}",
                f"Choice {choice} ordered template overrides: "
                f"{row.get('ordered_overrides', '(none)')}",
            ))
        title = f"Pedestrian - conditional candidates {', '.join(map(str, choices))}/3"
        if len(group_rows) > 1:
            title += " (shared position)"
        add_actor(
            actor_kind=actor_kind,
            palette_line=palette_line,
            raw_x=raw_x,
            raw_y=raw_y,
            heading=heading,
            title=title,
            details=details,
            conditional=True,
            population_class="conditional",
        )

    activity_path = asset_root / "field" / "entities" / "world-activity-actors.tsv"
    _, activity_rows = read_tsv(activity_path)
    for row in activity_rows:
        if as_int(row, "scene", -1) != scene_id:
            continue
        raw_x = as_int(row, "world_x", -1)
        raw_y = as_int(row, "world_y", -1)
        if raw_x < 0 or raw_y < 0:
            continue
        group_id = row.get("list_id", "unknown")
        source_order = as_int(row, "source_order", len(result))
        actor_kind = as_int(row, "entity_kind", -1)
        palette_line = as_int(row, "palette_line", -1)
        heading = as_int(row, "initial_heading")
        actor_name = row.get("actor_name", f"ROM actor kind {actor_kind}")
        sprite_stem = row.get("sprite_stem", "").strip()
        direction = actor_direction_from_heading(heading)
        filename = (
            f"{sprite_stem}-idle.bmp" if direction == 0
            else f"{sprite_stem}-idle-dir-{direction}.bmp"
        ) if sprite_stem else ""
        sprite_relative = f"field/entities/actors/{filename}" if filename else ""
        sprite_path = asset_root / sprite_relative if sprite_relative else None
        offset_x, offset_y, width, height = sprite_catalog.get(
            sprite_relative, (0, -16, 1, 1)
        )
        if sprite_path is not None and not sprite_path.is_file():
            sprite_path = None
        result.append(MapEvidenceRegion(
            kind="rom_world_activity_actor",
            x=(raw_x >> 4) + offset_x,
            y=(raw_y >> 4) + offset_y,
            width=max(1, width),
            height=max(1, height),
            title=f"{actor_name} - {group_id} row {source_order}",
            details="\n".join((
                "ROM world-activity actor candidate; separate from default map residents.",
                f"Scene: {scene_id:02d}; list: {group_id} at {row.get('list_offset', '(unknown)')}; source row {source_order}",
                f"Activation rule: {row.get('activation_rule', '(unresolved)')}",
                f"Template: {actor_name}; ROM entity kind {actor_kind}; palette line {palette_line}",
                f"Template record: {row.get('template_offset', '(unknown)')}; source row: {row.get('source_record_offset', '(unknown)')}",
                f"Fixed-point map coordinate: (0x{raw_x:04X}, 0x{raw_y:04X}); pixels ({raw_x >> 4}, {raw_y >> 4})",
                f"Direction word: 0x{heading:04X}; source direction: {direction}",
                f"Initial action: {row.get('action_kind', 'unknown')} {row.get('action_code', '')}; ordered overrides: {row.get('ordered_overrides', '(none)')}",
                f"Sprite asset: {sprite_relative or '(unsupported ROM kind)'}",
                "The three list groups are displayed as candidates for inspection; do not read them as simultaneously active actors.",
            )),
            sprite_path=sprite_path,
            sprite_origin_x=offset_x,
            sprite_origin_y=offset_y,
            conditional=True,
            population_class="world_activity",
            activity_group=group_id,
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
        self.profile_entry_positions: list[MapEvidenceRegion] = []
        self.behaviors: list[MapBehaviorRecord] = []
        self.instances: list[EntityInstance] = []
        self.entities: dict[str, EntityDefinition] = {}
        self.show_grid = True
        self.show_collision = False
        self.show_events = True
        self.show_actors = True
        self.show_actor_routes = False
        self.show_world_activity = False
        self.world_activity_group = "mode_13_entry"
        self.show_profile_entries = True
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
        self.actor_positions = read_actor_positions(assets, info)
        self.profile_entry_positions = read_profile_entry_positions(assets, info)
        self.scene().clear()
        # QGraphicsScene.clear() deletes its items; discard the Python wrappers
        # before a redraw tries to remove overlays from the previous scene.
        self.behavior_items = []
        base = QImage(str(info.asset)).convertToFormat(QImage.Format.Format_ARGB32)
        self.background_item = self.scene().addPixmap(QPixmap.fromImage(base))
        self.background_item.setZValue(0)
        self.background_item.setAcceptedMouseButtons(Qt.MouseButton.NoButton)
        # Keep ROM priority pixels as a real foreground layer. Compositing them
        # into the background made actor sprites appear over foreground art and
        # hid which image pixels belonged to the priority plane.
        if self.show_priority and info.priority_asset.is_file():
            overlay = QImage(str(info.priority_asset)).convertToFormat(QImage.Format.Format_ARGB32)
            if overlay.size() == base.size():
                self.priority_item = self.scene().addPixmap(QPixmap.fromImage(overlay))
                self.priority_item.setZValue(7)
                self.priority_item.setAcceptedMouseButtons(Qt.MouseButton.NoButton)
            else:
                self.priority_item = None
        else:
            self.priority_item = None
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

    def set_layer_options(self, priority: bool, grid: bool, collision: bool,
                          events: bool, actors: bool, actor_routes: bool,
                          world_activity: bool, activity_group: str,
                          profile_entries: bool, behaviors: bool) -> None:
        changed_priority = self.show_priority != priority
        self.show_priority = priority
        self.show_grid = grid
        self.show_collision = collision
        self.show_events = events
        self.show_actors = actors
        self.show_actor_routes = actor_routes
        self.show_world_activity = world_activity
        self.world_activity_group = activity_group
        self.show_profile_entries = profile_entries
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
        for index, region in enumerate(self.profile_entry_positions):
            if not self.show_profile_entries:
                continue
            color = QColor(93, 245, 218, 250)
            marker = self.scene().addEllipse(
                QRectF(region.x, region.y, region.width, region.height),
                QPen(color, 2),
                QColor(5, 16, 24, 90),
            )
            marker.setZValue(11)
            marker.setData(0, "map_evidence")
            marker.setData(1, "profile_entry")
            marker.setData(2, index)
            marker.setToolTip(region.title)
            self.evidence_items.append(marker)
            label_text = f"Party {index + 1}"
            label = self.scene().addText(label_text)
            label.setDefaultTextColor(color)
            label.setFont(QFont("Segoe UI", 8, QFont.Weight.DemiBold))
            label.setPos(region.x + region.width + 3, region.y - 4)
            label.setZValue(11)
            label.setData(0, "map_evidence")
            label.setData(1, "profile_entry")
            label.setData(2, index)
            label.setToolTip(region.title)
            self.evidence_items.append(label)
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
                if region.map_label:
                    label = self.scene().addText(region.map_label)
                    label.setDefaultTextColor(QColor(255, 224, 158, 255))
                    label.setFont(QFont("Segoe UI", 8, QFont.Weight.DemiBold))
                    label.setTextWidth(184)
                    label_rect = label.boundingRect().adjusted(-3, -2, 3, 2)
                    label_x = min(
                        max(2, region.x + region.width + 4),
                        max(2, self.map_info.width - label_rect.width() - 2),
                    )
                    label_y = min(
                        max(2, region.y),
                        max(2, self.map_info.height - label_rect.height() - 2),
                    )
                    background = self.scene().addRect(
                        QRectF(label_x, label_y, label_rect.width(), label_rect.height()),
                        QPen(QColor(255, 190, 35, 150), 1),
                        QColor(8, 13, 20, 210),
                    )
                    background.setZValue(9.5)
                    label.setPos(label_x + 3, label_y + 2)
                    label.setZValue(10)
                    for marker in (background, label):
                        marker.setData(0, "map_evidence")
                        marker.setData(1, "event")
                        marker.setData(2, index)
                        marker.setToolTip(region.title)
                        self.evidence_items.append(marker)
        if self.show_actors or self.show_world_activity:
            for index, region in enumerate(self.actor_positions):
                is_world_activity = region.population_class == "world_activity"
                if is_world_activity and not self.show_world_activity:
                    continue
                if (is_world_activity and self.world_activity_group != "all" and
                        region.activity_group != self.world_activity_group):
                    continue
                if not is_world_activity and not self.show_actors:
                    continue
                if region.sprite_path is not None:
                    marker = self.scene().addPixmap(QPixmap(str(region.sprite_path)))
                    marker.setPos(region.x, region.y)
                    marker.setTransformationMode(
                        Qt.TransformationMode.FastTransformation
                    )
                    marker.setZValue(6)
                    marker.setOpacity(0.66 if region.conditional else 1.0)
                else:
                    marker = self.scene().addEllipse(
                        QRectF(region.x, region.y, region.width, region.height),
                        QPen(QColor(90, 255, 190, 245), 2),
                        QColor(20, 90, 75, 185),
                    )
                    marker.setZValue(6)
                marker.setData(0, "map_evidence")
                marker.setData(1, "actor")
                marker.setData(2, index)
                marker.setToolTip(region.title)
                self.evidence_items.append(marker)
                if region.conditional:
                    # Gated lists and alternative candidates need a visible
                    # marker even when the route-line layer is hidden.
                    outline = self.scene().addRect(
                        QRectF(region.x, region.y, region.width, region.height),
                        QPen(QColor(255, 198, 82, 230), 1.5, Qt.PenStyle.DashLine),
                        QBrush(Qt.BrushStyle.NoBrush),
                    )
                    outline.setZValue(10)
                    outline.setData(0, "map_evidence")
                    outline.setData(1, "actor")
                    outline.setData(2, index)
                    outline.setToolTip(region.title)
                    self.evidence_items.append(outline)
                    label = self.scene().addText("?")
                    label.setDefaultTextColor(QColor(255, 220, 130))
                    label.setPos(region.x + region.width, region.y - 8)
                    label.setZValue(10)
                    label.setData(0, "map_evidence")
                    label.setData(1, "actor")
                    label.setData(2, index)
                    label.setToolTip(region.title)
                    self.evidence_items.append(label)
                if region.sprite_path is None and not region.conditional:
                    actor_name = region.title.partition(" - ")[0]
                    label = self.scene().addText(
                        f"{actor_name} [sprite unresolved]")
                    label.setDefaultTextColor(QColor(255, 177, 235, 255))
                    label.setFont(QFont("Segoe UI", 8, QFont.Weight.DemiBold))
                    label.setPos(region.x + region.width + 3, region.y - 4)
                    label.setZValue(10)
                    label.setData(0, "map_evidence")
                    label.setData(1, "actor")
                    label.setData(2, index)
                    label.setToolTip(region.title)
                    self.evidence_items.append(label)
        if self.show_actor_routes:
            for index, region in enumerate(self.actor_positions):
                if len(region.path_points) < 2:
                    continue
                route = QPainterPath()
                previous: tuple[int, int] | None = None
                has_segment = False
                for raw_x, raw_y in region.path_points:
                    x, y = raw_x >> 4, raw_y >> 4
                    if not (0 <= x < self.map_info.width and
                            0 <= y < self.map_info.height):
                        previous = None
                        continue
                    if previous is None:
                        route.moveTo(x, y)
                    else:
                        route.lineTo(x, y)
                        has_segment = True
                    previous = (x, y)
                if not has_segment:
                    continue
                route_item = self.scene().addPath(
                    route,
                    QPen(QColor(45, 228, 242, 185), 2,
                         Qt.PenStyle.DashLine),
                )
                route_item.setZValue(5.5)
                route_item.setData(0, "map_evidence")
                route_item.setData(1, "actor")
                route_item.setData(2, index)
                route_item.setToolTip(
                    f"ROM waypoint list for {region.title}; click for source list and gates"
                )
                self.evidence_items.append(route_item)

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
            "building": QColor(80, 210, 255, 225),
            "overhang": QColor(255, 135, 50, 235),
            "shadow": QColor(220, 220, 245, 245),
            "decal": QColor(240, 90, 255, 230),
            "foliage": QColor(80, 220, 100, 230),
            "roadway": QColor(190, 190, 205, 230),
            "unlabeled_structure": QColor(255, 130, 60, 230),
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
            if not entity.parts:
                box = self.scene().addRect(
                    QRectF(instance.origin_x + x0, instance.origin_y + y0, width, height),
                    QPen(color, 2),
                    QBrush(Qt.BrushStyle.NoBrush),
                )
                box.setZValue(12)
                box.setData(0, instance.instance_id)
                box.setToolTip(f"{entity.name} [{entity.category}]\n{instance.entity_id} @ {instance.origin_x},{instance.origin_y}")
                group.addToGroup(box)
            label = self.scene().addText(f"{entity.name} [{entity.category}]")
            label.setDefaultTextColor(color)
            label.setPos(instance.origin_x + x0 + 2, instance.origin_y + y0 - 18)
            label.setZValue(13)
            label.setData(0, instance.instance_id)
            label.setData(1, "entity_label")
            group.addToGroup(label)
            for part in entity.parts:
                part_color = role_colors.get(part.role, role_colors["unreviewed"])
                paths = parse_outline_paths(part.outline_points)
                if paths:
                    part_items = []
                    for outline in paths:
                        polygon = QPolygonF([
                            QPointF(instance.origin_x + point.x(), instance.origin_y + point.y())
                            for point in outline
                        ])
                        part_items.append(self.scene().addPolygon(
                            polygon,
                            QPen(part_color, 1.5, Qt.PenStyle.SolidLine),
                            QBrush(Qt.BrushStyle.NoBrush),
                        ))
                else:
                    part_items = [self.scene().addRect(
                        QRectF(
                            instance.origin_x + part.local_x,
                            instance.origin_y + part.local_y,
                            part.width,
                            part.height,
                        ),
                        QPen(part_color, 1.5, Qt.PenStyle.SolidLine),
                        QBrush(Qt.BrushStyle.NoBrush),
                    )]
                for part_item in part_items:
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
                "profile_entry": self.profile_entry_positions,
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
            item.setToolTip(record.title + f" — {record.review_status}; {record.evidence or 'evidence not recorded'}")
            label = self.scene().addText(labels.get(record.kind, "?"))
            label.setDefaultTextColor(color)
            label.setPos(record.x + (8 if record.kind == "actor_spawn" else 2), record.y - 2)
            label.setZValue(11)
            label.setData(0, "map_behavior")
            label.setData(1, index)
            label.setToolTip(record.title + f" — {record.review_status}; {record.evidence or 'evidence not recorded'}")
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
        self.contract_area_profiles = read_rom_contract_area_profiles(self.repo_root)
        _, self.field_activity_rows = read_tsv(
            assets / "field" / "world-activity-outcomes.tsv"
        )
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
        self.info_tabs.setCurrentWidget(self.scene_info)
        self.statusBar().showMessage("No automatic object classifier is active. New annotations start unreviewed.")

    def _scene_display_name(self, info: MapInfo) -> str:
        area = self.contract_area_profiles.get(info.scene_id)
        if area:
            return (
                f"{area['name']} — Scene {info.scene_id:02d} "
                f"({info.width}×{info.height})"
            )
        return f"Scene {info.scene_id:02d} — {info.width}×{info.height}"

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
            action = QAction(self._scene_display_name(info), self)
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
            self.scene_combo.addItem(self._scene_display_name(info), info.scene_id)
        self.scene_combo.currentIndexChanged.connect(self.scene_changed)
        layout.addWidget(QLabel("Map"))
        layout.addWidget(self.scene_combo)
        self.info = QPlainTextEdit()
        self.info.setReadOnly(True)
        self.info.setLineWrapMode(QPlainTextEdit.LineWrapMode.WidgetWidth)
        self.info.setMaximumHeight(220)
        self.info.setPlainText(
            "Click a map cell to inspect its source tile layers. Click a map interaction or collision box for its evidence. "
            "Click a character marker to inspect its type, position, appearance conditions, and source art. Click a scripted event, spawn, or encounter marker to inspect its game data. "
            "Drag entity outlines to move grouped annotations."
        )
        self.flow_info = QPlainTextEdit()
        self.flow_info.setReadOnly(True)
        self.flow_info.setLineWrapMode(QPlainTextEdit.LineWrapMode.WidgetWidth)
        self.flow_info.setPlainText(
            "Select a place, object, or map interaction to see what happens when the player reaches it, including any known destination, dialogue, menu, shop, or story condition."
        )
        self.scene_info = QPlainTextEdit()
        self.scene_info.setReadOnly(True)
        self.scene_info.setLineWrapMode(QPlainTextEdit.LineWrapMode.WidgetWidth)
        self.audio_info = QPlainTextEdit()
        self.audio_info.setReadOnly(True)
        self.audio_info.setLineWrapMode(QPlainTextEdit.LineWrapMode.WidgetWidth)
        self.activity_info = QPlainTextEdit()
        self.activity_info.setReadOnly(True)
        self.activity_info.setLineWrapMode(QPlainTextEdit.LineWrapMode.WidgetWidth)
        self.info_tabs = QTabWidget()
        self.info_tabs.addTab(self.info, "Source")
        self.info_tabs.addTab(self.flow_info, "What happens")
        self.info_tabs.addTab(self.scene_info, "Map profile")
        self.info_tabs.addTab(self.audio_info, "Map audio")
        self.info_tabs.addTab(self.activity_info, "Field events")
        self.info_tabs.setMinimumHeight(230)
        layout.addWidget(self.info_tabs)

        self.entity_list = QListWidget()
        self.entity_list.currentItemChanged.connect(self.list_selection_changed)
        layout.addWidget(QLabel("Places and objects on this map"))
        layout.addWidget(self.entity_list, 1)

        layout.addWidget(QLabel("Parts and attached graphics"))
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

        layout.addWidget(QLabel("Interaction points"))
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

        layout.addWidget(QLabel("Blocked areas (green dash-dot)"))
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

        layout.addWidget(QLabel("Scripted events (T), spawns (S), encounters (R)"))
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
        self.content_profile_edit = QLineEdit()
        self.content_profile_edit.setPlaceholderText("Dialogue/menu profile ID from gameplay/scripts/dialogue.lua")
        self.content_profile_edit.editingFinished.connect(self.properties_changed)
        self.speaker_id_edit = QLineEdit()
        self.speaker_id_edit.setPlaceholderText("Speaker ID from gameplay/scripts/dialogue.lua")
        self.speaker_id_edit.editingFinished.connect(self.properties_changed)
        self.shop_profile_edit = QLineEdit()
        self.shop_profile_edit.setPlaceholderText("Shop catalog profile ID from gameplay/scripts/shop.lua")
        self.shop_profile_edit.editingFinished.connect(self.properties_changed)
        form.addRow("Name", self.name_edit)
        form.addRow("Classification", self.category_combo)
        form.addRow("Origin X", self.x_spin)
        form.addRow("Origin Y", self.y_spin)
        form.addRow("Dialogue/menu profile", self.content_profile_edit)
        form.addRow("Speaker ID", self.speaker_id_edit)
        form.addRow("Shop profile", self.shop_profile_edit)
        form.addRow("Evidence", self.evidence_edit)
        layout.addLayout(form)
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
        self.right_scroll = scroll
        dock.setWidget(scroll)
        dock.setMinimumWidth(350)
        dock.setMaximumWidth(460)
        self.addDockWidget(Qt.DockWidgetArea.RightDockWidgetArea, dock)

    def _build_toolbar(self) -> None:
        bar = self.addToolBar("Map evidence layers")
        self.layer_toolbar = bar
        bar.setMovable(False)
        self.tool_buttons: dict[str, QPushButton] = {}
        scene_label = QLabel("  View layers: ")
        bar.addWidget(scene_label)
        self.priority_toggle = self._add_toggle(bar, "Priority pixels", True)
        self.grid_toggle = self._add_toggle(bar, "32 px grid", True)
        self.collision_toggle = self._add_toggle(bar, "Collision boxes", False)
        self.event_toggle = self._add_toggle(bar, "Map interactions", True)
        self.actor_toggle = self._add_toggle(bar, "Characters", True)
        self.actor_route_toggle = self._add_toggle(bar, "Character paths", False)
        self.world_activity_toggle = self._add_toggle(bar, "Event characters", False)
        self.world_activity_group_combo = QComboBox()
        self.world_activity_group_combo.addItem("All event lists", "all")
        self.world_activity_group_combo.addItem("Opening event: arrival", "mode_13_entry")
        self.world_activity_group_combo.addItem("Opening event: group A", "pending_list_1e5c4")
        self.world_activity_group_combo.addItem("Opening event: group B", "pending_list_1e628")
        self.world_activity_group_combo.setCurrentIndex(1)
        self.world_activity_group_combo.currentIndexChanged.connect(self.layers_changed)
        bar.addWidget(self.world_activity_group_combo)
        self.profile_entry_toggle = self._add_toggle(bar, "Usual party arrival", True)
        self.behavior_toggle = self._add_toggle(bar, "Scripted events / spawns", True)
        bar.addSeparator()
        legend = QLabel(
            "Characters: solid = usual placement; amber ? = conditional option; cyan dash = recorded route   "
            "Unreviewed: yellow   Building: cyan   Blocked area: green dash-dot"
        )
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

    @staticmethod
    def _set_toolbar_widget_visible(toolbar, widget: QWidget, visible: bool) -> None:
        for action in toolbar.actions():
            if toolbar.widgetForAction(action) is widget:
                action.setVisible(visible)
                widget.setVisible(visible)
                return

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
            self.actor_toggle.isChecked(), self.actor_route_toggle.isChecked(),
            self.world_activity_toggle.isChecked(),
            str(self.world_activity_group_combo.currentData()),
            self.profile_entry_toggle.isChecked(),
            self.behavior_toggle.isChecked(),
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
        has_event_characters = any(
            actor.population_class == "world_activity"
            for actor in self.map_view.actor_positions
        )
        self._set_toolbar_widget_visible(
            self.layer_toolbar, self.world_activity_toggle, has_event_characters
        )
        self._set_toolbar_widget_visible(
            self.layer_toolbar, self.world_activity_group_combo, has_event_characters
        )
        self._show_properties(None)
        self._update_scene_summary()
        self._update_scene_profile_info()
        self._update_scene_audio_info()
        self._update_scene_field_activities_info()

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
        default_actors = sum(
            1 for actor in self.map_view.actor_positions
            if actor.population_class == "default"
        )
        conditional_actors = sum(
            1 for actor in self.map_view.actor_positions
            if actor.population_class == "conditional"
        )
        activity_actors = sum(
            1 for actor in self.map_view.actor_positions
            if actor.population_class == "world_activity"
        )
        self.statusBar().showMessage(
            f"Scene {self.scene_id:02d}: {info.width}×{info.height}; {len(self.instances)} annotations, {unreviewed} unreviewed. "
            f"{len(self.map_view.event_rectangles)} map events; "
            f"{default_actors} usual characters, {conditional_actors} conditional choices, "
            f"{activity_actors} event-specific characters; "
            f"{len(self.map_view.collision_rectangles)} movement regions."
        )

    def _update_scene_profile_info(self) -> None:
        info = self.map_by_id[self.scene_id]
        descriptor_path = (
            self.assets / "records" / "field" / "scenes" /
            f"scene-{self.scene_id:02d}" / "descriptor.bin"
        )
        actor_types = info.default_actor_types
        actor_type_match = re.fullmatch(r"(\d+) x (.+)", actor_types.strip())
        if actor_type_match:
            actor_count = int(actor_type_match.group(1))
            actor_label = actor_type_match.group(2).strip()
            if actor_count != 1 and not actor_label.endswith("s"):
                actor_label += "s"
            actor_types = f"{actor_count} {actor_label}"
        lines = [
            f"Scene {self.scene_id:02d} map overview",
            f"Map size: {info.width}x{info.height} pixels; "
            f"{info.tilemap_width}x{info.tilemap_height} map cells "
            "(32 pixels per cell).",
            f"Initial camera focus: map pixel "
            f"({info.camera_x_fixed / 16:.2f}, {info.camera_y_fixed / 16:.2f}). "
            "This is where the camera starts, not where the party arrives.",
            "Characters listed for the usual scene setup: "
            + (actor_types or "none") + ".",
            "Each marked character uses its in-game sprite. Story and transition "
            "checks can still change which characters appear.",
        ]
        area_profile = self.contract_area_profiles.get(self.scene_id)
        if area_profile:
            lines.extend((
                "",
                f"District: {area_profile['name']}.",
                "Other destinations named by the game:",
            ))
            for location_index, location_name in sorted(
                area_profile["locations"].items()
            ):
                lines.append(f"  {location_name}")
            lines.append(
                "These are destination names, not proof that a particular door "
                "on this map leads there."
            )
        try:
            descriptor = descriptor_path.read_bytes()
        except OSError:
            descriptor = b""
        if len(descriptor) >= 0x4E:
            lines.append(
                "Usual party arrival points (an incoming doorway or map "
                "transition can set a different position):"
            )
            party_roles = ("lead actor", "party member 2", "party member 3")
            for slot, offset in enumerate((0x42, 0x46, 0x4A), start=1):
                raw_x = int.from_bytes(descriptor[offset:offset + 2], "big")
                raw_y = int.from_bytes(descriptor[offset + 2:offset + 4], "big")
                if raw_x == 0 and raw_y == 0:
                    continue
                lines.append(
                    f"  {party_roles[slot - 1].capitalize()}: "
                    f"map pixel ({raw_x / 16:.2f}, {raw_y / 16:.2f})"
                )
            lines.append(
                "These are the usual entry positions when the route that "
                "brought the party here does not set its own arrival point."
            )
            if self.scene_id == 2:
                lines.append(
                    "  New Game starts the lead actor at map pixel (974, 720) "
                    "and runs the opening walk toward Stoker's Coffin Motel."
                )
        else:
            lines.append("Usual party arrival positions are unavailable for this map.")
        default_actors = sum(
            1 for actor in self.map_view.actor_positions
            if actor.population_class == "default"
        )
        conditional_actors = sum(
            1 for actor in self.map_view.actor_positions
            if actor.population_class == "conditional"
        )
        world_activity_actors = sum(
            1 for actor in self.map_view.actor_positions
            if actor.population_class == "world_activity"
        )
        lines.extend((
            f"People in the map records: {default_actors} usual scene characters, "
            f"{conditional_actors} conditional choices, and "
            f"{world_activity_actors} event-specific character records.",
            "Character locations and usual party arrival points are shown as "
            "separate layers. A door or map transition can use another arrival point.",
            "Some timed and event-created character behavior still needs source "
            "tracing. Use Event characters to inspect the separately recorded lists.",
        ))
        lines.extend(("", "People and character positions recorded for this map:"))

        def readable_actor_position(actor: MapEvidenceRegion) -> str:
            name_match = re.search(r"^Actor: ([^(\n]+)", actor.details, re.MULTILINE)
            position_match = re.search(
                r"^Map anchor: \((\d+), (\d+)\) pixels$",
                actor.details,
                re.MULTILINE,
            )
            name = name_match.group(1).strip() if name_match else "Character"
            position = (
                f"({position_match.group(1)}, {position_match.group(2)})"
                if position_match else "position not mapped"
            )
            route_note = (
                f"; {len(actor.path_points)} recorded route points"
                if actor.path_points else ""
            )
            return f"{name} at map pixel {position}{route_note}"

        populations = self.map_view.actor_positions
        default_population = [
            actor for actor in populations
            if actor.population_class == "default"
        ]
        conditional_population = [
            actor for actor in populations
            if actor.population_class == "conditional"
        ]
        activity_population = [
            actor for actor in populations
            if actor.population_class == "world_activity"
        ]
        if default_population:
            lines.append(
                "Characters in the scene setup: " + "; ".join(
                    readable_actor_position(actor)
                    for actor in default_population
                ) + "."
            )
            lines.append(
                "The map uses each character's extracted sprite art. The game "
                "can still gate actor placement during a scene transition."
            )
        else:
            lines.append("Characters in the scene setup: none are listed for this map.")
        if conditional_population:
            lines.append(
                "Conditional character choices: " + "; ".join(
                    readable_actor_position(actor)
                    for actor in conditional_population
                ) + ". These are alternatives; the game does not place all choices at once."
            )
        if activity_population:
            lines.append(
                "Event-specific character records: " + "; ".join(
                    readable_actor_position(actor)
                    for actor in activity_population
                ) + ". These belong to separate event lists, not the ordinary scene population."
            )
        out_of_bounds_actors = read_out_of_bounds_actor_details(self.assets, info)
        if out_of_bounds_actors:
            lines.append(
                "Some listed character positions fall outside the extracted map "
                "image, so the viewer keeps them in the record list but does not "
                "draw them on the map."
            )
        lines.append(
            "Usual party arrival markers are visible by default. Turn off "
            "Usual party arrival to hide them."
        )
        self.scene_info.setPlainText("\n".join(lines))

    def _update_scene_audio_info(self) -> None:
        info = self.map_by_id[self.scene_id]
        self.audio_info.setPlainText(
            read_map_audio_details(self.repo_root, self.assets, info)
        )

    def _update_scene_field_activities_info(self) -> None:
        rows = [
            row for row in self.field_activity_rows
            if row.get("scene", "").strip() == str(self.scene_id)
        ]
        if not rows:
            self.activity_info.setPlainText(
                "The ROM field-activity outcome table is unavailable. "
                "Regenerate it with decompilation/tools/"
                "extract_world_activity_outcomes.py."
            )
            return

        rows.sort(key=lambda row: int(row.get("random_sum", "0")))
        lines = [
            f"Possible field events for Scene {self.scene_id:02d}",
            "",
            "These are conditional outcomes, not permanent residents. The game "
            "first checks the field-event gates and its random threshold, then "
            "uses two source random draws from 0 through 4. Their sum selects "
            "one of these rows.",
            "",
            "The source does not assign these handlers a fixed map coordinate. "
            "The viewer therefore lists their dialogue here instead of drawing "
            "a character marker on the map.",
        ]
        for row in rows:
            total = row.get("random_sum", "?")
            pair_count = row.get("roll_pair_count", "0")
            pairs = row.get("roll_pairs", "")
            pair_text = f"{pairs} ({pair_count} combinations)" if pairs else "none"
            if row.get("outcome_kind") == "dialogue_event":
                lines.extend((
                    "",
                    f"Roll sum {total} — {row.get('event_name', 'ROM event')}",
                    f"  Roll pairs: {pair_text}.",
                    "  Opening text:",
                ))
                opening = row.get("opening_text", "").replace("|", "\n    ")
                lines.append("    " + (opening or "No opening text decoded."))
                choices = [
                    (number, row.get(f"choice_{number}", "").strip())
                    for number in (1, 2, 3)
                ]
                choices = [(number, value) for number, value in choices if value]
                if choices:
                    lines.append("  Choices shown on the opening panel:")
                    lines.extend(f"    {number}. {value}" for number, value in choices)
                else:
                    lines.append(
                        "  The opening panel has no labeled choices; its "
                        "continuation and later branches are controlled by the ROM handler."
                    )
            else:
                lines.extend((
                    "",
                    f"Roll sum {total} — dynamic world actor/effect sequence",
                    f"  Roll pairs: {pair_text}.",
                    "  This table cell falls through to runtime-generated "
                    "world effects. It does not identify a fixed character "
                    "or map position.",
                ))
        lines.extend((
            "",
            "The opening text and visible choices above are decoded from the "
            "US ROM. Follow-up dialogue and effects can depend on the selected "
            "choice and current game state.",
        ))
        self.activity_info.setPlainText("\n".join(lines))

    def _entity_game_flow(self, entity: EntityDefinition) -> str:
        lines = [entity.name, f"Type: {friendly_identifier(entity.category)}", ""]
        route_rows = read_site_route_rows(self.repo_root, self.scene_id)
        text_catalog = read_lua_string_table(
            self.assets / "dialogue" / "script-text.lua"
        )

        def present(value: str) -> bool:
            return value.strip() not in {"", "-"}

        def place_key(value: str) -> str:
            key = re.sub(r"[^a-z0-9]+", " ", value.casefold()).strip()
            key = re.sub(r"^(?:the|a|an)\s+", "", key)
            key = re.sub(
                r"\s+(?:frontage|site shelter|perimeter barrier)$", "", key
            )
            return key

        entity_name = place_key(entity.name)
        route_sites: list[dict[str, str]] = []
        interaction_addresses = {
            address.group(0).upper()
            for interaction in entity.interactables
            if (address := re.search(r"0x[0-9A-Fa-f]+", interaction.source_event))
        }
        for site in route_rows:
            same_record = (
                site.get("rom_record_address", "").strip().upper()
                in interaction_addresses
            )
            same_profile = (
                present(entity.content_profile) and
                site.get("content_profile", "").strip() == entity.content_profile
            ) or (
                present(entity.speaker_id) and
                site.get("speaker_id", "").strip() == entity.speaker_id
            ) or (
                present(entity.shop_profile) and
                site.get("shop_profile", "").strip() == entity.shop_profile
            )
            same_name = (
                present(site.get("site_name", "")) and
                place_key(site.get("site_name", "")) == entity_name
            )
            if same_record or same_profile or same_name:
                route_sites.append(site)
        # Entities can have several source rectangles for one site. Show one
        # readable profile for repeated records while the Source tab retains
        # each exact ROM address and rectangle.
        unique_routes: list[dict[str, str]] = []
        route_keys: set[tuple[str, str, str]] = set()
        for site in route_sites:
            route_key = (
                site.get("site_name", ""),
                site.get("rom_route", ""),
                site.get("rom_destination", ""),
            )
            if route_key not in route_keys:
                route_keys.add(route_key)
                unique_routes.append(site)
        route_sites = unique_routes

        if entity.interactables:
            lines.append("Access points:")
            for interaction in entity.interactables:
                lines.append(
                    f"  {interaction.width}×{interaction.height} pixel interaction area "
                    f"at entity position ({interaction.local_x}, {interaction.local_y})."
                )
        if route_sites:
            for site in route_sites:
                site_name = site.get("site_name", "").strip() or entity.name
                behavior_summary = (
                    site.get("player_summary", "").strip() or
                    describe_site_behavior(site.get("rom_route", ""))
                )
                lines.extend(("", f"When you use {site_name}:",
                              "  " + behavior_summary))
                destination = site.get("player_destination", "").strip()
                if not destination:
                    destination = describe_site_destination(
                        site.get("rom_destination", ""), self.scene_id
                    )
                if destination:
                    lines.append(f"  Where it leads: {destination}")
                conditions = describe_site_conditions(site)
                if conditions:
                    lines.append(f"  Story conditions: {conditions}")
                name_evidence = describe_site_crosswalk(
                    site.get("crosswalk_status", "unresolved")
                )
                if name_evidence != "name evidence not recorded":
                    lines.append(f"  Name evidence: {name_evidence}.")
                content_lines = read_site_content_details(
                    self.repo_root, site, text_catalog, self.scene_id,
                    include_source_details=False,
                )
                if content_lines:
                    lines.extend(f"  {line}" for line in content_lines)
                else:
                    lines.append(
                        "  Dialogue, menu choices, shop stock, or quest text "
                        "has not been linked to this place yet."
                    )
        else:
            has_linked_profile = False
            if present(entity.content_profile) or present(entity.speaker_id):
                content_lines = read_dialogue_profile_details(
                    self.repo_root,
                    entity.content_profile if present(entity.content_profile) else "",
                    entity.speaker_id if present(entity.speaker_id) else "",
                    text_catalog,
                    include_source_details=False,
                )
                if content_lines:
                    lines.extend(("", "Known dialogue and choices:"))
                    lines.extend(content_lines)
                    has_linked_profile = True
            if present(entity.shop_profile):
                shop_lines = read_shop_profile_details(
                    self.repo_root, entity.shop_profile, text_catalog,
                    include_source_details=False,
                )
                lines.extend(("", *shop_lines))
                has_linked_profile = True
            lines.extend(("", "Route details:"))
            lines.append(
                "No map route has been linked to this place yet." if not has_linked_profile
                else "The dialogue profile is known, but its map entry condition and destination are not linked yet."
            )
        return "\n".join(lines)

    def _show_properties(self, instance: EntityInstance | None) -> None:
        self._loading_fields = True
        enabled = instance is not None
        for widget in (
            self.name_edit, self.category_combo, self.x_spin, self.y_spin,
            self.evidence_edit, self.content_profile_edit, self.speaker_id_edit,
            self.shop_profile_edit,
            self.delete_button, self.parts_list,
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
            self.content_profile_edit.clear(); self.speaker_id_edit.clear()
            self.shop_profile_edit.clear()
            self.x_spin.setValue(0); self.y_spin.setValue(0)
            self.category_combo.setCurrentText("unreviewed")
            self.parts_list.clear()
            self.part_preview.setPixmap(QPixmap())
            self.part_preview.setText("Select a visual part to preview its named asset.")
            self.interactions_list.clear()
            self.collisions_list.clear()
            self.info.setPlainText(
                "Click a map cell to inspect its source tile layers. Click a map interaction or collision box for its evidence. "
                "Click a character marker to inspect its type, position, appearance conditions, and source art. Click an authored event, spawn, or encounter marker to inspect its game data. "
                "Drag entity outlines to move grouped annotations."
            )
            self.flow_info.setPlainText(
                "Select a place, object, or map interaction to see what happens when the player reaches it, including any known destination, dialogue, menu, shop, or story condition."
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
        self.content_profile_edit.setText(entity.content_profile)
        self.speaker_id_edit.setText(entity.speaker_id)
        self.shop_profile_edit.setText(entity.shop_profile)
        part_summary = "\n".join(
            f"- {part.part_id}: {part.role}, source scene {part.source_scene:02d} at "
            f"{part.source_x},{part.source_y} {part.width}x{part.height}; "
            f"{part.draw_layer}; "
            f"{len(parse_outline_paths(part.outline_points))} polygon outline(s); "
            f"asset {part.asset_path or '(created on save)'}"
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
            "Part outlines and selection follow each source object independently; "
            "a union rectangle is not drawn across empty space. Map-composite part "
            "previews are clipped to their outlines. ROM collision remains separate "
            "evidence; these entity records do not "
            "modify the preassembled map or live field collision."
        )
        self.flow_info.setPlainText(self._entity_game_flow(entity))
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
            f"and {event_hits} map interactions. The base bitmap is precomposed; the priority toggle shows its sparse overlay."
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
        outline_points = QLineEdit(part.outline_points)
        outline_points.setPlaceholderText("x,y;x,y;x,y | another polygon")
        form.insertRow(0, "Part ID", part_id)
        form.insertRow(1, "Source kind", source_kind)
        form.addRow("Role", role); form.addRow("Draw depth", depth)
        form.addRow("Entity-local outline polygons", outline_points)
        form.addRow("Evidence", evidence)
        buttons = QDialogButtonBox(QDialogButtonBox.StandardButton.Save | QDialogButtonBox.StandardButton.Cancel)
        buttons.accepted.connect(dialog.accept); buttons.rejected.connect(dialog.reject)
        form.addRow(buttons)
        if dialog.exec() != QDialog.DialogCode.Accepted:
            return
        part.part_id = safe_id(part_id.text())
        part.source_kind = source_kind.currentText()
        try:
            parse_outline_paths(outline_points.text())
        except ValueError as error:
            QMessageBox.warning(self, "Invalid visual outline", str(error))
            return
        for key, spin in fields.items():
            setattr(part, key, spin.value())
        part.role = role.currentText(); part.draw_layer = depth.currentText(); part.evidence = evidence.text().strip()
        part.outline_points = outline_points.text().strip()
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
            if entity.contains_local_point(
                point.x() - instance.origin_x,
                point.y() - instance.origin_y,
            ):
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
        self.flow_info.setPlainText(
            evidence.route_details or
            (evidence.details if evidence.kind == "rom_actor" else
             "No separate dialogue, menu, shop, or quest-flow profile is linked to this evidence row.")
        )
        selected_panel = self.info
        if evidence.route_details or evidence.kind == "rom_actor":
            selected_panel = self.flow_info
        self.info_tabs.setCurrentWidget(selected_panel)
        self.right_scroll.ensureWidgetVisible(
            selected_panel,
            0,
            0,
        )
        if evidence.kind == "rom_event":
            selected_name = evidence.map_label or evidence.title
            status = f"Selected map interaction: {selected_name}. Evidence is read-only."
        else:
            status = f"Inspecting {evidence.title}; source-derived evidence is read-only."
        self.statusBar().showMessage(status, 7000)

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
        self.flow_info.setPlainText(record.describe(self.scene_id))
        self.info_tabs.setCurrentWidget(self.flow_info)
        self.right_scroll.ensureWidgetVisible(self.info, 0, 0)
        origin = "source-profiled" if record.review_status == "source_verified" else "authored"
        self.statusBar().showMessage(f"Selected {origin} {record.kind} record.", 6000)

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
        event_controls: dict[str, QWidget] = {}
        if record.kind == "scripted_trigger":
            event_id = QSpinBox()
            event_id.setRange(0, 0x7FFFFFFF)
            event_id.setValue(record.event_id)
            event_controls["event_id"] = event_id
            form.addRow("Runtime event ID (0 = editor only)", event_id)

            trigger_type = QComboBox()
            trigger_type.addItems(["scene_entry", "actor_within_scope"])
            trigger_type.setCurrentText(record.trigger_type)
            event_controls["trigger_type"] = trigger_type
            form.addRow("Runtime trigger", trigger_type)

            coordinate_space = QComboBox()
            coordinate_space.addItems(["map_pixels", "field_fixed_4"])
            coordinate_space.setCurrentText(record.coordinate_space)
            event_controls["coordinate_space"] = coordinate_space
            form.addRow("Runtime coordinate space", coordinate_space)

            for name, label in (
                ("scope_min_x", "Exact scope min X"),
                ("scope_max_x", "Exact scope max X"),
                ("scope_min_y", "Exact scope min Y"),
                ("scope_max_y", "Exact scope max Y"),
            ):
                spin = QSpinBox()
                spin.setRange(-1, 0xFFFF)
                spin.setValue(getattr(record, name))
                event_controls[name] = spin
                form.addRow(label, spin)

            required_flags = QLineEdit(record.required_flags)
            required_flags.setPlaceholderText("Comma-separated typed state flags")
            event_controls["required_flags"] = required_flags
            form.addRow("Required state flags", required_flags)

            action = QComboBox()
            action.addItems(["run_lua_handler", "dispatch_native_script"])
            action.setCurrentText(record.action)
            event_controls["action"] = action
            form.addRow("Runtime action", action)

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
        if record.kind == "scripted_trigger":
            next_event_id = event_controls["event_id"].value()
            if next_event_id > 0 and any(
                item is not record and item.event_id == next_event_id
                for item in self.behaviors
            ):
                QMessageBox.warning(
                    self,
                    "Duplicate runtime event ID",
                    f"Runtime event ID {next_event_id} is already used in this scene.",
                )
                return
        record.record_id = new_id
        if record.kind == "scripted_trigger":
            record.event_id = event_controls["event_id"].value()
            record.trigger_type = event_controls["trigger_type"].currentText()
            record.coordinate_space = event_controls["coordinate_space"].currentText()
            for name in ("scope_min_x", "scope_max_x", "scope_min_y", "scope_max_y"):
                setattr(record, name, event_controls[name].value())
            record.required_flags = event_controls["required_flags"].text().strip()
            record.action = event_controls["action"].currentText()
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
        entity.content_profile = self.content_profile_edit.text().strip()
        entity.speaker_id = self.speaker_id_edit.text().strip()
        entity.shop_profile = self.shop_profile_edit.text().strip()
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
    inspect_instance_id = getattr(viewer, "inspect_instance_id", "")
    if inspect_instance_id:
        # Close-up QA should expose the source pixels and the selected
        # entity's contours without unrelated map evidence obscuring them.
        if viewer.map_view.grid is not None:
            viewer.map_view.grid.setVisible(False)
        for evidence_item in viewer.map_view.evidence_items:
            evidence_item.setVisible(False)
        for instance_id, group in viewer.map_view.entity_groups.items():
            group.setVisible(instance_id == inspect_instance_id)
            if instance_id == inspect_instance_id:
                for child in group.childItems():
                    if child.data(1) == "entity_label":
                        child.setVisible(False)
        instance = next((
            item for item in viewer.instances
            if item.instance_id == inspect_instance_id
        ), None)
        if instance is not None:
            entity = viewer.entities[instance.entity_id]
            x, y, width, height = entity.bounds
            view = viewer.map_view
            view.resetTransform()
            margin = 1.25
            scale = min(
                view.viewport().width() / max(1, width * margin),
                view.viewport().height() / max(1, height * margin),
            )
            view.scale(scale, scale)
            view.centerOn(
                instance.origin_x + x + width / 2,
                instance.origin_y + y + height / 2,
            )
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
    parser.add_argument(
        "--show-actor-routes",
        action="store_true",
        help="show ROM default-actor waypoint lines (useful with --snapshot)",
    )
    parser.add_argument(
        "--show-world-activity",
        action="store_true",
        help="show a selected ROM special actor-list group (useful with --snapshot)",
    )
    parser.add_argument(
        "--activity-list",
        choices=("all", "mode_13_entry", "pending_list_1e5c4", "pending_list_1e628"),
        default="mode_13_entry",
        help="choose which scene-47 world-activity list to show",
    )
    parser.add_argument(
        "--inspect-rom-event",
        help="select a ROM event record address (for example 0x173138), useful with --snapshot",
    )
    parser.add_argument(
        "--inspect-entity",
        help="select an entity ID or scene instance ID and center the map on it",
    )
    parser.add_argument(
        "--inspect-tab",
        choices=("Source", "What happens", "Map profile", "Map audio", "Field events"),
        help="select a right-dock evidence tab, useful with --snapshot",
    )
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
    if args.show_actor_routes or args.show_world_activity:
        viewer.map_view.show_actor_routes = args.show_actor_routes
        viewer.map_view.show_world_activity = args.show_world_activity
        viewer.map_view.world_activity_group = args.activity_list
        viewer.map_view.draw_evidence_overlays()
    if args.inspect_rom_event:
        address = args.inspect_rom_event.strip().lower()
        evidence = next((
            event for event in viewer.map_view.event_rectangles
            if address in event.details.lower()
        ), None)
        if evidence is None:
            parser.error(
                f"No event record containing {args.inspect_rom_event} in scene {args.scene:02d}"
            )
        viewer.show_evidence(evidence)
    if args.inspect_entity:
        entity_item = next((
            viewer.entity_list.item(index)
            for index in range(viewer.entity_list.count())
            if viewer.entity_list.item(index).data(Qt.ItemDataRole.UserRole)
            == args.inspect_entity
            or any(
                instance.instance_id == viewer.entity_list.item(index).data(
                    Qt.ItemDataRole.UserRole
                ) and instance.entity_id == args.inspect_entity
                for instance in viewer.instances
            )
            or viewer.entity_list.item(index).text().split(" [", 1)[0]
            == args.inspect_entity
        ), None)
        if entity_item is None:
            parser.error(
                f"No entity or instance '{args.inspect_entity}' in scene {args.scene:02d}"
            )
        viewer.entity_list.setCurrentItem(entity_item)
        instance = next((
            item for item in viewer.instances
            if item.instance_id == entity_item.data(Qt.ItemDataRole.UserRole)
        ), None)
        if instance is not None:
            viewer.inspect_instance_id = instance.instance_id
    if args.inspect_tab:
        tab_index = next(
            index for index in range(viewer.info_tabs.count())
            if viewer.info_tabs.tabText(index) == args.inspect_tab
        )
        viewer.info_tabs.setCurrentIndex(tab_index)
    if args.snapshot:
        render_snapshot(viewer, args.snapshot)
        return 0
    viewer.show()
    return app.exec()


if __name__ == "__main__":
    raise SystemExit(main())
