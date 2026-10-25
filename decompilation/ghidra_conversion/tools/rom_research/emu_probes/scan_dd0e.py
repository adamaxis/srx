import sys,numpy as np
# Requires the headless Genesis Plus GX wrapper at /workspace/shadowrun/emu/retro.py and rom.bin (out-of-repo research setup).
sys.path.insert(0,'/workspace/shadowrun/emu')
from retro import Emu
e=Emu('/workspace/shadowrun/rom.bin'); e.run()
def w(r,a): a&=0xFFFF; v=(int(r[a])<<8)|int(r[a+1]); return v-65536 if v>=32768 else v
for p in open('/tmp/states.txt').read().split():
    try: e.load_state(open(p,'rb').read())
    except Exception as x: print(p,'ERR',x); continue
    vals=set()
    for i in range(120):
        e.run(); r=e.ram()
        vals.add((w(r,0xDD08),w(r,0xDD0C),w(r,0xDD0A),w(r,0xDD0E),w(r,0xE00A)))
    diffs=sorted({v[3]-v[2] for v in vals}); hd=sorted({v[1]-v[0] for v in vals})
    scene=int(r[0xDF88&0xFFFF]) if False else w(r,0xDFE0)
    print(p.split('shadowrun/')[1], 'B-A_v',diffs[:6], 'B-A_h',hd[:6], 'sample',sorted(vals)[0])
