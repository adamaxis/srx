#!/usr/bin/env python3
"""Crop the retrogames player from screen-capture.webm and dump a storyboard."""
import os
import sys
import time

import cv2
import numpy as np

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
PATH = os.path.join(ROOT, "screen-capture.webm")
OUT = os.path.join(ROOT, "decompilation", "webm_ffmpeg_microframes")


def find_crop(img):
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    mask = (gray < 200).astype(np.uint8) * 255
    kernel = np.ones((15, 15), np.uint8)
    mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel)
    mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel)
    cnts, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    if not cnts:
        h, w = img.shape[:2]
        return 0, 0, w, h
    c = max(cnts, key=cv2.contourArea)
    return cv2.boundingRect(c)


def main():
    os.makedirs(OUT, exist_ok=True)
    for name in os.listdir(OUT):
        os.remove(os.path.join(OUT, name))

    cap = cv2.VideoCapture(PATH)
    if not cap.isOpened():
        print("FAILED open", PATH)
        return 1
    fps = cap.get(cv2.CAP_PROP_FPS) or 30.3
    print("opened fps", fps, flush=True)

    crop = None
    for i in range(120):
        ok, fr = cap.read()
        if not ok:
            break
        x, y, w, h = find_crop(fr)
        aspect = w / max(h, 1)
        if w > 400 and h > 300 and 1.1 < aspect < 2.0:
            crop = (x, y, w, h)
            print("crop from frame", i, crop, "aspect", round(aspect, 3), flush=True)
            break
    if crop is None:
        ok, fr = cap.read()
        h, w = fr.shape[:2] if ok else (2160, 3614)
        crop = (0, 0, int(w * 0.62), h)
        print("fallback crop", crop, flush=True)

    x, y, w, h = crop
    pad = 8
    x += pad
    y += pad
    w -= 2 * pad
    h -= 2 * pad

    cap.release()
    cap = cv2.VideoCapture(PATH)

    prev = None
    saved = 0
    scenes = []
    t0 = time.time()
    fi = 0
    while True:
        ok, fr = cap.read()
        if not ok:
            break
        game = fr[y : y + h, x : x + w]
        small = cv2.resize(game, (320, 224), interpolation=cv2.INTER_AREA)
        gray = cv2.cvtColor(small, cv2.COLOR_BGR2GRAY)
        hist = cv2.calcHist([gray], [0], None, [32], [0, 256]).flatten()
        hist = hist / (hist.sum() + 1e-6)
        scene = False
        d = 0.0
        if prev is not None:
            d = float(np.abs(hist - prev).sum())
            if d > 0.22:
                scene = True
        t = fi / fps
        if scene or fi % 8 == 0 or fi == 0:
            name = f"{fi:06d}_t{t:07.2f}.png"
            cv2.imwrite(os.path.join(OUT, name), small)
            saved += 1
            if scene:
                scenes.append((fi, t, d))
        prev = hist
        fi += 1
        if fi % 150 == 0:
            print(
                f"fi={fi} t={t:.1f} saved={saved} elapsed={time.time() - t0:.1f}",
                flush=True,
            )
        if fi > 20000:
            print("cap", flush=True)
            break
    cap.release()
    print("DONE frames", fi, "saved", saved, "scenes", len(scenes), "dur", fi / fps, flush=True)
    with open(os.path.join(OUT, "scenes.txt"), "w", encoding="utf-8") as handle:
        for i, t, d in scenes:
            handle.write(f"{i} t={t:.2f} d={d:.3f}\n")
            print(f"{i:6d} t={t:7.2f} d={d:.3f}", flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
