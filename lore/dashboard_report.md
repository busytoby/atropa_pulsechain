# Atropa Ecosystem Knowledge Graph Dashboard

An interactive visualization has been generated for all mapped tokens in [addresses.sol](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/addresses.sol) and their liquidity pool connections on PulseChain.

*   **HTML Dashboard File**: [atropa_dashboard.html](file:///home/mariarahel/.gemini/antigravity-cli/brain/77cd3e01-149d-4c76-ab63-d64bf3f503e6/atropa_dashboard.html)
*   **Total Logged Tokens**: 378
*   **Total Active LP Connections**: 246

---

### Dashboard Features

1.  **Physics-Based Layout**: Uses `vis.js` network algorithms to organize the complex topology of the ecosystem automatically.
2.  **Color Coding**:
    *   **Atropa** is rendered as a prominent pink/red node (`#ff0055`).
    *   **dOWN** is highlighted as a glowing cyan node (`#00f2fe`).
    *   Other active tokens are depicted as blue nodes.
3.  **Variable Mapping**: Every node is tagged with its Solidity variable name as declared in the smart contracts (e.g. `HegelBetContract`, `KaContract`).
4.  **Search Feature**: The search bar in the sidebar lets you filter by name, symbol, address, or Solidity constant. It centers and focuses on the node automatically.
5.  **Interactive Sidebar Details**: Clicking any token displays its metadata alongside all of its active pool connections and prices.
