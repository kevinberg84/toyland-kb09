"""Assemble the validated 0.12 personal Windows test download, without installing."""
from hashlib import sha256
from pathlib import Path
import re
import shutil
import subprocess
import sys
import zipfile

root = Path(__file__).resolve().parents[1]
version = "0.12.0"
build = Path(r"C:\Dev\SH-09\build-kb09\SH09_artefacts\Release")
juce = Path(r"C:\Dev\JUCE")
bundle = build / "VST3/Toyland KB-09.vst3"
exe = build / "Standalone/Toyland KB-09.exe"
inner = Path("Contents/x86_64-win/Toyland KB-09.vst3")
expected_module = "e0605e732f9bb6e6d5cc1ab01633927f2de9ee7ecf7daedae1ed5d7e6d99ee86"
expected_exe = "d9cf9e9e558179c7a25deff7b1cc686ea76cfc345eb6ff26226c441856e3cfde"


def digest(path):
    return sha256(path.read_bytes()).hexdigest()


assert digest(bundle / inner) == expected_module, "Untested VST3 source"
assert digest(exe) == expected_exe, "Untested standalone source"
assert re.search(r'"Version"\s*:\s*"0\.12\.0"', (bundle / "Contents/Resources/moduleinfo.json").read_text())
bank = sorted((root / "artifacts/Presets-0.12-final").glob("*.sh09state"))
assert len(bank) == 16, "Incomplete factory bank"
release = root / "artifacts/Release"
package = release / f"Toyland KB-09-{version}-Windows-x64"
assert package.resolve().is_relative_to((root / "artifacts/Release").resolve())
assert not package.exists(), "Preserve the existing package; choose a fresh destination"
package.mkdir(parents=True)
shutil.copytree(bundle, package / "VST3/Toyland KB-09.vst3")
(package / "Standalone").mkdir()
shutil.copy2(exe, package / "Standalone/Toyland KB-09.exe")
guide = package / "User Guide"
for relative in ("index.html", "styles.css", "assets/marks.png", "assets/panel-0.12.png", "reference/evidence.html"):
    destination = guide / relative
    destination.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(root / "docs/manual" / relative, destination)
(package / "Factory Presets").mkdir()
for path in bank:
    shutil.copy2(path, package / "Factory Presets" / path.name)


def write(relative, text):
    destination = package / relative
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_text(text.strip() + "\n", encoding="utf-8")


