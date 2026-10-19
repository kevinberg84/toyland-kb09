from pathlib import Path
import base64, re
from manual_platforms import document, PLATFORMS
root=Path(__file__).resolve().parents[1]
guide=root/'docs/manual'
source=(guide/'index.html').read_text(encoding='utf-8-sig')

def embed(m):
    path=guide/m.group(2)
    mime={'.png':'image/png','.jpg':'image/jpeg','.svg':'image/svg+xml'}[path.suffix]
    return m.group(1)+'="data:'+mime+';base64,'+base64.b64encode(path.read_bytes()).decode()+'"'
for platform in PLATFORMS:
    text=document(source,platform)
    folder=guide/'platforms'/platform;folder.mkdir(parents=True,exist_ok=True)
    linked=re.sub(r'(src|href)="(assets/|styles\.css|bug-report\.js)',r'\1="../../\2',text)
    (folder/'index.html').write_text(linked,encoding='utf-8')
    text=text.replace('<link rel="stylesheet" href="styles.css">','<style>'+(guide/'styles.css').read_text(encoding='utf-8-sig')+'</style>')
    text=re.sub(r'(src|href)="(assets/[^\"]+)"',embed,text)
    out=folder/'User Manual.html';out.write_text(text,encoding='utf-8');print(out)
    # Compatibility path for older Windows packaging helpers. Other platforms
    # must choose their explicit edition; this is no longer an all-OS guide.
    if platform=='windows':(guide/'User Manual.html').write_text(text,encoding='utf-8')
