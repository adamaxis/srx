#!/usr/bin/env python3
"""Structural ROM-to-port source map for the active Ghidra conversion.

This report tracks explicit source references. It does not establish that the
runtime behavior matches the ROM.
"""

from __future__ import annotations

import argparse
import csv
from dataclasses import dataclass
from pathlib import Path
import re
import sys
from typing import Iterable


ROOT = Path(__file__).resolve().parents[3]
STATUS_PATH = Path("decompilation/docs/ghidra/decompiled-function-status.tsv")
FUNCTIONS_PATH = Path("decompilation/export/functions.tsv")
MAP_PATH = Path("decompilation/docs/ghidra/rom-coverage-map.tsv")
BASELINE_PATH = Path("decompilation/docs/ghidra/rom-coverage-baseline.txt")
REPORT_PATH = Path("decompilation/docs/ghidra/rom-coverage.md")

MAP_HEADER = ["rom_entry", "mapping_kind", "implementation", "evidence"]
REFERENCE_PATH = re.compile(
    r"(?P<path>(?:decompilation/)?(?:gameplay|ghidra_conversion)/"
    r"[A-Za-z0-9_./-]+\.(?:lua|c|cc|cpp|h|hpp))"
    r"(?::(?P<symbol>[A-Za-z_][A-Za-z0-9_.:]*)?)?"
)
QUALIFIED_SYMBOL = re.compile(
    r"\b[A-Za-z_][A-Za-z0-9_]*(?:::[A-Za-z_][A-Za-z0-9_]*)+"
)
BARE_SYMBOL = re.compile(r"\b(?:[A-Za-z][A-Za-z0-9]*[A-Z][A-Za-z0-9]*|[a-z][a-z0-9]*(?:_[a-z0-9]+)+)\b")


class CoverageError(Exception):
    pass


@dataclass(frozen=True)
class Routine:
    address: str
    subsystem: str
    owner: str
    status: str
    function_name: str = ""
    size_bytes: int | None = None


@dataclass(frozen=True)
class Mapping:
    address: str
    kind: str
    implementation: str
    evidence: str


@dataclass
class Coverage:
    routines: list[Routine]
    mappings: dict[str, Mapping]
    baseline: set[str]
    errors: list[str]
    status_rows: int = 0
    ledger_conflicts: list[str] | None = None

    @property
    def mapped(self) -> list[Routine]:
        return [routine for routine in self.routines if routine.address in self.mappings]

    @property
    def unmapped(self) -> list[Routine]:
        return [routine for routine in self.routines if routine.address not in self.mappings]


def normalize_address(value: str) -> str:
    text = value.strip()
    if text.lower().startswith("0x"):
        text = text[2:]
    if not text or not re.fullmatch(r"[0-9a-fA-F]+", text):
        raise CoverageError(f"invalid ROM entry {value!r}")
    return f"0x{int(text, 16):04X}"


def read_tsv(path: Path) -> list[dict[str, str]]:
    if not path.is_file():
        raise CoverageError(f"missing TSV: {path}")
    with path.open(newline="", encoding="utf-8") as stream:
        return list(csv.DictReader(stream, delimiter="\t"))


