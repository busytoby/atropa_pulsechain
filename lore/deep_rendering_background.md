# Background Deep Rendering Architecture

To provide the best user experience, we implemented a **two-stage hybrid rendering pipeline** for the card synthesizer. 

---

## 1. How It Works

When a user clicks "Synthesize Art" on the card viewer:

```
[User Click] ──> [Stage 1: Fast Render (4 Steps)] ──> [Instant UI Return (~5.8s)]
                                                             │
                                                     (Spawns Detached Process)
                                                             │
                                                             ▼
                                                [Stage 2: Deep Render (15 Steps)] ──> [Overwrite File with High-Quality Art]
```

### Stage 1: Fast Render (Euler A, 4 Steps)
- **Objective:** Get a draft render of the card to the user as quickly as possible.
- **Latency:** **~5.8 seconds** (fully pipelined concurrently).
- **Outcome:** The generated art is returned immediately to the frontend and displayed to the user.

### Stage 2: Deep Render (Euler A, 15 Steps)
- **Objective:** Perform high-fidelity denoising iterations to generate detailed, high-quality vector artwork.
- **Process:** Spawns a fully detached Python process running concurrently in the background (`--is-deep-render`).
- **Outcome:** Once finished, it overwrites the generated assets (e.g. `assets/storybook/page_dragon_dna.png` and `assets/{address}.png`) with the high-quality, 15-step masterpiece. The next time the user loads or views the card, it displays the high-quality render.

---

## 2. Command Line Invocation

If you run the script manually, the background deep render trigger runs by default when `--steps <= 4`:
```bash
python3 scripts/render_vlm_synthesizer.py 1 --steps 4 --address <address>
```

To prevent infinite loops, the background process is spawned with the `--is-deep-render` flag, which forces it to execute synchronously without spawning another subprocess:
```bash
python3 scripts/render_vlm_synthesizer.py 1 --steps 15 --is-deep-render --address <address>
```
