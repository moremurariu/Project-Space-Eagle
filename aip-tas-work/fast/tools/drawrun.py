import sys, re
from PIL import Image, ImageDraw
grid=[l.rstrip('\n') for l in open('kogmap.txt')]
H=len(grid); W=len(grid[0]); S=4
img=Image.new('RGB',(W*S,H*S),(20,20,25))
d=ImageDraw.Draw(img)
col={'#':(90,90,100),'N':(60,60,60),'f':(40,90,160),'D':(20,40,120),'u':(200,200,60),'S':(0,200,0),'F':(200,0,0),'E':(200,100,200),'o':(120,60,0)}
for y,row in enumerate(grid):
    for x,c in enumerate(row):
        if c in col: d.rectangle([x*S,y*S,x*S+S-1,y*S+S-1],fill=col[c])
def speedcol(v):
    t=max(0,min(1,(v-10)/30))
    return (int(255*t),int(255*(1-abs(t-0.5)*2)),int(255*(1-t)))
for i,fn in enumerate(sys.argv[2:]):
    pts=[]
    for l in open(fn):
        m=re.match(r't (\d+) pos ([-\d.]+) ([-\d.]+) vel ([-\d.]+) ([-\d.]+)',l)
        if m: pts.append((int(m[1]),float(m[2]),float(m[3]),float(m[4]),float(m[5])))
    for a,b in zip(pts,pts[1:]):
        v=(b[3]**2+b[4]**2)**0.5
        d.line([a[1]*S/32,a[2]*S/32,b[1]*S/32,b[2]*S/32],fill=speedcol(v),width=2)
    for p in pts:
        if p[0]%50==0:
            d.text((p[1]*S/32+3,p[2]*S/32-10),str(p[0]-68),fill=(255,255,255))
img.save(sys.argv[1])
