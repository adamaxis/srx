#!/usr/bin/env python3
"""Regenerate the whole offline FLAC audio pack and loop-points.tsv.

One command rebuilds every rendered resource with the current extractor
timing (Z80_TICKS_PER_FRAME), so tempo fixes in extract_genesis_audio.py reach
the shipped assets.  Run from decompilation/:

    python ghidra_conversion/tools/rom_research/audio/regen_audio_assets.py \
        input/shadowrun_genesis_us.bin assets build/audio-vgm [--jobs 8]

then regenerate the id manifest from gameplay/scripts:

    lua5.4 tools/gen_audio_manifest.lua ../../assets > ../../assets/audio/audio_manifest.tsv

The capture policy per resource is the one documented in
assets/audio/README.md ("Duration policy"); it is encoded in RESOURCES below
so it is not reconstructed by hand.  Needs the z80 package, NumPy,
SoundFile and libgme (SHADOWRUN_LIBGME or the system library).
"""

from __future__ import annotations

import argparse
import enum
import multiprocessing
import pathlib
import sys
from dataclasses import dataclass

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import extract_genesis_audio as extract  # noqa: E402
import render_vgm_flac as renderer  # noqa: E402


class Capture(enum.Enum):
    # Traced through two successive source restarts of the stream that first
    # restarts; loop-points.tsv gets [first, second) restart.
    LOOPED_MUSIC = "looped"
    # No proven whole-track boundary: fixed observation window, no loop row.
    OBSERVATION = "observation"
    # Final chip event plus a release tail.
    ONE_SHOT = "one_shot"
    # Source-cadenced repeated starts (SEGA shock 0x5C, logo reveal 0x5D).
    SEQUENCE = "sequence"


ONE_SHOT_TRACE_SECONDS = 60.0
ONE_SHOT_TAIL_SECONDS = 2.0
MUSIC_SECONDS = 120.0
LONG_MUSIC_SECONDS = 180.0
PERSISTENT_SECONDS = 600.0


@dataclass(frozen=True)
class Resource:
    resource_id: int
    path: str
    alias: str
    capture: Capture
    seconds: float = ONE_SHOT_TRACE_SECONDS
    repeat_count: int = 1
    repeat_interval_frames: int = 0
    max_duration_seconds: float | None = None


def _music(resource_id: int, capture: Capture, seconds: float) -> Resource:
    return Resource(resource_id, f"audio/music/resource-{resource_id:03d}.flac",
                    f"audio.music.resource.{resource_id:03d}", capture, seconds)


def _effect(resource_id: int) -> Resource:
    return Resource(resource_id, f"audio/sound-effects/resource-{resource_id:03d}.flac",
                    f"audio.effects.resource.{resource_id:03d}", Capture.ONE_SHOT)


RESOURCES: tuple[Resource, ...] = (
    *(_music(i, Capture.LOOPED_MUSIC, MUSIC_SECONDS) for i in (0, 1, 2, 3, 4, 6, 11, 19)),
    *(_music(i, Capture.LOOPED_MUSIC, LONG_MUSIC_SECONDS) for i in (7, 8, 9)),
    *(_music(i, Capture.OBSERVATION, MUSIC_SECONDS) for i in (5, 14, 15, 40)),
    *(_music(i, Capture.OBSERVATION, PERSISTENT_SECONDS) for i in (12, 13)),
    Resource(10, "audio/music/resource-010.flac", "audio.music.resource.010", Capture.ONE_SHOT),
    Resource(0x10, "audio/intro/title-stage-1.flac", "audio.intro.title.stage-1", Capture.ONE_SHOT),
    # 0x11 drives YM DAC samples from Timer A; it rendered silent until the
    # extractor modelled the timers (0x02B9 busy-wait).
    Resource(0x11, "audio/intro/title-stage-2.flac", "audio.intro.title.stage-2", Capture.ONE_SHOT),
    # 0x12 plays from the tri-vid until the field handoff stops it (~35 s);
    # no restart is traced inside the window.
    Resource(0x12, "audio/intro/title-stage-3.flac", "audio.intro.title.stage-3",
             Capture.OBSERVATION, ONE_SHOT_TRACE_SECONDS),
    Resource(0x16, "audio/field/effect-016.flac", "audio.field.effect-016", Capture.ONE_SHOT),
    Resource(0x24, "audio/menu/menu-024.flac", "audio.menu.command-024", Capture.ONE_SHOT),
    Resource(0x29, "audio/menu/menu-029.flac", "audio.menu.command-029", Capture.ONE_SHOT),
    Resource(0x30, "audio/field/world-effect-030.flac", "audio.field.world-effect-030",
             Capture.OBSERVATION, PERSISTENT_SECONDS),
    *(_effect(i) for i in (20, 21, 23, 24, 28, 29, 30, 31, 32, 33, 34, 35, 37, 38, 39,
                           42, 43, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 61, 62,
                           63, 74, 75, 76, 77, 78, 80, 81, 91, 92, 93)),
    # Timer-A-paced DAC sample effects; silent before the YM timer model.
    *(_effect(i) for i in (25, 26, 27, 44, 45, 46, 47, 60, 64, 65, 66, 67, 68, 69,
                           70, 71, 72, 73, 79, 82, 83, 84, 85, 86, 87, 88, 89, 90)),
    # README "Source-cadenced intro effects": 0x5C every 2 VBlanks x60,
    # 0x5D every 4 VBlanks x19.
    Resource(0x5C, "audio/intro/sega-logo-shock-sequence.flac",
             "audio.intro.sega-logo-shock-sequence", Capture.SEQUENCE, 3.0, 60, 2),
    Resource(0x5D, "audio/intro/shadowrun-logo-reveal-sequence.flac",
             "audio.intro.shadowrun-logo-reveal-sequence", Capture.SEQUENCE, 3.0, 19, 4,
             max_duration_seconds=2.7),
)


