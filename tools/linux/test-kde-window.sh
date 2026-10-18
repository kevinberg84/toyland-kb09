#!/usr/bin/env bash
# Isolated CI container only. No user's desktop, preferences or audio hardware.
set -euo pipefail
pacman -Syu --noconfirm --needed kwin xorg-xwayland xorg-xclock mesa alsa-lib freetype2 fontconfig curl dbus pipewire wireplumber gdb libx11 libxext libxinerama libxrandr libxcursor libxrender libxi libxcomposite
# Arch gives KWin a scheduling capability. Docker cannot execute that capped
# binary with its default capability ceiling. Remove the file capability only
# inside this disposable container; no host or real desktop settings change.
setcap -r /usr/bin/kwin_wayland
useradd -m kb09probe
install -d -m 700 -o kb09probe /tmp/kb09-runtime
chown kb09probe /results
probe=$(find /validation -type f -name SH09GUIRender -print -quit)
test -n "$probe"
install -m 755 "$probe" /tmp/kb09-window-probe
startup=$(find /validation -type f -name KB09StartupDialogTests -print -quit)
test -n "$startup"
install -m 755 "$startup" /tmp/kb09-startup-dialog-test
cat >/tmp/kb09-session <<'SESSION'
#!/usr/bin/env bash
set -euo pipefail
pipewire >/results/pipewire.log 2>&1 &
wireplumber >/results/wireplumber.log 2>&1 &
printf 'DISPLAY=%s\nWAYLAND_DISPLAY=%s\n' "${DISPLAY:-}" "${WAYLAND_DISPLAY:-}" >/results/session.txt
pacman -Q kwin xorg-xwayland pipewire >>/results/session.txt
cd /results
printf 'Starting independent X11 clock control\n' >> /results/session.txt
set +e
timeout 2 xclock -geometry 64x64 > /results/xclock.log 2>&1
control=$?
set -e
printf 'X11 clock control exit=%s (124 means successful timed observation)\n' "$control" >> /results/session.txt
if [ "$control" -ne 124 ] && [ "$control" -ne 0 ]; then exit 1; fi
ldd /tmp/kb09-startup-dialog-test > /results/libraries.txt
if ! timeout 30 /tmp/kb09-startup-dialog-test > /results/startup-dialogs.log 2>&1; then
  timeout 45 gdb --batch -ex run -ex 'thread apply all bt' -ex 'info sharedlibrary' --args /tmp/kb09-startup-dialog-test > /results/startup-stack.log 2>&1 || true
  exit 1
fi
# The fullscreen fixture has its own prompt. Suppress the real update service
# only in this disposable test account, so it cannot alter that fixture.
mkdir -p "$HOME/Toyland"
printf '<PROPERTIES><VALUE name="asked" val="1"/><VALUE name="annual" val="0"/></PROPERTIES>' > "$HOME/Toyland/Toyland KB-09 Updates.settings"
timeout 120 /tmp/kb09-window-probe --native-window285=/results > /results/probe.log 2>&1
SESSION
chmod 755 /tmp/kb09-session
set +e
timeout 180 runuser -u kb09probe -- env XDG_RUNTIME_DIR=/tmp/kb09-runtime XDG_CURRENT_DESKTOP=KDE LIBGL_ALWAYS_SOFTWARE=1 \
  dbus-run-session -- gdb --batch --return-child-result -ex 'set debuginfod enabled off' -ex 'set print thread-events off' -ex run -ex 'thread apply all bt' \
  --args kwin_wayland --virtual --xwayland --no-lockscreen --width 1920 --height 1080 --exit-with-session /tmp/kb09-session \
  > /results/kwin.log 2>&1
result=$?
set -e
cat /results/session.txt /results/startup-dialogs.log /results/probe.log 2>/dev/null || true
if [ "$result" -ne 0 ]; then tail -60 /results/kwin.log; exit "$result"; fi
grep -q 'RESULT failures=0 checks=6' /results/probe.log
grep -q 'RESULT failures=0 checks=16' /results/startup-dialogs.log
