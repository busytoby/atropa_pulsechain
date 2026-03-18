#!/bin/bash
# TSFi YouTube Backup Stream Bridge (Live Image Update)
# Reads the broadcast heartbeat and pushes to RTMP

STREAM_KEY_FILE="$HOME/.config/gemini/stream_key.txt"
if [ ! -f "$STREAM_KEY_FILE" ]; then
    echo "[FRACTURE] Stream key not found in $STREAM_KEY_FILE"
    exit 1
fi
STREAM_KEY=$(cat "$STREAM_KEY_FILE")
STREAM_URL="rtmp://b.rtmp.youtube.com/live2/$STREAM_KEY"
SOURCE_FILE="/tmp/tsfi_broadcast.jpg"

echo "[SYSTEM] Starting Live-Image YouTube Backup Stream..."
echo "[SOURCE] $SOURCE_FILE"

# Persistent Loop using a steady python feeder to force continuous image updates
while true; do
    python3 tools/tsfi_feeder.py "$SOURCE_FILE" | \
    ffmpeg -f image2pipe -vcodec mjpeg -i - \
        -f lavfi -i anullsrc=channel_layout=stereo:sample_rate=44100 \
        -c:v libx264 -preset veryfast -tune zerolatency \
        -b:v 4500k -maxrate 4500k -bufsize 9000k \
        -pix_fmt yuv420p -g 60 -shortest \
        -c:a aac -b:a 128k \
        -f flv "$STREAM_URL"
    echo "[RETRY] FFmpeg exited. Reconnecting in 2s..."
    sleep 2
done
