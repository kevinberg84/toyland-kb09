"""Generate the shared KB-09 launcher and manual emblem from simple geometry."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
import shutil
r=Path(__file__).resolve().parents[1]
out=r/'Assets/Icons';out.mkdir(parents=True,exist_ok=True)
im=Image.new('RGBA',(1024,1024));d=ImageDraw.Draw(im)
d.rounded_rectangle((32,32,992,992),radius=184,fill='#171e21',outline='#bba16d',width=18)
d.rounded_rectangle((78,78,946,946),radius=143,outline='#414b4e',width=4)
title=ImageFont.truetype('C:/Windows/Fonts/arialbd.ttf',91)
number=ImageFont.truetype('C:/Windows/Fonts/arialbd.ttf',340)
d.text((133,118),'TOYLAND',font=title,fill='#dfbe7e')
d.text((115,225),'09',font=number,fill='#e9e5d6',stroke_width=1)
# A recessed rotary control, including the unmistakable indicator tick.
d.ellipse((647,330,855,538),fill='#aaa992',outline='#e0d9bc',width=9)
d.ellipse((664,347,838,521),fill='#30383b',outline='#111719',width=10)
d.line((751,367,751,416),fill='#edcd8a',width=14)
for i in range(10):
 x=126+i*77
 d.rounded_rectangle((x,630,x+70,865),radius=9,fill='#e4dfcf',outline='#858476',width=3)
for i in (0,1,3,4,5,7,8):
 x=126+(i+1)*77-22
 d.rounded_rectangle((x,630,x+43,776),radius=5,fill='#11191c',outline='#4b5355',width=3)
d.line((126,591,896,591),fill='#c4a46d',width=6)
im.resize((512,512),Image.Resampling.LANCZOS).save(out/'KB-09.png')
im.save(out/'KB-09.ico',sizes=[(16,16),(24,24),(32,32),(48,48),(64,64),(128,128),(256,256)])
shutil.copy2(out/'KB-09.png',r/'docs/manual/assets/kb09-icon.png')
