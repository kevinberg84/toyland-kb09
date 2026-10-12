#!/usr/bin/env bash
set -euo pipefail
folder="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
desktop="$(xdg-user-dir DESKTOP 2>/dev/null || true)"
desktop="${desktop:-$HOME/Desktop}"
mkdir -p -- "$desktop"
# Desktop Entry quoted arguments escape these four characters; never evaluate
# a folder name as shell code. Launch directly, rather than via sh -c.
escape_exec() { printf '%s' "$1" | sed 's/\\/\\\\/g; s/"/\\"/g; s/`/\\`/g; s/\$/\\$/g; s/%/%%/g'; }
chmod u+x -- "$folder/Toyland KB-09 Standalone"
exec_path="$(escape_exec "$folder/Toyland KB-09 Standalone")"
cat > "$desktop/Toyland KB-09.desktop" <<EOF
[Desktop Entry]
Type=Application
Version=1.0
Name=Toyland KB-09
Comment=Standalone monophonic synthesizer
Exec="$exec_path"
Terminal=false
Categories=AudioVideo;Audio;Midi;
EOF
chmod u+x -- "$desktop/Toyland KB-09.desktop"
printf 'Created %s/Toyland KB-09.desktop\nKeep the extracted synth folder in this location.\n' "$desktop"
