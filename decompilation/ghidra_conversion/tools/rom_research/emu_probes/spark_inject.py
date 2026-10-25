# Inject a 0xB2B2-style hit spark (routine 0xAC4E) into a free FF0B00 slot and capture ROM-rendered frames.
# Requires the headless Genesis Plus GX wrapper at /workspace/shadowrun/emu/retro.py and rom.bin (out-of-repo research setup).
import sys,ctypes as C,numpy as np
sys.path.insert(0,'/workspace/shadowrun/emu')
from retro import Emu
from PIL import Image
state=sys.argv[1]; out=sys.argv[2]
import os; os.makedirs(out,exist_ok=True)
e=Emu('/workspace/shadowrun/rom.bin'); e.run()
raw=lambda: np.ctypeslib.as_array(C.cast(e.ram_ptr,C.POINTER(C.c_uint8)),shape=(e.ram_size,))
def wb(a,v): raw()[(a&0xFFFF)^1]=v&0xFF
def ww(a,v): wb(a,v>>8); wb(a+1,v)
def rw(a): r=e.ram(); return (int(r[a&0xFFFF])<<8)|int(r[(a+1)&0xFFFF])
st=open(state,'rb').read()
base=[];
e.load_state(st)
for i in range(40): e.run(); base.append(e.frame.copy())
e.load_state(st); e.run()
slot=None
for n in range(6):
    a=0xFF0B00+n*0x100
    if not (e.ram()[(a+0x10)&0xFFFF]&1): slot=a; idx=9+n; break
assert slot, 'no free slot'
lead=(rw(0xFFFC04)<<16|rw(0xFFFC06))&0xFFFFFF
lx,ly=rw(lead),rw(lead+4)
print('leader rec',hex(lead))
for k in range(0x100): wb(slot+k,0)
ww(slot+0x3E,idx); ww(slot+0x16,4); ww(slot+0xE,1); ww(slot+0x3A,0); ww(slot+0x3C,0xAC4E)
wb(slot+0x10,1); ww(slot,lx); ww(slot+4,ly+0x10); ww(slot+0x2C,0x2000); wb(slot+0x11,1)
print('slot',hex(slot),'idx',idx,'leader',lx,ly)
inj=[]
for i in range(39):
    e.run(); inj.append(e.frame.copy())
    alive=e.ram()[(slot+0x10)&0xFFFF]&1
    d=np.abs(inj[-1].astype(int)-base[i+1].astype(int)).sum(2)>0
    ys,xs=np.where(d)
    bb=(xs.min(),ys.min(),xs.max(),ys.max()) if len(xs) else None
    print(i,'alive',alive,'anim+16',hex(rw(slot+0x16)),'diffbbox',bb)
    Image.fromarray(inj[-1]).save(f'{out}/inj_{i:02d}.png'); Image.fromarray(base[i+1]).save(f'{out}/base_{i:02d}.png')
