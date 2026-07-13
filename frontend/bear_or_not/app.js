document.addEventListener('DOMContentLoaded', () => {
    // Current Active genome state
    let state = {
        fur_r: 120,
        fur_g: 120,
        fur_b: 120,
        fur_len: 150, // Mapping 150 -> 0.150m
        scale: 120,   // Mapping 120 -> 1.20x
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
        bar_r: document.getElementById('bar-fur-r'),
        bar_g: document.getElementById('bar-fur-g'),
        bar_b: document.getElementById('bar-fur-b'),
        bar_len: document.getElementById('bar-fur-len'),
        bar_scale: document.getElementById('bar-scale'),
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

        elements.bar_r.style.width = (state.fur_r / 255 * 100) + '%';
        elements.bar_g.style.width = (state.fur_g / 255 * 100) + '%';
        elements.bar_b.style.width = (state.fur_b / 255 * 100) + '%';
        elements.bar_len.style.width = (state.fur_len / 255 * 100) + '%';
        elements.bar_scale.style.width = (state.scale / 200 * 100) + '%';

        // Dynamically shift the rendered bear image based on active genome color ranges
        if (state.fur_r > 150) {
            elements.bear_image.src = "assets/crimson_bear.jpg";
        } else if (state.fur_r <= 110) {
            elements.bear_image.src = "assets/gray_bear.jpg";
        } else {
            elements.bear_image.src = "assets/teddy_render.jpg";
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

        // Shift parameters to cold gray
        state.fur_r = 100;
        state.fur_g = 100;
        state.fur_b = 100;
        updateUI();
    });

    // Evolve action (simulating predictive compiler calculations)
    elements.btn_evolve.addEventListener('click', () => {
        // Randomly select between the three visual bear phenotypes (Crimson, Gray, Brown) on each evolution click
        const phenotypes = ['crimson', 'gray', 'brown'];
        const chosen = phenotypes[Math.floor(Math.random() * phenotypes.length)];

        if (chosen === 'crimson') {
            state.fur_r = 180 + Math.floor((Math.random() - 0.5) * 20);
            state.fur_g = 20 + Math.floor(Math.random() * 15);
            state.fur_b = 20 + Math.floor(Math.random() * 15);
            state.fur_len = 180 + Math.floor((Math.random() - 0.5) * 30);
            state.scale = 135 + Math.floor((Math.random() - 0.5) * 20);
        } else if (chosen === 'gray') {
            state.fur_r = 80 + Math.floor((Math.random() - 0.5) * 20);
            state.fur_g = 80 + Math.floor(Math.random() * 15);
            state.fur_b = 80 + Math.floor(Math.random() * 15);
            state.fur_len = 45 + Math.floor((Math.random() - 0.5) * 10);
            state.scale = 90 + Math.floor((Math.random() - 0.5) * 10);
        } else {
            // Default Brown/Original
            state.fur_r = 125 + Math.floor((Math.random() - 0.5) * 15);
            state.fur_g = 125 + Math.floor(Math.random() * 15);
            state.fur_b = 125 + Math.floor(Math.random() * 15);
            state.fur_len = 110 + Math.floor((Math.random() - 0.5) * 20);
            state.scale = 105 + Math.floor((Math.random() - 0.5) * 15);
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