def read_inventory(root: Path) -> tuple[list[Routine], int, int, list[str]]:
    status_rows = read_tsv(root / STATUS_PATH)
    required_status = {"rom_entry", "subsystem", "native_owner", "status"}
    if not status_rows or not required_status.issubset(status_rows[0]):
        raise CoverageError(f"status ledger must contain columns: {', '.join(sorted(required_status))}")

    status_by_address: dict[str, list[dict[str, str]]] = {}
    for number, row in enumerate(status_rows, start=2):
        try:
            address = normalize_address(row["rom_entry"])
        except CoverageError as error:
            raise CoverageError(f"{STATUS_PATH}:{number}: {error}") from error
        status_by_address.setdefault(address, []).append(row)

    function_rows = read_tsv(root / FUNCTIONS_PATH)
    required_functions = {"entry", "name", "size"}
    if function_rows and not required_functions.issubset(function_rows[0]):
        raise CoverageError(f"Ghidra function list must contain columns: {', '.join(sorted(required_functions))}")
    export_by_address: dict[str, tuple[str, int]] = {}
    for number, row in enumerate(function_rows, start=2):
        try:
            address = normalize_address(row["entry"])
            size = int(row["size"], 0) if row["size"].lower().startswith("0x") else int(row["size"])
        except (CoverageError, ValueError) as error:
            raise CoverageError(f"{FUNCTIONS_PATH}:{number}: invalid entry or size ({error})") from error
        if address in export_by_address:
            raise CoverageError(f"duplicate Ghidra function entry: {address}")
        export_by_address[address] = (row["name"].strip(), size)

    # Status rows can describe source call sites as well as function entries,
    # and this checkout has a few duplicate address labels. Use the Ghidra
    # function name to select the matching status row where possible, retain
    # other status-only entries, and surface collisions in the generated doc.
    addresses = set(status_by_address) | set(export_by_address)
    routines: list[Routine] = []
    conflicts: list[str] = []
    for address in addresses:
        rows = status_by_address.get(address, [])
        function = export_by_address.get(address)
        function_name, size = function if function else ("", None)
        if rows:
            if len(rows) > 1 and function_name:
                matching = [
                    row for row in rows
                    if function_name.casefold() in row.get("subsystem", "").casefold()
                    or function_name.casefold() in row.get("native_owner", "").casefold()
                ]
                selected = matching[0] if matching else rows[0]
                other_rows = [row for row in rows if row is not selected]
                for row in other_rows:
                    conflicts.append(
                        f"{address}: Ghidra names {function_name}; ignored conflicting status label {row.get('subsystem', '').strip()}"
                    )
                rows = [selected]
            elif len(rows) > 1:
                conflicts.append(f"{address}: {len(rows)} status-ledger rows share this entry")
            subsystem_values = list(dict.fromkeys(row.get("subsystem", "").strip() for row in rows if row.get("subsystem", "").strip()))
            owner_values = list(dict.fromkeys(row.get("native_owner", "").strip() for row in rows if row.get("native_owner", "").strip()))
            status_values = list(dict.fromkeys(row.get("status", "").strip() for row in rows if row.get("status", "").strip()))
            subsystem = " / ".join(subsystem_values) or (f"Ghidra export: {function_name}" if function_name else "Unclassified routine")
            owner = ";".join(owner_values)
            status = "/".join(status_values) or "unclassified"
        else:
            subsystem = f"Ghidra export: {function_name}"
            owner = ""
            status = "untracked"
        routines.append(Routine(address, subsystem, owner, status, function_name, size))
    return sorted(routines, key=lambda routine: int(routine.address, 16)), len(export_by_address), len(status_rows), conflicts


def parse_reference(reference: str) -> tuple[Path, str | None]:
    path_text, separator, symbol = reference.partition("#")
    if not path_text or (separator and not symbol):
        raise CoverageError(f"invalid implementation reference {reference!r}")
    return Path(path_text), symbol if separator else None


def reference_source(root: Path, reference: str) -> tuple[Path, str | None]:
    relative, symbol = parse_reference(reference)
    if relative.is_absolute() or ".." in relative.parts:
        raise CoverageError(f"implementation reference must stay inside the repository: {reference}")
    if relative.suffix.lower() not in {".lua", ".c", ".cc", ".cpp", ".h", ".hpp"}:
        raise CoverageError(f"implementation must name a Lua/C++ source file: {reference}")
    source_path = root / relative
    if not source_path.is_file():
        raise CoverageError(f"implementation source does not exist: {reference}")
    if symbol:
        text = source_path.read_text(encoding="utf-8", errors="replace")
        if not symbol_is_present(source_path, text, symbol):
            raise CoverageError(f"symbol {symbol!r} is absent from {relative}")
    return relative, symbol


