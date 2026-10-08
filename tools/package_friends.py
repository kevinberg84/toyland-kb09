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
shutil.copy2(root / 'docs/manual/index.html', support / 'User Guide/index.html')
(support / 'START HERE.html').unlink()
shutil.copy2(root / 'tools/install_friends.ps1', support / 'Install.ps1')
# Keep the tucked-away source kit current with this packaging/manual revision.
kit = next((support / 'Development/Mac AU build kit').glob('*.zip'))
updates = {
    'KB-09-source/SH-09/.github/workflows/build-macos.yml': root / 'tools/build-macos.yml',
    'KB-09-source/SH-09/tools/install-mac.command': root / 'tools/install-mac.command',
    'KB-09-source/SH-09/docs/manual/index.html': root / 'docs/manual/index.html',
}
temporary = kit.with_suffix('.new.zip')
with zipfile.ZipFile(kit) as old, zipfile.ZipFile(temporary, 'w', zipfile.ZIP_DEFLATED) as new:
    for entry in old.infolist():
        if entry.filename not in updates: new.writestr(entry, old.read(entry.filename))
    for name, path in updates.items(): new.writestr(name, path.read_bytes())
temporary.replace(kit)
files = []
for folder in ('VST3', 'Standalone', 'User Guide', 'Licenses', 'Factory Presets'):
    for path in sorted((support / folder).rglob('*')):
        if path.is_file():
            files.append({'path': path.relative_to(support).as_posix(), 'sha256': sha256(path.read_bytes()).hexdigest()})
(support / 'install-files.json').write_text(json.dumps(files, indent=2), encoding='utf-8')
(package / 'INSTALL KB-09.cmd').write_text('@echo off\r\npowershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Support\\Install.ps1"\r\nif errorlevel 1 pause\r\n', encoding='ascii')
(package / 'START HERE.html').write_text('''<!doctype html>
<html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<title>Start here — Toyland KB-09</title><style>body{background:#f4f0e7;color:#252620;font:19px/1.65 system-ui;margin:auto;padding:32px;max-width:800px}h1{line-height:1.2}li{margin:20px 0}a{color:#695019}strong{font-weight:750}.box{padding:20px;background:#e6dfd1;border-radius:10px}code{overflow-wrap:anywhere}</style>
<h1>Let's play KB-09.</h1><p>A recreation of Kevin's Roland SH-09. Windows 64-bit · Version 0.13.2.</p>
<div class="box"><strong>Received a ZIP file?</strong> Right-click it, choose <strong>Extract All…</strong>, then <strong>Extract</strong>. Open the newly extracted folder before installing.</div>
<ol><li><strong>Close your music software.</strong></li>
<li>Go back to the folder beside this page. Double-click <strong>INSTALL KB-09</strong> (the file ending in <strong>.cmd</strong>). If Windows asks whether to allow changes, choose <strong>Yes</strong>. In the installation window, press <strong>Enter</strong> when prompted.<br>The installer puts everything in the correct place. You do not need to copy or choose any files.</li>
<li><strong>Open your music software.</strong> Find <strong>Toyland KB-09</strong> in its instrument list and add it to an instrument/MIDI track. If it is missing, rescan VST3 plug-ins in your music software's settings, then reopen the software.</li></ol>
<h2>No music software? You can still play.</h2><p>After installing, open the Windows <strong>Start menu</strong>, type <strong>Play KB-09</strong>, and open it. In its audio settings, select your speakers or headphones. Click the piano keys to play, or select your connected MIDI keyboard.</p>
<p><a href="Support/User Guide/index.html">Open the illustrated user guide</a> · <a href="Support/User Guide/index.html#daws">Help with your music software</a></p>
<h2>Need help?</h2><p>Send Kevin any bugs or mistakes you find. Include what you were doing, your Windows and music software versions, and a screenshot or the exact error message.</p>
<p>If Windows reports a missing VCRUNTIME or MSVCP file, install the <a href="https://aka.ms/vc14/vc_redist.x64.exe">Microsoft Visual C++ runtime for Windows x64</a> and try again. If your computer blocks the installer, ask Kevin for help; do not disable your security software.</p>
<p>The <strong>Support</strong> folder holds the files the installer needs, the manual, licenses and advanced development material. Leave it together with the installer. You don't need to open it to install or play.</p>
<p>This download is for Windows VST3 and standalone use. AAX files inside Support are unsigned development files. No Mac AU is included in this Windows download. Future Mac AU builds will be labeled <strong>unverified</strong> until tested on a Mac.</p>
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
