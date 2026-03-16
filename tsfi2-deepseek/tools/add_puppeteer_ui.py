import sys
import os

filepath = 'tools/tsfi_spider_alligator_vtuber.py'
with open(filepath, 'r') as f:
    code = f.read()

# 1. Update the HTML to add a clear button and send the path data to the server
html_old = """                    <canvas id="scribbleCanvas" width="512" height="512" style="z-index: 2; cursor: crosshair;"></canvas>
                </div>
                <div id="telemetry">Initializing Neural Lifecycle...</div>
                
                <div class="control-panel">"""

html_new = """                    <canvas id="scribbleCanvas" width="512" height="512" style="z-index: 2; cursor: crosshair;"></canvas>
                </div>
                <div style="margin-top:10px;">
                    <button onclick="clearScribble()">CLEAR PATH</button>
                    <button onclick="sendPathToPuppet()">PUPPETEER EXECUTE</button>
                </div>
                <div id="telemetry">Initializing Neural Lifecycle...</div>
                
                <div class="control-panel">"""

code = code.replace(html_old, html_new)


# 2. Update the JavaScript to track the coordinates and POST them
js_old = """                    scribbleCanvas.addEventListener('mouseup', () => isPainting = false);
                    scribbleCanvas.addEventListener('mouseleave', () => isPainting = false);
                    
                    // Optional clear button logic could go here, but for now we just let it accumulate"""

js_new = """                    let pathData = [];
                    
                    scribbleCanvas.addEventListener('mousedown', (e) => {
                        isPainting = true;
                        pathData = []; // Start a new path
                        const rect = scribbleCanvas.getBoundingClientRect();
                        lastX = e.clientX - rect.left;
                        lastY = e.clientY - rect.top;
                        pathData.push({x: lastX, y: lastY});
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
                        pathData.push({x: lastX, y: lastY});
                    });

                    scribbleCanvas.addEventListener('mouseup', () => isPainting = false);
                    scribbleCanvas.addEventListener('mouseleave', () => isPainting = false);
                    
                    window.clearScribble = function() {
                        sCtx.clearRect(0, 0, 512, 512);
                        pathData = [];
                    };
                    
                    window.sendPathToPuppet = function() {
                        if(pathData.length === 0) return;
                        fetch('/puppet_path', {
                            method: 'POST',
                            headers: { 'Content-Type': 'application/json' },
                            body: JSON.stringify({ path: pathData })
                        }).then(r => {
                            if(r.ok) {
                                document.getElementById('telemetry').innerText = "IK Target Path Received! Commencing Puppeteer Override...";
                                clearScribble();
                            }
                        });
                    };"""

code = code.replace(js_old, js_new)

# 3. Add the Python Backend Handlers for the Path Data
backend_old = """global_frame_idx = 0
current_genome = "a soft stuffed animal collection featuring a green alligator plush, a fuzzy spider plush, and a brown teddy bear"
latest_frame_bytes = None
latest_jpeg_bytes = None"""

backend_new = """global_frame_idx = 0
current_genome = "a soft stuffed animal collection featuring a green alligator plush, a fuzzy spider plush, and a brown teddy bear"
latest_frame_bytes = None
latest_jpeg_bytes = None

# Puppeteer Globals
puppet_path_queue = []
puppet_is_active = False
puppet_target_x = 0
puppet_target_y = 0"""

code = code.replace(backend_old, backend_new)

# Add the HTTP POST route for '/puppet_path'
http_old = """                else:
                    self.send_response(400)
                    self.end_headers()
            except Exception as e:
                self.send_response(500)
                self.end_headers()

    def do_GET(self):"""

http_new = """                else:
                    self.send_response(400)
                    self.end_headers()
            except Exception as e:
                self.send_response(500)
                self.end_headers()
        elif self.path == '/puppet_path':
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length).decode('utf-8')
            try:
                data = json.loads(post_data)
                global puppet_path_queue, puppet_is_active
                puppet_path_queue = data.get("path", [])
                if len(puppet_path_queue) > 0:
                    puppet_is_active = True
                self.send_response(200)
                self.end_headers()
            except Exception as e:
                self.send_response(500)
                self.end_headers()

    def do_GET(self):"""

code = code.replace(http_old, http_new)

# 4. Wire the IK Solver to use the Puppet Data instead of the Sine Wave
ik_old = """    # Choreography: Grand Battement (Right leg sweeps up)
    sweep_angle = math.pi * 0.5 * math.sin(t * math.pi) 
    
    l_foot_target = (hip[0] - 30, hip[1] + 130)
    r_foot_target_x = hip[0] + 40 + math.sin(sweep_angle) * 120
    r_foot_target_y = hip[1] + 130 - math.sin(sweep_angle) * 200
    arm_lift = math.sin(t * math.pi) * 80
    l_hand_target = (neck[0] - 100 + arm_lift, neck[1] + 50 - arm_lift)
    r_hand_target = (neck[0] + 100 - arm_lift, neck[1] + 50 - arm_lift)"""

ik_new = """    global puppet_is_active, puppet_path_queue, puppet_target_x, puppet_target_y
    
    # Base Idle State
    l_foot_target = (hip[0] - 30, hip[1] + 130)
    r_foot_target_x = hip[0] + 40
    r_foot_target_y = hip[1] + 130
    l_hand_target = (neck[0] - 100, neck[1] + 50)
    r_hand_target = (neck[0] + 100, neck[1] + 50)

    # If the user is actively drawing a path, override the RIGHT HAND IK target
    if puppet_is_active and len(puppet_path_queue) > 0:
        # Pop the next coordinate in the path
        coord = puppet_path_queue.pop(0)
        puppet_target_x = coord['x']
        puppet_target_y = coord['y']
        
        # Override the Right Hand Target
        r_hand_target = (puppet_target_x, puppet_target_y)
        
        if len(puppet_path_queue) == 0:
            puppet_is_active = False # Path finished
            
    elif not puppet_is_active and puppet_target_x != 0:
        # If the path finished, freeze the hand at the last drawn position
        r_hand_target = (puppet_target_x, puppet_target_y)
    else:
        # Fallback to automated idle ballet if no path has ever been drawn
        arm_lift = math.sin(t * math.pi) * 40
        l_hand_target = (neck[0] - 100 + arm_lift, neck[1] + 50 - arm_lift)
        r_hand_target = (neck[0] + 100 - arm_lift, neck[1] + 50 - arm_lift)
"""

code = code.replace(ik_old, ik_new)

with open(filepath, 'w') as f:
    f.write(code)

