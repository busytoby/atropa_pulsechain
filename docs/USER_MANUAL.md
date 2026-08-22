# User Manual

This manual details how to navigate, configure, and utilize the NoNukes Dashboard interfaces.

---

## 1. NoNukes Dashboard Navigation

Access the dashboard via: `http://localhost:8080/nonukes/`

### A. Pool Listings Table
* **Search**: Use the top search bar to filter pools by name, token symbol, or contract address.
* **Filtering Tabs**: Select dedicated filter tabs to highlight pools associated with specific child tokens.
* **Sorting**: Click column headers (e.g. reserves, swap counts, or USD volume) to sort pool data.

### B. Details Viewer Panel
Click any pool row in the table to display the Details panel:
* **Reserve Depth**: Displays aligned balances for both paired tokens.
* **Volume Telemetry**: Highlights total USD volume metrics calculated from resolved swap lists.
* **Interactive Charts**: Plots price trends over time derived from transaction values.

### C. Live QING Telemetry Stream
* View the terminal widget to read real-time chat feeds and event logs broadcasted directly from **Auncient** WinchesterMQ contract registries on the blockchain.

---

## 2. Interactive Setup Wizard
Use the on-screen Setup Wizard tab to configure application profiles:
1. **Step 1 (Select Presets)**: Choose target tokens or presets (e.g. `slugcat` / `teddy_bear`).
2. **Step 2 (Parameters)**: Fine-tune parameters that modulate coordinate scaling or visual styles.
3. **Step 3 (Sealing)**: Confirm configuration bounds to update local cache files and refresh dashboards.
