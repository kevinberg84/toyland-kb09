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
         prefix+'Support/User Guide/assets/panel-0.13.19.png':(root/'docs/manual/assets/panel-0.13.19.png').read_bytes(),
         prefix+'INSTALL KB-09.command':(root/'tools/install-mac.command').read_bytes().replace(b'\r\n',b'\n'),
         prefix+'OPEN ME FIRST!.html':(root/'docs/open-me-first-mac.html').read_bytes()}
with zipfile.ZipFile(source) as old,zipfile.ZipFile(dest,'w',zipfile.ZIP_DEFLATED) as new:
    for entry in old.infolist():
        if entry.filename.endswith(('User Manual.pdf','START HERE.txt','OPEN ME FIRST!.txt')): continue
        if entry.filename not in updates: new.writestr(entry,old.read(entry))
        elif entry.filename.endswith('/INSTALL KB-09.command'):
            new.writestr(entry,updates.pop(entry.filename))
    for key,value in updates.items(): new.writestr(key,value)
with zipfile.ZipFile(dest) as z:
    assert z.testzip() is None
    z.extractall(release)
print(dest)
print(hashlib.sha256(dest.read_bytes()).hexdigest())
