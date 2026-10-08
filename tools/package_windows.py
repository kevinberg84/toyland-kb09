"""Assemble the verified personal release. Does not install or publish."""
from hashlib import sha256
from pathlib import Path
import json
import re
import shutil
import subprocess
import sys
import zipfile

root = Path(__file__).resolve().parents[1]
source = Path(r"C:\Dev\SH-09")
juce = Path(r"C:\Dev\JUCE")
manifest = json.loads((root / "artifacts/Validation-KB09/release-0.13.2.json").read_text(encoding="utf-8-sig"))
version = manifest["version"]
assert version == "0.13.2"
assert all(c["exit"] == 0 for c in manifest["checks"].values())
assert manifest["formats"]["aax"] == "unsigned development; Pro Tools untested"
assert manifest["formats"]["au"] == "not built; Mac required"
assert "Windows file versions" in manifest["checks"]
build = source / "build-kb09/SH09_artefacts/Release"
vst_inner = Path("Contents/x86_64-win/Toyland KB-09.vst3")
aax_inner = Path("Contents/x64/Toyland KB-09.aaxplugin")
inputs = {
    "vst3": build / "VST3/Toyland KB-09.vst3" / vst_inner,
    "standalone": build / "Standalone/Toyland KB-09.exe",
    "aax": build / "AAX/Toyland KB-09.aaxplugin" / aax_inner,
}


def digest(path):
    return sha256(path.read_bytes()).hexdigest()


for name, path in inputs.items():
    assert digest(path) == manifest["sha256"][name].lower(), f"Changed {name} binary"
assert re.search(r'"Version"\s*:\s*"0\.13\.2"',
                 (inputs["vst3"].parents[1] / "Resources/moduleinfo.json").read_text())
bank = sorted((root / f"artifacts/Presets-{version}-final").glob("*.sh09state"))
assert len(bank) == 16, "Incomplete tested factory bank"
release = root / "artifacts/Release"
package = release / f"Toyland KB-09-{version}-Windows-x64"
assert package.resolve().is_relative_to(release.resolve())
assert not package.exists(), "Preserve existing package; choose a fresh destination"
package.mkdir(parents=True)
shutil.copytree(build / "VST3/Toyland KB-09.vst3", package / "VST3/Toyland KB-09.vst3")
(package / "Standalone").mkdir()
shutil.copy2(inputs["standalone"], package / "Standalone/Toyland KB-09.exe")
shutil.copytree(build / "AAX/Toyland KB-09.aaxplugin",
                package / "Development/AAX unsigned Windows/Toyland KB-09.aaxplugin")
guide = package / "User Guide"
for relative in ("index.html", "styles.css", "assets/marks.png", f"assets/panel-{version}.png"):
    destination = guide / relative
    destination.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(root / "docs/manual" / relative, destination)
(package / "Factory Presets").mkdir()
for path in bank:
    shutil.copy2(path, package / "Factory Presets" / path.with_suffix('.kb09').name)


def write(relative, content):
    destination = package / relative
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_text(content.strip() + "\n", encoding="utf-8")


