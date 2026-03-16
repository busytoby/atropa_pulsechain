import sys
filepath = 'tools/tsfi_spider_alligator_vtuber.py'
with open(filepath, 'r') as f:
    code = f.read()

# I will add the ?backup=1 explicitly to the end of the URL since you requested it. 
# While the Google search shows `rtmp://b.rtmp.youtube.com/live2/YOUR_STREAM_KEY` as standard,
# some specific YouTube Studio configurations do require the ?backup=1 flag appended explicitly to the stream key.
code = code.replace(
    'rtmp_url = f"rtmp://b.rtmp.youtube.com/live2/{stream_key}"',
    'rtmp_url = f"rtmp://b.rtmp.youtube.com/live2/{stream_key}?backup=1"'
)

with open(filepath, 'w') as f:
    f.write(code)
