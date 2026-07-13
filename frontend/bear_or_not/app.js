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
        bar_r: document.getElementById('bar-fur-r'),
        bar_g: document.getElementById('bar-fur-g'),
        bar_b: document.getElementById('bar-fur-b'),
        bar_len: document.getElementById('bar-fur-len'),
        bar_scale: document.getElementById('bar-scale'),
        bar_twitch: document.getElementById('bar-twitch'),
        bar_sickness: document.getElementById('bar-sickness'),
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

        elements.bar_r.style.width = (state.fur_r / 255 * 100) + '%';
        elements.bar_g.style.width = (state.fur_g / 255 * 100) + '%';
        elements.bar_b.style.width = (state.fur_b / 255 * 100) + '%';
        elements.bar_len.style.width = (state.fur_len / 255 * 100) + '%';
        elements.bar_scale.style.width = (state.scale / 200 * 100) + '%';
        elements.bar_twitch.style.width = (state.twitch_intensity / 255 * 100) + '%';
        elements.bar_sickness.style.width = (state.sickness_intensity / 255 * 100) + '%';

        // Dynamically select the base image phenotype
        if (state.fur_r > 150) {
            elements.bear_image.src = "assets/crimson_bear.jpg";
        } else if (state.fur_r <= 110) {
            elements.bear_image.src = "assets/gray_bear.jpg";
        } else {
            elements.bear_image.src = "assets/teddy_render.jpg";
        }

        // Real-time Visual Modulation: map exact genome values to CSS filters and scaling
        const hueShift = ((state.fur_r + state.fur_g + state.fur_b) * 1.5) % 360;
        const satPercent = 60 + (state.fur_g / 255 * 80);
        const brightPercent = 70 + (state.fur_b / 255 * 40);
        const scaleVal = 0.75 + (state.scale / 250);

        // Map scaleVal CSS variable to drive the twitch jitter keyframe reference
        elements.bear_image.style.setProperty('--bear-scale', scaleVal);

        // Dynamic Sickness Filter: pulsing toxic green chromatic glow shadow
        let filterStr = `hue-rotate(${hueShift}deg) saturate(${satPercent}%) brightness(${brightPercent}%)`;
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

    // Upvote Bear Action
    elements.btn_bear.addEventListener('click', () => {
        state.score += 5;
        if (state.score > 100) state.score = 100;
        
        // Push activity event
        const mockHash = "0x" + Math.random().toString(16).slice(2, 10) + "b8a6...";
        state.history.unshift({
            id: mockHash,
            event: "VOTE_UP_BEAR",
            score: state.score.toString(),
            status: "active"
        });
        
        // Dynamic minor color shift to prove dynamic rendering reaction
        state.fur_r = Math.min(255, state.fur_r + 10);
        state.fur_g = Math.min(255, state.fur_g + 5);
        state.twitch_intensity = Math.min(255, state.twitch_intensity + 8);
        updateUI();
    });

    // Downvote/Ban Action
    elements.btn_not_bear.addEventListener('click', () => {
        state.score = Math.max(0, state.score - 20);
        
        // Push quarantine event (triggers Yul Rule 9001 simulation)
        const mockHash = "0x" + Math.random().toString(16).slice(2, 10) + "9001...";
        state.history.unshift({
            id: mockHash,
            event: "VOTE_DOWN_QUARANTINE",
            score: state.score.toString(),
            status: "quarantined"
        });

        // Shift parameters to cold gray and increment sickness
        state.fur_r = 100;
        state.fur_g = 100;
        state.fur_b = 100;
        state.sickness_intensity = Math.min(255, state.sickness_intensity + 50);
        state.twitch_intensity = 0; // Freeze movement
        updateUI();
    });

    // Evolve action (simulating predictive compiler calculations)
    elements.btn_evolve.addEventListener('click', () => {
        // Mutation multiplier scaled by the Mutation Slider
        const m = state.mutation_intensity / 100;

        const phenotypes = ['crimson', 'gray', 'brown'];
        const chosen = phenotypes[Math.floor(Math.random() * phenotypes.length)];

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
            // Default Brown/Original
            state.fur_r = 125 + Math.floor((Math.random() - 0.5) * 30 * m);
            state.fur_g = 125 + Math.floor(Math.random() * 30 * m);
            state.fur_b = 125 + Math.floor(Math.random() * 30 * m);
            state.fur_len = 110 + Math.floor((Math.random() - 0.5) * 40 * m);
            state.scale = 105 + Math.floor((Math.random() - 0.5) * 30 * m);
            state.twitch_intensity = Math.floor(10 + Math.random() * 40 * m);
            state.sickness_intensity = Math.max(0, state.sickness_intensity - 10);
        }

        const mockHash = "0x" + Math.random().toString(16).slice(2, 10) + "ace8...";
        state.history.unshift({
            id: mockHash,
            event: "DEEPSEEK_GENOME_EVOLVED",
            score: state.score.toString(),
            status: "active"
        });

        updateUI();
    });

    // Initial load
    updateUI();
});
