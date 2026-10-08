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
(support / 'Standalone/desktop.ini').write_text('[.ShellClassInfo]\nIconResource=Toyland KB-09.exe,0\n',encoding='utf-8')
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
    'KB-09-source/SH-09/docs/manual/assets/panel-0.13.10.png': root / 'docs/manual/assets/panel-0.13.10.png',
}
# Ship the complete current source changes, including new headers and DSP tests.
for folder in ('Source','Tests','Assets/Icons'):
    for file in (root/folder).rglob('*'):
        if file.is_file(): updates['KB-09-source/SH-09/'+file.relative_to(root).as_posix()]=file
updates['KB-09-source/SH-09/CMakeLists.txt']=root/'CMakeLists.txt'
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
kit.rename(kit.with_name('KB-09-0.13.10-source.zip'))
for note in (support/'Development').rglob('Read me.txt'):
    text=note.read_text(encoding='utf-8').replace('SH-09/','KB-09/').replace('-S SH-09 ','-S KB-09 ').replace('.sh09state','.kb09')
    note.write_text(text,encoding='utf-8')
dev_aax = support / 'Development/AAX unsigned Windows'
for p in dev_aax.glob('*.aaxplugin'):
    shutil.rmtree(p)
shutil.copytree(build / 'AAX/Toyland KB-09.aaxplugin', dev_aax / 'Toyland KB-09.aaxplugin')
(support / 'Validation Summary.txt').write_text('KB-09 0.13.10 control and display fixes � 8 October 2026\nWindows VST3, standalone and unsigned AAX rebuilt.\nValidation results are recorded in the accompanying release record. Earlier Steinberg/actual-host evidence belongs to 0.13.2.\nMac AU/VST3 supplied separately as unverified test builds. AAX unsigned; Pro Tools untested.\n', encoding='utf-8')
(support / 'Release Notes.txt').write_text('KB-09 0.13.10 � 8 October 2026\nAGE supports immediate track clicks and humorous condition phrases. Standalone MIDI setup is documented. Slightly denser fine dust grains, without blurred haze, build gradually between knob collars, around PORTAMENTO and the bender, and along protected panel edges. TUNE follows the original horizontal rule. Shared KB-09 standalone, folder and manual-shortcut icons.\nOptional DESCRIPTION button (off by default); switch value boxes use readable dark text.\nResponsive AGE drag with cached visual wear until release; larger utility sliders. Switch readouts match bold knob readouts. DESCRIPTION lights when enabled and provides actual explanations.\nUseful audio-taper VOLUME; existing intermediate volume settings now sound quieter.\nModern FREE/SYNC with 21 straight, dotted and triplet RATE divisions.\nReliable RATE display: half-cycle flashes below 8Hz, steady increasing glow above.\nSHIFT enables fine dragging; temporary switch value boxes replace focus outlines.\nAdded IN audio meter; removed OPTIONAL; refined aged fader strips and black-key sheen.\nHeld keys depress/darken; only the active key has an amber bar.\nRevised TUNE/NOISE labels and removed the PWM bracket.\nWindows floating title bars can request L by dragging to the top, with pull-down restore; host placement restrictions may require the internal header.\nHTML-only User Manual includes all changes and GarageBand setup instructions.\nPlease report bugs and manual corrections to Kevin.\n', encoding='utf-8')
files = []
for folder in ('VST3', 'Standalone', 'User Guide', 'Licenses', 'Factory Presets'):
    for path in sorted((support / folder).rglob('*')):
        if path.is_file():
            files.append({'path': path.relative_to(support).as_posix(), 'sha256': sha256(path.read_bytes()).hexdigest()})
(support / 'install-files.json').write_text(json.dumps(files, indent=2), encoding='utf-8')
(package / 'INSTALL KB-09.cmd').write_text('@echo off\r\npowershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Support\\Install.ps1"\r\nif errorlevel 1 pause\r\n', encoding='ascii')
(package / 'START HERE.html').write_text('''<!doctype html>
<html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<title>Start here � Toyland KB-09</title><style>body{background:#f4f0e7;color:#252620;font:19px/1.65 system-ui;margin:auto;padding:32px;max-width:800px}h1{line-height:1.2}li{margin:20px 0}a{color:#695019}strong{font-weight:750}.box{padding:20px;background:#e6dfd1;border-radius:10px}code{overflow-wrap:anywhere}</style>
<h1>Let's play KB-09.</h1><p>A recreation of Kevin's Roland SH-09. Windows 64-bit � Version 0.13.10.</p>
<div class="box"><strong>Received a ZIP file?</strong> Right-click it, choose <strong>Extract All�</strong>, then <strong>Extract</strong>. Open the newly extracted folder before installing.</div>
<ol><li><strong>Close your music software.</strong></li>
<li>Go back to the folder beside this page. Double-click <strong>INSTALL KB-09</strong> (the file ending in <strong>.cmd</strong>). If Windows asks whether to allow changes, choose <strong>Yes</strong>. In the installation window, press <strong>Enter</strong> when prompted.<br>The installer puts everything in the correct place. You do not need to copy or choose any files.</li>
<li><strong>Open your music software.</strong> Find <strong>Toyland KB-09</strong> in its instrument list and add it to an instrument/MIDI track. If it is missing, rescan VST3 plug-ins in your music software's settings, then reopen the software.</li></ol>
<h2>No music software? You can still play.</h2><p>After installing, open the Windows <strong>Start menu</strong>, type <strong>Play KB-09</strong>, and open it. In its audio settings, select your speakers or headphones. Click the piano keys to play, or select your connected MIDI keyboard.</p>
<p><a href="User Manual.html">Open the illustrated User Manual</a> � <a href="User Manual.html#daws">Help with your music software</a></p>
<h2>Need help?</h2><p>Send Kevin any bugs or mistakes you find. Include what you were doing, your Windows and music software versions, and a screenshot or the exact error message.</p>
<p>If Windows reports a missing VCRUNTIME or MSVCP file, install the <a href="https://aka.ms/vc14/vc_redist.x64.exe">Microsoft Visual C++ runtime for Windows x64</a> and try again. If your computer blocks the installer, ask Kevin for help; do not disable your security software.</p>
<p>The <strong>Support</strong> folder holds the files the installer needs, the manual, licenses and advanced development material. Leave it together with the installer. You don't need to open it to install or play.</p>
<p>This download is for Windows VST3 and standalone use. AAX files inside Support are unsigned development files. The separate Mac AU/VST3 test download is labeled <strong>unverified</strong>.</p>
<p>To share with a friend, send the complete <strong>Toyland KB-09 for Windows.zip</strong>, not just the installer file.</p></html>''', encoding='utf-8')
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
