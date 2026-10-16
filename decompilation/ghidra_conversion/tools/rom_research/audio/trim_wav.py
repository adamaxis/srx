#!/usr/bin/env python3
"""Trim only the trailing digital silence from an extracted PCM WAV."""

from __future__ import annotations

import argparse
import pathlib
import wave


def trim(source: pathlib.Path, destination: pathlib.Path, threshold: int,
         tail_seconds: float) -> None:
    with wave.open(str(source), "rb") as reader:
        parameters = reader.getparams()
        frames = reader.readframes(reader.getnframes())
    width = parameters.sampwidth
    channels = parameters.nchannels
    if width != 2:
        raise ValueError(f"expected 16-bit PCM WAV: {source}")
    samples = memoryview(frames).cast("h")
    last_frame = 0
    for frame in range(parameters.nframes - 1, -1, -1):
        start = frame * channels
        if any(abs(samples[start + channel]) > threshold
               for channel in range(channels)):
            last_frame = frame + 1
            break
    tail_frames = round(tail_seconds * parameters.framerate)
    keep_frames = min(parameters.nframes, last_frame + tail_frames)
    destination.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(destination), "wb") as writer:
        writer.setparams(parameters._replace(nframes=keep_frames))
        writer.writeframes(frames[:keep_frames * channels * width])


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=pathlib.Path)
    parser.add_argument("destination", type=pathlib.Path)
    parser.add_argument("--threshold", type=int, default=32)
    parser.add_argument("--tail-seconds", type=float, default=0.12)
    args = parser.parse_args()
    trim(args.source, args.destination, args.threshold, args.tail_seconds)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
