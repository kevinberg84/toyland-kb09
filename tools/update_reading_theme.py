"""Keep the platform opening pages on the same dark screen theme as the manual."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
style = '''<style id="kb09-dark-reading">
@media screen{html{color-scheme:dark}body{background:#171b1c;color:#e6e6df}
a{color:#83d7ce}a:hover{color:#b5f0e9}.box{background:#262c2d}
.security-first{border-color:#cbb572}code{background:#283032;padding:2px 4px}
a:focus-visible{outline:3px solid #83d7ce;outline-offset:3px}}
</style>'''
for filename in ['docs/open-me-first.html', 'docs/open-me-first-mac.html', 'docs/open-me-first-linux.html']:
    path = root / filename
    text = path.read_text(encoding='utf-8-sig')
    if 'id="kb09-dark-reading"' not in text:
        text = text.replace('</style>', '</style>\n' + style, 1)
    text = text.replace('send Kevin the exact message', 'send Kevin the exact <a href="User Manual.html#feedback">message</a>')
    text = text.replace('contact Kevin or your computer administrator', '<a href="User Manual.html#feedback">contact Kevin</a> or your computer administrator')
    path.write_text(text, encoding='utf-8')
    print(path)
