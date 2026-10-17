#!/usr/bin/env python3
"""Set ROM sprite origins for the field player (idle/movement/attack).

The catalog stored only the transparent-crop offset; the ROM descriptor's
signed screen offset (+8/+0xA, consumed by 0x3EFE) was dropped, so the player
was drawn with its top-left at the world anchor. origin = descriptor offset +
crop offset. Descriptors come from the player record 0x7291A, the kind state
table 0x62AC, the idle state lists (record +0x1E index) and the movement
(0xC062) / attack (0xC042) selector tables.

usage (from decompilation/):
  python ghidra_conversion/tools/rom_research/graphics/field_player_sprite_origins.py \
      assets [--check] [--rom input/shadowrun_genesis_us.bin]
"""
import sys
from PIL import Image

ROM = sys.argv[sys.argv.index('--rom') + 1] if '--rom' in sys.argv else 'input/shadowrun_genesis_us.bin'
rom = open(ROM, 'rb').read()
PLAYER_RECORD = 0x7291A
KIND_STATE_TABLE = 0x62AC
MOVEMENT_SELECTORS = 0xC062
ATTACK_SELECTORS = 0xC042
DIRECTIONS = 8


def r16(o): return int.from_bytes(rom[o:o + 2], 'big')
def r32(o): return int.from_bytes(rom[o:o + 4], 'big')
def s16(o): v = r16(o); return v - 0x10000 if v & 0x8000 else v


def frame_geometry(desc):
    """0x8002 descriptor: +2 tile source, +8/+0xA signed offsets, +0xC grid."""
    if r16(desc) != 0x8002:
        raise ValueError(f'not an 0x8002 descriptor: {desc:#x}')
    src, dim = r32(desc + 2), r16(desc + 0xC)
    cols, rows = (dim >> 2) + 1, (dim & 3) + 1
    width, height = cols * 8, rows * 8
    pix = [[0] * width for _ in range(height)]
    i = 0
    for c in range(cols):            # 0x3EFE: column first, then row
        for rr in range(rows):
            for y in range(8):
                for xb in range(4):
                    b = rom[src + i]; i += 1
                    pix[rr * 8 + y][c * 8 + xb * 2] = b >> 4
                    pix[rr * 8 + y][c * 8 + xb * 2 + 1] = b & 15
    ys = [y for y in range(height) if any(pix[y])]
    xs = [x for x in range(width) if any(pix[y][x] for y in range(height))]
    return dict(dx=s16(desc + 8), dy=s16(desc + 0xA), left=min(xs), top=min(ys),
                w=max(xs) - min(xs) + 1, h=max(ys) - min(ys) + 1)


def state_table():
    return r32(KIND_STATE_TABLE + r16(PLAYER_RECORD + 0x0E) * 4)


def sequences(selectors):
    first, out = state_table(), []
    for d in range(DIRECTIONS):
        sl = r32(first + r16(selectors + d * 2))
        out.append([r32(sl + 4 + 4 * f) for f in range(r16(sl) & 0x7FFF)])
    return out


def idle_descriptors():
    first, index, out = state_table(), r16(PLAYER_RECORD + 0x1E), []
    for d in range(DIRECTIONS):
        sl = r32(first + d * 4)
        marker = r16(sl)
        if marker & 0x8000:
            break
        out.append(r32(sl + 4 + (0 if index >= marker else index) * 4))
    return out


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    if '--rom' in sys.argv:
        args.remove(ROM)
    if not args:
        raise SystemExit(__doc__)
    root = args[0].rstrip('/') + '/'
    rows = {f'field.entities.idle.{i}': d for i, d in enumerate(idle_descriptors())}
    for d, descs in enumerate(sequences(MOVEMENT_SELECTORS)):
        for f, desc in enumerate(descs):
            rows[f'field.entities.movement.{d}.{f}'] = desc
    for d, descs in enumerate(sequences(ATTACK_SELECTORS)):
        rows[f'field.entities.attack.{d}'] = descs[0]
    lines = open(root + 'resources.tsv').read().split('\n')
    changed = 0
    for n, line in enumerate(lines):
        cols = line.split('\t')
        if cols[0] not in rows:
            continue
        g = frame_geometry(rows[cols[0]])
        if Image.open(root + cols[1]).size != (g['w'], g['h']):
            raise SystemExit(f'size mismatch {cols[0]}')
        new = [str(g['dx'] + g['left']), str(g['dy'] + g['top'])]
        if cols[3:5] not in ([str(g['left']), str(g['top'])], new):
            raise SystemExit(f'unexpected existing origin {cols[0]} {cols[3:5]}')
        if cols[3:5] != new:
            cols[3:5] = new
            lines[n] = '\t'.join(cols)
            changed += 1
    print(f'rows {len(rows)} changed {changed}')
    if '--check' not in sys.argv:
        open(root + 'resources.tsv', 'w').write('\n'.join(lines))


if __name__ == '__main__':
    main()
