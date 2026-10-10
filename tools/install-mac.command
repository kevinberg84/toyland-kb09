#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"
echo 'Toyland KB-09 — Mac release 1.0.1'
echo 'Close your music software before continuing.'
read -r -p 'Press Return when ready. '
backup="$HOME/Library/Application Support/Toyland/KB-09/Backups/$(date +%Y%m%d-%H%M%S)"
app='Toyland KB-09 Standalone.app'
test -d "$app" || { echo 'Incomplete download. Extract the complete ZIP again.'; exit 1; }
mkdir -p "$HOME/Applications"
if [ -e "$HOME/Applications/$app" ]; then mkdir -p "$backup"; mv "$HOME/Applications/$app" "$backup/$app"; fi
if ! ditto "$app" "$HOME/Applications/$app"; then
  if [ -d "$backup/$app" ]; then ditto "$backup/$app" "$HOME/Applications/$app"; fi
  echo 'Standalone installation failed. Please contact Kevin.'; exit 1
fi
read -r -p 'Create a desktop shortcut for the standalone? [Y/n] ' shortcut_choice
if [[ ! "$shortcut_choice" =~ ^[Nn]([Oo])?$ ]]; then
  desktop_shortcut="$HOME/Desktop/Toyland KB-09.app"
  if [ ! -e "$desktop_shortcut" ] && [ ! -L "$desktop_shortcut" ]; then
    ln -s "$HOME/Applications/$app" "$desktop_shortcut"
  else
    echo 'An item named Toyland KB-09.app already exists on your Desktop; it was kept.'
  fi
fi
read -r -p 'Install AU/VST3 plug-ins for a DAW as well? [y/N] ' plugins_choice
if [[ "$plugins_choice" =~ ^[Yy]([Ee][Ss])?$ ]]; then
echo 'Plug-in installation needs your Mac login password. Typed characters stay hidden.'
sudo -v
sudo mkdir -p '/Library/Audio/Plug-Ins/Components' '/Library/Audio/Plug-Ins/VST3'
for ext in component vst3; do
  if [ "$ext" = component ]; then parent="/Library/Audio/Plug-Ins/Components"; else parent="/Library/Audio/Plug-Ins/VST3"; fi
  target="$parent/Toyland KB-09.$ext"
  source="Support/Toyland KB-09.$ext"
  test -d "$source" || { echo 'Incomplete download. Extract the complete ZIP again.'; exit 1; }
  if [ -e "$target" ]; then mkdir -p "$backup"; sudo mv "$target" "$backup/"; fi
  if ! sudo ditto "$source" "$target"; then
    if [ -d "$backup/Toyland KB-09.$ext" ]; then sudo ditto "$backup/Toyland KB-09.$ext" "$target"; fi
    echo 'Installation failed. Please contact Kevin.'; exit 1
  fi
done
echo 'Plug-ins installed. Reopen your music software and rescan.'
fi
echo 'Standalone installed in your home Applications folder.'
echo 'Please report bugs or guide mistakes to Kevin.'
read -r -p 'Press Return to close. '

