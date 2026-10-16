#!/usr/bin/env python3
"""Build timestamped overview sheets from the sectioned 30-fps longplay dump."""

from __future__ import annotations

import argparse
import math
import re
from bisect import bisect_right
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


FPS = 30
EARLY_SECTION_START_SECONDS = {
    1: 0,
    2: 56,
    3: 118,
    4: 238,
    5: 390,
    6: 510,
    7: 600,
}


def section_start_seconds(section_number: int) -> int:
    if section_number in EARLY_SECTION_START_SECONDS:
        return EARLY_SECTION_START_SECONDS[section_number]
    if section_number < 8:
        raise ValueError(f"Unknown section number: {section_number}")
    return 720 + (section_number - 8) * 300


def timestamp(seconds: float) -> str:
    total = int(seconds)
    hours, remainder = divmod(total, 3600)
    minutes, secs = divmod(remainder, 60)
    return f"{hours:02}:{minutes:02}:{secs:02}"


def load_font(size: int) -> ImageFont.ImageFont:
    try:
        return ImageFont.truetype("arial.ttf", size)
    except OSError:
        return ImageFont.load_default()


def build_sheets(
    section_roots: list[Path],
    output_root: Path,
    sample_seconds: int,
    columns: int,
    rows: int,
    thumb_width: int,
) -> int:
    output_root.mkdir(parents=True, exist_ok=True)
    tile_count = columns * rows
    label_height = 20
    thumb_height = thumb_width * 7 // 10
    tile_height = thumb_height + label_height
    font = load_font(12)
    sheet_count = 0

    section_paths = sorted(
        path
        for root in section_roots
        for path in root.iterdir()
        if path.is_dir() and re.match(r"section_(\d+)_", path.name)
    )
    for section_path in section_paths:
        match = re.match(r"section_(\d+)_", section_path.name)
        if match is None:
            continue
        section_number = int(match.group(1))
        first_frame = section_start_seconds(section_number) * FPS
        frame_paths = sorted(section_path.glob("frame_*.png"))
        if not frame_paths:
            continue

        frame_count = len(frame_paths)
        step = sample_seconds * FPS
        samples = list(range(0, frame_count, step))
        for page_start in range(0, len(samples), tile_count):
            page_samples = samples[page_start : page_start + tile_count]
            canvas = Image.new(
                "RGB",
                (columns * thumb_width, rows * tile_height),
                (15, 15, 15),
            )
            draw = ImageDraw.Draw(canvas)
            for tile_index, local_frame in enumerate(page_samples):
                image_path = section_path / f"frame_{local_frame:06d}.png"
                if not image_path.exists():
                    continue
                with Image.open(image_path) as source:
                    thumb = source.convert("RGB").resize(
                        (thumb_width, thumb_height), Image.Resampling.LANCZOS
                    )
                col = tile_index % columns
                row = tile_index // columns
                x = col * thumb_width
                y = row * tile_height
                canvas.paste(thumb, (x, y))
                source_seconds = first_frame / FPS + local_frame / FPS
                label = (
                    f"{timestamp(source_seconds)}  f{local_frame:06d}"
                )
                draw.text((x + 3, y + thumb_height + 2), label,
                          font=font, fill=(235, 235, 235))

            page_number = page_start // tile_count + 1
            output_path = output_root / (
                f"section_{section_number:02d}_page_{page_number:02d}.jpg"
            )
            canvas.save(output_path, quality=93, subsampling=0)
            sheet_count += 1
    return sheet_count


