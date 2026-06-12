# 🧠 Bayesian Filter Networks for Preference-Based Organic Arpeggiator Control

---

## Abstract
This document outlines the theoretical and architectural design for a **Bayesian Filter Network of Arpeggiators**. By observing real-time musical performance telemetry (key velocity, note density, held durations, and manual parameter tweaks), the network recursively updates a probability distribution of user aesthetic preferences. This posterior distribution maps directly to the mutation vectors of the **Biotika DNA Engine**, allowing the agentic DJ AI to organically adapt its synthesis, envelope shaping, and cascading arpeggiator parameters to the performer's intent without explicit rating feedback.

---

## 1. System Architecture

The Bayesian control loop operates as a state estimator that tracks the performer's hidden aesthetic preference state ($\mathbf{x}_t$) based on observable play vectors ($\mathbf{y}_t$):

```mermaid
graph TD
    %% Styling
    classDef user fill:#38bdf8,stroke:#0284c7,stroke-width:2px;
    classDef filter fill:#a855f7,stroke:#7e22ce,stroke-width:2px;
    classDef dna fill:#ec4899,stroke:#be185d,stroke-width:2px;
    classDef synth fill:#f59e0b,stroke:#b45309,stroke-width:2px;

    %% User Interaction
    subgraph Input ["1. Performer Interface"]
        Keys["Piano Keys / MIDI Telemetry"]:::user
        Knobs["Parameter Knobs (BPM, Pitch Blend)"]:::user
    end

    %% Observation Vector
    Input -->|Observable Telemetry y_t| Observ["Observation Vector Extraction"]

    %% Bayesian Filter
    subgraph BayesianFilter ["2. Bayesian Estimation Network"]
        Prior["Prior Distribution P(x_t-1)"]:::filter
        Likelihood["Interaction Likelihood model P(y_t | x_t)"]:::filter
        Posterior["Posterior Preference P(x_t | y_t)"]:::filter
        
        Observ --> Likelihood
        Prior & Likelihood -->|Recursive Update| Posterior
    end

    %% DNA Modulation
    subgraph GeneticLoop ["3. Organic Control Net"]
        DNA["TsfiBiotikaDna (8 Genes)"]:::dna
        Mutation["Preference-Steered Mutation Vector"]:::dna
        Posterior -->|Expected Values E[x]| Mutation
        Mutation -->|Shift Operating Point| DNA
    end

    %% Output
    subgraph Synthesis ["4. Transduced Sound Engine"]
        Arp["Cascading Arpeggiators (One YI)"]:::synth
        Osc["Wien-Bridge / Transducer Stage"]:::synth
        DNA -->|Synthesis Presets| Osc
        DNA -->|ADSR & LFO Rates| Arp
    end
    
    Osc & Arp -->|Acoustic Feedback| Input
```

---

## 2. Mathematical Formulation

### 2.1 Hidden Preference State Space ($\mathbf{x}_t$)
We define the performer's aesthetic preference state at time $t$ as a vector:
$$\mathbf{x}_t = [x_{tempo}, x_{entropy}, x_{timbre}, x_{envelope}]^T \in [0.0, 1.0]^4$$
Where:
* $x_{tempo}$: Preference for high-speed cascading rhythms vs. slow, sparse chords.
* $x_{entropy}$: Preference for chaotic/random arpeggiation patterns vs. predictable ascending sequences.
* $x_{timbre}$: Preference for highly distorted, saturated Germanium transistor overtones vs. clean, fundamental sinusoidal tones.
* $x_{envelope}$: Preference for long, ambient release tails vs. sharp, staccato plucks.

### 2.2 Observable Interaction Vector ($\mathbf{y}_t$)
The network measures the following observable features over a sliding $3$-second window:
$$\mathbf{y}_t = [y_{density}, y_{held\_time}, y_{pitch\_tension}, y_{tweak\_rate}]^T$$
Where:
* $y_{density}$: The frequency of note-on events per second.
* $y_{held\_time}$: The mean duration of key-down states before release.
* $y_{pitch\_tension}$: The variance of the interval offsets between held notes.
* $y_{tweak\_rate}$: The frequency of manual slider adjustments on the dashboard.

