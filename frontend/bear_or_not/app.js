document.addEventListener('DOMContentLoaded', () => {
    // Current Active genome state
    let state = {
        fur_r: 120,
        fur_g: 120,
        fur_b: 120,
        fur_len: 150, // Mapping 150 -> 0.150m
        scale: 120,   // Mapping 120 -> 1.20x
        twitch_intensity: 10,
        sickness_intensity: 0,
        mutation_intensity: 30,
        phenotype: "brown",
        score: 85,
        history: [
            { id: "0x89178a88...", event: "GENOME_INITIALIZED", score: "80", status: "active" },
            { id: "0x156ba3d8...", event: "COAXIAL_MODULATION", score: "92", status: "active" }
        ]
    };

    // UI elements
    const elements = {
        fur_r: document.getElementById('lbl-fur-r'),
        fur_g: document.getElementById('lbl-fur-g'),
        fur_b: document.getElementById('lbl-fur-b'),
        fur_len: document.getElementById('lbl-fur-len'),
        scale: document.getElementById('lbl-scale'),
        twitch: document.getElementById('lbl-twitch'),
        sickness: document.getElementById('lbl-sickness'),
        mutation_label: document.getElementById('lbl-mutation'),
        mutation_slider: document.getElementById('slide-mutation'),
        slide_r: document.getElementById('slide-fur-r'),
        slide_g: document.getElementById('slide-fur-g'),
        slide_b: document.getElementById('slide-fur-b'),
        slide_len: document.getElementById('slide-fur-len'),
        slide_scale: document.getElementById('slide-scale'),
        slide_twitch: document.getElementById('slide-twitch'),
        slide_sickness: document.getElementById('slide-sickness'),
        select_phenotype: document.getElementById('select-phenotype'),
        btn_bear: document.getElementById('btn-bear'),
        btn_not_bear: document.getElementById('btn-not-bear'),
        btn_evolve: document.getElementById('btn-evolve'),
        leaderboard: document.getElementById('leaderboard-body'),
        bear_image: document.getElementById('bear-image')
    };

    function updateUI() {
        elements.fur_r.innerText = state.fur_r;
        elements.fur_g.innerText = state.fur_g;
        elements.fur_b.innerText = state.fur_b;
        elements.fur_len.innerText = (state.fur_len / 1000).toFixed(3) + 'm';
        elements.scale.innerText = (state.scale / 100).toFixed(2) + 'x';
        elements.twitch.innerText = state.twitch_intensity;
        elements.sickness.innerText = state.sickness_intensity;
        elements.mutation_label.innerText = state.mutation_intensity + '%';

        // Keep slider thumb positions and dropdown selection synchronized with state
        elements.slide_r.value = state.fur_r;
        elements.slide_g.value = state.fur_g;
        elements.slide_b.value = state.fur_b;
        elements.slide_len.value = state.fur_len;
        elements.slide_scale.value = state.scale;
        elements.slide_twitch.value = state.twitch_intensity;
        elements.slide_sickness.value = state.sickness_intensity;
        elements.select_phenotype.value = state.phenotype;

        // Use the procedurally selected base image phenotype
        if (state.phenotype === 'crimson') {
            elements.bear_image.src = "assets/crimson_bear.jpg";
        } else if (state.phenotype === 'gray') {
            elements.bear_image.src = "assets/gray_bear.jpg";
        } else {
            elements.bear_image.src = "assets/teddy_render.jpg";
        }

        // Real-time Visual Modulation: map exact genome values to CSS filters and scaling
        // R, G, B are combined to calculate a smooth, continuous hue rotation angle
        const hueShift = ((state.fur_r * 2.5 + state.fur_g - state.fur_b * 1.5) + 360) % 360;
        const satPercent = 50 + (state.fur_g / 255 * 100);
        const brightPercent = 70 + (state.fur_b / 255 * 50);
        const scaleVal = 0.75 + (state.scale / 250);
        
        // Map fur length to a subtle soft blur filter representing the fuzzy density of long fur filaments
        const furBlur = (state.fur_len / 250) * 1.6;

        // Map scaleVal CSS variable to drive the twitch jitter keyframe reference
        elements.bear_image.style.setProperty('--bear-scale', scaleVal);

        // Dynamic Sickness Filter: pulsing toxic green chromatic glow shadow
        let filterStr = `hue-rotate(${hueShift}deg) saturate(${satPercent}%) brightness(${brightPercent}%) blur(${furBlur}px)`;
        if (state.sickness_intensity > 0) {
            const glowRadius = Math.max(2, state.sickness_intensity / 10);
            filterStr += ` drop-shadow(0 0 ${glowRadius}px rgba(16, 185, 129, 0.75))`;
        }
        elements.bear_image.style.filter = filterStr;

        // Dynamic Twitch Animation: jitter frequency scales with twitch intensity
        if (state.twitch_intensity > 0) {
            const jitterDuration = 0.15 + (255 - state.twitch_intensity) / 255 * 0.85; // Speed increases with twitch
            elements.bear_image.style.animation = `twitch-jitter ${jitterDuration}s infinite linear`;
        } else {
            elements.bear_image.style.animation = 'none';
            elements.bear_image.style.transform = `scale(${scaleVal})`;
        }

        // Render activity table
        elements.leaderboard.innerHTML = state.history.map(item => `
            <tr>
                <td style="font-family: monospace; color: var(--primary); font-weight: 600;">${item.id}</td>
                <td>${item.event}</td>
                <td style="font-weight: bold;">${item.score}</td>
                <td><span class="status-badge status-${item.status}">${item.status.toUpperCase()}</span></td>
            </tr>
        `).join('');
    }

    // Monitor Mutation slider shifts
    elements.mutation_slider.addEventListener('input', (e) => {
        state.mutation_intensity = parseInt(e.target.value);
        elements.mutation_label.innerText = state.mutation_intensity + '%';
    });

    // Real-time Slider input bindings to immediately adjust visual rendering
    elements.slide_r.addEventListener('input', (e) => { state.fur_r = parseInt(e.target.value); updateUI(); });
    elements.slide_g.addEventListener('input', (e) => { state.fur_g = parseInt(e.target.value); updateUI(); });
    elements.slide_b.addEventListener('input', (e) => { state.fur_b = parseInt(e.target.value); updateUI(); });
    elements.slide_len.addEventListener('input', (e) => { state.fur_len = parseInt(e.target.value); updateUI(); });
    elements.slide_scale.addEventListener('input', (e) => { state.scale = parseInt(e.target.value); updateUI(); });
    elements.slide_twitch.addEventListener('input', (e) => { state.twitch_intensity = parseInt(e.target.value); updateUI(); });
    elements.slide_sickness.addEventListener('input', (e) => { state.sickness_intensity = parseInt(e.target.value); updateUI(); });

    // Bind Phenotype selector changes to update the base bear phenotype render instantly
    elements.select_phenotype.addEventListener('change', (e) => {
        state.phenotype = e.target.value;
        updateUI();
    });
    elements.btn_bear.addEventListener('click', () => {
        state.score += 5;
        if (state.score > 100) state.score = 100;
        
        // Push activity event
        const mockHash = "0x" + Math.random().toString(16).slice(2, 10) + "b8a6...";
        const voteEvent = {
            id: mockHash,
            event: "VOTE_UP_BEAR",
            score: state.score.toString(),
            status: "active",
            genome: { fur_r: state.fur_r, fur_g: state.fur_g, fur_b: state.fur_b, scale: state.scale }
        };
        state.history.unshift(voteEvent);
        
        // Dynamic minor color shift to prove dynamic rendering reaction
        state.fur_r = Math.min(255, state.fur_r + 10);
        state.fur_g = Math.min(255, state.fur_g + 5);
        state.twitch_intensity = Math.min(255, state.twitch_intensity + 8);
        updateUI();

        // POST to backend API
        fetch('/api/vote', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(voteEvent)
        }).catch(err => console.warn("Local fallback: backend ledger unreachable."));
    });

    // Downvote/Ban Action
    elements.btn_not_bear.addEventListener('click', () => {
        state.score = Math.max(0, state.score - 20);
        
        // Push quarantine event (triggers Yul Rule 9001 simulation)
        const mockHash = "0x" + Math.random().toString(16).slice(2, 10) + "9001...";
        const banEvent = {
            id: mockHash,
            event: "VOTE_DOWN_QUARANTINE",
            score: state.score.toString(),
            status: "quarantined",
            genome: { fur_r: state.fur_r, fur_g: state.fur_g, fur_b: state.fur_b, scale: state.scale }
        };
        state.history.unshift(banEvent);

        // Shift parameters to cold gray and increment sickness
        state.fur_r = 100;
        state.fur_g = 100;
        state.fur_b = 100;
        state.sickness_intensity = Math.min(255, state.sickness_intensity + 50);
        state.twitch_intensity = 0; // Freeze movement
        updateUI();

        // POST to backend API
        fetch('/api/vote', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(banEvent)
        }).catch(err => console.warn("Local fallback: backend ledger unreachable."));
    });

    // Evolve action (fetching from real predictive backend compiler)
    elements.btn_evolve.addEventListener('click', () => {
        fetch('/api/evolve', { method: 'POST' })
            .then(res => res.json())
            .then(data => {
                state.fur_r = data.fur_r;
                state.fur_g = data.fur_g;
                state.fur_b = data.fur_b;
                state.fur_len = data.fur_len;
                state.scale = data.scale;
                state.twitch_intensity = data.twitch_intensity;
                state.sickness_intensity = data.sickness_intensity;
                state.phenotype = data.phenotype;
                
                state.history.unshift({
                    id: data.id,
                    event: "DEEPSEEK_GENOME_EVOLVED",
                    score: state.score.toString(),
                    status: "active"
                });
                updateUI();
            })
            .catch(err => {
                console.warn("Server offline. Running local evolution fallback.");
                // Local Fallback simulation
                const m = state.mutation_intensity / 100;
                const phenotypes = ['crimson', 'gray', 'brown'];
                const chosen = phenotypes[Math.floor(Math.random() * phenotypes.length)];
                state.phenotype = chosen;

                if (chosen === 'crimson') {
                    state.fur_r = 180 + Math.floor((Math.random() - 0.5) * 40 * m);
                    state.fur_g = 20 + Math.floor(Math.random() * 30 * m);
                    state.fur_b = 20 + Math.floor(Math.random() * 30 * m);
                    state.fur_len = 180 + Math.floor((Math.random() - 0.5) * 60 * m);
                    state.scale = 135 + Math.floor((Math.random() - 0.5) * 40 * m);
                    state.twitch_intensity = Math.floor(20 + Math.random() * 80 * m);
                    state.sickness_intensity = Math.max(0, state.sickness_intensity - 15);
                } else if (chosen === 'gray') {
                    state.fur_r = 80 + Math.floor((Math.random() - 0.5) * 30 * m);
                    state.fur_g = 80 + Math.floor(Math.random() * 30 * m);
                    state.fur_b = 80 + Math.floor(Math.random() * 30 * m);
                    state.fur_len = 45 + Math.floor((Math.random() - 0.5) * 20 * m);
                    state.scale = 90 + Math.floor((Math.random() - 0.5) * 20 * m);
                    state.twitch_intensity = Math.floor(Math.random() * 20 * m);
                    state.sickness_intensity = Math.floor(Math.random() * 40 * m);
                } else {
                    state.fur_r = 125 + Math.floor((Math.random() - 0.5) * 30 * m);
                    state.fur_g = 125 + Math.floor(Math.random() * 30 * m);
                    state.fur_b = 125 + Math.floor(Math.random() * 30 * m);
                    state.fur_len = 110 + Math.floor((Math.random() - 0.5) * 40 * m);
                    state.scale = 105 + Math.floor((Math.random() - 0.5) * 30 * m);
                    state.twitch_intensity = Math.floor(10 + Math.random() * 40 * m);
                    state.sickness_intensity = Math.max(0, state.sickness_intensity - 10);
                }

                state.history.unshift({
                    id: "0x" + Math.random().toString(16).slice(2, 10) + "ace8...",
                    event: "LOCAL_GENOME_EVOLVED",
                    score: state.score.toString(),
                    status: "active"
                });
                updateUI();
            });
    });

    // Initial load
    updateUI();
});
