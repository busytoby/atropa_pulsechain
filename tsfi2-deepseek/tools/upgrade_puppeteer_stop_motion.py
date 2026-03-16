import sys

filepath = 'tools/tsfi_spider_alligator_vtuber.py'
with open(filepath, 'r') as f:
    code = f.read()

# 1. Update the UI to reflect a true Stop Motion Ballet system.
# Instead of drawing a single path, the user acts as a choreographer placing "Keyframes" (poses).
# The system will automatically construct the 15-step ballet interpolation between these keyframes.

html_old = """                <div style="margin-top:10px;">
                    <button onclick="clearScribble()">CLEAR PATH</button>
                    <button onclick="sendPathToPuppet()">PUPPETEER EXECUTE</button>
                </div>"""

html_new = """                <div style="margin-top:10px;">
                    <button onclick="clearScribble()">CLEAR POSE</button>
                    <button onclick="saveKeyframe()">SAVE KEYFRAME (<span id="kfCount">0</span>/15)</button>
                    <button onclick="sendBalletToPuppet()" style="background:#800080;">EXECUTE 15-STEP BALLET</button>
                </div>"""

code = code.replace(html_old, html_new)

# 2. Update JavaScript to collect keyframes (specific target coordinates) instead of a messy continuous path.
js_old = """                    window.sendPathToPuppet = function() {
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

js_new = """                    let keyframes = [];
                    
                    window.saveKeyframe = function() {
                        if(pathData.length === 0) return;
                        // Grab the very last coordinate drawn as the definitive IK target for this pose
                        const target = pathData[pathData.length - 1];
                        keyframes.push(target);
                        document.getElementById('kfCount').innerText = keyframes.length;
                        
                        // Draw a permanent red dot on the UI to signify the keyframe lock
                        sCtx.fillStyle = 'red';
                        sCtx.beginPath();
                        sCtx.arc(target.x, target.y, 8, 0, Math.PI*2);
                        sCtx.fill();
                        
                        pathData = []; // Reset for next stroke
                    };

                    window.sendBalletToPuppet = function() {
                        if(keyframes.length < 2) {
                            alert("A ballet requires at least 2 keyframes to interpolate.");
                            return;
                        }
                        
                        fetch('/puppet_ballet', {
                            method: 'POST',
                            headers: { 'Content-Type': 'application/json' },
                            body: JSON.stringify({ keyframes: keyframes })
                        }).then(r => {
                            if(r.ok) {
                                document.getElementById('telemetry').innerText = "Ballet Choreography Uploaded! Executing 15-Step Interpolation...";
                                sCtx.clearRect(0, 0, 512, 512);
                                keyframes = [];
                                document.getElementById('kfCount').innerText = "0";
                            }
                        });
                    };"""

code = code.replace(js_old, js_new)

# 3. Update the Python HTTP Handler to receive the 15-step Ballet array
http_old = """        elif self.path == '/puppet_path':
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
                self.end_headers()"""

http_new = """        elif self.path == '/puppet_ballet':
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length).decode('utf-8')
            try:
                data = json.loads(post_data)
                global ballet_keyframes, ballet_active, ballet_current_step, ballet_frame_counter
                
                raw_keyframes = data.get("keyframes", [])
                
                # We need to mathematically interpolate these keyframes across a 15-step ballet.
                # If they gave us 3 keyframes, we generate 15 total frames moving smoothly between them.
                ballet_keyframes = []
                TOTAL_BALLET_STEPS = 15
                segments = len(raw_keyframes) - 1
                steps_per_segment = TOTAL_BALLET_STEPS // segments
                
                for i in range(segments):
                    p1 = raw_keyframes[i]
                    p2 = raw_keyframes[i+1]
                    for step in range(steps_per_segment):
                        # Linear interpolation for now, could be upgraded to Spline/Sine easing
                        t = step / float(steps_per_segment)
                        interp_x = p1['x'] + (p2['x'] - p1['x']) * t
                        interp_y = p1['y'] + (p2['y'] - p1['y']) * t
                        ballet_keyframes.append({'x': interp_x, 'y': interp_y})
                
                # Ensure the final keyframe is perfectly locked
                ballet_keyframes.append(raw_keyframes[-1])
                
                if len(ballet_keyframes) > 0:
                    ballet_active = True
                    ballet_current_step = 0
                    ballet_frame_counter = 0
                
                self.send_response(200)
                self.end_headers()
            except Exception as e:
                self.send_response(500)
                self.end_headers()"""

code = code.replace(http_old, http_new)

# 4. Update the Python Globals
backend_old = """# Puppeteer Globals
puppet_path_queue = []
puppet_is_active = False
puppet_target_x = 0
puppet_target_y = 0"""

backend_new = """# Ballet Kinematics Globals
ballet_keyframes = []
ballet_active = False
ballet_current_step = 0
ballet_frame_counter = 0
puppet_target_x = 0
puppet_target_y = 0"""

code = code.replace(backend_old, backend_new)


# 5. Wire the IK Solver to the new 15-step Ballet routine
ik_old = """    global puppet_is_active, puppet_path_queue, puppet_target_x, puppet_target_y
    
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
        r_hand_target = (neck[0] + 100 - arm_lift, neck[1] + 50 - arm_lift)"""

ik_new = """    global ballet_active, ballet_keyframes, ballet_current_step, ballet_frame_counter, puppet_target_x, puppet_target_y
    
    # Base Idle State
    l_foot_target = (hip[0] - 30, hip[1] + 130)
    r_foot_target_x = hip[0] + 40
    r_foot_target_y = hip[1] + 130
    l_hand_target = (neck[0] - 100, neck[1] + 50)
    r_hand_target = (neck[0] + 100, neck[1] + 50)

    # If a custom Ballet was uploaded, execute the 15 steps strictly
    if ballet_active and ballet_current_step < len(ballet_keyframes):
        # We hold each interpolated step for 1 frame (can increase to slow down)
        coord = ballet_keyframes[ballet_current_step]
        puppet_target_x = coord['x']
        puppet_target_y = coord['y']
        
        r_hand_target = (puppet_target_x, puppet_target_y)
        
        ballet_frame_counter += 1
        if ballet_frame_counter >= 1: # Move to next keyframe step every 1 frame
            ballet_current_step += 1
            ballet_frame_counter = 0
            
        if ballet_current_step >= len(ballet_keyframes):
            ballet_active = False # Ballet routine finished
            
    elif not ballet_active and puppet_target_x != 0:
        # If the ballet is finished, freeze the limb in the final dramatic pose
        r_hand_target = (puppet_target_x, puppet_target_y)
    else:
        # Fallback automated breathing
        arm_lift = math.sin(t * math.pi) * 40
        l_hand_target = (neck[0] - 100 + arm_lift, neck[1] + 50 - arm_lift)
        r_hand_target = (neck[0] + 100 - arm_lift, neck[1] + 50 - arm_lift)"""

code = code.replace(ik_old, ik_new)

with open(filepath, 'w') as f:
    f.write(code)

