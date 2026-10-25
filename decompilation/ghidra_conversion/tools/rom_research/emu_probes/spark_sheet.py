from PIL import Image, ImageDraw
# Requires the headless Genesis Plus GX wrapper at /workspace/shadowrun/emu/retro.py and rom.bin (out-of-repo research setup).
import glob
O='/workspace/shadowrun/out/hit_spark_compare'; A='/workspace/srx_core_wt/decompilation/assets/field/entities/effects'
Z=5; rows=[]
for st,(cx,cy) in [('hw1',(125,84)),('s5',(140,154))]:
    for kind in ['base','inj']:
        row=[Image.open(f'{O}/{st}/{kind}_{i:02d}.png').convert('RGB').crop((cx-16,cy-16,cx+16,cy+16)).resize((32*Z,32*Z),Image.NEAREST) for i in range(7)]
        rows.append((f'{st} {kind}',row))
bm=[Image.open(f'{A}/hit-spark-frame-{i}.bmp').convert('RGBA') for i in range(5)]
print([b.size for b in bm])
row=[]
for b in bm:
    c=Image.new('RGB',(32,32),(40,40,40)); c.paste(b,((32-b.width)//2,(32-b.height)//2),b); row.append(c.resize((32*Z,32*Z),Image.NEAREST))
rows.append(('port BMP 0-4',row))
W=110+7*(32*Z+4); H=len(rows)*(32*Z+4)+20
s=Image.new('RGB',(W,H),(0,0,0)); d=ImageDraw.Draw(s)
for i in range(7): d.text((110+i*(32*Z+4),4),f'f{i}',fill=(255,255,255))
for r,(lab,ims) in enumerate(rows):
    y=20+r*(32*Z+4); d.text((4,y+60),lab,fill=(255,255,255))
    for i,im in enumerate(ims): s.paste(im,(110+i*(32*Z+4),y))
s.save(O+'/contact_sheet.png')