### 2.3 Recursive Bayesian Filtering
The network updates the posterior probability density function of the preference state using a recursive Bayes filter:
$$P(\mathbf{x}_t | \mathbf{y}_{1:t}) = \eta \cdot P(\mathbf{y}_t | \mathbf{x}_t) \int P(\mathbf{x}_t | \mathbf{x}_{t-1}) \cdot P(\mathbf{x}_{t-1} | \mathbf{y}_{1:t-1}) \, d\mathbf{x}_{t-1}$$
Where:
* $\eta$ is a normalization constant: $\eta = 1 / P(\mathbf{y}_t | \mathbf{y}_{1:t-1})$.
* $P(\mathbf{x}_t | \mathbf{x}_{t-1})$ is the transition probability (modeling temporal drift in user mood/preference).
* $P(\mathbf{y}_t | \mathbf{x}_t)$ is the likelihood function mapping how likely a play style $\mathbf{y}_t$ is under a specific preference $\mathbf{x}_t$.

The likelihood function is modeled as a multivariate Gaussian:
$$P(\mathbf{y}_t | \mathbf{x}_t) = \frac{1}{\sqrt{(2\pi)^k |\mathbf{\Sigma}|}} \exp \left( -\half (\mathbf{y}_t - \mathbf{h}(\mathbf{x}_t))^T \mathbf{\Sigma}^{-1} (\mathbf{y}_t - \mathbf{h}(\mathbf{x}_t)) \right)$$
Where $\mathbf{h}(\mathbf{x}_t)$ is the measurement model mapping preferences to expected playing behaviors, and $\mathbf{\Sigma}$ is the measurement noise covariance matrix.

---

## 3. Genetic Mutation & Control Net Mapping

Once the posterior distribution $P(\mathbf{x}_t | \mathbf{y}_{1:t})$ is calculated, we extract the expected value (mean preference vector) $\mathbf{\mu}_t = \mathbb{E}[\mathbf{x}_t]$:
$$\mathbf{\mu}_t = \int \mathbf{x}_t \cdot P(\mathbf{x}_t | \mathbf{y}_{1:t}) \, d\mathbf{x}_t$$

This expected vector dynamically modulates the mutation step inside the genetic algorithm of the Wien-Bridge and transistor simulation engines:
$$\mathbf{g}_{new} = \mathbf{g}_{old} + \mathbf{v}_{mutation} + \mathbf{v}_{bayesian}$$
Where the Bayesian steering vector $\mathbf{v}_{bayesian}$ pulls the active genes toward the user's inferred preference:
$$\mathbf{v}_{bayesian} = \alpha \cdot \left( \mathbf{M} \cdot \mathbf{\mu}_t - \mathbf{g}_{old} \right)$$
* $\alpha \in [0.0, 0.1]$ represents the coupling coefficient of the organic control net.
* $\mathbf{M}$ is a projection matrix mapping the 4-dimensional preference vector to the 8-dimensional gene space.

---

## 4. Practical Implementation Benefits

1.  **Zero-Configuration Personalization:** The synthesizer automatically shifts its base Germanium saturation limits and LFO depth profiles to match the player’s style (e.g., entering a harsh, fast, spacey state during rapid keys tapping, or smoothing out into a warm ambient pad when slow chords are held).
2.  **Cascading Alignment:** The arpeggiator cascading clocks (Arp A and Arp B) dynamically scale their rate multipliers ($arpBMult$) to match the tempo preference $x_{tempo}$, introducing complex rhythmic divisions only when the performer increases note play density.
3.  **Strict Parity Preservation:** Because the output remains bound to the single-voice transduction engine, this preference steering occurs without violating the **"One YI, One Phonic"** baseline.
