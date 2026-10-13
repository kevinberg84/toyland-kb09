"""Package a separate Windows-only listening build. Never installs or publishes."""
from pathlib import Path
from hashlib import sha256
import copy
import html
import json
import re
import shutil
import struct
import sys
import xml.etree.ElementTree as ET

root = Path(__file__).resolve().parents[1]
source = Path('C:/Dev/SH-09')
build = source / 'build-dsp-test'
revision = int(sys.argv[1]) if len(sys.argv) > 1 else 1
assert revision in (1, 2)
name = f'Toyland KB-09 DSP Test {revision}'
log_prefix = 'local-dsp-test' if revision == 1 else 'local-dsp-test2'
destination = root / f'output/KB-09 Local DSP Test {revision}/Windows'
assert not destination.exists(), 'Preserve existing tests; use a fresh destination.'
exe = build / f'SH09_artefacts/Release/Standalone/{name}.exe'
bundle = build / f'SH09_artefacts/Release/VST3/{name}.vst3'
assert exe.is_file() and bundle.is_dir()
for log, marker in [(f'{log_prefix}-ctest.log', '100% tests passed'),
                    (f'{log_prefix}-host.log', 'PASS 0 failures,')]:
    assert marker in (source / 'artifacts' / log).read_text(encoding='utf-8-sig')
host_checks = int(re.search(r'PASS 0 failures, (\d+) assertions passed',
                           (source / 'artifacts' / f'{log_prefix}-host.log').read_text(encoding='utf-8-sig'))[1])
assert host_checks >= (77 if revision == 2 else 73)
destination.mkdir(parents=True)
shutil.copy2(exe, destination / exe.name)
shutil.copytree(bundle, destination / 'VST3' / bundle.name)
presets = destination / 'Test Presets'
presets.mkdir()

# Keep all parameter IDs and the exact JUCE XML binary envelope of a processor export.
raw = (build / 'factory-auditions/Factory Presets/01 - Calibrated Init.sh09state').read_bytes()
magic, length = struct.unpack('<II', raw[:8])
assert magic == 0x21324356 and len(raw) == length + 9
base = ET.fromstring(raw[8:8+length])
assert base.tag == 'SH09State' and len(base) == 35
common = dict(tune=0, range=2, wave=1, vcoMod=0, subLevel=0, extLevel=0,
              lfoRate=0, lfoDelay=0, lfoWave=0, lfoSync=0, modern=0, age=0,
              cutoff=.58, resonance=1, vcfMod=.45, vcfEnv=0, vcfEnvMode=0,
              vcaMode=1, attack=0, decay=0, sustain=.7, release=.2,
              envMode=0, portamento=0, volume=.5, velocity=0, aftertouch=0)
patches = [
    ('01 - Saw sweep - VCO 10', dict(wave=1, vcoLevel=1)),
    ('02 - Saw sweep - VCO 5', dict(wave=1, vcoLevel=.5)),
    ('03 - Filter alone - VCO 0', dict(wave=1, vcoLevel=0)),
    ('04 - Square sweep - VCO 10', dict(wave=2, vcoLevel=1)),
    ('05 - Square sweep - VCO 5', dict(wave=2, vcoLevel=.5)),
    ('06 - Square sweep - VCO 2', dict(wave=2, vcoLevel=.2)),
    ('07 - Playing latency', dict(wave=1, vcoLevel=.7, cutoff=.8, resonance=0,
                                  vcfMod=0, vcaMode=2, sustain=1, release=0)),
]
patch_manifest = []
for title, overrides in patches:
    state = copy.deepcopy(base)
    settings = common | overrides
    by_id = {p.attrib['id']: p for p in state}
    assert set(settings) <= by_id.keys()
    for param, value in settings.items():
        by_id[param].set('value', str(value))
    xml = ET.tostring(state, encoding='utf-8', xml_declaration=True)
    path = presets / (title + '.kb09')
    path.write_bytes(struct.pack('<II', magic, len(xml)) + xml + b'\0')
    patch_manifest.append({'file': path.name, 'values': settings})

