# Vaesen Folklore Integration & Mechanics Review

The gothic Nordic folklore RPG theme of **Vaesen** is deeply woven into this project's code, styling, and gameplay logic. From CSS-themed variable schemes in the web-based zmachine interface to real-time physical/mental trauma mechanics running on an EVM/Yul-inspired state machine, this document reviews the architecture and execution of *Vaesen* features in the codebase.

---

## 1. Visual & Theme System (`zmachine.html`)
The frontend terminal styling uses a curated, premium gothic color scheme inspired by *Vaesen*:

```css
:root {
    --vaesen-dark: #12100e;       /* Deep twilight shadows */
    --vaesen-sepia: #1c1815;      /* Worn leather and lore books */
    --vaesen-parchment: #25201b;  /* Old scroll backgrounds */
    --vaesen-gold: #c5a059;       /* Muted brass, status borders */
    --vaesen-gold-dim: #8c7241;   /* Tarnished gold accents */
    --vaesen-cream: #e6dfd3;      /* Crisp text mimicking old ink paper */
    --vaesen-ink: #0d0c0a;        /* Black void background */
    --vaesen-muted: #a39785;      /* Faded ash-brown text */
}
```

---

## 2. Sprite Sheets and Art Style (`vaesen_sprite_graphics.md`)
The game assets represent gothic folklore archetypes using dynamic retro 2D pixel sprites:
1. **Dark Crow**: Mystical bird companion with neon red eyes and wingspans.
2. **Small Blue Forest Troll**: A grumpy, textured wild blue skin entity wearing a pointed white felt witch-like hat (a folkloric Smurf).
3. **Alchemist**: Hunched dark hooded figure representing the antagonist (a folkloric Gargamel).

---

## 3. Folklore CPU Registry Map
The Yul-native processor architecture stores character state values directly in EVM storage slots:

*   **`0x06` (`physical_trauma`)**: Level of bodily fatigue (0 = Normal, 1 = Exhausted, 2 = Battered, 3 = Broken).
*   **`0x07` (`mental_trauma`)**: Level of psychological horror (0 = Normal, 1 = Shaken, 2 = Terrified, 3 = Panicked).

---

## 4. Gameplay Engine Conditions (`test_wayland_smurfs.c`)

When running, the game loop retrieves these variables via register peeking and translates them into physical mechanics:

### 4.1 Physical Trauma Effects
*   **`Exhausted` (Level 1)**: Player movement speed is penalized by 33% (skips input evaluation every 3rd frame).
*   **`Battered` (Level 2)**: Player speed is penalized by 50% (skips input every 2nd frame).
*   **`Broken` (Level 3)**: Player is unable to jump.

### 4.2 Mental Trauma Effects
*   **`Terrified` (Level 2)**: Control inversion (pressing `A` moves right, `D` moves left).
*   **`Panicked` (Level 3)**: Periodic freezing (fear paralysis) for 8 frames every 20-frame cycle.

### 4.3 Visual Shaders & Micro-Animations
```c
// Fear-induced shiver: adds rapid coordinate jitter proportional to mental trauma
if (ment_trauma > 0) {
    draw_sx += (rand() % 3 - 1) * ment_trauma;
    draw_sy += (rand() % 3 - 1) * ment_trauma;
}

// Asymmetric walking limp: dips vertical offset deeper on the injured side
if (anim_state == 1) { // Walk Animation
    float bob = sinf(frame_counter * 0.6f);
    if (phys_trauma > 0 && bob < 0.0f) {
        draw_sy += (int)(4.0f * scale * phys_trauma);
    } else {
        draw_sy += (int)(bob * 2.0f * scale);
    }
}
```

### 4.4 Shadow Vignette rendering
The renderer runs a ray-distance attenuation shader. The light level decays outward from the player's amber lantern (180px radius), leaving everything else shrouded in the gothic forest darkness, except when near glowing mushrooms, crystals, or cage lights:

$$\text{Visibility} = 1.0 - \left( \frac{\text{dist}^2}{\text{radius}^2} \right)$$

### 4.5 Melancholic State
*   **Trigger**: Automatically engaged if player energy falls below 30.
*   **Effects**:
    *   **Movement Penalty**: Sluggishness (skips inputs every 3rd frame).
    *   **Visual Overlay**: Renders a floating `*Sigh*` text bubble above the player sprite during idle/walking phases.
    *   **HUD Status**: Fear status reports `^UMelancholic^C`.
