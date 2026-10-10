"""Connect both manual variants to the owner's deployed Apps Script web app."""
from pathlib import Path
import argparse, json, re, subprocess, sys

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--deployment-url', required=True)
args = parser.parse_args()
url = args.deployment_url.strip()
if not re.fullmatch(r'https://script\.google\.com/macros/s/[A-Za-z0-9_-]+/exec', url):
    parser.error('Use the HTTPS script.google.com web-app deployment URL ending in /exec.')
guide = root/'docs/manual'
(guide/'bug-report-config.json').write_text(json.dumps({'deploymentUrl':url}, indent=2)+'\n', encoding='utf-8')
index = guide/'index.html'
text = index.read_text(encoding='utf-8-sig')
text, count = re.subn(r'(id="bug-report-form" action=")[^"]*(")', lambda m:m[1]+url+m[2], text)
assert count == 1, 'Expected one bug-report form'
index.write_text(text, encoding='utf-8')
subprocess.run([sys.executable, str(root/'tools/make_manual_html.py')], check=True)
print('Both manuals connected. Verify a real submission before sharing the update.')