def symbol_is_present(path: Path, text: str, symbol: str) -> bool:
    if "::" not in symbol:
        return declared_symbol(path, text, symbol)
    parts = symbol.split("::")
    leaf = parts[-1]
    if not declared_symbol(path, text, leaf):
        return False
    qualified_prefix = "::".join(parts[:-1])
    if re.search(r"\b" + re.escape(qualified_prefix) + r"::" + re.escape(leaf) + r"\s*\(", text):
        return True
    namespace_parts = parts[:-1]
    scope_parts: list[str] = []
    for scope in re.findall(r"\bnamespace\s+([A-Za-z_][A-Za-z0-9_:]*)\s*\{", text):
        scope_parts.extend(scope.split("::"))
    cursor = 0
    for part in namespace_parts:
        try:
            cursor = scope_parts.index(part, cursor) + 1
        except ValueError:
            return False
    return bool(namespace_parts)


def read_mappings(root: Path, path: Path, routine_addresses: set[str]) -> tuple[dict[str, Mapping], list[str]]:
    errors: list[str] = []
    try:
        rows = read_tsv(path)
    except CoverageError as error:
        return {}, [str(error)]
    if rows and not set(MAP_HEADER).issubset(rows[0]):
        return {}, [f"{path} must contain columns: {', '.join(MAP_HEADER)}"]
    mappings: dict[str, Mapping] = {}
    for number, row in enumerate(rows, start=2):
        try:
            address = normalize_address(row["rom_entry"])
        except CoverageError as error:
            errors.append(f"{path}:{number}: {error}")
            continue
        if address in mappings:
            errors.append(f"duplicate mapping for {address} in {path}")
            continue
        kind = row["mapping_kind"].strip()
        implementation = row["implementation"].strip()
        evidence = row["evidence"].strip()
        mapping = Mapping(address, kind, implementation, evidence)
        mappings[address] = mapping
        if address not in routine_addresses:
            errors.append(f"mapping has no routine in inventory: {address}")
        if kind not in {"port", "replaced", "not_needed"}:
            errors.append(f"unsupported mapping_kind {kind!r} for {address}")
            continue
        if not evidence:
            errors.append(f"mapping evidence is blank for {address}")
        if kind == "not_needed":
            if implementation:
                errors.append(f"not_needed mapping must leave implementation blank for {address}")
            if not evidence:
                errors.append(f"not_needed mapping must document why it is unnecessary for {address}")
            continue
        references = [reference.strip() for reference in implementation.split(";") if reference.strip()]
        if not references:
            errors.append(f"{kind} mapping has no implementation reference for {address}")
            continue
        for reference in references:
            try:
                reference_source(root, reference)
            except CoverageError as error:
                errors.append(f"{address}: {error}")
    return mappings, errors


def read_baseline(path: Path, routine_addresses: set[str]) -> tuple[set[str], list[str]]:
    errors: list[str] = []
    if not path.is_file():
        return set(), [f"missing mapped-routine baseline: {path}"]
    baseline: set[str] = set()
    for number, raw in enumerate(path.read_text(encoding="utf-8").splitlines(), start=1):
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        try:
            address = normalize_address(line)
        except CoverageError as error:
            errors.append(f"{path}:{number}: {error}")
            continue
        if address in baseline:
            errors.append(f"duplicate baseline routine {address} in {path}")
        baseline.add(address)
        if address not in routine_addresses:
            errors.append(f"baseline routine is absent from inventory: {address}")
    if not baseline:
        errors.append(f"mapped-routine baseline is empty: {path}")
    return baseline, errors


def inspect(root: Path = ROOT, map_path: Path | None = None, baseline_path: Path | None = None) -> tuple[Coverage, int]:
    root = root.resolve()
    try:
        routines, export_count, status_rows, conflicts = read_inventory(root)
    except CoverageError as error:
        return Coverage([], {}, set(), [str(error)]), 0
    routine_addresses = {routine.address for routine in routines}
    mappings, map_errors = read_mappings(root, map_path or (root / MAP_PATH), routine_addresses)
    baseline, baseline_errors = read_baseline(baseline_path or (root / BASELINE_PATH), routine_addresses)
    errors = map_errors + baseline_errors
    for address in sorted(baseline):
        if address not in mappings:
            errors.append(f"baseline routine lost its mapping: {address}")
    return Coverage(routines, mappings, baseline, errors, status_rows, conflicts), export_count


