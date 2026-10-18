"""Generate OS-specific guides from the shared editorial source (standard library only)."""
import re

PLATFORMS = {'windows': 'Windows', 'mac': 'Mac', 'linux': 'Linux — UNVERIFIED'}

def span(text, identifier):
    opening = re.search(r'<(\w+)\b[^>]*\bid="' + re.escape(identifier) + r'"[^>]*>', text)
    if not opening:
        raise ValueError(f'Missing manual element: {identifier}')
    tag, depth = opening[1], 1
    for token in re.finditer(r'</?' + tag + r'\b[^>]*>', text[opening.end():]):
        depth += -1 if token[0].startswith('</') else 1
        if depth == 0:
            return opening.start(), opening.end() + token.end()
    raise ValueError(f'Unclosed manual element: {identifier}')

def element(text, identifier):
    a, b = span(text, identifier)
    return text[a:b]

def replace(text, identifier, new):
    a, b = span(text, identifier)
    return text[:a] + new + text[b:]

def host(identifier, title, body):
    return f'<details id="{identifier}" class="host"><summary>{title}</summary><div class="disclosure-body">{body}</div></details>'

WINDOWS_DISPLAY = '<strong>Keep the plug-in sharp:</strong> in Ableton Live, right-click KB-09 in the Browser and turn off <strong>Auto-Scale Plug-in Window</strong> if offered, then reopen its editor. Use KB-09’s S/M/L buttons for size. Other hosts may have a similar bitmap-scaling option; see <a href="#daws">your DAW’s instructions</a>. <a href="https://help.ableton.com/hc/en-us/articles/4405388230674-Recommended-Graphics-Settings-Windows">Ableton’s display guidance</a>.'
MAC_DISPLAY = '<strong>Keep the plug-in sharp:</strong> use KB-09’s S/M/L buttons to resize its panel. If the AU or VST3 looks blurry, check your host for a plug-in scaling or compatibility option; if offered, let KB-09 handle its own scale. The name and availability depend on the host and format. See <a href="#blurry-display">display troubleshooting</a> and <a href="#daws">your DAW’s instructions</a>.'