write("START HERE.html", r"""<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Welcome to Toyland KB-09</title>
<style>body{margin:0;background:#f4f0e7;color:#252620;font:18px/1.65 Arial,sans-serif}main{max-width:860px;margin:40px auto;padding:28px}header{background:#111311;color:#eee9db;padding:28px}svg{display:block;max-width:100%;height:auto}.toyland{width:190px;margin-bottom:30px}.model{width:620px}h1{font-size:1.2em;letter-spacing:.1em}h2{font-size:1.3em}a{color:#755319}header p{color:#d6cdb9}li{margin-bottom:16px}code{overflow-wrap:anywhere;background:#e5dfd1;padding:2px 4px}small{font-size:.75em}a:focus-visible{outline:3px solid #a27d35;outline-offset:3px}table{border-collapse:collapse;width:100%;font-size:.9em}td,th{text-align:left;vertical-align:top;padding:12px;border-bottom:1px solid #cfc6b5}@media(max-width:560px){main{margin:0;padding:16px}header{padding:20px}td,th{padding:8px;font-size:.85em}}</style></head><body><main>
<header><svg class="toyland" viewBox="85 308 722 158" role="img" aria-label="Toyland"><image href="User Guide/assets/marks.png" width="2172" height="724" /></svg><svg class="model" viewBox="846 308 1253 158" role="img" aria-label="KEVINBERG 09 KB-09"><image href="User Guide/assets/marks.png" width="2172" height="724" /></svg><h1>WELCOME TO KB-09</h1><p>A recreation of Kevin's Roland SH-09.</p><p><small>Windows x64 · 0.13.2 · Personal project · 7 October 2026</small></p></header>
<p>Explore a hands-on synth modeled after Kevin's own aged SH-09. Shape basses, leads and pulses, use <strong>AGE</strong> to change the panel and circuit model, or send external audio through the filter and let its envelope follower respond to the incoming level.</p>
<h2>Your formats</h2><table><thead><tr><th>Format</th><th>Status</th></tr></thead><tbody><tr><td>Windows VST3</td><td>Built and validated; ready to install</td></tr><tr><td>Windows standalone</td><td>Built; ready to open</td></tr><tr><td>Windows AAX Native</td><td>Unsigned development build only; Pro Tools loading untested</td></tr><tr><td>Mac AU</td><td>Not built; Mac/Xcode required. Source build kit included</td></tr></tbody></table>
<h2>Start playing</h2><ol><li><strong>In your DAW:</strong> copy the complete <code>VST3/Toyland KB-09.vst3</code> bundle to <code>C:\Program Files\Common Files\VST3\</code>, then rescan VST3 instruments. <a href="User Guide/index.html#install">Installation help</a>.</li><li><strong>Without a DAW:</strong> open <code>Standalone/Toyland KB-09.exe</code> and select your audio/MIDI devices. <a href="User Guide/index.html#start">Play your first sound</a>.</li></ol>
<p>Extract the full package before opening files. Graphics and sixteen factory sounds are built in. Optional exported presets are in <code>Factory Presets</code>.</p>
<p><strong>Ableton Live:</strong> Kevin confirmed sharper rendering after disabling <strong>Auto-Scale Plug-in Window</strong>. <a href="User Guide/index.html#live">See the steps</a>. Use S/M/L inside KB-09 for a comfortable size.</p>
<h2>Explore</h2><p><a href="User Guide/index.html#age">AGE</a> · <a href="User Guide/index.html#external">External filter and follower</a> · <a href="User Guide/index.html#daws">Your DAW</a> · <a href="User Guide/index.html#presets">Save sounds</a></p>
<h2>AU and AAX development files</h2><p>The <a href="Development/AAX unsigned Windows/Read me.txt">AAX development bundle</a> is unsigned. Normal Pro Tools requires AAX signing, which is unavailable here; it is not a ready-to-install Pro Tools release. No installable AU component is supplied. The <a href="Development/Mac AU build kit/Read me.txt">Mac AU build kit</a> contains the project, JUCE source and build instructions for a Mac.</p>
<p><a href="User Guide/index.html">Open the offline user guide →</a> · <a href="Release Notes.txt">Release notes</a></p><p>Keep the complete <code>User Guide</code> folder together. Vendor documentation links need internet.</p>
<p><small>UNOFFICIAL PERSONAL PROJECT — NOT AFFILIATED WITH ROLAND CORPORATION — NOT FOR SALE</small></p></main></body></html>""")
write("Factory Presets/Read me.txt", f"""The sixteen factory sounds are built into KB-09.
These optional .kb09 copies were exported and tested from the {version} VST3.
Click LOAD to open a copy. Click SAVE to save your own sound as a .kb09 file.
Loading a user sound does not remove the factory bank.""")
notes = (root / "docs/RELEASE_NOTES.md").read_text(encoding="utf-8-sig").split("# Historical", 1)[0]
write("Release Notes.txt", notes)
rows = [f"{name}: {check['summary']} (exit {check['exit']})" for name, check in manifest["checks"].items()]
write("Validation Summary.txt", f"""Toyland KB-09 {version} — Release x64 — 7 October 2026
{chr(10).join(rows)}

VST3 SHA-256: {manifest['sha256']['vst3']}
Standalone SHA-256: {manifest['sha256']['standalone']}
AAX SHA-256: {manifest['sha256']['aax']}

VST3 passed GUI-enabled Pluginval strictness 10 and actual-module tests.
AAX has only compilation, x64/export/dependency inspection; no Pro Tools load,
AAX validation or signing. AU is unbuilt because no Mac is available.
Editor exports do not establish physical 4K or fresh native-DAW testing.
The audio engine is unchanged; no universal aged-hardware distortion is claimed.""")
write("Development/AAX unsigned Windows/Read me.txt", """Toyland KB-09 0.13.2 — Windows x64 AAX Native DEVELOPMENT ONLY

This is a real compiled Windows AAX binary, using JUCE's bundled AAX SDK 2.9.
Its x64 headers, ACF entry points and imports were inspected. It is UNSIGNED
and has NOT been loaded or validated in Pro Tools. Normal Pro Tools requires
AAX digital signing. No signing tool/credentials or Pro Tools developer host
are available here. Do not install this as a normal Pro Tools release.

The adjacent Mac AU build kit folder contains source for future development.
Avid SDK guide: https://learn-cdn.avid.com/AAX_SDK_2p1p1/Documentation/Doxygen/output/html/a00274.html""")
write("Development/Mac AU build kit/Read me.txt", """Toyland KB-09 0.13.2 — AU SOURCE BUILD KIT, NOT AN AU BINARY

No installable .component is included. AU requires a native Mac build; this
Windows workstation has no Mac/Xcode host. Logic/GarageBand and auval are untested.

Extract KB-09-0.13.2-source.zip on your Mac. It contains:
  KB-09-source/SH-09/   project, embedded artwork, tests and docs
  KB-09-source/JUCE/    JUCE 9.0.2 source and bundled SDK/license files
  KB-09-source/BUILD-MAC.txt   native Mac build and validation commands

Install Xcode command-line tools and CMake, then follow BUILD-MAC.txt.
Supplying source is not a claim that AU or signed AAX is release-ready.""")