def source_files(root: Path) -> list[Path]:
    candidates: list[Path] = []
    for pattern in (
        "decompilation/gameplay/scripts/**/*.lua",
        "decompilation/ghidra_conversion/src/**/*.cpp",
        "decompilation/ghidra_conversion/src/**/*.hpp",
        "decompilation/ghidra_conversion/qt_runtime/src/**/*.cpp",
        "decompilation/ghidra_conversion/qt_runtime/src/**/*.hpp",
    ):
        candidates.extend(root.glob(pattern))
    return sorted(set(path for path in candidates if path.is_file()), key=lambda path: path.as_posix())


def declared_symbol(path: Path, text: str, symbol: str) -> bool:
    leaf = symbol.replace("::", ".").split(".")[-1]
    escaped = re.escape(leaf)
    if path.suffix == ".lua":
        patterns = (
            rf"(?m)^\s*(?:local\s+)?function\s+(?:[A-Za-z_][A-Za-z0-9_.:]*[.:])?{escaped}\b",
            rf"(?m)^\s*(?:[A-Za-z_][A-Za-z0-9_.:]*[.:])?{escaped}\s*=\s*function\b",
        )
        if any(re.search(pattern, text) for pattern in patterns):
            return True
        return bool(re.search(rf"(?m)^\s*(?:local\s+)?{escaped}\s*=\s*\{{", text))
    type_pattern = rf"\b(?:class|struct|enum\s+class)\s+{escaped}\b"
    function_pattern = rf"(?m)^\s*[^/#\n]*\b{escaped}\s*\("
    return bool(re.search(type_pattern, text) or re.search(function_pattern, text))


def find_symbol_reference(root: Path, files: list[Path], texts: dict[Path, str], symbol: str, cache: dict[str, str | None]) -> str | None:
    # Only map identifiers that occur as a declaration/definition in an active
    # Lua or C++ source file, not arbitrary names from descriptive prose.
    if symbol in cache:
        return cache[symbol]
    leaf = symbol.replace("::", ".").split(".")[-1]
    for path in files:
        text = texts[path]
        if leaf not in text:
            continue
        if symbol_is_present(path, text, symbol):
            relative = path.relative_to(root).as_posix()
            mapped_symbol = symbol if "::" in symbol else leaf
            cache[symbol] = f"{relative}#{mapped_symbol}"
            return cache[symbol]
    cache[symbol] = None
    return None


def derive_owner_references(root: Path, owner: str, files: list[Path], texts: dict[Path, str], symbol_cache: dict[str, str | None]) -> list[str]:
    references: list[str] = []
    for match in REFERENCE_PATH.finditer(owner):
        path_text = match.group("path")
        symbol = match.group("symbol")
        if not path_text.startswith("decompilation/"):
            path_text = f"decompilation/{path_text}"
        reference = path_text + (f"#{symbol}" if symbol else "")
        try:
            reference_source(root, reference)
        except CoverageError:
            continue
        references.append(reference)

    if not references:
        symbols = QUALIFIED_SYMBOL.findall(owner) + BARE_SYMBOL.findall(owner)
        for symbol in symbols:
            reference = find_symbol_reference(root, files, texts, symbol, symbol_cache)
            if reference:
                references.append(reference)
                break
    return list(dict.fromkeys(references))


