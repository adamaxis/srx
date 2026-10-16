import sys,re
from pathlib import Path
ROOT = Path(__file__).resolve().parents[2]
L=(ROOT / 'export/disassembly.s').read_text().split('\n')
s,e=int(sys.argv[1],16),int(sys.argv[2],16)
for l in L:
    m=re.match(r'^([0-9A-F]{6}) ',l)
    if m and s<=int(m.group(1),16)<e: print(l[:8]+l[29:])