manual = (root / 'docs/manual/User Manual.html').read_text(encoding='utf-8')
manual = manual.replace('Version 1.5.2 · 12 October 2026', '1.5.3 DSP Test 1 · 13 October 2026')
manual = manual.replace('value="1.5.2"', 'value="1.5.3 DSP Test 1"')
manual = re.sub(r'<aside class="notice"><strong>KB-09 1\.5\.2\.</strong>.*?</aside>',
    '<aside class="notice"><strong>Windows local DSP test only.</strong> '
    'Use <a href="OPEN ME FIRST!.html">OPEN ME FIRST!</a> for this package. '
    'The general Mac, Linux and public-installer instructions below do not apply to this test folder. '
    'No installer is needed for the standalone.</aside>', manual, count=1, flags=re.S)
manual = re.sub(r'<section id="updates">.*?</section>',
    '<section id="updates"><h2>Local test identification</h2><p>'
    'This copy is Toyland KB-09 DSP Test 1, version 1.5.3. Public update checks are disabled. '
    'The standalone keeps separate audio/MIDI and startup preferences. '
    'The VST3 has its own identity and does not replace the regular instrument. '
    'Use a separate test project for this plug-in; a project saved with the test identity '
    'will still require that identity when reopened.</p></section>', manual, count=1, flags=re.S)
manual = manual.replace('DSP Test 1', f'DSP Test {revision}')
(destination / 'User Manual.html').write_text(manual, encoding='utf-8')