def initialize_mapping(root: Path, map_path: Path, baseline_path: Path, replace_generated: bool = False) -> tuple[int, int]:
    if map_path.exists() or baseline_path.exists():
        generated_marker = "# ROM routines with explicit source mappings when this baseline was established."
        if not replace_generated or not baseline_path.is_file() or not baseline_path.read_text(encoding="utf-8").startswith(generated_marker):
            raise CoverageError("refusing to overwrite an existing mapping or baseline")
    routines, _, _, _ = read_inventory(root)
    files = source_files(root)
    texts = {path: path.read_text(encoding="utf-8", errors="replace") for path in files}
    symbol_cache: dict[str, str | None] = {}
    map_path.parent.mkdir(parents=True, exist_ok=True)
    mapped: list[Mapping] = []
    for routine in routines:
        if not routine.owner:
            continue
        references = derive_owner_references(root, routine.owner, files, texts, symbol_cache)
        if references:
            mapped.append(Mapping(
                address=routine.address,
                kind="port",
                implementation=";".join(references),
                evidence=f"{STATUS_PATH.as_posix()}:native_owner",
            ))
    with map_path.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.writer(stream, delimiter="\t", lineterminator="\n")
        writer.writerow(MAP_HEADER)
        for mapping in mapped:
            writer.writerow([mapping.address, mapping.kind, mapping.implementation, mapping.evidence])
    baseline_path.parent.mkdir(parents=True, exist_ok=True)
    with baseline_path.open("w", encoding="utf-8") as stream:
        stream.write("# ROM routines with explicit source mappings when this baseline was established.\n")
        stream.write("# Remove an address only after an intentional mapping change is documented.\n")
        for mapping in mapped:
            stream.write(mapping.address + "\n")
    return len(mapped), len(routines) - len(mapped)


def category(routine: Routine) -> str:
    text = f"{routine.function_name} {routine.subsystem}".lower()
    groups = (
        ("Combat and magic", ("combat", "attack", "damage", "weapon", "armor", "spell", "magic", "karma", "stun", "hit roll")),
        ("Actors and movement", ("actor", "entity", "pedestrian", "enemy", "runner", "movement", "collision", "creature", "projectile")),
        ("Menus and dialogue", ("menu", "notebook", "matrix", "deck", "cyber", "inventory", "dialogue", "conversation", "vid-phone", "phone", "taxi", "shop", "screen")),
        ("Field and world", ("field", "scene", "world", "map", "building", "door", "elevator", "terrain", "region", "travel")),
        ("Opening and story", ("opening", "intro", "title", "boot", "airport", "orc", "chopper", "story")),
        ("Presentation and audio", ("render", "draw", "palette", "vblank", "frame", "audio", "sound", "music", "graphics", "sprite")),
    )
    for label, words in groups:
        if any(word in text for word in words):
            return label
    return "Core and shared"


def markdown_cell(value: str) -> str:
    return value.replace("|", "\\|").replace("\n", " ").replace("\r", " ")


def display_name(routine: Routine) -> str:
    return routine.function_name or routine.subsystem


