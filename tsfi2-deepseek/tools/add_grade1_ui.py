import sys
filepath = 'tools/tsfi_spider_alligator_vtuber.py'
with open(filepath, 'r') as f:
    code = f.read()

# Replace the old telemetry logic with a dynamic one that fetches the multi-party Grade 1 Truth status 
# directly from the shared memory block, allowing the front-end dashboard to display it.

telemetry_old = """        elif self.path == '/telemetry':
            self.send_response(200)
            self.send_header('Content-type', 'text/plain')
            self.end_headers()
            
            phase_str = "Sitting (Decay)"
            if global_frame_idx > 150: phase_str = "Standing Up & Walking"
            
            score = f"[LIVE] Entity: {current_genome} | Phase: {phase_str} | Epoch {global_frame_idx}/60"
            try:
                self.wfile.write(score.encode('utf-8'))
            except BrokenPipeError:
                pass"""

telemetry_new = """        elif self.path == '/telemetry':
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.end_headers()
            
            phase_str = "Sitting (Decay)"
            if global_frame_idx > 150: phase_str = "Standing Up & Walking"
            
            # Read from SHM if available
            alli_score = "WAITING"
            spider_score = "WAITING"
            ds_obs = "WAITING"
            genie_obs = "WAITING"
            
            if shm_reg is not None:
                alli_score = f"{shm_reg.alligator_form_ratio:.2f} (0.05-2.0)"
                spider_score = f"{shm_reg.spider_texture_sharpness:.2f} (>50.0)"
                ds_obs = shm_reg.agent_monologue.decode('utf-8', 'ignore').strip()
                genie_obs = shm_reg.genie_poetic_observation.decode('utf-8', 'ignore').strip()
            
            html_telem = f\"\"\"
            <div style="text-align: left; font-size: 0.9em; margin-top: 15px; border: 1px solid #333; padding: 10px; background: #0a0a0a;">
                <h3 style="color:#ff0; margin-top:0;">GRADE 1 AUDIT: ACOUSTIC PROOF SPACE</h3>
                <p><b>[GENIE]:</b> {genie_obs}</p>
                <p><b>[DEEPSEEK]:</b> {ds_obs}</p>
                <p><b>[ALLIGATOR]:</b> Form IoU Mass: {alli_score}</p>
                <p><b>[SPIDER]:</b> Texture Variance: {spider_score}</p>
                <hr style="border-color: #333;">
                <p style="color:#0ff;"><b>[SYSTEM]:</b> Epoch {global_frame_idx}/60 | Phase: {phase_str}</p>
            </div>
            \"\"\"
            
            try:
                self.wfile.write(html_telem.encode('utf-8'))
            except BrokenPipeError:
                pass"""

code = code.replace(telemetry_old, telemetry_new)

# Update the HTML div to handle HTML insertion instead of innerText
html_telemetry_old = """                    setInterval(() => {
                        fetch('/telemetry').then(r => r.text()).then(t => {
                            document.getElementById('telemetry').innerText = t;
                        });
                    }, 1000);"""

html_telemetry_new = """                    setInterval(() => {
                        fetch('/telemetry').then(r => r.text()).then(t => {
                            document.getElementById('telemetry').innerHTML = t;
                        });
                    }, 1000);"""

code = code.replace(html_telemetry_old, html_telemetry_new)

with open(filepath, 'w') as f:
    f.write(code)