# Archive tracked project files, not arbitrary workstation contents/build caches.
git = ["git", "-c", f"safe.directory={source.as_posix()}"]
tracked = subprocess.check_output(git + ["ls-files", "-z"], cwd=source).decode().split("\0")
tracked = sorted(f for f in tracked if f and (source / f).is_file())
assert "docs/manual/assets/panel-0.13.2.png" in tracked, "Commit the final source snapshot first"
revision = subprocess.check_output(git + ["rev-parse", "HEAD"], cwd=source).decode().strip()
source_zip = package / "Development/Mac AU build kit/KB-09-0.13.2-source.zip"
mac_help = """KB-09 0.13.2 — native Mac build (not executed on Windows)

From KB-09-source in Terminal, after installing Xcode tools and CMake:
  cmake -S SH-09 -B build-mac -DKB09_BUILD_AU=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=\"arm64;x86_64\"
  cmake --build build-mac --config Release --target SH09_AU SH09_VST3 SH09_Standalone --parallel 4

AU output: build-mac/SH09_artefacts/Release/AU/Toyland KB-09.component
Copy the entire .component to ~/Library/Audio/Plug-Ins/Components/ for testing.
  auval -v aumu Sh09 PrIv
Then test MIDI, external routing, automation, state and editor lifecycle in
Logic Pro/GarageBand. Sign/notarize as needed for your distribution route.
No Mac build, universal-architecture or AU host pass is claimed in this kit.

AAX Native can be enabled on Mac/Windows with -DKB09_BUILD_AAX=ON. It requires
its own wrapper/host validation and AAX signing for normal Pro Tools loading.
Read SH-09/docs/BUILD.md and SH-09/docs/FORMAT_ROADMAP.md for details.
Preserve JUCE/LICENSE.md, JUCE/JUCE.spdx.json and bundled SDK notices.
No new project license is assigned by this source kit.
"""
source_hashes = []
with zipfile.ZipFile(source_zip, "w", zipfile.ZIP_DEFLATED, compresslevel=6) as output:
    for relative in tracked:
        path = source / relative
        output.write(path, "KB-09-source/SH-09/" + relative)
        source_hashes.append(f"{digest(path)}  SH-09/{relative}")
    for path in sorted(juce.rglob("*")):
        if not path.is_file():
            continue
        relative = path.relative_to(juce)
        if any(p in {".git", ".github", "__pycache__", "CMakeFiles"} for p in relative.parts):
            continue
        if relative.as_posix() in {"DemoRunner.exe", "Projucer.exe"}:
            continue
        output.write(path, "KB-09-source/JUCE/" + relative.as_posix())
        source_hashes.append(f"{digest(path)}  JUCE/{relative.as_posix()}")
    output.writestr("KB-09-source/BUILD-MAC.txt", mac_help)
    output.writestr("KB-09-source/SOURCE-SNAPSHOT.txt", f"Project Git snapshot: {revision}\nJUCE 9.0.2; included source notices and inventory apply.\n")
    output.writestr("KB-09-source/SHA256SUMS.txt", "\n".join(source_hashes) + "\n")
