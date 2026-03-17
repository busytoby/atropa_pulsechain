#!/bin/bash
# TSFi YouTube Backup Stream Bridge (Secure)
# Broadcasts the 720p Neural Masterpiece to RTMP

STREAM_KEY_FILE="$HOME/.config/gemini/stream_key.txt"
if [ ! -f "$STREAM_KEY_FILE" ]; then
    echo "[FRACTURE] Stream key not found in $STREAM_KEY_FILE"
    exit 1
fi

STREAM_KEY=$(cat "$STREAM_KEY_FILE")
STREAM_URL="rtmp://a.rtmp.youtube.com/live2/$STREAM_KEY"
SOURCE_URL="http://127.0.0.1:9093/serpent_stream"

echo "[SYSTEM] Starting Secure YouTube Backup Stream..."
echo "[SOURCE] $SOURCE_URL"

ffmpeg -f mjpeg -i "$SOURCE_URL" \
    -c:v libx264 -preset veryfast -tune zerolatency \
    -b:v 4500k -maxrate 4500k -bufsize 9000k \
    -pix_fmt yuv420p -g 60 -f flv "$STREAM_URL"
