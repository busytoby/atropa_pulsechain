import sys

filepath = 'tools/tsfi_c_vtuber.py'
with open(filepath, 'r') as f:
    code = f.read()

# Completely disable the RTMP push mechanism in the primary channel script
# to ensure it can never accidentally broadcast to the primary URL.
code = code.replace(
    '        if ffmpeg_process is not None and latest_frame_bytes is not None:',
    '        # PRIMARY BROADCAST DISABLED AS PER PROTOCOL\n        if False and ffmpeg_process is not None and latest_frame_bytes is not None:'
)

with open(filepath, 'w') as f:
    f.write(code)
