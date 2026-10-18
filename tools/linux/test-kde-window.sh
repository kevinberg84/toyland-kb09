#!/usr/bin/env bash
# Isolated CI container only. No user's desktop, preferences or audio hardware.
set -euo pipefail
pacman -Syu --noconfirm --needed kwin xorg-xwayland mesa alsa-lib freetype2 fontconfig curl dbus pipewire wireplumber
useradd -m kb09probe
install -d -m 700 -o kb09probe /tmp/kb09-runtime
chown kb09probe /results
probe=$(find /validation -type f -name SH09GUIRender -print -quit)
test -n "$probe"
install -m 755 "$probe" /tmp/kb09-window-probe
cat >/tmp/kb09-session <<'SESSION'
#!/usr/bin/env bash
set -euo pipefail
pipewire >/results/pipewire.log 2>&1 &
wireplumber >/results/wireplumber.log 2>&1 &
printf 'DISPLAY=%s\nWAYLAND_DISPLAY=%s\n' "${DISPLAY:-}" "${WAYLAND_DISPLAY:-}" >/results/session.txt
pacman -Q kwin xorg-xwayland pipewire >>/results/session.txt
cd /results
timeout 120 /tmp/kb09-window-probe --native-window285=/results > /results/probe.log 2>&1
SESSION
chmod 755 /tmp/kb09-session
set +e
timeout 180 runuser -u kb09probe -- env XDG_RUNTIME_DIR=/tmp/kb09-runtime XDG_CURRENT_DESKTOP=KDE LIBGL_ALWAYS_SOFTWARE=1 \
  dbus-run-session -- kwin_wayland --virtual --xwayland --no-lockscreen --width 1920 --height 1080 --exit-with-session /tmp/kb09-session \
  > /results/kwin.log 2>&1
result=$?
set -e
cat /results/session.txt /results/probe.log 2>/dev/null || true
if [ "$result" -ne 0 ]; then tail -60 /results/kwin.log; exit "$result"; fi
grep -q 'RESULT failures=0 checks=6' /results/probe.log
