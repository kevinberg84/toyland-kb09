"""Package a native Linux x86-64 build without DAW plug-ins."""
from pathlib import Path
import argparse
import hashlib
import json
import shutil
import struct
import zipfile

ROOT = Path(__file__).resolve().parents[1]
NAME = 'Toyland KB-09 for Linux (Standalone Only) - UNVERIFIED'

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--build', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    binary = args.build / 'SH09_artefacts/Release/Standalone/Toyland KB-09'
    header = binary.read_bytes()[:64]
    if header[:5] != b'\x7fELF\x02' or header[5] != 1 or struct.unpack_from('<H', header, 18)[0] != 62:
        raise ValueError('Expected a little-endian Linux ELF64 x86-64 executable')
    package = args.output / NAME
    package.mkdir(parents=True, exist_ok=True)
    shutil.copy2(binary, package / 'Toyland KB-09 Standalone')
    shutil.copy2(ROOT / 'docs/open-me-first-linux.html', package / 'OPEN ME FIRST!.html')
    shutil.copy2(ROOT / 'docs/manual/platforms/linux/User Manual.html', package / 'User Manual.html')
    support = package / 'Support'
    support.mkdir(exist_ok=True)
    shutil.copytree(args.build / 'factory-auditions/Factory Presets', support / 'Factory Presets', dirs_exist_ok=True)
    (support / 'Licenses').mkdir(exist_ok=True)
    shutil.copy2(ROOT / 'Assets/Fonts/OFL.txt', support / 'Licenses/OFL-Jost.txt')
    for name in ('Start KB-09.sh', 'Create desktop shortcut.sh'):
        (package / name).write_text((ROOT / 'tools/linux' / name).read_text(), encoding='utf-8', newline='\n')
    executable = {'Toyland KB-09 Standalone', 'Start KB-09.sh', 'Create desktop shortcut.sh'}
    manifest = {}
    with zipfile.ZipFile(args.output / f'{NAME}.zip', 'w', zipfile.ZIP_DEFLATED) as archive:
        for path in sorted(package.rglob('*')):
            if not path.is_file():
                continue
            relative = path.relative_to(package).as_posix()
            data = path.read_bytes()
            manifest[relative] = hashlib.sha256(data).hexdigest()
            info = zipfile.ZipInfo(f'{NAME}/{relative}')
            info.create_system = 3
            info.external_attr = (0o100755 if relative in executable else 0o100644) << 16
            info.compress_type = zipfile.ZIP_DEFLATED
            archive.writestr(info, data)
        archive.writestr(f'{NAME}/Support/SHA256.json', json.dumps(manifest, indent=2))
    print(args.output / f'{NAME}.zip')

if __name__ == '__main__':
    main()