def _samples(ticks: int) -> int:
    return round(ticks * extract.VGM_RATE / (extract.Z80_TICKS_PER_FRAME * 60))


def build(job: tuple[Resource, bytes, pathlib.Path, pathlib.Path]) -> dict:
    resource, rom, asset_root, vgm_dir = job
    machine = extract.GenesisSoundMachine(
        rom, resource.resource_id, (0x10,), resource.repeat_count,
        resource.repeat_interval_frames)
    writes = machine.run(resource.seconds)
    origin = machine.first_command_ticks if resource.capture is Capture.SEQUENCE else 0
    vgm = vgm_dir / f"{resource.alias}.vgm"
    extract.write_vgm(vgm, writes, machine.ticks, origin or 0)
    tail = None if resource.capture in (Capture.LOOPED_MUSIC, Capture.OBSERVATION) else ONE_SHOT_TAIL_SECONDS
    frames = renderer.render(vgm, asset_root / resource.path, None, tail, 32,
                             resource.max_duration_seconds)
    result = {"alias": resource.alias, "frames": frames,
              "dropped": machine.dropped_interrupts, "loop": None}
    if resource.capture is Capture.LOOPED_MUSIC:
        start = machine.loop_start_ticks
        restarts = next((points for points in machine.stream_restart_ticks.values()
                         if start in points), [])
        if start is None or not restarts:
            raise RuntimeError(f"{resource.alias}: fewer than two source restarts traced")
        # The driver can refetch the base marker a few cycles after a
        # restart; the loop end is the next restart a full minimum loop later.
        end = next((t for t in restarts if t - start >= extract.STREAM_LOOP_MIN_TICKS), None)
        if end is None:
            raise RuntimeError(f"{resource.alias}: no second source restart traced")
        if _samples(end) > frames:
            raise RuntimeError(f"{resource.alias}: loop end outside the rendered window")
        result["loop"] = (_samples(start), _samples(end), start, end)
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("rom", type=pathlib.Path)
    parser.add_argument("asset_root", type=pathlib.Path)
    parser.add_argument("vgm_dir", type=pathlib.Path)
    parser.add_argument("--jobs", type=int, default=multiprocessing.cpu_count())
    parser.add_argument("--only", help="comma-separated aliases to rebuild (loop table is then not rewritten)")
    args = parser.parse_args()
    rom = args.rom.read_bytes()
    selected = [r for r in RESOURCES if not args.only or r.alias in args.only.split(",")]
    # Longest traces first so the pool drains evenly.
    selected.sort(key=lambda r: -r.seconds)
    jobs = [(r, rom, args.asset_root, args.vgm_dir) for r in selected]
    with multiprocessing.Pool(args.jobs) as pool:
        results = {r["alias"]: r for r in pool.imap_unordered(build, jobs)}
    for resource in RESOURCES:
        if resource.alias in results:
            r = results[resource.alias]
            print(f"{r['alias']}: {r['frames']} frames, dropped VBlanks {r['dropped']}, loop {r['loop'] and r['loop'][:2]}")
    if args.only:
        return 0
    rows = ["alias\tloop_start_frame\tloop_end_frame\tsample_rate\tsource_start_ticks\tsource_end_ticks"]
    for resource in RESOURCES:
        loop = results[resource.alias]["loop"]
        if loop:
            rows.append(f"{resource.alias}\t{loop[0]}\t{loop[1]}\t{extract.VGM_RATE}\t{loop[2]}\t{loop[3]}")
    header, body = rows[0], sorted(rows[1:])
    (args.asset_root / "audio" / "loop-points.tsv").write_text("\n".join([header, *body]) + "\n")
    add_catalog_rows(args.asset_root / "resources.tsv")
    return 0


def add_catalog_rows(catalog: pathlib.Path) -> None:
    """Add a resources.tsv audio row for every rendered resource lacking one.

    graphics_extract preserves audio rows across image regeneration, so the
    audio pass owns them; existing rows (and their order) are left alone."""
    lines = catalog.read_text().splitlines()
    known = {line.split("\t", 1)[0] for line in lines}
    missing = [f"{r.alias}\t{r.path}\taudio\t0\t0" for r in
               sorted(RESOURCES, key=lambda r: r.resource_id) if r.alias not in known]
    if not missing:
        return
    last_audio = max(i for i, line in enumerate(lines) if "\taudio\t" in line)
    lines[last_audio + 1:last_audio + 1] = missing
    catalog.write_text("\n".join(lines) + "\n")


if __name__ == "__main__":
    sys.exit(main())