with zipfile.ZipFile(source_zip) as check:
    assert check.testzip() is None
    for line in source_hashes:
        expected, name = line.split("  ", 1)
        assert sha256(check.read("KB-09-source/" + name)).hexdigest() == expected
    for essential in ("SH-09/CMakeLists.txt", "SH-09/Source/Plugin/Processor.cpp",
                      "JUCE/CMakeLists.txt", "JUCE/extras/Build/CMake/JUCEUtils.cmake",
                      "JUCE/modules/juce_audio_plugin_client/AAX/SDK/LICENSE.txt"):
        assert "KB-09-source/" + essential in check.namelist(), essential
print(f"PASS: source kit {len(source_hashes)} files verified; snapshot {revision}")

licenses = {
    "JUCE-LICENSE.md": juce / "LICENSE.md",
    "JUCE-9.0.2.spdx.json": juce / "JUCE.spdx.json",
    "VST3-SDK-LICENSE.txt": juce / "modules/juce_audio_processors_headless/format_types/VST3_SDK/LICENSE.txt",
    "AAX-SDK-LICENSE.txt": juce / "modules/juce_audio_plugin_client/AAX/SDK/LICENSE.txt",
    "AU-SDK-LICENSE.txt": juce / "modules/juce_audio_plugin_client/AU/AudioUnitSDK/LICENSE.txt",
}
(package / "Licenses").mkdir()
for name, path in licenses.items():
    shutil.copy2(path, package / "Licenses" / name)
write("Licenses/Read me.txt", """This personal package uses JUCE 9.0.2 and its bundled SDKs.
Actual JUCE/VST3/AAX/AU notices and JUCE's inventory are included; the source
kit retains dependency source and notices. https://github.com/juce-framework/JUCE
No commercial JUCE license or new license for Kevin's project is asserted.
Supplied personal reference photos and marks belong to the owner.
No third-party commercial emulator source or copied plugin artwork is used.""")
for name, path in {
    "vst3": package / "VST3/Toyland KB-09.vst3" / vst_inner,
    "standalone": package / "Standalone/Toyland KB-09.exe",
    "aax": package / "Development/AAX unsigned Windows/Toyland KB-09.aaxplugin" / aax_inner,
}.items():
    assert digest(path) == manifest["sha256"][name].lower()
check = subprocess.run([sys.executable, str(root / "docs/manual/qa/check_manual.py"),
                        str(package)], check=True, capture_output=True, text=True)
print(check.stdout.strip())
files = sorted(p for p in package.rglob("*") if p.is_file())
write("SHA256SUMS.txt", "\n".join(f"{digest(p)}  {p.relative_to(package).as_posix()}" for p in files))
archive = release / f"{package.name}.zip"
assert not archive.exists(), "Preserve existing archive"
with zipfile.ZipFile(archive, "w", zipfile.ZIP_DEFLATED, compresslevel=6) as output:
    for path in sorted(package.rglob("*")):
        if path.is_file():
            output.write(path, f"{package.name}/{path.relative_to(package).as_posix()}")
with zipfile.ZipFile(archive) as check:
    assert check.testzip() is None
    for path in package.rglob("*"):
        if path.is_file():
            assert sha256(check.read(f"{package.name}/{path.relative_to(package).as_posix()}")).hexdigest() == digest(path)
print(f"PASS: {len(files) + 1} package files, binary identity and every ZIP entry verified")
print(f"FOLDER: {package}")
print(f"ZIP: {archive} ({archive.stat().st_size:,} bytes)")
print(f"ZIP SHA-256: {digest(archive).upper()}")