write("START HERE.html", """
<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Welcome to Toyland KB-09</title>
<style>body{margin:0;background:#f4f0e7;color:#252620;font:18px/1.65 Arial,sans-serif}main{max-width:860px;margin:40px auto;padding:32px}header{background:#111311;color:#eee9db;padding:32px}svg{display:block;max-width:100%;height:auto}.toyland{width:190px;margin-bottom:30px}.model{width:620px}h1{font-size:1.2em;letter-spacing:.1em}h2{font-size:1.3em}a{color:#755319}header p{color:#d6cdb9}li{margin-bottom:16px}code{overflow-wrap:anywhere;background:#e5dfd1;padding:2px 4px}small{font-size:.7em}a:focus-visible{outline:3px solid #a27d35;outline-offset:3px}</style></head><body><main>
<header><svg class="toyland" viewBox="85 308 722 158" role="img" aria-label="Toyland"><image href="User Guide/assets/marks.png" width="2172" height="724" /></svg><svg class="model" viewBox="846 308 1253 158" role="img" aria-label="KEVINBERG 09 KB-09"><image href="User Guide/assets/marks.png" width="2172" height="724" /></svg><h1>WELCOME TO KB-09</h1><p>AGE — the sound of a life well played.</p><p><small>Windows x64 · 0.12.0 · Personal test build · 7 October 2026</small></p></header>
<h2>Choose how you want to play</h2><ol><li><strong>In your DAW:</strong> copy the entire <code>VST3/Toyland KB-09.vst3</code> bundle into <code>C:\\Program Files\\Common Files\\VST3\\</code>, then rescan VST3 instruments in your DAW. <a href="User Guide/index.html#install">Open the installation walkthrough</a>.</li><li><strong>Without a DAW:</strong> open <code>Standalone/Toyland KB-09.exe</code> in File Explorer. Select your audio output and MIDI keyboard in its settings. <a href="User Guide/index.html#start">Play your first sound</a>.</li></ol>
<p>Extract this complete ZIP before opening the guide or synth. The plug-in already contains its graphics and sixteen factory sounds. The optional <code>Factory Presets</code> folder contains copies you can load or keep for reference.</p>
<h2>Get comfortable</h2><p><a href="User Guide/index.html#age">Explore AGE</a> · <a href="User Guide/index.html#daws">Find your DAW</a> · <a href="User Guide/index.html#presets">Save your sounds</a> · <a href="User Guide/index.html#test-drive">Take the personal test drive</a></p>
<p><strong>Ableton Live:</strong> Kevin confirmed sharper rendering after disabling <strong>Auto-Scale Plug-in Window</strong>. <a href="User Guide/index.html#live">See the steps</a>. Use S/M/L inside KB-09 to choose a comfortable window size.</p>
<p>Keep the complete <code>User Guide</code> folder together. It works offline; vendor documentation links need internet. It is a working manual for this Windows build. AU for Mac and AAX for Pro Tools follow after VST3 owner testing.</p>
<p><a href="User Guide/index.html">Open the full user guide →</a> · <a href="Release Notes.txt">What changed?</a> · <a href="Validation Summary.txt">Build checks</a></p>
<p><small>UNOFFICIAL PERSONAL PROJECT — NOT AFFILIATED WITH ROLAND CORPORATION — NOT FOR SALE</small></p>
</main></body></html>
""")
write("Factory Presets/Read me.txt", """
These sixteen factory sounds are already available from the preset field in KB-09.
The .sh09state files are optional exported copies from the validated 0.12 VST3.
To load a copy, click LOAD in KB-09 and choose its file.
To save your own sound, click SAVE; new files use the .kb09 extension.
Loading a user sound does not remove the factory bank.
""")
write("Release Notes.txt", """
Toyland KB-09 0.12.0 — 7 October 2026

Photo-informed controls and AGE wear follow Kevin's personal SH-09 at AGE 7.
Compact opaque fader caps now have their stripe over the top and down the sides;
the front finger face stays black. Knob tops have black position ticks.
Raised ribbed switches, stepped metal slider posts and an arched bender replace
the earlier approximations. The owner's replacement Volume knob is excluded.

Dust, yellowing and dulling are gradual. Gray metallic screw heads replace red
rust stamps; exposed damaged panel paint shows brown corrosion. The ENV coffee
ring is retained. One key develops a very thin hairline crack at AGE 5.6, which
can lengthen a little. VCO mixer and RELEASE caps disappear at 6.4 and 8.8;
the bender tip breaks at 9.2. Controls keep working and AGE reverses the wear.

S joins M/L. The full header, panel and footer fit uniformly. The panel includes
the small unofficial/unaffiliated/not-for-sale notice. This download adds a
branded offline working guide, DAW display instructions and personal checklist.

Audio engine, 35 parameter identities and schema-1 presets remain compatible.
Modern MOD retains the deeper mapping introduced in 0.11, so older Modern
sounds with nonzero MOD may sound deeper; Authentic mappings are unchanged.
Keep old backups outside DAW scan folders and load one KB-09 identity.

This is the Windows personal test package. AU/macOS, AAX, platform installers
and a final paginated PDF follow later. Start with START HERE.html.
""")
write("Validation Summary.txt", f"""
Toyland KB-09 {version} — Release x64 — 7 October 2026
DSP 92 passed / 0 failed
Processor 20 passed / 0 failed, zero tracked C++ new allocations
Preset files 58 passed / 0 failed
Compiled editor 278 checks / 0 failures
Steinberg VST3 validator 47 passed / 0 failed, exit 0
GUI-enabled Pluginval strictness 10 SUCCESS, exit 0
Actual VST3 host 75 passed / 0 failed, 600-second finite/audible render

VST3 module SHA-256: {expected_module.upper()}
Standalone SHA-256: {expected_exe.upper()}

The complete guide includes the exact binary identity and practical limits.
Compiled 3840x2160 previews verify export dimensions, not a physical 4K monitor.
Kevin confirmed the Live Auto-Scale remedy; new-build Live Set recall and other
DAW walkthroughs still need owner testing. No measured hardware audio match or
AU/AAX compatibility is claimed. The audio engine is unchanged in this update.
""")
(package / "Licenses").mkdir()
shutil.copy2(juce / "LICENSE.md", package / "Licenses/JUCE-LICENSE.md")
shutil.copy2(juce / "JUCE.spdx.json", package / "Licenses/JUCE-9.0.2.spdx.json")
shutil.copy2(juce / "modules/juce_audio_processors_headless/format_types/VST3_SDK/LICENSE.txt", package / "Licenses/VST3-SDK-LICENSE.txt")
write("Licenses/Read me.txt", """
KB-09 uses JUCE 9.0.2 and the included Steinberg VST3 SDK.
Their supplied license texts and JUCE's component inventory accompany this
personal test package. JUCE source: https://github.com/juce-framework/JUCE
Project source and build instructions are retained in the development workspace.
Public distribution licensing, platform signing and installers remain part of
the later public-release preparation. No commercial JUCE license is asserted.
The supplied Toyland/KEVINBERG marks and personal reference photos belong to
the owner; selected runtime artwork combines generated materials and native UI.
""")
result = subprocess.run([sys.executable, str(root / "docs/manual/qa/check_manual.py"), str(guide)], check=True, capture_output=True, text=True)
print(result.stdout.strip())
assert digest(package / "VST3/Toyland KB-09.vst3" / inner) == expected_module
assert digest(package / "Standalone/Toyland KB-09.exe") == expected_exe
files = sorted(path for path in package.rglob("*") if path.is_file())
write("SHA256SUMS.txt", "\n".join(f"{digest(path)}  {path.relative_to(package).as_posix()}" for path in files))
archive = release / f"{package.name}.zip"
assert not archive.exists(), "Preserve the existing archive"
with zipfile.ZipFile(archive, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as output:
    for path in sorted(package.rglob("*")):
        if path.is_file():
            output.write(path, f"{package.name}/{path.relative_to(package).as_posix()}")
with zipfile.ZipFile(archive) as output:
    assert output.testzip() is None
    for path in package.rglob("*"):
        if path.is_file():
            archived = output.read(f"{package.name}/{path.relative_to(package).as_posix()}")
            assert sha256(archived).hexdigest() == digest(path), path
print(f"PASS: {len(files)+1} package files, bundle/EXE identity and every ZIP entry verified")
print(f"ZIP: {archive} ({archive.stat().st_size:,} bytes)")
print(f"ZIP SHA-256: {digest(archive).upper()}")
