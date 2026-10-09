"""Create the simple friend-facing distribution from the validated release."""
from pathlib import Path
from hashlib import sha256
import json, shutil, zipfile

root = Path(__file__).resolve().parents[1]
release = root / 'artifacts/Release'
source = release / 'Toyland KB-09-0.13.2-Windows-x64'
package = release / 'Toyland KB-09 for Windows'
package.mkdir(exist_ok=False)
support = package / 'Support'
shutil.copytree(source, support)
for preset in (support/'Factory Presets').glob('*.sh09state'):
    preset.rename(preset.with_suffix('.kb09'))
for note in (support/'Factory Presets').glob('*.txt'):
    note.write_text(note.read_text(encoding='utf-8').replace('.sh09state','.kb09'),encoding='utf-8')
build = Path('C:/Dev/SH-09/build-kb09/SH09_artefacts/Release')
for folder in ('VST3', 'Standalone'):
    if (support / folder).exists(): shutil.rmtree(support / folder)
    shutil.copytree(build / folder, support / folder)
shutil.move(support/'Standalone/Toyland KB-09.exe', package/'Toyland KB-09 Standalone.exe')
shutil.rmtree(support/'Standalone')
shutil.copy2(root / 'docs/manual/User Manual.html', package / 'User Manual.html')
shutil.copy2(root / 'docs/manual/index.html', support / 'User Guide/index.html')
shutil.copytree(root / 'docs/manual/assets', support / 'User Guide/assets', dirs_exist_ok=True)
(support / 'START HERE.html').unlink()
shutil.copy2(root / 'tools/install_friends.ps1', support / 'Install.ps1')
# Friend distributions contain only usable formats and their required notices.
for folder in ('Development', 'AAX'):
    shutil.rmtree(support / folder, ignore_errors=True)
for name in ('AAX-SDK-LICENSE.txt', 'JUCE-9.0.2.spdx.json'):
    (support / 'Licenses' / name).unlink(missing_ok=True)
(support / 'Licenses/Read me.txt').write_text('This package uses JUCE 9.0.2 and the included VST3/AU notices. https://github.com/juce-framework/JUCE\n', encoding='utf-8')
(support / 'Validation Summary.txt').write_text('KB-09 0.13.23 panel and interaction refinement - 9 October 2026\nWindows VST3 and standalone rebuilt.\nValidation results are recorded in the accompanying release record. Earlier Steinberg/actual-host evidence belongs to 0.13.2.\nMac AU/VST3 supplied separately as unverified test builds.\n', encoding='utf-8')
(support / 'Release Notes.txt').write_text('KB-09 0.13.23 - 9 October 2026\nThe continuous upper panel now has a shallow reflection beneath its printed stripe, with all screws retained. Plastic side cheeks show fewer, varied scuffs and soft smears. Worn black keys retain some uneven reflection. POWER screws are evenly spaced around the button; C octave labels remain in Authentic and Modern modes. HOLD keeps its last sounding key visibly depressed. Header and footer use symmetric 25-unit outer margins. Plastic side scratches are halved, with varied widths, feathered edges and tapered ends. The lower textured rail is shallower with a soft fold reflection. The dedication has tighter spacing. The standalone configures its native title bar and final size before first display, then requests foreground once. The closing save prompt accepts arrow keys to cycle Save / Don\'t Save / Cancel, Enter to choose, and Esc to cancel.\nWindows standalone/VST3. Mac universal AU/VST3/standalone distributed separately and marked UNVERIFIED pending everyday host testing.\n', encoding='utf-8')
files = []
for folder in ('VST3', 'User Guide', 'Licenses', 'Factory Presets'):
    for path in sorted((support / folder).rglob('*')):
        if path.is_file():
            files.append({'path': path.relative_to(support).as_posix(), 'sha256': sha256(path.read_bytes()).hexdigest()})
files.append({'path':'../Toyland KB-09 Standalone.exe','sha256':sha256((package/'Toyland KB-09 Standalone.exe').read_bytes()).hexdigest()})
(support / 'install-files.json').write_text(json.dumps(files, indent=2), encoding='utf-8')
(package / 'INSTALL KB-09.cmd').write_text('@echo off\r\npowershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Support\\Install.ps1"\r\nif errorlevel 1 pause\r\n', encoding='ascii')
(package / 'OPEN ME FIRST!.html').write_text((root/'docs/open-me-first.html').read_text(encoding='utf-8'),encoding='utf-8')
# Recalculate technical checksums after updating the included guide.
checks = [f'{sha256(p.read_bytes()).hexdigest()}  {p.relative_to(support).as_posix()}' for p in sorted(support.rglob('*')) if p.is_file() and p.name != 'SHA256SUMS.txt']
(support / 'SHA256SUMS.txt').write_text('\n'.join(checks)+'\n', encoding='utf-8')
archive = release / (package.name + '.zip')
with zipfile.ZipFile(archive, 'w', zipfile.ZIP_DEFLATED) as z:
    for p in sorted(package.rglob('*')):
        if p.is_file(): z.write(p, p.relative_to(release))
with zipfile.ZipFile(archive) as z:
    assert z.testzip() is None
print(package)
print(archive)
print('ZIP SHA256:', sha256(archive.read_bytes()).hexdigest())
