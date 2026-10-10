#!/usr/bin/env python3
"""Render an offline VGM capture to lossless stereo FLAC with libgme."""

from __future__ import annotations

import argparse
import ctypes
import ctypes.util
import os
import pathlib
import struct

import numpy as np
import soundfile as sf


SAMPLE_RATE = 44_100


def _load_gme(library_path: pathlib.Path | None):
    resolved = library_path
    if resolved is None:
        environment_path = os.environ.get("SHADOWRUN_LIBGME")
        if environment_path:
            resolved = pathlib.Path(environment_path)
        else:
            discovered = ctypes.util.find_library("gme")
            if discovered:
                resolved = pathlib.Path(discovered)
    if resolved is None:
        raise RuntimeError(
            "libgme was not found; pass --library or set SHADOWRUN_LIBGME")

    dll_directory = None
    if os.name == "nt" and resolved.is_file():
        dll_directory = os.add_dll_directory(str(resolved.resolve().parent))
        library = ctypes.WinDLL(str(resolved.resolve()))
    else:
        library = ctypes.CDLL(str(resolved))

    library.gme_open_file.argtypes = [
        ctypes.c_char_p,
        ctypes.POINTER(ctypes.c_void_p),
        ctypes.c_int,
    ]
    library.gme_open_file.restype = ctypes.c_char_p
    library.gme_start_track.argtypes = [ctypes.c_void_p, ctypes.c_int]
    library.gme_start_track.restype = ctypes.c_char_p
    library.gme_play.argtypes = [
        ctypes.c_void_p,
        ctypes.c_int,
        ctypes.POINTER(ctypes.c_short),
    ]
    library.gme_play.restype = ctypes.c_char_p
    library.gme_delete.argtypes = [ctypes.c_void_p]
    library.gme_delete.restype = None
    return library, dll_directory


def render(source: pathlib.Path, destination: pathlib.Path,
           library_path: pathlib.Path | None, tail_seconds: float | None,
           silence_threshold: int,
           max_duration_seconds: float | None) -> int:
    data = source.read_bytes()
    if len(data) < 0x40 or data[:4] != b"Vgm ":
        raise ValueError(f"not a supported VGM file: {source}")
    frame_count = struct.unpack_from("<I", data, 0x18)[0]
    if frame_count == 0:
        raise ValueError(f"VGM has no declared sample duration: {source}")
    if max_duration_seconds is not None:
        frame_count = min(
            frame_count, round(max_duration_seconds * SAMPLE_RATE))

    library, dll_directory = _load_gme(library_path)
    emulator = ctypes.c_void_p()
    error = library.gme_open_file(
        os.fsencode(source.resolve()), ctypes.byref(emulator), SAMPLE_RATE)
    if error:
        raise RuntimeError(error.decode("utf-8", errors="replace"))
    try:
        error = library.gme_start_track(emulator, 0)
        if error:
            raise RuntimeError(error.decode("utf-8", errors="replace"))
        interleaved = np.empty(frame_count * 2, dtype=np.int16)
        output = interleaved.ctypes.data_as(ctypes.POINTER(ctypes.c_short))
        error = library.gme_play(emulator, frame_count * 2, output)
        if error:
            raise RuntimeError(error.decode("utf-8", errors="replace"))
    finally:
        library.gme_delete(emulator)
        if dll_directory is not None:
            dll_directory.close()

    stereo = interleaved.reshape((-1, 2))
    if tail_seconds is not None:
        audible = np.flatnonzero(
            np.max(np.abs(stereo.astype(np.int32)), axis=1) > silence_threshold)
        if audible.size:
            keep_frames = min(
                frame_count,
                int(audible[-1]) + 1 + round(tail_seconds * SAMPLE_RATE),
            )
            stereo = stereo[:keep_frames]

    destination.parent.mkdir(parents=True, exist_ok=True)
    sf.write(destination, stereo, SAMPLE_RATE, format="FLAC", subtype="PCM_16")
    return len(stereo)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("vgm", type=pathlib.Path)
    parser.add_argument("flac", type=pathlib.Path)
    parser.add_argument(
        "--library", type=pathlib.Path,
        help="libgme shared library path; defaults to SHADOWRUN_LIBGME/find_library",
    )
    parser.add_argument(
        "--tail-seconds", type=float,
        help="trim digital silence after the final audible PCM frame, then retain this tail",
    )
    parser.add_argument(
        "--max-duration-seconds", type=float,
        help="stop the render at a source transition even if the VGM observation continues",
    )
    parser.add_argument("--silence-threshold", type=int, default=32)
    args = parser.parse_args()
    if args.tail_seconds is not None and args.tail_seconds < 0:
        parser.error("--tail-seconds must not be negative")
    if args.max_duration_seconds is not None and args.max_duration_seconds <= 0:
        parser.error("--max-duration-seconds must be positive")
    frames = render(args.vgm, args.flac, args.library, args.tail_seconds,
                    args.silence_threshold, args.max_duration_seconds)
    print(f"{args.flac}: {frames} stereo frames ({frames / SAMPLE_RATE:.3f}s)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
