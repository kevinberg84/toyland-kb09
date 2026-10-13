"""Finalize and verify the private Windows test package without external writes."""
from hashlib import sha256
import json
import sys
from pathlib import Path

root = Path(__file__).resolve().parents[1]
revision = int(sys.argv[1]) if len(sys.argv) > 1 else 1
folder = root / f'output/KB-09 Local DSP Test {revision}/Windows'
path = folder / 'Validation/test-manifest.json'
manifest = json.loads(path.read_text(encoding='utf-8'))
for relative, expected in manifest['files_sha256'].items():
    assert sha256((folder / relative).read_bytes()).hexdigest() == expected, relative
assert 'RESULT passed=82 failures=0' in (folder / 'Validation/local-test-presets-validation.log').read_text(encoding='utf-8-sig')
assert len(list((folder / 'Test Presets').glob('*.kb09'))) == 7
assert [p.name for p in folder.parent.iterdir()] == ['Windows']
manifest['validation']['packaged_presets'] = '82 assertions passed, including seven listening preset load/save/reload checks'
manifest['files_sha256'] = {
    p.relative_to(folder).as_posix(): sha256(p.read_bytes()).hexdigest()
    for p in sorted(folder.rglob('*')) if p.is_file() and p != path
}
path.write_text(json.dumps(manifest, indent=2), encoding='utf-8')
assert len(list(folder.rglob('*.exe'))) == 1
assert len(list((folder / 'VST3').glob('*.vst3'))) == 1
print(f'PASS: {len(manifest["files_sha256"])} package file hashes; Windows only; seven validated presets.')