guide = '''<!doctype html><html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1"><title>KB-09 · Local DSP Test 1</title>
<style>body{background:#171b1c;color:#e6e6df;font:17px/1.6 system-ui,sans-serif;margin:0}
main{max-width:900px;margin:auto;padding:35px 24px 70px}h1,h2{line-height:1.25}h1{font-size:2.1rem}
a{color:#83d7ce}strong{color:#fff}section{margin:35px 0}table{border-collapse:collapse;width:100%}
td,th{padding:10px;text-align:left;border-bottom:1px solid #444}code{background:#303638;padding:2px 5px}
.notice{border-left:4px solid #cbb572;background:#262c2d;padding:16px}li{margin:10px 0}
@media print{body{background:white;color:black}strong{color:black}main{padding:0}a{color:#155b63}}
</style></head><body><main>
<p>WINDOWS x64 · 1.5.3 DSP TEST 1 · 13 OCTOBER 2026</p>
<h1>Listen. Play. Push the filter.</h1>
<p>A private local test of the revised analog filter model, with the approved GUI changes and key crack.</p>
<p class="notice"><strong>Open Toyland KB-09 DSP Test 1.exe in this folder.</strong>
This test has a separate name and plug-in identity. The installed release is untouched.
No files have been published to GitHub or Google Drive. Automatic update checks are disabled.</p>
<section><h2>Start here</h2><ol>
<li>Open the EXE directly in File Explorer. Click <strong>AUDIO / MIDI</strong>, select your normal audio output and enable your MIDI keyboard.
The test's audio/MIDI preferences are separate, so select these again on first launch.</li>
<li>Start at <strong>48 kHz / 256 samples</strong>. Keep listening volume modest when testing self-oscillation.
Close the other standalone if it occupies the audio device.</li>
<li>Click <strong>LOAD</strong> and open a <code>.kb09</code> file from <strong>Test Presets</strong>.
These presets are starting points; saving edits under new names keeps the supplied settings available.</li>
<li>For DAW testing, use a separate test project. In your DAW's VST3 settings, scan this folder's <strong>VST3</strong> directory if custom VST3 folders are supported.
Otherwise, copy the entire <strong>Toyland KB-09 DSP Test 1.vst3</strong> bundle to your usual supported VST3 location and rescan.
Keep the bundle intact, including Contents; do not copy just its inner binary.
Look for <strong>Toyland KB-09 DSP Test 1</strong>. The test does not replace Toyland KB-09.</li>
</ol><p><a href="User Manual.html">Open the updated manual</a> · includes the filter-modeling story and SH-09 / KB-09 waveform comparisons.</p></section>
<section><h2>1. Revisit the slow resonant sweep</h2>
<p>Load <strong>01 - Saw sweep - VCO 10</strong>. Hold <strong>F3, MIDI note 53</strong> for about 20 seconds.
DAWs label octaves differently; MIDI note 53 identifies the intended note. The patch uses AUTHENTIC, Minty, sine LFO,
RATE 0, VCO MOD 0, CUTOFF 5.8, RESONANCE 10, VCF MOD 4.5, VCF ENV 0, SUB/EXT 0 and RANGE 8′.
The VCA envelope sustains steadily, so it should not create rhythmic level changes.</p>
<p>Repeat with <strong>02 - Saw sweep - VCO 5</strong> and <strong>03 - Filter alone - VCO 0</strong>.
Listen for deep volume collapses, abrupt pitch jumps, and whether the sweep feels continuous.
Some beating and interaction are expected when the VCO is present. We are checking excessive behavior, not trying to eliminate every ripple.</p>
</section><section><h2>2. Push the square-wave interaction</h2>
<p>Use <strong>04, 05 and 06</strong> to repeat the same sweep with square-wave VCO levels of 10, 5 and 2.
This waveform produced stronger residual interaction in our tests. Listen especially for repeated jump–hold–jump behavior,
harsh buzzing or large level dips. Compare with the VCO 0 patch, and try the same note one octave above and below.</p>
<p>Next, select variable pulse and try several manual pulse widths, then try PWM. Leave VCO MOD at 0 for the first comparison;
add pitch modulation only afterward. Check whether any problem follows the waveform, mixer level, or filter resonance.
If a setting sounds wrong, record 20 seconds and save its preset. Include the MIDI note and sample rate.</p>
</section><section><h2>3. Check playing latency and headroom</h2>
<p>Load <strong>07 - Playing latency</strong>. It removes envelope attack, portamento and heavy resonance.
Play short repeated notes on your MIDI keyboard. With the same audio device and driver, try 256, then 128, then 64 samples if stable.
Compare with the released instrument at the same sample rate, buffer, patch and DAW routing. Bypass other effects, particularly look-ahead or linear-phase processors.</p>
<p>The instrument still reports <strong>128 samples</strong> of internal processing latency: <strong>2.67 ms at 48 kHz</strong>.
This is not the total key-to-speaker delay; device buffers, drivers, MIDI and the host add their own delay.
The revised filter costs more CPU, so your smallest reliable buffer still needs testing.</p>
<p>Repeat with the resonant square patch while moving the bender, mod wheel and cutoff. Listen for crackles or dropouts and watch control responsiveness.
Try the same with the editor closed in your DAW. If audio breaks up, note the buffer and try the next larger size.
Finally, optionally try 96 kHz and a realistic DAW project load.</p>
</section><section><h2>4. Make sure ordinary sounds still work</h2>
<p>Play your favourite basses and leads; check low and high notes, envelope attacks/releases, sub oscillator, PWM,
legato and portamento. Compare Minty and Fire Hazard. In MODERN, try velocity and aftertouch.
Save and reload a patch, and reopen the test DAW project. The test VST3's separate identity is required to reopen that project.</p>
</section><section><h2>Keep these notes</h2>
<table><tr><th>Setup</th><th>Your result</th></tr>
<tr><td>Standalone / DAW and version</td><td>________________________</td></tr>
<tr><td>Audio interface and driver</td><td>________________________</td></tr>
<tr><td>Sample rate / buffer</td><td>________________________</td></tr>
<tr><td>Smallest stable buffer / playing feel</td><td>________________________</td></tr>
<tr><td>Patch / MIDI note / problem and timestamp</td><td>________________________</td></tr></table>
<p>Record without compression, limiting or normalization for useful amplitude comparisons.
When comparing the SH-09, match audible pitches and relative VCO levels; identical slider positions need not match on worn hardware.</p>
<p>This test is unsigned. If Windows blocks it, use the Windows security guidance in the manual.
If Windows reports missing VCRUNTIME/MSVCP files, the manual links the official Microsoft x64 runtime.</p>
</section></main></body></html>'''
if revision == 2:
    guide = guide.replace('<section><h2>1. Revisit the slow resonant sweep</h2>', '''<section><h2>First: MPK249 mod wheel in Live</h2>
<p><strong>This is DSP Test 2.</strong> Load its new VST3 on a fresh MIDI track, rather than reopening the Test 1 device.
Add this test folder's VST3 directory to your supported plug-in scan location and rescan if needed.
Search for <strong>Toyland KB-09 DSP Test 2</strong>. Its distinct name and identity prevent Live from reusing the old test.</p>
<ol><li>Choose Calibrated Init and enable MIDI monitoring or arm the track. For built-in mod-wheel control,
enable <strong>Track</strong> for the MPK249 input. Begin with no Live Remote mapping of CC1 to another control;
Live can reserve a mapped CC instead of passing it to the instrument.</li>
<li>Move the wheel slowly through its full range, then quickly up/down while holding notes. VCO MOD should follow
without a pickup dead zone in the VST3. Repeat after changing VCO MOD by mouse and after loading another preset.</li>
<li>Record wheel movement as MIDI CC1 and play it back, first with the editor open and then closed.
Check that the sound and fader agree, and save/reopen the test project.</li>
<li>Test your other Live mappings with <strong>Remote</strong> enabled for the controller input. Try continuous parameters
(cutoff, resonance, envelope, mixer, portamento, volume) and choices (waveform, range, mode switches).
Confirm minimum/maximum mapping ranges and Absolute mode for ordinary knobs/sliders. Save and reopen the project.</li>
<li>Check pitch bend and channel pressure in MODERN with AFTERTOUCH above zero. The standalone retains its usual
pickup behavior; the VST3 takes over on the next CC1 movement.</li></ol>
<p><a href="https://help.ableton.com/hc/en-us/articles/209774205-Live-s-MIDI-Settings">Ableton: Track and Remote</a> ·
<a href="https://www.ableton.com/en/live-manual/12/midi-and-key-remote-control/">Ableton: mapping behavior</a></p>
<p>The regression test reproduced dropped wheel control during delayed feedback and now passes.
Every exposed automatable parameter was exercised through the actual VST3 at five legal positions.
This does not replace testing your specific Live Set and MPK249 mappings.</p></section>
<section><h2>1. Revisit the slow resonant sweep</h2>''')
guide = guide.replace('DSP Test 1', f'DSP Test {revision}').replace('DSP TEST 1', f'DSP TEST {revision}')
(destination / 'OPEN ME FIRST!.html').write_text(guide, encoding='utf-8')
(destination / 'START HERE.txt').write_text(
    'KB-09 1.5.3 DSP Test 1 — Windows only\n\n'
    'Open Toyland KB-09 DSP Test 1.exe directly. No installation is required.\n'
    'Open OPEN ME FIRST!.html for setup, the seven test presets and listening/latency tests.\n'
    'Public update checks are disabled. This uses separate startup/audio preferences and a separate VST3 identity.\n'
    'Your installed release is unchanged. No GitHub or Google Drive publication.\n'.replace('DSP Test 1', f'DSP Test {revision}'), encoding='utf-8')
