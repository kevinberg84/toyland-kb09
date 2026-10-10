"""Build a portable QA gallery from the actual JUCE dialog renders."""
from pathlib import Path
import json
import base64
from PIL import Image, ImageDraw, ImageFont

root = Path(__file__).resolve().parents[1] / 'output' / 'dialog-theme-preview'
windows = [
 ('update-menu', 'Toyland / KB-09 menu', 'Exact panel logo; update controls.'),
 ('external-audio', 'External audio help', 'The information window from EXT AUDIO.'),
 ('audio-midi-settings', 'Audio / MIDI settings', 'Actual device selector. This offline preview has no audio device or MIDI controller open; your available devices will appear here.'),
 ('close-0', 'Close — Save selected', 'Save / Don’t Save / Cancel retain arrow-key navigation.'),
 ('close-1', 'Close — Don’t Save selected', 'Default selection.'),
 ('close-2', 'Close — Cancel selected', 'Third keyboard selection.'),
 ('update-consent', 'Automatic-update consent', 'First-run opt-in; no preferences are changed by this gallery.'),
 ('update-available', 'Update available', 'Example version 1.0.2 is preview data, not a release announcement.'),
 ('up-to-date', 'You’re up to date!', 'Current-version confirmation.'),
 ('update-error', 'Update check failed', 'Network/update-server failure.'),
 ('updates-unconfigured', 'Updates not configured', 'Fallback for builds without an announcement endpoint.'),
 ('instructions-windows', 'Windows update instructions', 'Download, copy instructions, and close controls.'),
 ('instructions-windows-copied', 'Windows — instructions copied', 'Copied! button feedback.'),
 ('instructions-mac', 'Mac update instructions', 'Mac-specific installer and standalone instructions; rendered on Windows for this styling review.'),
 ('instructions-mac-copied', 'Mac — instructions copied', 'Copied! button feedback.'),
 ('preset-menu', 'Factory preset menu', 'All 16 actual factory preset names.'),
 ('preset-replace', 'Replace preset?', 'Overwrite confirmation.'),
 ('preset-load-error', 'Preset could not be opened', 'Example error; the current patch stays intact.'),
 ('preset-invalid', 'Invalid preset', 'Example of validation-error wording in the same warning window.'),
 ('preset-save-error', 'Preset could not be saved', 'Example of a file-permission or storage failure.'),
 ('audio-device-error', 'Audio device could not be opened', 'Device-error wording depends on the operating system and driver.'),
]
for key, _, _ in windows:
    for age in range(3):
        assert (root / f'{key}-{age}.png').exists(), (key, age)

html = '''<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>KB-09 — Dialog styling review</title><style>
*{box-sizing:border-box}body{margin:0;background:#131313;color:#eee;font:16px system-ui,sans-serif}header{position:sticky;top:0;z-index:2;background:#131313f5;border-bottom:1px solid #444;padding:20px 28px}h1{font-size:24px;margin:0 0 8px}p{color:#bbb;line-height:1.5;margin:8px 0}nav{display:flex;gap:10px;flex-wrap:wrap;margin-top:14px}button,a{font:inherit}button{border:1px solid #777;border-radius:5px;background:#171717;color:#fff;padding:9px 16px;cursor:pointer}button[aria-pressed=true]{background:#eee;color:#111}main{padding:24px 28px;display:grid;grid-template-columns:repeat(auto-fit,minmax(min(100%,430px),1fr));gap:22px;align-items:start}.card{background:#1b1b1b;border:1px solid #444;border-radius:8px;padding:16px}.card h2{font-size:17px;margin:0 0 8px}.card p{font-size:14px;min-height:42px}.card img{display:block;max-width:100%;width:auto;max-height:620px;margin:14px auto;object-fit:contain;cursor:zoom-in}.card a{color:#ddd;font-size:14px}dialog{background:#121212;border:1px solid #888;color:#fff;max-width:96vw;max-height:96vh;padding:16px}dialog::backdrop{background:#000c}dialog img{display:block;max-width:90vw;max-height:78vh;object-fit:contain;margin:12px auto}dialog .bar{display:flex;justify-content:space-between;align-items:center;gap:16px}footer{padding:0 28px 30px;color:#aaa;font-size:14px}
</style><header><h1>KB-09 dialog styling review</h1>
<p>21 window/menu states × 3 ages. These are renders of the shared application UI, including otherwise inaccessible update and error messages.</p>
<p>Black background, geometric Toyland-style lettering, exact panel logo, and restrained ink wear. Native file pickers and driver control panels retain their operating-system styling.</p>
<nav aria-label="Condition / Age"><button data-age="0" aria-pressed="false">Clean · AGE 0</button><button data-age="1" aria-pressed="false">Half aged · AGE 0.5</button><button data-age="2" aria-pressed="true">Fully aged · AGE 1</button></nav></header><main id="cards"></main>
<footer>Uncommitted styling preview. Your installed 1.0.1 release, presets, preferences and shared downloads are unchanged. Click any window to enlarge it.</footer>
<dialog id="zoom"><div class="bar"><strong id="zoom-title"></strong><button id="dismiss">Close preview</button></div><img id="zoom-image" alt=""></dialog>
<script>const windows=__DATA__;let age=2;const cards=document.querySelector('#cards');const zoom=document.querySelector('#zoom');
function render(){cards.replaceChildren();windows.forEach(([id,title,note],i)=>{const card=document.createElement('article');card.className='card';const h=document.createElement('h2');h.textContent=String(i+1).padStart(2,'0')+' · '+title;const p=document.createElement('p');p.textContent=note;const img=document.createElement('img');img.src=id+'-'+age+'.png';img.alt=title;img.loading='lazy';img.onclick=()=>{document.querySelector('#zoom-title').textContent=title;const z=document.querySelector('#zoom-image');z.src=img.src;z.alt=title;zoom.showModal()};const a=document.createElement('a');a.href=img.src;a.target='_blank';a.textContent='Open full-size image';card.append(h,p,img,a);cards.append(card)})}
document.querySelectorAll('[data-age]').forEach(b=>b.onclick=()=>{age=Number(b.dataset.age);document.querySelectorAll('[data-age]').forEach(x=>x.setAttribute('aria-pressed',String(x===b)));render()});document.querySelector('#dismiss').onclick=()=>zoom.close();zoom.onclick=e=>{if(e.target===zoom)zoom.close()};render();</script></html>'''.replace('__DATA__',json.dumps(windows))
(root/'index.html').write_text(html,encoding='utf-8')
embedded = {f'{key}-{age}.png': 'data:image/png;base64,' + base64.b64encode((root/f'{key}-{age}.png').read_bytes()).decode('ascii')
            for key, _, _ in windows for age in range(3)}
