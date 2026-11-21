#!/usr/bin/env bash
set -euo pipefail

export DISPLAY=:99
export XDG_RUNTIME_DIR=/tmp/phonebook-runtime
mkdir -p "$XDG_RUNTIME_DIR"
chmod 700 "$XDG_RUNTIME_DIR"

# An Xvfb restart may leave a lock and Unix socket in this container.
# Only one X display is used here, and the previous processes have stopped
# before this startup script is run again.
rm -f /tmp/.X99-lock /tmp/.X11-unix/X99

Xvfb :99 -screen 0 460x990x24 -nolisten tcp &
XVFB_PID=$!
for i in $(seq 1 50); do
  if ! kill -0 "$XVFB_PID" 2>/dev/null; then
    echo "ERROR: Xvfb exited before the display became ready" >&2
    exit 1
  fi
  if [ -S /tmp/.X11-unix/X99 ]; then break; fi
  sleep 0.2
done
if [ ! -S /tmp/.X11-unix/X99 ]; then
  echo "ERROR: Xvfb did not start" >&2
  exit 1
fi
sleep 0.3

openbox >/tmp/openbox.log 2>&1 &
x11vnc -display :99 -forever -shared -nopw -quiet \
  -listen 127.0.0.1 -rfbport 5900 >/tmp/x11vnc.log 2>&1 &
websockify --web=/usr/share/novnc 0.0.0.0:6080 \
  127.0.0.1:5900 >/tmp/websockify.log 2>&1 &

exec /opt/phonebook/PhoneBookManagement