def build_report(coverage: Coverage, export_count: int) -> str:
    total = len(coverage.routines)
    mapped = len(coverage.mapped)
    unmapped = len(coverage.unmapped)
    code_routines = [routine for routine in coverage.routines if routine.function_name]
    code_mapped = sum(routine.address in coverage.mappings for routine in code_routines)
    code_unmapped = [routine for routine in code_routines if routine.address not in coverage.mappings]
    ledger_only = [routine for routine in coverage.routines if not routine.function_name]
    ledger_only_mapped = sum(routine.address in coverage.mappings for routine in ledger_only)
    ledger_only_unmapped = [routine for routine in ledger_only if routine.address not in coverage.mappings]
    status_counts: dict[str, int] = {}
    for routine in coverage.routines:
        status_counts[routine.status] = status_counts.get(routine.status, 0) + 1
    gap_groups: dict[str, list[Routine]] = {}
    for routine in coverage.unmapped:
        gap_groups.setdefault(category(routine), []).append(routine)
    largest = sorted(
        (routine for routine in coverage.unmapped if routine.size_bytes is not None),
        key=lambda routine: (-int(routine.size_bytes or 0), int(routine.address, 16)),
    )[:20]

    lines = [
        "# ROM coverage",
        "",
        "This is a structural source map, not a claim of behavioral ROM fidelity. A mapped row has an explicit Lua/C++ source file or symbol in the committed mapping table, or a documented replacement entry. Runtime equivalence still requires behavior-specific ROM traces and measurements.",
        "",
        "The 66-entry historical B2 triage is recorded in [rom-coverage-triage.md](rom-coverage-triage.md) and [rom-coverage-triage.tsv](rom-coverage-triage.tsv). That ledger distinguishes port owners, host or asset replacements, and routines filed as missing bugs.",
        "",
        f"Inventory: the union of `{STATUS_PATH.as_posix()}` and `{FUNCTIONS_PATH.as_posix()}`. The status ledger has {coverage.status_rows} rows; after address deduplication and including any export-only entries, the inventory contains **{total} routine entries** ({export_count} unique Ghidra function exports). Ghidra names and sizes take precedence where the export list has the same address.",
        "",
        f"Mapped: **{mapped}**. Unmapped backlog: **{unmapped}**. Committed mapped-routine baseline: **{len(coverage.baseline)}**.",
        "",
        "| Inventory category | Entries | Mapped | Unmapped |",
        "|---|---:|---:|---:|",
        f"| Ghidra code function exports | {export_count} | {code_mapped} | {len(code_unmapped)} |",
        f"| Status-ledger-only addresses | {len(ledger_only)} | {ledger_only_mapped} | {len(ledger_only_unmapped)} |",
        f"| Combined unique addresses | {total} | {mapped} | {unmapped} |",
        "",
        "The `shadowrun_rom_coverage` CTest validates the current mapping references and fails if a routine in the baseline loses its mapping. Existing unmapped entries are reported but do not fail CTest. Run `python3 decompilation/ghidra_conversion/tools/rom_coverage.py --write-report` after updating the map.",
        "",
        "## Status ledger counts",
        "",
        "| Status | Routines |",
        "|---|---:|",
    ]
    for status, count in sorted(status_counts.items()):
        lines.append(f"| {markdown_cell(status)} | {count} |")
    if coverage.ledger_conflicts:
        lines.extend(["", "## Status ledger address conflicts", "", "These duplicate labels are retained as provenance warnings. Where a Ghidra function name exists at the address, its matching status row supplies the mapping owner.", ""])
        for conflict in sorted(coverage.ledger_conflicts):
            lines.append(f"- {markdown_cell(conflict)}")
    lines.extend(["", "## Largest unmapped routines", ""])
    if largest:
        lines.extend([
            "Code size comes from the Ghidra export list; only exported routines have a size here.",
            "",
            "| ROM entry | Ghidra name | Size (bytes) | Subsystem |",
            "|---|---|---:|---|",
        ])
        for routine in largest:
            lines.append(
                f"| `{routine.address}` | {markdown_cell(routine.function_name)} | {routine.size_bytes} | {markdown_cell(routine.subsystem)} |"
            )
    else:
        lines.append("No unmapped routine with a Ghidra size entry.")
    lines.extend(["", "## Largest unmapped subsystem groups", "", "Groups are keyword buckets derived from the Ghidra name and status-ledger subsystem text, for triage only.", "", "| Group | Unmapped entries (combined) |", "|---|---:|"])
    for group, routines in sorted(gap_groups.items(), key=lambda item: (-len(item[1]), item[0])):
        lines.append(f"| {group} | {len(routines)} |")
    code_gap_groups: dict[str, list[Routine]] = {}
    for routine in code_unmapped:
        code_gap_groups.setdefault(category(routine), []).append(routine)
    lines.extend(["", "| Group | Unmapped Ghidra code routines |", "|---|---:|"])
    for group, routines in sorted(code_gap_groups.items(), key=lambda item: (-len(item[1]), item[0])):
        lines.append(f"| {group} | {len(routines)} |")
    lines.extend(["", "## Mapped routines", "", "| ROM entry | Routine | Size (bytes) | Port implementation | Mapping kind |", "|---|---|---:|---|---|"])
    for routine in coverage.mapped:
        mapping = coverage.mappings[routine.address]
        size = str(routine.size_bytes) if routine.size_bytes is not None else "—"
        lines.append(
            f"| `{routine.address}` | {markdown_cell(display_name(routine))} | {size} | {markdown_cell(mapping.implementation or mapping.evidence)} | {mapping.kind} |"
        )
    lines.extend(["", "## Unmapped routines", "", "| ROM entry | Routine | Size (bytes) | Subsystem | Status |", "|---|---|---:|---|---|"])
    for routine in coverage.unmapped:
        size = str(routine.size_bytes) if routine.size_bytes is not None else "—"
        lines.append(
            f"| `{routine.address}` | {markdown_cell(display_name(routine))} | {size} | {markdown_cell(routine.subsystem)} | {markdown_cell(routine.status)} |"
        )
    lines.append("")
    return "\n".join(lines)


