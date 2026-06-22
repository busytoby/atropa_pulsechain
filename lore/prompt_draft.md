# Teamwork Project Prompt — Draft

> Status: Launched (Conversation ID: 2ea803de-9c86-4141-a854-fff5f8f7462c)
> Goal: Craft prompt → get user approval → delegate to teamwork_preview

An interactive real-time single-page web dashboard displaying the token prices, reserves depth, swap telemetry, and transaction volumes for the 480 identified pools of the NoNukes token family ecosystem.

Working directory: /home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/nonukes_dashboard
Integrity mode: benchmark

## Requirements

### R1. Interactive Frontend Dashboard SPA
Implement a premium, responsive single-page visual dashboard application built using Vanilla HTML, CSS, and modern Javascript. The interface must dynamically present the list of pools, highlight those associated with each of the 7 NoNukes child tokens, and display charts or visual graphs for real-time telemetry (prices, swap counts, reserve depths).

### R2. Backend API Integration
Connect the dashboard directly to the existing Python server API to query state, load `nonukes_pools.json`, and fetch the telemetry data in real-time. 

### R3. Premium Design System
Apply rich visual aesthetics: a dark-mode tailormade color palette (e.g. sleek HSL gradients, glassmorphism), premium typography, and responsive grid layouts. Avoid default browser elements.

## Acceptance Criteria

### Core Dashboard Interface Verification
- [ ] A clean visual dashboard HTML page must load in the workspace without any Console errors.
- [ ] The app must render an interactive table/list showing all 480 pools with their respective target token group, version, reserves, and swap counts.
- [ ] Clicking on a pool must display a details panel containing real-time price trends or reserve balances.
- [ ] The frontend must fetch its data dynamically from the python server API without hardcoded mockup lists.
