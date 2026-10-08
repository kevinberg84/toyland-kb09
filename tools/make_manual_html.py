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
text=text.replace('Open <strong>User Manual.pdf</strong> in the main download folder to read or print this guide. An HTML copy is retained in Support.','This illustrated manual is a single file: <strong>User Manual.html</strong>. Its contents links, pictures and interactive sections work offline. Use Print / Save PDF for a printable copy, or open the included User Manual.pdf.')
out=guide/'User Manual.html'
out.write_text(text,encoding='utf-8')
print(out)
