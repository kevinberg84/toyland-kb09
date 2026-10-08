from pathlib import Path
import zipfile, hashlib
root=Path(__file__).resolve().parents[1]
release=root/'artifacts/Release'
name='Toyland KB-09 for Mac - UNVERIFIED'
source=release/'Mac-artifact'/f'{name}.zip'
dest=release/f'{name}.zip'
prefix=name+'/'
updates={prefix+'User Manual.html':(root/'docs/manual/User Manual.html').read_bytes(),
         prefix+'Support/User Guide/index.html':(root/'docs/manual/index.html').read_bytes(),
         prefix+'Support/User Guide/assets/panel-0.13.8.png':(root/'docs/manual/assets/panel-0.13.8.png').read_bytes(),
         prefix+'INSTALL KB-09.command':(root/'tools/install-mac.command').read_bytes().replace(b'\r\n',b'\n'),
         prefix+'START HERE.txt':'''Toyland KB-09 -- UNVERIFIED MAC TEST BUILD

1. Extract this ZIP. Keep the extracted folder together.
2. Close your music software.
3. Double-click INSTALL KB-09.command and press Return when prompted.
   Enter your Mac login password when asked; typing stays hidden.
4. Reopen your music software and rescan plug-ins.

Logic Pro and GarageBand: choose Toyland KB-09 as an Audio Unit instrument.
Other compatible music software can use its VST3 instrument.
Open User Manual.html for the illustrated guide and working navigation links.
Use the HTML manualâ€™s Print / Save PDF button for a printable copy.

macOS 12 or later; Intel and Apple Silicon. Ad-hoc signed, not Developer ID
signed or notarized. macOS may warn that Apple cannot verify the developer.
If this copy came from Kevin and you want to allow it, try opening it first,
then go to Apple menu > System Settings > Privacy & Security > Open Anyway.
Authenticate and confirm Open if offered, then reopen your music software.
See User Manual.html for details and Apple's help link. If no option appears,
contact Kevin with the exact warning. Do not disable Gatekeeper or override
an alert that says malware was found or the software will damage your Mac.

UNVERIFIED means that everyday Mac hosts have not been tested. Apple AU
validation passed on the build runner after system-wide installation.
The included logs record the result; this is not proof of host compatibility.
Please send bugs and manual mistakes to Kevin, including your Mac model,
macOS version, music software/version, steps and exact error or screenshot.

Share this complete ZIP with Mac friends. Windows friends need the separate
Windows download. Support contains installer files and validation evidence.
'''.encode('utf-8')}
with zipfile.ZipFile(source) as old,zipfile.ZipFile(dest,'w',zipfile.ZIP_DEFLATED) as new:
    for entry in old.infolist():
        if entry.filename.endswith('User Manual.pdf'): continue
        if entry.filename not in updates: new.writestr(entry,old.read(entry))
        elif entry.filename.endswith('/INSTALL KB-09.command'):
            new.writestr(entry,updates.pop(entry.filename))
    for key,value in updates.items(): new.writestr(key,value)
with zipfile.ZipFile(dest) as z:
    assert z.testzip() is None
    z.extractall(release)
print(dest)
print(hashlib.sha256(dest.read_bytes()).hexdigest())
