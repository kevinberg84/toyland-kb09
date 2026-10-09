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
for folder in ('VST3', 'Standalone', 'AAX'):
    if (support / folder).exists(): shutil.rmtree(support / folder)
    shutil.copytree(build / folder, support / folder)
shutil.move(support/'Standalone/Toyland KB-09.exe', package/'Toyland KB-09 Standalone.exe')
shutil.rmtree(support/'Standalone')
shutil.copy2(root / 'docs/manual/User Manual.html', package / 'User Manual.html')
shutil.copy2(root / 'docs/manual/index.html', support / 'User Guide/index.html')
shutil.copytree(root / 'docs/manual/assets', support / 'User Guide/assets', dirs_exist_ok=True)
(support / 'START HERE.html').unlink()
shutil.copy2(root / 'tools/install_friends.ps1', support / 'Install.ps1')
# Keep the tucked-away source kit current with this packaging/manual revision.
kit = next((support / 'Development/Mac AU build kit').glob('*.zip'))
updates = {
    'KB-09-source/SH-09/.github/workflows/build-macos.yml': root / 'tools/build-macos.yml',
    'KB-09-source/SH-09/tools/install-mac.command': root / 'tools/install-mac.command',
    'KB-09-source/SH-09/docs/manual/index.html': root / 'docs/manual/index.html',
    'KB-09-source/SH-09/Source/GUI/PresetFiles.cpp': root / 'Source/GUI/PresetFiles.cpp',
    'KB-09-source/SH-09/Source/GUI/Editor.cpp': root / 'Source/GUI/Editor.cpp',
    'KB-09-source/SH-09/Source/GUI/PowerDetails.h': root / 'Source/GUI/PowerDetails.h',
    'KB-09-source/SH-09/Tests/GUIRender.cpp': root / 'Tests/GUIRender.cpp',
    'KB-09-source/SH-09/docs/manual/assets/panel-0.13.19.png': root / 'docs/manual/assets/panel-0.13.19.png',
}
# Ship the complete current source changes, including new headers and DSP tests.
for folder in ('Source','Tests','Assets/Icons'):
    for file in (root/folder).rglob('*'):
        if file.is_file(): updates['KB-09-source/SH-09/'+file.relative_to(root).as_posix()]=file
updates['KB-09-source/SH-09/CMakeLists.txt']=root/'CMakeLists.txt'
updates['KB-09-source/SH-09/docs/open-me-first-mac.html']=root/'docs/open-me-first-mac.html'
updates['KB-09-source/SH-09/docs/open-me-first.html']=root/'docs/open-me-first.html'
updates['KB-09-source/SH-09/docs/manual/assets/kb09-icon.png']=root/'docs/manual/assets/kb09-icon.png'
updates['KB-09-source/SH-09/docs/manual/User Manual.html']=root/'docs/manual/User Manual.html'

temporary = kit.with_suffix('.new.zip')
with zipfile.ZipFile(kit) as old, zipfile.ZipFile(temporary, 'w', zipfile.ZIP_DEFLATED) as new:
    for entry in old.infolist():
        if entry.filename.endswith(('User Manual.pdf', 'make_manual_pdf.py')): continue
        if entry.filename not in updates:
            data=old.read(entry.filename)
            entry.filename=entry.filename.replace('KB-09-source/SH-09/','KB-09-source/KB-09/')
            new.writestr(entry,data)
    for name, path in updates.items(): new.writestr(name.replace('KB-09-source/SH-09/','KB-09-source/KB-09/'), path.read_bytes())
temporary.replace(kit)
kit.rename(kit.with_name('KB-09-0.13.19-source.zip'))
for note in (support/'Development').rglob('Read me.txt'):
    text=note.read_text(encoding='utf-8').replace('SH-09/','KB-09/').replace('-S SH-09 ','-S KB-09 ').replace('.sh09state','.kb09')
    note.write_text(text,encoding='utf-8')
dev_aax = support / 'Development/AAX unsigned Windows'
for p in dev_aax.glob('*.aaxplugin'):
    shutil.rmtree(p)
shutil.copytree(build / 'AAX/Toyland KB-09.aaxplugin', dev_aax / 'Toyland KB-09.aaxplugin')
(support / 'Validation Summary.txt').write_text('KB-09 0.13.19 control and display fixes - 9 October 2026\nWindows VST3, standalone and unsigned AAX rebuilt.\nValidation results are recorded in the accompanying release record. Earlier Steinberg/actual-host evidence belongs to 0.13.2.\nMac AU/VST3 supplied separately as unverified test builds. AAX unsigned; Pro Tools untested.\n', encoding='utf-8')
(support / 'Release Notes.txt').write_text('KB-09 0.13.19: thicker molded bender rim with recessed thin web, clearer circumference seam and photo-traced asymmetric fracture; more fine dust in sheltered areas and the textured lower rail; short unbranched F crack and unbranched D crack. Cached whole-instrument previews keep AGE thumb movement independent of material rendering. Mac OPEN ME FIRST is now HTML. Version numbering and unsigned AAX limitations explained in the manual.\nKB-09 0.13.19 - 9 October 2026\nStandalone-only KEYBOARD mode plays computer keys A W S E D F T G Y H U J K, with Z/X octave changes and automatic note release on focus loss. VELOCITY, AFTERTOUCH and INPUT TRIM now support immediate track clicks. Standalone has a visible AUDIO / MIDI button that opens device settings directly. AGE supports immediate track clicks and humorous condition phrases. Standalone MIDI setup is documented. Slightly denser fine dust grains, without blurred haze, build gradually between knob collars, around PORTAMENTO and the bender, and along protected panel edges. TUNE follows the original horizontal rule. Shared KB-09 standalone, folder and manual-shortcut icons.\nOptional DESCRIPTION button (off by default); switch value boxes use readable dark text.\nLive AGE material previews blend while dragging; full detail returns on release. Cached material rendering improves manual resizing; native titlebar snapping preserves the previous size. Three distinct hairline key cracks grow with age. Wider glossy bender pointer with rounded corners, molded disc seam and irregular fracture extending into the disc. Bender sits below its screw row with the original arrowed scale and VCO/VCF connection lines. Uniform RANGE legend spacing, WAVEFORM shifted right with its detents and PWM link, eleven VOLUME ticks, reference-shaped VCF envelope glyphs and follower-to-ENV bracket. PWM double-triangle glyph and RATE lamp placement follow the original. The top-left brand opens the update menu: manual checks and optional 90-day background checks use a public version-only announcement and user-initiated Drive downloads. Switch readouts match bold knob readouts. DESCRIPTION lights when enabled and provides actual explanations.\nUseful audio-taper VOLUME; existing intermediate volume settings now sound quieter.\nModern FREE/SYNC with 21 straight, dotted and triplet RATE divisions.\nReliable RATE display: half-cycle flashes below 8Hz, steady increasing glow above.\nSHIFT enables fine dragging; temporary switch value boxes replace focus outlines.\nAdded IN audio meter; removed OPTIONAL; refined aged fader strips and black-key sheen.\nHeld keys depress/darken; only the active key has an amber bar.\nRevised TUNE/NOISE labels and restored the original PWM linking rule.\nWindows floating title bars can request L by dragging to the top, with pull-down restore; host placement restrictions may require the internal header.\nHTML-only User Manual includes all changes and GarageBand setup instructions.\nPlease report bugs and manual corrections to Kevin.\n', encoding='utf-8')
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
