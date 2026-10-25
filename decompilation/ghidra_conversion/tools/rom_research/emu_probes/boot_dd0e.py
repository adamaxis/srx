import sys,numpy as np
# Requires the headless Genesis Plus GX wrapper at /workspace/shadowrun/emu/retro.py and rom.bin (out-of-repo research setup).
sys.path.insert(0,'/workspace/shadowrun/emu')
from retro import Emu
from PIL import Image
e=Emu('/workspace/shadowrun/rom.bin')
def w(r,a): a&=0xFFFF; v=(int(r[a])<<8)|int(r[a+1]); return v-65536 if v>=32768 else v
out='/workspace/shadowrun/out/ffdd0e_capture'; prev=None; saved=0; log=open(out+'/trace.txt','w')
for f in range(9000):
    e.run(); r=e.ram()
    t=(w(r,0xDD08),w(r,0xDD0C),w(r,0xDD0A),w(r,0xDD0E),w(r,0xE00A))
    if t!=prev: log.write(f"{f} DD08={t[0]} DD0C={t[1]} DD0A={t[2]} DD0E={t[3]} FFE00A={t[4]}\n"); prev=t
    if t[3]!=t[2] and f%120==0 and saved<12:
        Image.fromarray(e.frame).save(f'{out}/f{f:05d}_B{t[3]}_A{t[2]}.png'); saved+=1
print('saved',saved)
