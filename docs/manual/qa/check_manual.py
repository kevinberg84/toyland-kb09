"""Check the distributable guide's internal anchors and local dependencies."""
from collections import Counter
from html.parser import HTMLParser
from pathlib import Path
import sys
import re
from urllib.parse import unquote, urlsplit


class GuideParser(HTMLParser):
    def __init__(self):
        super().__init__()
        self.ids = []
        self.targets = []
        self.nav_depth=0;self.nav_entries=[];self.nav_href=None;self.nav_text=''
        self.section=None;self.heading_id=None;self.heading_text='';self.headings={}

    def handle_starttag(self, tag, attrs):
        attrs = dict(attrs)
        if tag=='nav':self.nav_depth+=1
        if tag=='section':self.section=attrs.get('id')
        if tag=='h2':self.heading_id=self.section;self.heading_text=''
        if self.nav_depth and tag=='a':self.nav_href=attrs.get('href');self.nav_text=''
        if "id" in attrs:
            self.ids.append(attrs["id"])
        for key in ("href", "src"):
            if key in attrs:
                self.targets.append((tag, attrs[key]))

    def handle_data(self,data):
        if self.heading_id:self.heading_text+=data
        if self.nav_href:self.nav_text+=data

    def handle_endtag(self,tag):
        if tag=='h2' and self.heading_id:
            self.headings[self.heading_id]=' '.join(self.heading_text.split());self.heading_id=None
        if tag=='a' and self.nav_href:
            if self.nav_href.startswith('#'):self.nav_entries.append((self.nav_href[1:],' '.join(self.nav_text.split())))
            self.nav_href=None
        if tag=='nav':self.nav_depth-=1


root = (Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parents[1]).resolve()
documents = {}
for path in sorted(root.rglob("*.html")):
    if "qa" in path.relative_to(root).parts:
        continue
    parser = GuideParser()
    parser.feed(path.read_text(encoding="utf-8-sig"))
    documents[path] = parser
errors = []
anchors = files = external = 0
for path, parser in documents.items():
    for target,label in parser.nav_entries:
        expected=re.sub(r'^\d+\s*·\s*','',label)
        actual=parser.headings.get(target)
        if actual!=expected:errors.append(f'Navigation/heading mismatch in {path.relative_to(root)}: {label!r} -> {actual!r}')
    for identifier, count in Counter(parser.ids).items():
        if count > 1:
            errors.append(f"Duplicate id in {path.name}: {identifier}")
    for tag, target in parser.targets:
        url = urlsplit(target)
        if url.scheme or url.netloc:
            external += 1
            continue
        local = (path.parent / unquote(url.path)).resolve() if url.path else path
        if not local.is_relative_to(root):
            errors.append(f"Dependency escapes standalone manual folder: {target}")
        elif not local.is_file():
            errors.append(f"Missing file from {path.name}: {target}")
        if url.path:
            files += 1
        if url.fragment:
            if local not in documents or unquote(url.fragment) not in documents[local].ids:
                errors.append(f"Missing anchor from {path.name}: {target}")
            anchors += 1
print(f"{len(documents)} HTML documents; {sum(len(p.ids) for p in documents.values())} IDs; {anchors} fragment links; {files} local references; {external} external citations")
for error in errors:
    print("FAIL:", error)
print("PASS: standalone guide dependencies and anchors" if not errors else "FAIL: guide checks")
raise SystemExit(bool(errors))
