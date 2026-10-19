"""Build the public GitHub Pages site without changing the app or its packages."""
from html.parser import HTMLParser
from pathlib import Path
from urllib.parse import urlsplit, unquote
import argparse
import re
import shutil

ROOT = Path(__file__).resolve().parents[1]


class References(HTMLParser):
    def __init__(self, source):
        super().__init__()
        self.references = []
        self.ids = set()
        self.feed(source)

    def handle_starttag(self, tag, attrs):
        attrs = dict(attrs)
        if 'id' in attrs:
            self.ids.add(attrs['id'])
        for key in ('href', 'src'):
            if key in attrs:
                self.references.append(attrs[key])


def validate(output):
    documents = {p.resolve(): References(p.read_text(encoding='utf-8-sig')) for p in output.rglob('*.html')}
    count = 0
    for path, document in documents.items():
        for reference in document.references:
            url = urlsplit(reference)
            if url.scheme or url.netloc:
                continue
            target = (path.parent / unquote(url.path)).resolve() if url.path else path
            if target.is_dir():
                target /= 'index.html'
            if not target.is_relative_to(output.resolve()) or not target.is_file():
                raise ValueError(f'Missing local reference in {path.name}: {reference}')
            if url.fragment and target in documents and unquote(url.fragment) not in documents[target].ids:
                raise ValueError(f'Missing anchor in {path.name}: {reference}')
            count += 1
    return count


def build(output):
    output = output.resolve()
    source = ROOT / 'website'
    if output == ROOT or output == source or output.is_relative_to(source):
        raise ValueError('Output must be a separate build directory')
    output.mkdir(parents=True, exist_ok=True)
    shutil.copytree(source, output, dirs_exist_ok=True)
    guide = ROOT / 'docs/manual'
    manual = output / 'manual'
    manual.mkdir(exist_ok=True)
    pending = [Path('index.html'), Path('styles.css'), Path('bug-report.js'), Path('bug-report-config.json')]
    copied = set()
    while pending:
        relative = pending.pop()
        src = (guide / relative).resolve()
        if src in copied:
            continue
        if not src.is_relative_to(guide.resolve()) or not src.is_file():
            raise ValueError(f'Missing manual resource: {relative}')
        copied.add(src)
        target = manual / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(src, target)
        if src.suffix == '.html':
            for reference in References(src.read_text(encoding='utf-8-sig')).references:
                url = urlsplit(reference)
                if not url.scheme and not url.netloc and url.path:
                    pending.append(relative.parent / unquote(url.path))
        elif src.suffix == '.css':
            for reference in re.findall(r'url\([\"\']?([^\)\"\']+)', src.read_text(encoding='utf-8-sig')):
                if not urlsplit(reference).scheme:
                    pending.append(relative.parent / reference)
    (output / '.nojekyll').write_text('', encoding='utf-8')
    checks = validate(output)
    print(f'Built {output}: {checks} local file/anchor references verified; {len(copied)} manual resources copied.')


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, default=ROOT / '_site')
    build(parser.parse_args().output)