offline = html.replace('let age=2;', 'const embedded=' + json.dumps(embedded) + ';let age=2;')
offline = offline.replace("img.src=id+'-'+age+'.png'", "img.src=embedded[id+'-'+age+'.png']")
(root/'KB-09 Dialog Review.html').write_text(offline,encoding='utf-8')

# Full inventory contact sheets; individual original images stay full size.
font=ImageFont.truetype('C:/Windows/Fonts/arial.ttf',18)
small=ImageFont.truetype('C:/Windows/Fonts/arial.ttf',14)
for age, label in [(0,'Clean'),(2,'Fully aged')]:
    width, cellw, cellh = 1600, 800, 600
    rows=(len(windows)+1)//2
    sheet=Image.new('RGB',(width,80+rows*cellh),(24,24,24));d=ImageDraw.Draw(sheet)
    d.text((24,24),f'KB-09 — {label} dialog inventory (click gallery images for full size)',font=font,fill='white')
    for i,(key,title,_) in enumerate(windows):
        x=(i%2)*cellw;y=80+(i//2)*cellh
        d.text((x+20,y+12),f'{i+1:02} · {title}',font=font,fill='white')
        im=Image.open(root/f'{key}-{age}.png').convert('RGB');im.thumbnail((760,535))
        sheet.paste(im,(x+(cellw-im.width)//2,y+47))
        d.line((x+16,y+cellh-1,x+cellw-16,y+cellh-1),fill=(70,70,70))
    sheet.save(root/f'overview-{age}.jpg',quality=92)

# Compact comparison for the chat preview.
compare=Image.new('RGB',(1300,710),(24,24,24));d=ImageDraw.Draw(compare)
for col,age in enumerate([0,2]):
    x=col*650;d.text((x+20,18),'Clean' if age==0 else 'Fully aged',font=font,fill='white')
    y=55
    for key,limit in [('update-menu',170),('external-audio',470)]:
        im=Image.open(root/f'{key}-{age}.png').convert('RGB');im.thumbnail((610,limit));compare.paste(im,(x+(650-im.width)//2,y));y+=im.height+18
compare.save(root/'comparison.png')

# Small, readable sheets for inline delivery: no browser/server is required.
groups = [
 ['audio-midi-settings'],
 ['close-0','close-1','close-2'],
 ['update-consent','update-available'],
 ['up-to-date','update-error','updates-unconfigured'],
 ['instructions-windows','instructions-windows-copied'],
 ['instructions-mac','instructions-mac-copied'],
 ['preset-menu','preset-replace','preset-load-error'],
 ['preset-invalid','preset-save-error','audio-device-error'],
]
titles = {key:title for key,title,_ in windows}
for number,keys in enumerate(groups,1):
    cols=2 if number in (5,6,7) else 1
    cellw=650 if cols==2 else 1000
    entries=[]
    for key in keys:
        im=Image.open(root/f'{key}-2.png').convert('RGB');im.thumbnail((cellw-36,1050))
        entries.append((key,im))
    rowheights=[max(im.height for _,im in entries[i:i+cols])+64 for i in range(0,len(entries),cols)]
    sheet=Image.new('RGB',(cellw*cols,sum(rowheights)+12),(24,24,24));d=ImageDraw.Draw(sheet)
    y=8
    for row,height in enumerate(rowheights):
        for col,(key,im) in enumerate(entries[row*cols:(row+1)*cols]):
            x=col*cellw;d.text((x+18,y+4),titles[key]+' · Fully aged',font=font,fill='white')
            sheet.paste(im,(x+(cellw-im.width)//2,y+36))
        y+=height
    sheet.save(root/f'review-sheet-{number}.png')
print(f'Gallery ready: {len(windows)*3} images, clean/aged inventory sheets, and comparison.')