def main(argv: Iterable[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--initialize-map", action="store_true", help="create the initial explicit map and baseline from source-validated status owner references; refuses to overwrite")
    mode.add_argument("--write-report", action="store_true", help="regenerate the committed Markdown report")
    mode.add_argument("--check", action="store_true", help="validate mappings, baseline, and committed report")
    parser.add_argument("--root", type=Path, default=ROOT, help=argparse.SUPPRESS)
    parser.add_argument("--map", type=Path, default=None, help=argparse.SUPPRESS)
    parser.add_argument("--baseline", type=Path, default=None, help=argparse.SUPPRESS)
    parser.add_argument("--report", type=Path, default=None, help=argparse.SUPPRESS)
    parser.add_argument("--replace-generated-map", action="store_true", help="reseed only a map/baseline carrying this tool's generated baseline marker")
    args = parser.parse_args(argv)
    root = args.root.resolve()
    map_path = args.map or (root / MAP_PATH)
    baseline_path = args.baseline or (root / BASELINE_PATH)
    report_path = args.report or (root / REPORT_PATH)

    if args.initialize_map:
        try:
            mapped_count, total_unmapped = initialize_mapping(root, map_path, baseline_path, args.replace_generated_map)
        except CoverageError as error:
            print(f"ROM coverage initialization failed: {error}", file=sys.stderr)
            return 1
        print(f"Initialized {mapped_count} explicit mappings; {total_unmapped} routines remain unmapped.")
        return 0

    coverage, export_count = inspect(root, map_path, baseline_path)
    if coverage.errors:
        print("ROM coverage check failed:", file=sys.stderr)
        for error in coverage.errors:
            print(f"  {error}", file=sys.stderr)
        return 1
    report = build_report(coverage, export_count)
    if args.write_report:
        report_path.parent.mkdir(parents=True, exist_ok=True)
        report_path.write_text(report, encoding="utf-8")
        code_routines = [routine for routine in coverage.routines if routine.function_name]
        code_mapped = sum(routine.address in coverage.mappings for routine in code_routines)
        print(f"Wrote {report_path}: {code_mapped}/{len(code_routines)} exported routines mapped; {len(coverage.mapped)}/{len(coverage.routines)} total entries mapped; {len(coverage.baseline)} baseline.")
        return 0
    if not report_path.is_file():
        print(f"ROM coverage report is missing: {report_path}", file=sys.stderr)
        return 1
    if report_path.read_text(encoding="utf-8") != report:
        print(f"ROM coverage report is stale: run python3 {Path(__file__).relative_to(root).as_posix()} --write-report", file=sys.stderr)
        return 1
    code_routines = [routine for routine in coverage.routines if routine.function_name]
    code_mapped = sum(routine.address in coverage.mappings for routine in code_routines)
    print(f"ROM coverage check passed: {code_mapped}/{len(code_routines)} exported routines mapped; {len(coverage.mapped)}/{len(coverage.routines)} total entries mapped; {len(coverage.baseline)} baseline.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
