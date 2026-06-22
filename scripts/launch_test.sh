#!/bin/bash
WAYLAND_DISPLAY=wayland-1 MOZ_ENABLE_WAYLAND=1 MOZ_WEBRENDER=1 /home/mariarahel/src/mozilla/obj-tsfi/dist/bin/firefox --new-window https://youtube.com > /tmp/firefox_stdout.log 2> /tmp/firefox_stderr.log
echo "Firefox exited with code: $?"
echo "=== STDERR ==="
cat /tmp/firefox_stderr.log
