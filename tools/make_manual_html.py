from pathlib import Path
import base64, re
root=Path(__file__).resolve().parents[1]
guide=root/'docs/manual'
text=(guide/'index.html').read_text(encoding='utf-8-sig')
text=text.replace('<link rel="stylesheet" href="styles.css">','<style>'+(guide/'styles.css').read_text(encoding='utf-8-sig')+'</style>')
def embed(m):
    path=guide/m.group(2)
    mime={'.png':'image/png','.jpg':'image/jpeg','.svg':'image/svg+xml'}[path.suffix]
    return m.group(1)+'="data:'+mime+';base64,'+base64.b64encode(path.read_bytes()).decode()+'"'
text=re.sub(r'(src|href)="(assets/[^\"]+)"',embed,text)
out=guide/'User Manual.html'
out.write_text(text,encoding='utf-8')
print(out)
