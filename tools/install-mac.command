#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"
echo 'Toyland KB-09 — UNVERIFIED Mac test build'
echo 'Close your music software before continuing.'
read -r -p 'Press Return when ready. '
mkdir -p "$HOME/Library/Audio/Plug-Ins/Components" "$HOME/Library/Audio/Plug-Ins/VST3"
backup="$HOME/Library/Application Support/Toyland/KB-09/Backups/$(date +%Y%m%d-%H%M%S)"
for ext in component vst3; do
  if [ "$ext" = component ]; then parent="$HOME/Library/Audio/Plug-Ins/Components"; else parent="$HOME/Library/Audio/Plug-Ins/VST3"; fi
  target="$parent/Toyland KB-09.$ext"
  source="Support/Toyland KB-09.$ext"
  test -d "$source" || { echo 'Incomplete download. Extract the complete ZIP again.'; exit 1; }
  if [ -e "$target" ]; then mkdir -p "$backup"; mv "$target" "$backup/"; fi
  if ! ditto "$source" "$target"; then
    if [ -d "$backup/Toyland KB-09.$ext" ]; then ditto "$backup/Toyland KB-09.$ext" "$target"; fi
    echo 'Installation failed. Please contact Kevin.'; exit 1
  fi
done
echo 'Installed for your Mac account. Reopen your music software and rescan.'
echo 'Please report bugs or guide mistakes to Kevin. This Mac build is unverified.'
read -r -p 'Press Return to close. '