def document(source, platform):
    text = source
    selected = {'windows':'install-windows', 'mac':'install-mac', 'linux':'linux-standalone'}[platform]
    for identifier in ('install-windows','install-mac','linux-standalone'):
        if identifier != selected:
            text = replace(text, identifier, '')
    block = element(text, selected)
    block = re.sub(r'^<details[^>]*><summary>.*?</summary>', f'<div id="{selected}" class="platform-instructions">', block, count=1, flags=re.S)
    block = re.sub(r'</details>$', '</div>', block)
    text = replace(text, selected, block)
    text = replace(text, 'system-requirements', f'<p id="system-requirements">Requirements, installation and first-play instructions for {PLATFORMS[platform]}.</p>')
    if platform != 'linux':
        notice = WINDOWS_DISPLAY if platform == 'windows' else MAC_DISPLAY
        marker = '<h3>Extract and open</h3>'
        start, end = span(text, 'install')
        install = text[start:end].replace(marker, '<aside class="notice">'+notice+'</aside>'+marker, 1)
        text = text[:start]+install+text[end:]
    text = text.replace('User Manual · Version', f'{PLATFORMS[platform]} User Manual · Version')
    text = text.replace("<title>Toyland KB-09 — A recreation of Kevin's Roland SH-09</title>", f'<title>Toyland KB-09 — {PLATFORMS[platform]} User Manual</title>')
    text = re.sub(r'<aside class="notice"><strong>KB-09 1\.8\.0\.</strong>.*?</aside>',
        '<aside class="notice"><strong>KB-09 1.8.0 for '+PLATFORMS[platform]+'.</strong> '+
        ('This download includes the standalone app and '+('VST3.' if platform=='windows' else 'AU and VST3.') if platform!='linux' else 'This download is standalone only. Real Arch desktop audio and MIDI playback remain unverified.')+'</aside>',text,count=1,flags=re.S)
    # Cross-platform hosts stay collapsible, with only applicable recipes.
    if platform == 'windows':
        for identifier in ('garageband','future-hosts'):
            text = replace(text, identifier, '')
        text = text.replace("Option availability varies by Live version and plug-in format. Ableton's general description excludes VST3, although this toggle worked for Kevin's KB-09 setup.", 'Ableton documents disabling Auto-Scale for freely scalable VST3 interfaces. Option availability still depends on the Live version and plug-in format.')
        text = text.replace('https://help.ableton.com/hc/en-us/articles/209775985-High-DPI-monitor-support','https://help.ableton.com/hc/en-us/articles/4405388230674-Recommended-Graphics-Settings-Windows')
        text = text.replace('Ctrl+W</strong> (Windows / Linux) or <strong>Cmd+W</strong> (Mac)', 'Ctrl+W</strong>')
        text = text.replace('e.g. macOS 15.7 / Windows 11', 'e.g. Windows 11')
        text = text.replace('Logic requires AU; use the separate Mac download for AU.', '')
    elif platform == 'mac':
        text = replace(text,'live',host('live','Ableton Live','<p>Load Toyland KB-09 as an AU or VST3 instrument on a MIDI track. Enable the corresponding plug-in system folders in Live’s Settings, and allow scanning to finish. Arm or monitor the track and choose your controller under MIDI From.</p><p>Use KB-09’s S/M/L buttons for size. If the panel looks blurry, check the plug-in’s context menu for a scaling option, such as Auto-Scale Plug-in Window, <strong>if your version offers one</strong>. Allow native rendering rather than bitmap enlargement. This menu option has not been confirmed for KB-09 on Mac.</p><p>For audio processing, use <a href="#external-live">Live external-input routing</a>.</p><p class="source-note"><a href="https://help.ableton.com/hc/en-us/articles/4410323149074-Plug-ins-on-Mac-in-Live-11-1-and-later">Ableton’s Mac plug-in guidance</a>.</p>'))
        for identifier,title in [('cubase','Cubase / Nuendo'),('studio-one','Studio One / Fender Studio Pro'),('fl-studio','FL Studio'),('reaper','REAPER'),('bitwig','Bitwig Studio')]:
            text=replace(text,identifier,host(identifier,title,'<p>Load Toyland KB-09 as an instrument using a format your host supports. Use KB-09’s S/M/L buttons to choose the panel size. If it looks blurry, check the host’s plug-in window or compatibility settings for bitmap scaling, if available, and let KB-09 handle its own scale. Reopen the editor after a change.</p>'))
        text=text.replace('Use the <strong>Mac download</strong>; the Windows files and VST3 do not load in GarageBand.', 'GarageBand uses the included <strong>AU instrument</strong>.')
        text=text.replace('Use the separate Mac download for AU installation.', 'Install the included AU using INSTALL KB-09.command.')
        text=text.replace('Ctrl+W</strong> (Windows / Linux) or <strong>Cmd+W</strong> (Mac)', 'Cmd+W</strong>')
        text=text.replace('e.g. macOS 15.7 / Windows 11','e.g. macOS 15.7')
        text=text.replace('run INSTALL KB-09.cmd, and rescan VST3 plug-ins in your music software. Look under instruments. Use a 64-bit VST3 host. Logic requires AU; use the separate Mac download for AU.', 'run INSTALL KB-09.command, and rescan plug-ins. Look under instruments. Logic Pro and GarageBand use AU; compatible hosts can also use VST3.')
        text=text.replace('Windows display scale','macOS display settings')
        text=re.sub(r'<details><summary>Windows reports a missing VCRUNTIME or MSVCP file</summary>.*?</details>','',text,flags=re.S)
        text=text.replace('In Live, disabling Auto-Scale Plug-in Window fixed this for the owner.', 'In Live, check for a per-plug-in scaling option if your version offers one. The Auto-Scale menu option has not been confirmed for KB-09 on Mac.')
    else:
        text=replace(text,'daws','<section id="daws"><p class="section-number">09 / YOUR WORKSPACE</p><h2>Standalone workspace</h2><p>This Linux download runs directly as a standalone instrument. Choose audio inputs, outputs and your MIDI controller in AUDIO / MIDI. Use MIDI MAP to assign physical controls. For recording or external routing, connect the standalone to your audio interface or configured audio server.</p><p>For launch or routing trouble, see <a href="#linux-launch-help">Linux setup fixes</a>.</p></section>')
        text=text.replace('09 · Find your DAW','09 · Standalone workspace').replace('Choose your DAW <span','Audio setup <span')
        text=text.replace('Ctrl+W</strong> (Windows / Linux) or <strong>Cmd+W</strong> (Mac)', 'Ctrl+W</strong>')
        text=text.replace('e.g. macOS 15.7 / Windows 11','e.g. Arch Linux / Wayland').replace('e.g. Standalone / Live 12.3','Standalone')
        text=re.sub(r'<details(?: open)?><summary>I cannot find KB-09 in my DAW</summary>.*?</details>','',text,flags=re.S)
        text=re.sub(r'<details><summary>My MIDI fader jumps or controls more than one thing in Live</summary>.*?</details>','<details><summary>A MIDI control does not respond</summary><p>Enable the controller in AUDIO / MIDI, then check its assignment in MIDI MAP. Continuous controls may need to cross the current position before taking over. Try KEYBOARD to distinguish MIDI setup from audio output trouble.</p></details>',text,flags=re.S)
        text=re.sub(r'<details><summary>Windows reports a missing VCRUNTIME or MSVCP file</summary>.*?</details>','',text,flags=re.S)
        text=text.replace('install the version for your computer</a> and load <strong>Toyland KB-09</strong> on a DAW instrument/MIDI track.','follow the Linux launch instructions</a>.')
        text=text.replace('Open the standalone app</a>, or','Open the standalone app</a>, or')
        text=text.replace('(enable MIDI monitoring if using a DAW)','(enable the controller in AUDIO / MIDI)')
        text=text.replace('Windows display scale','desktop/session and display scale').replace("Your DAW’s requirements also apply.",'')
        text=text.replace('which compatible DAWs compensate for during playback.','in addition to the selected device buffer.')
        text=text.replace('Your DAW also saves the patch with its project; external track routing belongs to that DAW project.', 'External audio routing belongs to your audio device or audio server setup.')
        text=text.replace('DAW projects continue to restore their own plug-in state.','')
        text=text.replace('The standalone includes MIDI mapping; plug-in mapping uses your DAW’s controls.','The standalone includes MIDI mapping.').replace('Use your DAW for recording, effects and arrangement.','Use separate music software for recording, effects and arrangement.')
        text=text.replace('KB-09 automatically uses CC1 for VCO MOD in a DAW; other controller faders are mapped by the host.','')
        # Remove DAW routing recipes while retaining the filter explanation.
        a=text.index('<h3 id="external-live">');b=text.index('</section>',a)
        text=text[:a]+'<h3>In the standalone</h3><p>Select the input connected to your source in AUDIO / MIDI, raise EXT, and choose External Filter or External Follower. Avoid sending the output back into its own input. Audio-server routing and MIDI playback on a real Arch desktop remain unverified.</p>\n'+text[b:]
        text=text.replace("KB-09's VST3 instrument exposes an <strong>External Input</strong> auxiliary audio bus. A DAW may call this a sidechain. Here the incoming audio can become part of the sound, passing through the mixer, filter and amplifier.", 'The standalone can process audio from the input selected in AUDIO / MIDI. That source passes through the mixer, filter and amplifier.')
        text=text.replace('Confirm the host enabled the auxiliary input','Confirm the intended audio input is selected in AUDIO / MIDI')
        text=text.replace('Play in your DAW or use the standalone with a MIDI keyboard.', 'Play the standalone with a MIDI keyboard or the onscreen keys.')
        text=text.replace('A DAW controls its own plug-in frame and may restrict movement or size requests.','')
        text=text.replace("retain the DAW's normal HiDPI support", 'retain your desktop’s normal display scaling')
        text=text.replace('In Live, disable Auto-Scale Plug-in Window if offered.', '')
        text=text.replace('Save and reopen a DAW project containing your patch and external-audio routing.', 'Save a patch, reopen the standalone and load it again.')
        text=text.replace('DAW / version: ____________________','Audio device / setup: ____________________')
        text=text.replace('Your DAW may display the stored tuning value rather than semitones; the knob’s value bubble shows the effective pitch change.', 'The knob’s value bubble shows the effective pitch change.')
        text=replace(text,'mod-wheel','<section id="mod-wheel"><h2>Play VCO MOD from your mod wheel</h2><p>Your controller’s modulation wheel (MIDI CC1) controls VCO MOD in both AUTHENTIC and MODERN. It changes the amount of LFO pitch modulation; RATE and the selected waveform determine the movement.</p><p>The standalone uses pickup: after a manual edit or preset change, move the wheel to or through the fader’s position to take control without a jump. MIDI MAP assigns other knobs and buttons; assigning CC1 elsewhere replaces its normal VCO MOD function until cleared. The resulting fader setting is saved with the sound.</p></section>')
        text=text.replace('In a DAW, use the host’s mapping system; MIDI MAP is available only in standalone.','')
        text=text.replace('In a DAW, record or edit MIDI Pitch Bend envelopes; the bender position is a MIDI performance control, while its VCO and VCF depth faders are automatable plug-in parameters.', 'The wheel’s position is a performance control; the VCO and VCF depth faders set its sensitivities.')
        text=text.replace('Check track monitoring, MIDI input, VOLUME and the DAW output.', 'Check the selected audio output, MIDI input and VOLUME.')
        text=replace(text,'blurry-display','<details id="blurry-display"><summary>The display is blurry or low resolution</summary><p>Keep your monitor at its native resolution and use KB-09’s S/M/L buttons for size. Reopen the app after a desktop scaling change. On Wayland, XWayland scaling can affect appearance; compare an X11 session if available. Include your desktop/session, scale and resolution in a <a href="#feedback">bug report</a>. Real desktop behavior remains unverified.</p></details>')
        text=text.replace('note the DAW version,','note')
        text=text.replace('a host can constrain a plug-in\'s requested dimensions.', 'a desktop can constrain a window’s requested dimensions.')
        text=text.replace('Select the input connected to your source in AUDIO / MIDI, raise EXT,', 'Select the input connected to your source in AUDIO / MIDI and turn off <strong>Mute audio input</strong> only when the routing avoids feedback. Raise EXT,')
    # OS-exclusive update and display gesture directions.
    if platform != 'windows':
        text=text.replace('on Windows the taskbar remains accessible. Compatible floating plug-in windows on Windows support the same gesture, but a DAW may control placement.', 'Use the standalone header for this gesture; host-owned plug-in frames may behave differently.' if platform=='mac' else 'Use the standalone header for this gesture.')
    updates=element(text,'updates')
    a=updates.index('<ol>');b=updates.index('</ol>',a)+len('</ol>')
    installer='INSTALL KB-09.cmd' if platform=='windows' else 'INSTALL KB-09.command'
    steps='<ol><li>Download the '+('Linux' if platform=='linux' else PLATFORMS[platform])+' ZIP and extract it completely. Keep personal presets stored inside the old folder before replacing it.</li><li>Save your patch and close KB-09.'+(' Close your DAW too if using plug-ins.' if platform!='linux' else '')+'</li><li>For a portable copy, replace the extracted folder and open the new standalone.'+(' For an installed copy, rerun <strong>'+installer+'</strong>; it also offers the standalone desktop shortcut.' if platform!='linux' else ' If you move the folder, rerun Create desktop shortcut.sh.')+'</li><li>Confirm the new version under TOYLAND / KB-09.'+(' Reopen your DAW and rescan plug-ins if necessary.' if platform!='linux' else '')+'</li></ol>'
    updates=updates[:a]+steps+updates[b:]
    text=replace(text,'updates',updates)
    return text