def build_window_sheets(
    section_roots: list[Path],
    output_root: Path,
    windows: list[tuple[float, float]],
    sample_fps: float,
    columns: int,
    rows: int,
    thumb_width: int,
) -> int:
    """Build full-screen sheets for selected source-video time windows."""
    output_root.mkdir(parents=True, exist_ok=True)
    tile_count = columns * rows
    label_height = 20
    thumb_height = thumb_width * 7 // 10
    tile_height = thumb_height + label_height
    font = load_font(12)
    sections: list[tuple[int, Path, int]] = []
    for root in section_roots:
        for section_path in root.iterdir():
            match = re.match(r"section_(\d+)_", section_path.name)
            if not section_path.is_dir() or match is None:
                continue
            number = int(match.group(1))
            start_frame = section_start_seconds(number) * FPS
            frame_count = sum(1 for _ in section_path.glob("frame_*.png"))
            if frame_count:
                sections.append((start_frame, section_path, frame_count))
    sections.sort(key=lambda entry: entry[0])
    section_starts = [entry[0] for entry in sections]
    if not sections:
        raise ValueError("No section frames found")

    sheet_count = 0
    for window_index, (start_seconds, end_seconds) in enumerate(windows, 1):
        samples: list[tuple[int, Path]] = []
        sample_count = math.ceil((end_seconds - start_seconds) * sample_fps)
        for sample_index in range(sample_count):
            source_seconds = start_seconds + sample_index / sample_fps
            if source_seconds >= end_seconds:
                break
            source_frame = int(round(source_seconds * FPS))
            section_index = bisect_right(section_starts, source_frame) - 1
            if section_index < 0:
                continue
            section_start, section_path, frame_count = sections[section_index]
            local_frame = source_frame - section_start
            if local_frame >= frame_count:
                continue
            frame_path = section_path / f"frame_{local_frame:06d}.png"
            if frame_path.exists():
                samples.append((source_frame, frame_path))

        for page_start in range(0, len(samples), tile_count):
            page_samples = samples[page_start : page_start + tile_count]
            canvas = Image.new(
                "RGB",
                (columns * thumb_width, rows * tile_height),
                (15, 15, 15),
            )
            draw = ImageDraw.Draw(canvas)
            for tile_index, (source_frame, image_path) in enumerate(page_samples):
                with Image.open(image_path) as source:
                    thumb = source.convert("RGB").resize(
                        (thumb_width, thumb_height), Image.Resampling.LANCZOS
                    )
                col = tile_index % columns
                row = tile_index // columns
                x = col * thumb_width
                y = row * tile_height
                canvas.paste(thumb, (x, y))
                label = (
                    f"{timestamp(source_frame / FPS)}  f{source_frame:06d}"
                )
                draw.text((x + 3, y + thumb_height + 2), label,
                          font=font, fill=(235, 235, 235))

            stem = (
                f"window_{timestamp(start_seconds).replace(':', '-')}_"
                f"{timestamp(end_seconds).replace(':', '-')}_page_"
                f"{page_start // tile_count + 1:02d}"
            )
            canvas.save(output_root / f"{stem}.jpg", quality=93, subsampling=0)
            sheet_count += 1
    return sheet_count


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--section-root",
        action="append",
        type=Path,
        required=True,
        help="A directory containing section_NN_* subdirectories; repeatable.",
    )
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--sample-seconds", type=int, default=5)
    parser.add_argument(
        "--window",
        action="append",
        default=[],
        metavar="START:END",
        help="Make a targeted source-time sheet for a seconds range; repeatable.",
    )
    parser.add_argument(
        "--sample-fps",
        type=float,
        default=1.0,
        help="Sampling rate used with --window (default: 1 fps).",
    )
    parser.add_argument("--columns", type=int, default=5)
    parser.add_argument("--rows", type=int, default=6)
    parser.add_argument("--thumb-width", type=int, default=180)
    args = parser.parse_args()
    if (args.sample_seconds <= 0 or args.columns <= 0 or args.rows <= 0 or
            args.sample_fps <= 0):
        parser.error("sample interval and sheet dimensions must be positive")
    if args.window:
        windows = []
        for value in args.window:
            try:
                start, end = (float(part) for part in value.split(":", 1))
            except ValueError:
                parser.error(f"invalid --window seconds range: {value}")
            if start < 0 or end <= start:
                parser.error(f"invalid --window seconds range: {value}")
            windows.append((start, end))
        count = build_window_sheets(
            args.section_root,
            args.output,
            windows,
            args.sample_fps,
            args.columns,
            args.rows,
            args.thumb_width,
        )
    else:
        count = build_sheets(
            args.section_root,
            args.output,
            args.sample_seconds,
            args.columns,
            args.rows,
            args.thumb_width,
        )
    print(f"Wrote {count} contact sheets to {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