support = destination / 'Validation'
support.mkdir()
for filename in [f'{log_prefix}-ctest.log', f'{log_prefix}-host.log']:
    shutil.copy2(source / 'artifacts' / filename, support / filename)
if revision == 2:
    for filename in ['midi-regression-before.log', 'midi-regression-after.log']:
        shutil.copy2(source / 'artifacts' / filename, support / filename)
manifest = {'name': name, 'version': '1.5.3', 'local_test_only': True,
            'platform': 'Windows x64', 'reported_latency_samples': 128,
            'public_update_checks': False, 'presets': patch_manifest,
            'validation': {'ctest': f'{7 if revision == 1 else 8} of {7 if revision == 1 else 8} passed',
                           'actual_vst3_host': f'{host_checks} assertions passed'},
            'source_sha256': {}, 'files_sha256': {}}
for filename in ['CMakeLists.txt', 'Source/DSP/Engine.cpp', 'Source/DSP/Engine.h',
                 'Source/DSP/ResonanceModel.h', 'Source/GUI/Editor.cpp',
                 'Source/GUI/UpdateChecker.h', 'Source/Plugin/Standalone.cpp',
                 'Source/Plugin/Processor.cpp', 'Source/Plugin/Processor.h']:
    manifest['source_sha256'][filename] = sha256((source / filename).read_bytes()).hexdigest()
for path in sorted(destination.rglob('*')):
    if path.is_file():
        manifest['files_sha256'][path.relative_to(destination).as_posix()] = sha256(path.read_bytes()).hexdigest()
(support / 'test-manifest.json').write_text(json.dumps(manifest, indent=2), encoding='utf-8')
print(destination)
