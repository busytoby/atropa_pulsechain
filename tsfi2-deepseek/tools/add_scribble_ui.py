import sys

filepath = 'tools/tsfi_spider_alligator_vtuber.py'
with open(filepath, 'r') as f:
    code = f.read()

# We need to inject a secondary "scribble" canvas directly over the main VTuber canvas.
# This allows the user to click and drag to paint lines over the video feed on localhost:9092.

html_old = """                <div class="compositor">
                    <canvas id="painterCanvas" width="512" height="512"></canvas>
                </div>"""

html_new = """                <div class="compositor">
                    <canvas id="painterCanvas" width="512" height="512" style="z-index: 1;"></canvas>
                    <canvas id="scribbleCanvas" width="512" height="512" style="z-index: 2; cursor: crosshair;"></canvas>
                </div>"""

code = code.replace(html_old, html_new)

script_old = """                    img.onload = () => { requestAnimationFrame(updateCanvas); };"""

script_new = """                    img.onload = () => { requestAnimationFrame(updateCanvas); };
                    
                    // --- THE SCRIBBLE BRUSH LOGIC ---
                    const scribbleCanvas = document.getElementById('scribbleCanvas');
                    const sCtx = scribbleCanvas.getContext('2d');
                    sCtx.lineCap = 'round';
                    sCtx.lineJoin = 'round';
                    sCtx.lineWidth = 4;
                    sCtx.strokeStyle = '#0f0'; // Neon Green matrix ink
                    
                    let isPainting = false;
                    let lastX = 0;
                    let lastY = 0;
                    
                    scribbleCanvas.addEventListener('mousedown', (e) => {
                        isPainting = true;
                        const rect = scribbleCanvas.getBoundingClientRect();
                        lastX = e.clientX - rect.left;
                        lastY = e.clientY - rect.top;
                    });
                    
                    scribbleCanvas.addEventListener('mousemove', (e) => {
                        if(!isPainting) return;
                        const rect = scribbleCanvas.getBoundingClientRect();
                        const currentX = e.clientX - rect.left;
                        const currentY = e.clientY - rect.top;
                        
                        sCtx.beginPath();
                        sCtx.moveTo(lastX, lastY);
                        sCtx.lineTo(currentX, currentY);
                        sCtx.stroke();
                        
                        lastX = currentX;
                        lastY = currentY;
                    });
                    
                    scribbleCanvas.addEventListener('mouseup', () => isPainting = false);
                    scribbleCanvas.addEventListener('mouseleave', () => isPainting = false);
                    
                    // Optional clear button logic could go here, but for now we just let it accumulate
"""

code = code.replace(script_old, script_new)

with open(filepath, 'w') as f:
    f.write(code)

