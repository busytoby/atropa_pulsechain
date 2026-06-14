# Semantic Namespace Standard: Resolving the "SSR" Terminology Collision

This document establishes a namespacing and design pattern standard to resolve the abbreviation collision of **SSR** between modern web engineering and physical synthesis modeling.

---

## 1. The Conflict: Two Worlds, One Acronym

In the **TSFi2 Synthesis Studio** codebase, "SSR" refers to two completely different architectures:

1. **Server-Side Rendering (SSR)**: The generation of HTML and application state on a Node.js server before delivering it to the client web browser.
2. **Solid-State Relay (SSR)**: A physical circuit simulator that routes control signals and shunts currents using opto-triac zero-crossing logic.

Without a strict semantic separation, code symbols, UI labels, and developer telemetry logs risk confusing the two.

---

## 2. Namespace Compatibility Layer

To prevent collision, the following naming standard is implemented across all codebase files:

### A. Code Symbol Namespacing
* All Web Server-Side Rendering classes and variables must be suffixed or prefixed with `Web` or `Render` (e.g., `ssrWebRenderer`, `isWebSsrActive`).
* All Solid-State Relay circuit simulation classes and variables must be suffixed or prefixed with `Relay` or `Opto` (e.g., `ssrRelayActive`, `zeroCrossSsrRelay`).

### B. Directory and Routing Structures
* Web SSR files are grouped inside `server/ssr/` or `frontend/ssr/`.
* Relay physical model configurations are placed in `solidity/bin/` or under the `bionika` physical model variables inside `frontend/studio.html`.

### C. Comparative Vocabulary Reference

| Domain | Abbreviation | Standard Term | Code Symbol Example |
| :--- | :--- | :--- | :--- |
| **Web App** | `SSR` | **Server-Side Rendering** | `renderServerPage()` |
| **Electronics** | `SSR` | **Solid-State Relay** | `triggerSsrRelay()` |
