# Handoff Report: NoNukes Token Family Dashboard Design (Milestones 2 & 3)

This report details the read-only investigation and complete design specification for the backend API integration (Milestone 2) and frontend SPA (Milestone 3) of the NoNukes Token Family Dashboard.

---

## 1. Observation

### Codebase and Data Files Structure
1. **Scope and Interface Contracts (`.agents/nonukes_dashboard_implementation/SCOPE.md`)**:
   - Outlines the required REST endpoints: `/api/nonukes/pools` and `/api/nonukes/pool_details?address=<addr>`.
   - Interface contracts:
     ```json
     // /api/nonukes/pools response:
     {
       "success": true,
       "pools": [
         {
           "address": "0x...",
           "symbol": "$INCOGNITO",
           "name": "$INCOGNITO",
           "target_group": "NoNukes",
           "version": "V1",
           "reserves": {
             "token0": 1000.0,
             "token1": 250000.0
           },
           "swap_count": 12,
           "volume_usd": 1500.50
         }
       ]
     }
     ```
     ```json
     // /api/nonukes/pool_details?address=<addr> response:
     {
       "success": true,
       "address": "0x...",
       "price_trends": [
         {"timestamp": 1781441813, "price": 1.25}
       ],
       "reserves": {
         "token0": 1000.0,
         "token1": 250000.0,
         "token0_symbol": "$INCOGNITO",
         "token1_symbol": "WPLS"
       },
       "swap_history": [
         {
           "tx_hash": "0x...",
           "timestamp": 1781441813,
           "amount0": 10.0,
           "amount1": 2500.0,
           "usd_value": 12.50
         }
       ]
     }
     ```

2. **Dashboard Server (`scripts/dashboard_server.py`)**:
   - Runs a basic HTTP server using `http.server.SimpleHTTPRequestHandler` on port `8080`.
   - Currently serves `/api/data`, `/api/nonukes` (returning simple pool config & reserves mapping), `/api/pools` (querying on-chain node), and `/api/ignore`.
   - The `/api/nonukes` route loads data as follows:
     ```python
     pools = {}
     reserves = {}
     if os.path.exists("nonukes_pools.json"):
         try:
             with open("nonukes_pools.json", "r") as f:
                 pools = json.load(f)
         except Exception:
             pass
     # Load reserves from scratch directory
     res_path = "/home/mariarahel/.gemini/antigravity-cli/brain/5a8d4144-99a3-4e64-93ac-47c55dad5b24/scratch/nonukes_pulsex_reserves.json"
     if os.path.exists(res_path):
         try:
             with open(res_path, "r") as f:
                 reserves = json.load(f)
         except Exception:
             pass
     ```

3. **Pools Configuration (`nonukes_pools.json`)**:
   - Contains 480 pool mappings. Example entry:
     ```json
     "0x326f7e37bef85c7f74a5a2b8c10b99a47e575cda": {
         "target_group": "NoNukes",
         "version": "V1",
         "symbol": "$INCOGNITO",
         "name": "$INCOGNITO",
         "other_addr": "0x9b3b6b8ff7434e9ec2b6d3b032b98152ccf4d266"
     }
     ```

4. **Reserves Registry File (`nonukes_pulsex_reserves.json`)**:
   - Located at `/home/mariarahel/.gemini/antigravity-cli/brain/5a8d4144-99a3-4e64-93ac-47c55dad5b24/scratch/nonukes_pulsex_reserves.json`.
   - Stores real-time reserve quantities and token details. Example entry:
     ```json
     "0x326f7e37bef85c7f74a5a2b8c10b99a47e575cda": {
       "pair": "0x326F7e37bEf85c7f74A5a2b8C10b99a47E575CDa",
       "token0": "0x174A0ad99c60c20D9B3D94c3095BC1fb9ddEFd62",
       "token1": "0x9B3B6b8fF7434e9ec2b6D3B032b98152CCF4D266",
       "reserve0": 0.025460828078310255,
       "reserve1": 513.2766244261666,
       "raw_reserve0": 25460828078310256,
       "raw_reserve1": 513276624426166516741,
       "total_supply": 3.596553906171851,
       "swaps": 0,
       "mints": 0,
       "burns": 0
     }
     ```

5. **Resolved Swaps File (`resolved_swaps.json`)**:
   - Contains list of resolved swaps parsed by `monitor_pulsex.py`. Example entry:
     ```json
     {
         "tx_hash": "652936fcd681f9d047198245b30e2577f4c12e19ea2d8bccadbe89ba0d07d645",
         "pool_address": "0x6753560538ECa67617A9Ce605178F788bE7E524E",
         "version": "PulseX V2",
         "token0": {
             "name": "USD Coin from Ethereum",
             "symbol": "USDC",
             "decimals": 6,
             "address": "0x15d38573d2feeb82e7ad5187ab8c1d52810b1f07",
             "is_treasury": false,
             "treasury_owner": null
         },
         "token1": {
             "name": "Wrapped Pulse",
             "symbol": "WPLS",
             "decimals": 18,
             "address": "0xa1077a294dde1b09bb078844df40758a5d0f9a27",
             "is_treasury": false,
             "treasury_owner": null
         },
         "amount0": 22.536413,
         "amount1": 3688780.026219026,
         "usd_value": 0.0,
         "timestamp": 1781441813.4551547
     }
     ```

---

## 2. Logic Chain

1. **Correlating Configuration & Reserves**:
   - The keys in `nonukes_pools.json` represent the pair pool addresses.
   - These same addresses are used as keys in the reserves file `nonukes_pulsex_reserves.json`.
   - Therefore, for any pool from the configuration, we can fetch real-time reserves by performing a case-insensitive lookup in the reserves file using the pool address.

2. **Calculating Swap Statistics**:
   - Swaps in `resolved_swaps.json` contain the key `"pool_address"`.
   - By scanning `resolved_swaps.json` and matching the swap's `"pool_address"` case-insensitively with the pool's address, we can aggregate:
     - `swap_count`: Count of matching swaps.
     - `volume_usd`: Sum of `usd_value` from matching swaps.

3. **Computing Price Trends**:
   - Each pool pairs the `NoNukes` base token (`0x174a0ad99c60c20d9b3d94c3095bc1fb9ddefd62`) with a personal minter token (`other_addr`).
   - For each matching swap, we identify which token is the minter token by comparing addresses (`other_addr` vs `swap.token0.address` or `swap.token1.address`). Let `amt_m` be the swapped amount of this minter token.
   - If `swap.usd_value > 0` and `amt_m > 0`, the USD price of the minter token at that timestamp is:
     $$\text{price} = \frac{\text{usd\_value}}{\text{amt\_m}}$$
   - Sorting these chronologically yields the `price_trends` history.

4. **Serving Static SPA Files**:
   - Extending the backend request handler's routing allows it to serve:
     - `/nonukes/` or `/nonukes/index.html` $\rightarrow$ `frontend/nonukes_dashboard/index.html`
     - `/nonukes/style.css` $\rightarrow$ `frontend/nonukes_dashboard/style.css`
     - `/nonukes/app.js` $\rightarrow$ `frontend/nonukes_dashboard/app.js`
   - This keeps the implementation localized under `/nonukes/` without disrupting the existing oracle dashboard served at `/`.

---

## 3. Caveats

- **Swap Resolution Status**: Many swaps in `resolved_swaps.json` have `usd_value: 0.0` due to pricing paths not yet fully resolved on-chain. If `usd_value` is `0.0`, the computed price point is `0.0`. We design a fallback: if `usd_value` is `0.0`, the system attempts to calculate the USD value by checking if the other token (like WPLS or stablecoins) has a positive price in `price_cache.json`.
- **Scaling considerations**: Scanning `resolved_swaps.json` (which contains 5,002 lines) on every endpoint request is fast enough for development (~2ms in Python), but for production, we suggest caching the parsed swaps in memory and reloading only if the file modification time (`os.path.getmtime`) changes.

---

## 4. Conclusion & Complete Implementation Design

The implementation of Milestones 2 and 3 should follow this exact specification.

### A. Python Backend Server Changes (`scripts/dashboard_server.py`)
Replace/add the following paths under `do_GET` inside `scripts/dashboard_server.py`:

```python
        # ----------------------------------------------------
        # NoNukes Dashboard SPA static files serving
        # ----------------------------------------------------
        elif self.path == '/nonukes' or self.path == '/nonukes/' or self.path == '/nonukes/index.html':
            self.send_response(200)
            self.send_header('Content-Type', 'text/html; charset=utf-8')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            file_path = os.path.join(os.path.dirname(os.path.dirname(__file__)), 'frontend', 'nonukes_dashboard', 'index.html')
            with open(file_path, 'rb') as f:
                self.wfile.write(f.read())
            return
        elif self.path == '/nonukes/style.css':
            self.send_response(200)
            self.send_header('Content-Type', 'text/css; charset=utf-8')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            file_path = os.path.join(os.path.dirname(os.path.dirname(__file__)), 'frontend', 'nonukes_dashboard', 'style.css')
            with open(file_path, 'rb') as f:
                self.wfile.write(f.read())
            return
        elif self.path == '/nonukes/app.js':
            self.send_response(200)
            self.send_header('Content-Type', 'application/javascript; charset=utf-8')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            file_path = os.path.join(os.path.dirname(os.path.dirname(__file__)), 'frontend', 'nonukes_dashboard', 'app.js')
            with open(file_path, 'rb') as f:
                self.wfile.write(f.read())
            return

        # ----------------------------------------------------
        # /api/nonukes/pools API Endpoint
        # ----------------------------------------------------
        elif self.path == '/api/nonukes/pools':
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            
            pools_data = {}
            if os.path.exists("nonukes_pools.json"):
                try:
                    with open("nonukes_pools.json", "r") as f:
                        pools_data = json.load(f)
                except Exception:
                    pass
                    
            reserves_data = {}
            res_path = "/home/mariarahel/.gemini/antigravity-cli/brain/5a8d4144-99a3-4e64-93ac-47c55dad5b24/scratch/nonukes_pulsex_reserves.json"
            if os.path.exists(res_path):
                try:
                    with open(res_path, "r") as f:
                        reserves_data = json.load(f)
                except Exception:
                    pass
                    
            resolved_swaps = []
            if os.path.exists("resolved_swaps.json"):
                try:
                    with open("resolved_swaps.json", "r") as f:
                        resolved_swaps = json.load(f)
                except Exception:
                    pass
            
            # Load price cache for dynamic USD valuation fallback
            prices = {}
            if os.path.exists("price_cache.json"):
                try:
                    with open("price_cache.json", "r") as f:
                        prices = json.load(f)
                except Exception:
                    pass
            
            # Aggregate stats from swaps
            pool_stats = {}
            for swap in resolved_swaps:
                p_addr = swap.get("pool_address", "").lower()
                if not p_addr:
                    continue
                if p_addr not in pool_stats:
                    pool_stats[p_addr] = {"count": 0, "volume": 0.0}
                
                usd_val = float(swap.get("usd_value", 0.0))
                # Fallback if usd_value is 0.0
                if usd_val == 0.0:
                    t0_addr = swap.get("token0", {}).get("address", "").lower()
                    t1_addr = swap.get("token1", {}).get("address", "").lower()
                    p0 = float(prices.get(t0_addr, {}).get("price", 0.0) if isinstance(prices.get(t0_addr), dict) else prices.get(t0_addr, 0.0))
                    p1 = float(prices.get(t1_addr, {}).get("price", 0.0) if isinstance(prices.get(t1_addr), dict) else prices.get(t1_addr, 0.0))
                    if p0 > 0:
                        usd_val = float(swap.get("amount0", 0.0)) * p0
                    elif p1 > 0:
                        usd_val = float(swap.get("amount1", 0.0)) * p1
                        
                pool_stats[p_addr]["count"] += 1
                pool_stats[p_addr]["volume"] += usd_val
                
            pools_list = []
            for addr, info in pools_data.items():
                addr_lower = addr.lower()
                res = reserves_data.get(addr_lower, {})
                stats = pool_stats.get(addr_lower, {"count": 0, "volume": 0.0})
                
                pools_list.append({
                    "address": addr,
                    "symbol": info.get("symbol", ""),
                    "name": info.get("name", ""),
                    "target_group": info.get("target_group", ""),
                    "version": info.get("version", ""),
                    "reserves": {
                        "token0": float(res.get("reserve0", 0.0)),
                        "token1": float(res.get("reserve1", 0.0))
                    },
                    "swap_count": stats["count"],
                    "volume_usd": stats["volume"]
                })
                
            self.wfile.write(json.dumps({"success": True, "pools": pools_list}, indent=2).encode('utf-8'))
            return

        # ----------------------------------------------------
        # /api/nonukes/pool_details API Endpoint
        # ----------------------------------------------------
        elif self.path.startswith('/api/nonukes/pool_details'):
            parsed_url = urllib.parse.urlparse(self.path)
            params = urllib.parse.parse_qs(parsed_url.query)
            address = params.get('address', [None])[0]
            
            if not address:
                self.send_response(400)
                self.send_header('Content-Type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(json.dumps({"success": False, "error": "Missing address parameter"}).encode('utf-8'))
                return
                
            address_lower = address.lower()
            
            pools_data = {}
            if os.path.exists("nonukes_pools.json"):
                try:
                    with open("nonukes_pools.json", "r") as f:
                        pools_data = json.load(f)
                except Exception:
                    pass
                    
            if address_lower not in pools_data:
                self.send_response(404)
                self.send_header('Content-Type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(json.dumps({"success": False, "error": "Pool not found"}).encode('utf-8'))
                return
                
            pool_info = pools_data[address_lower]
            
            reserves_data = {}
            res_path = "/home/mariarahel/.gemini/antigravity-cli/brain/5a8d4144-99a3-4e64-93ac-47c55dad5b24/scratch/nonukes_pulsex_reserves.json"
            if os.path.exists(res_path):
                try:
                    with open(res_path, "r") as f:
                        reserves_data = json.load(f)
                except Exception:
                    pass
                    
            res = reserves_data.get(address_lower, {})
            
            prices = {}
            if os.path.exists("price_cache.json"):
                try:
                    with open("price_cache.json", "r") as f:
                        prices = json.load(f)
                except Exception:
                    pass
                    
            token0_addr = res.get("token0", "0x174A0ad99c60c20D9B3D94c3095BC1fb9ddEFd62").lower()
            token1_addr = res.get("token1", pool_info.get("other_addr", "")).lower()
            
            def get_symbol(addr):
                if addr == "0x174a0ad99c60c20d9b3d94c3095bc1fb9ddefd62":
                    return "NoNukes"
                if addr == pool_info.get("other_addr", "").lower():
                    return pool_info.get("symbol", "UNKNOWN")
                entry = prices.get(addr, {})
                if isinstance(entry, dict):
                    return entry.get("symbol", "UNKNOWN")
                return "UNKNOWN"
                
            token0_symbol = get_symbol(token0_addr)
            token1_symbol = get_symbol(token1_addr)
            
            resolved_swaps = []
            if os.path.exists("resolved_swaps.json"):
                try:
                    with open("resolved_swaps.json", "r") as f:
                        resolved_swaps = json.load(f)
                except Exception:
                    pass
                    
            matching_swaps = []
            for swap in resolved_swaps:
                if swap.get("pool_address", "").lower() == address_lower:
                    matching_swaps.append(swap)
            
            # Sort chronologically for trends
            matching_swaps.sort(key=lambda s: float(s.get("timestamp", 0)))
            
            price_trends = []
            swap_history = []
            minter_addr = pool_info.get("other_addr", "").lower()
            
            for swap in matching_swaps:
                timestamp = int(swap.get("timestamp", 0))
                amt0 = float(swap.get("amount0", 0.0))
                amt1 = float(swap.get("amount1", 0.0))
                usd_val = float(swap.get("usd_value", 0.0))
                
                # Dynamic USD fallback
                if usd_val == 0.0:
                    t0_addr = swap.get("token0", {}).get("address", "").lower()
                    t1_addr = swap.get("token1", {}).get("address", "").lower()
                    p0 = float(prices.get(t0_addr, {}).get("price", 0.0) if isinstance(prices.get(t0_addr), dict) else prices.get(t0_addr, 0.0))
                    p1 = float(prices.get(t1_addr, {}).get("price", 0.0) if isinstance(prices.get(t1_addr), dict) else prices.get(t1_addr, 0.0))
                    if p0 > 0:
                        usd_val = amt0 * p0
                    elif p1 > 0:
                        usd_val = amt1 * p1
                
                t0_addr = swap.get("token0", {}).get("address", "").lower()
                t1_addr = swap.get("token1", {}).get("address", "").lower()
                
                amt_m = 0.0
                if t0_addr == minter_addr:
                    amt_m = amt0
                elif t1_addr == minter_addr:
                    amt_m = amt1
                else:
                    amt_m = amt1
                    
                price = 0.0
                if usd_val > 0 and amt_m > 0:
                    price = usd_val / amt_m
                
                price_trends.append({
                    "timestamp": timestamp,
                    "price": price
                })
                
                tx_hash = swap.get("tx_hash", "")
                if tx_hash and not tx_hash.startswith("0x"):
                    tx_hash = "0x" + tx_hash
                    
                swap_history.append({
                    "tx_hash": tx_hash,
                    "timestamp": timestamp,
                    "amount0": amt0,
                    "amount1": amt1,
                    "usd_value": usd_val
                })
                
            # History is returned newest first
            swap_history.reverse()
            
            response = {
                "success": True,
                "address": address,
                "price_trends": price_trends,
                "reserves": {
                    "token0": float(res.get("reserve0", 0.0)),
                    "token1": float(res.get("reserve1", 0.0)),
                    "token0_symbol": token0_symbol,
                    "token1_symbol": token1_symbol
                },
                "swap_history": swap_history
            }
            
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps(response, indent=2).encode('utf-8'))
            return
```

---

### B. Frontend SPA Code Structure (`frontend/nonukes_dashboard/`)

#### 1. `frontend/nonukes_dashboard/index.html`
```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>NoNukes Token Family Dashboard</title>
    <link href="https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;600;800&family=Share+Tech+Mono&display=swap" rel="stylesheet">
    <link rel="stylesheet" href="style.css">
</head>
<body>
    <div class="glass-bg"></div>
    
    <header class="glass">
        <div class="header-logo">
            <div class="pulse-indicator"></div>
            <h1>NoNukes Family</h1>
        </div>
        <div class="header-status">
            <span>Ecosystem Node Connected</span>
            <button id="btn-refresh" class="btn">↻ Refresh</button>
        </div>
    </header>

    <main>
        <!-- Statistics Section -->
        <section class="stats-grid">
            <div class="card glass">
                <div class="card-label">Total Unique Pools</div>
                <div id="stat-total-pools" class="card-value">-</div>
            </div>
            <div class="card glass">
                <div class="card-label">Active Groups</div>
                <div id="stat-active-groups" class="card-value">-</div>
            </div>
            <div class="card glass">
                <div class="card-label">Total Volume (USD)</div>
                <div id="stat-total-volume" class="card-value text-cyan">-</div>
            </div>
            <div class="card glass">
                <div class="card-label">Total Swap Count</div>
                <div id="stat-total-swaps" class="card-value text-magenta">-</div>
            </div>
        </section>

        <!-- Catalog Section -->
        <section class="catalog-section">
            <div class="filter-bar glass">
                <div class="search-container">
                    <input type="text" id="pool-search" placeholder="Search by Minter symbol or Address...">
                </div>
                <div class="group-filters" id="group-filters">
                    <!-- Dynamic group filters -->
                </div>
            </div>

            <div class="table-container glass">
                <table class="pool-table" id="pool-table">
                    <thead>
                        <tr>
                            <th onclick="changeSort('symbol')">Minter Token ⇅</th>
                            <th onclick="changeSort('version')">Version ⇅</th>
                            <th onclick="changeSort('target_group')">Group ⇅</th>
                            <th onclick="changeSort('reserves')">Reserves (NoNukes / Minter) ⇅</th>
                            <th onclick="changeSort('swap_count')">Swaps ⇅</th>
                            <th onclick="changeSort('volume_usd')">Volume (USD) ⇅</th>
                        </tr>
                    </thead>
                    <tbody>
                        <tr>
                            <td colspan="6" class="text-center text-muted">Loading NoNukes family catalog...</td>
                        </tr>
                    </tbody>
                </table>
            </div>
        </section>
    </main>

    <!-- Detail Modal -->
    <div id="detail-modal" class="modal-overlay">
        <div class="modal-content glass">
            <button class="modal-close" id="modal-close-btn">&times;</button>
            <div class="modal-header">
                <h2 id="modal-title">Minter Pool Details</h2>
                <div id="modal-subtitle">0x...</div>
            </div>
            
            <div class="modal-grid">
                <!-- Left Column: Reserves & Swap History -->
                <div class="modal-col">
                    <div class="subcard glass">
                        <h3>Pool Reserves</h3>
                        <div class="reserves-display">
                            <div class="reserve-bar">
                                <div class="reserve-lbl" id="res0-lbl">NoNukes</div>
                                <div class="reserve-val" id="res0-val">-</div>
                            </div>
                            <div class="reserve-bar">
                                <div class="reserve-lbl" id="res1-lbl">Minter</div>
                                <div class="reserve-val" id="res1-val">-</div>
                            </div>
                        </div>
                    </div>

                    <div class="subcard glass flex-fill">
                        <h3>Swap History</h3>
                        <div class="history-table-container">
                            <table id="history-table">
                                <thead>
                                    <tr>
                                        <th>Tx Hash</th>
                                        <th>Amount In</th>
                                        <th>Amount Out</th>
                                        <th>USD Value</th>
                                        <th>Time</th>
                                    </tr>
                                </thead>
                                <tbody>
                                    <!-- Dynamic history -->
                                </tbody>
                            </table>
                        </div>
                    </div>
                </div>

                <!-- Right Column: Price Trend Chart -->
                <div class="modal-col">
                    <div class="subcard glass flex-fill">
                        <h3>Price Trend (USD)</h3>
                        <div id="chart-container" class="chart-container">
                            <!-- SVG chart will be drawn here -->
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <script src="app.js"></script>
</body>
</html>
```

#### 2. `frontend/nonukes_dashboard/style.css`
```css
:root {
    --bg-dark: hsl(220, 40%, 3%);
    --bg-card: hsla(220, 35%, 8%, 0.65);
    --border-glass: hsla(0, 0%, 100%, 0.06);
    --border-active: hsla(180, 100%, 50%, 0.3);
    
    --text-primary: hsl(210, 40%, 95%);
    --text-muted: hsl(215, 20%, 65%);
    
    --cyan: hsl(180, 100%, 50%);
    --magenta: hsl(320, 100%, 50%);
    --yellow: hsl(45, 100%, 50%);
    --green: hsl(140, 100%, 50%);
    
    --glow-cyan: rgba(0, 240, 255, 0.15);
    --glow-magenta: rgba(255, 0, 127, 0.15);
}

* {
    box-sizing: border-box;
    margin: 0;
    padding: 0;
}

body {
    background-color: var(--bg-dark);
    color: var(--text-primary);
    font-family: 'Outfit', sans-serif;
    min-height: 100vh;
    overflow-x: hidden;
    position: relative;
    padding-bottom: 2rem;
}

.glass-bg {
    position: fixed;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    z-index: -1;
    background-image: 
        radial-gradient(at 0% 0%, hsla(180, 100%, 50%, 0.08) 0px, transparent 50%),
        radial-gradient(at 100% 100%, hsla(320, 100%, 50%, 0.08) 0px, transparent 50%),
        radial-gradient(at 50% 50%, hsla(45, 100%, 50%, 0.03) 0px, transparent 50%);
}

.glass {
    background: var(--bg-card);
    backdrop-filter: blur(16px);
    -webkit-backdrop-filter: blur(16px);
    border: 1px solid var(--border-glass);
    border-radius: 16px;
    box-shadow: 0 8px 32px 0 rgba(0, 0, 0, 0.35);
}

/* Header */
header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 1.25rem 2rem;
    margin: 1.5rem 2rem 0;
}

.header-logo {
    display: flex;
    align-items: center;
    gap: 1rem;
}

.header-logo h1 {
    font-size: 1.5rem;
    font-weight: 800;
    letter-spacing: 1.5px;
    background: linear-gradient(90deg, var(--cyan), var(--magenta));
    -webkit-background-clip: text;
    -webkit-text-fill-color: transparent;
    text-transform: uppercase;
}

.pulse-indicator {
    width: 12px;
    height: 12px;
    background-color: var(--green);
    border-radius: 50%;
    box-shadow: 0 0 10px var(--green);
    animation: heartbeat 2s infinite ease-in-out;
}

@keyframes heartbeat {
    0%, 100% { transform: scale(0.95); opacity: 0.5; }
    50% { transform: scale(1.1); opacity: 1; }
}

.header-status {
    display: flex;
    align-items: center;
    gap: 1.5rem;
    font-size: 0.85rem;
    color: var(--text-muted);
}

.btn {
    background: rgba(255, 255, 255, 0.04);
    border: 1px solid var(--border-glass);
    color: var(--text-primary);
    padding: 0.5rem 1rem;
    border-radius: 8px;
    cursor: pointer;
    font-family: 'Share Tech Mono', monospace;
    font-weight: 600;
    transition: all 0.2s;
}

.btn:hover {
    background: var(--cyan);
    color: #000;
    box-shadow: 0 0 12px var(--cyan);
    border-color: var(--cyan);
}

/* Main Layout */
main {
    padding: 2rem;
    display: flex;
    flex-direction: column;
    gap: 2rem;
}

.stats-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(240px, 1fr));
    gap: 1.5rem;
}

.card {
    padding: 1.5rem;
    transition: transform 0.3s, border-color 0.3s;
}

.card:hover {
    transform: translateY(-4px);
    border-color: var(--border-active);
}

.card-label {
    font-size: 0.75rem;
    text-transform: uppercase;
    letter-spacing: 2px;
    color: var(--text-muted);
    margin-bottom: 0.5rem;
}

.card-value {
    font-size: 2.2rem;
    font-weight: 800;
    font-family: 'Share Tech Mono', monospace;
}

.text-cyan { color: var(--cyan); text-shadow: 0 0 10px var(--glow-cyan); }
.text-magenta { color: var(--magenta); text-shadow: 0 0 10px var(--glow-magenta); }

/* Filter Bar */
.filter-bar {
    display: flex;
    flex-direction: column;
    gap: 1rem;
    padding: 1.5rem;
    margin-bottom: 1.5rem;
}

.search-container input {
    width: 100%;
    background: rgba(0, 0, 0, 0.2);
    border: 1px solid var(--border-glass);
    padding: 0.8rem 1.2rem;
    border-radius: 10px;
    color: #fff;
    font-family: inherit;
    outline: none;
    transition: border-color 0.2s;
}

.search-container input:focus {
    border-color: var(--cyan);
}

.group-filters {
    display: flex;
    flex-wrap: wrap;
    gap: 0.5rem;
}

.filter-chip {
    background: rgba(255, 255, 255, 0.02);
    border: 1px solid var(--border-glass);
    color: var(--text-muted);
    padding: 0.4rem 0.9rem;
    border-radius: 20px;
    font-size: 0.8rem;
    cursor: pointer;
    transition: all 0.2s;
}

.filter-chip.active, .filter-chip:hover {
    background: rgba(255, 0, 127, 0.1);
    border-color: var(--magenta);
    color: #fff;
    box-shadow: 0 0 8px rgba(255, 0, 127, 0.2);
}

/* Tables */
.table-container {
    overflow-x: auto;
    padding: 1rem;
}

.pool-table {
    width: 100%;
    border-collapse: collapse;
    text-align: left;
    font-size: 0.9rem;
}

.pool-table th, .pool-table td {
    padding: 1rem;
    border-bottom: 1px solid var(--border-glass);
}

.pool-table th {
    color: var(--text-muted);
    font-weight: 600;
    cursor: pointer;
    user-select: none;
}

.pool-table tbody tr {
    cursor: pointer;
    transition: all 0.2s;
}

.pool-table tbody tr:hover {
    background: rgba(255, 255, 255, 0.02);
    transform: translateX(4px);
}

/* Modal details */
.modal-overlay {
    position: fixed;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    background: rgba(3, 4, 8, 0.8);
    backdrop-filter: blur(8px);
    display: none;
    justify-content: center;
    align-items: center;
    z-index: 100;
    opacity: 0;
    transition: opacity 0.3s ease;
}

.modal-overlay.active {
    display: flex;
    opacity: 1;
}

.modal-content {
    width: 90%;
    max-width: 1000px;
    height: 80vh;
    padding: 2rem;
    display: flex;
    flex-direction: column;
    gap: 1.5rem;
    position: relative;
    transform: translateY(20px);
    transition: transform 0.3s ease;
}

.modal-overlay.active .modal-content {
    transform: translateY(0);
}

.modal-close {
    position: absolute;
    top: 1.5rem;
    right: 1.5rem;
    background: none;
    border: none;
    color: var(--text-muted);
    font-size: 2rem;
    cursor: pointer;
}

.modal-close:hover {
    color: #fff;
}

.modal-header h2 {
    font-size: 1.6rem;
    font-weight: 800;
    background: linear-gradient(90deg, var(--cyan), var(--magenta));
    -webkit-background-clip: text;
    -webkit-text-fill-color: transparent;
}

.modal-header div {
    font-family: 'Share Tech Mono', monospace;
    color: var(--text-muted);
    font-size: 0.85rem;
    margin-top: 0.25rem;
}

.modal-grid {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 1.5rem;
    flex: 1;
    overflow: hidden;
}

.modal-col {
    display: flex;
    flex-direction: column;
    gap: 1.5rem;
    overflow: hidden;
}

.subcard {
    padding: 1.25rem;
    display: flex;
    flex-direction: column;
    gap: 1rem;
}

.subcard h3 {
    font-size: 1rem;
    font-weight: 600;
    text-transform: uppercase;
    letter-spacing: 1px;
    border-bottom: 1px solid var(--border-glass);
    padding-bottom: 0.5rem;
    color: var(--cyan);
}

.flex-fill {
    flex: 1;
    overflow: hidden;
}

/* Reserves Display */
.reserves-display {
    display: flex;
    flex-direction: column;
    gap: 0.75rem;
}

.reserve-bar {
    display: flex;
    justify-content: space-between;
    background: rgba(0, 0, 0, 0.2);
    padding: 0.75rem 1rem;
    border-radius: 8px;
    border-left: 3px solid var(--magenta);
}

.reserve-bar:first-child {
    border-left-color: var(--cyan);
}

.reserve-lbl {
    font-weight: 600;
}

.reserve-val {
    font-family: 'Share Tech Mono', monospace;
}

/* History table */
.history-table-container {
    flex: 1;
    overflow-y: auto;
}

#history-table {
    width: 100%;
    border-collapse: collapse;
    font-size: 0.8rem;
}

#history-table th, #history-table td {
    padding: 0.5rem;
    border-bottom: 1px solid var(--border-glass);
    text-align: left;
}

#history-table th {
    color: var(--text-muted);
}

/* Chart Container */
.chart-container {
    flex: 1;
    display: flex;
    justify-content: center;
    align-items: center;
    position: relative;
    background: rgba(0, 0, 0, 0.15);
    border-radius: 8px;
    padding: 1rem;
}

.chart-svg {
    width: 100%;
    height: 100%;
}
```

#### 3. `frontend/nonukes_dashboard/app.js`
```javascript
let pools = [];
let groups = new Set();
let currentGroup = 'ALL';
let currentSort = { column: 'volume_usd', direction: 'desc' };
let selectedAddress = null;

// Initialize Elements
const searchInput = document.getElementById('pool-search');
const groupFiltersContainer = document.getElementById('group-filters');
const poolTableBody = document.querySelector('#pool-table tbody');
const detailModal = document.getElementById('detail-modal');

async function fetchPools() {
    try {
        const res = await fetch('/api/nonukes/pools');
        const data = await res.json();
        if (data.success) {
            pools = data.pools;
            extractGroups();
            renderStats();
            renderGroupFilters();
            renderPools();
        }
    } catch (err) {
        console.error("Error fetching pools:", err);
    }
}

function extractGroups() {
    groups.clear();
    pools.forEach(pool => {
        if (pool.target_group) groups.add(pool.target_group);
    });
}

function renderStats() {
    document.getElementById('stat-total-pools').innerText = pools.length;
    document.getElementById('stat-active-groups').innerText = groups.size;
    
    const totalVolume = pools.reduce((acc, p) => acc + (p.volume_usd || 0), 0);
    const totalSwaps = pools.reduce((acc, p) => acc + (p.swap_count || 0), 0);
    
    document.getElementById('stat-total-volume').innerText = `$${totalVolume.toLocaleString(undefined, {minimumFractionDigits: 2, maximumFractionDigits: 2})}`;
    document.getElementById('stat-total-swaps').innerText = totalSwaps.toLocaleString();
}

function renderGroupFilters() {
    let html = `<button class="filter-chip ${currentGroup === 'ALL' ? 'active' : ''}" onclick="setGroupFilter('ALL')">ALL</button>`;
    Array.from(groups).sort().forEach(g => {
        html += `<button class="filter-chip ${currentGroup === g ? 'active' : ''}" onclick="setGroupFilter('${g}')">${g}</button>`;
    });
    groupFiltersContainer.innerHTML = html;
}

function setGroupFilter(group) {
    currentGroup = group;
    renderGroupFilters();
    renderPools();
}

function changeSort(column) {
    if (currentSort.column === column) {
        currentSort.direction = currentSort.direction === 'asc' ? 'desc' : 'asc';
    } else {
        currentSort.column = column;
        currentSort.direction = 'desc';
    }
    renderPools();
}

function renderPools() {
    const searchVal = searchInput.value.toLowerCase();
    
    let filtered = pools.filter(p => {
        const matchGroup = currentGroup === 'ALL' || p.target_group === currentGroup;
        const matchSearch = p.symbol.toLowerCase().includes(searchVal) || p.address.toLowerCase().includes(searchInput.value.toLowerCase());
        return matchGroup && matchSearch;
    });
    
    // Sort
    filtered.sort((a, b) => {
        let valA, valB;
        if (currentSort.column === 'reserves') {
            valA = (a.reserves.token0 || 0) + (a.reserves.token1 || 0);
            valB = (b.reserves.token0 || 0) + (b.reserves.token1 || 0);
        } else {
            valA = a[currentSort.column];
            valB = b[currentSort.column];
        }
        
        if (typeof valA === 'string') {
            return currentSort.direction === 'asc' ? valA.localeCompare(valB) : valB.localeCompare(valA);
        } else {
            return currentSort.direction === 'asc' ? valA - valB : valB - valA;
        }
    });
    
    if (filtered.length === 0) {
        poolTableBody.innerHTML = `<tr><td colspan="6" class="text-center text-muted">No pools matching current filters.</td></tr>`;
        return;
    }
    
    poolTableBody.innerHTML = filtered.map(p => {
        const volStr = `$${(p.volume_usd || 0).toLocaleString(undefined, {minimumFractionDigits: 2, maximumFractionDigits: 2})}`;
        const formatAddr = p.address.slice(0, 6) + '...' + p.address.slice(-4);
        return `
            <tr onclick="openPoolDetails('${p.address}')">
                <td>
                    <div style="font-weight: 700; color: #fff;">${p.symbol}</div>
                    <div style="font-size:0.75rem; color:var(--text-muted); font-family:'Share Tech Mono'">${formatAddr}</div>
                </td>
                <td><span class="btn" style="padding: 0.15rem 0.5rem; font-size:0.7rem; border-color:${p.version === 'V1' ? 'var(--cyan)' : 'var(--magenta)'}">${p.version}</span></td>
                <td style="color:var(--magenta); font-weight:600">${p.target_group}</td>
                <td style="font-family:'Share Tech Mono'">${p.reserves.token0.toFixed(2)} / ${p.reserves.token1.toFixed(2)}</td>
                <td style="font-family:'Share Tech Mono'; color:var(--cyan); font-weight:600">${p.swap_count}</td>
                <td style="font-family:'Share Tech Mono'; font-weight:700">${volStr}</td>
            </tr>
        `;
    }).join('');
}

async function openPoolDetails(address) {
    selectedAddress = address;
    detailModal.classList.add('active');
    
    document.getElementById('modal-title').innerText = "Loading details...";
    document.getElementById('modal-subtitle').innerText = address;
    
    try {
        const res = await fetch(`/api/nonukes/pool_details?address=${encodeURIComponent(address)}`);
        const data = await res.json();
        
        if (data.success) {
            document.getElementById('modal-title').innerText = `${data.reserves.token1_symbol} / ${data.reserves.token0_symbol} Details`;
            
            // Render reserves
            document.getElementById('res0-lbl').innerText = data.reserves.token0_symbol;
            document.getElementById('res0-val').innerText = data.reserves.token0.toFixed(4);
            document.getElementById('res1-lbl').innerText = data.reserves.token1_symbol;
            document.getElementById('res1-val').innerText = data.reserves.token1.toFixed(4);
            
            // Render swaps
            const historyBody = document.querySelector('#history-table tbody');
            if (data.swap_history.length === 0) {
                historyBody.innerHTML = `<tr><td colspan="5" class="text-center text-muted">No swaps recorded for this pool.</td></tr>`;
            } else {
                historyBody.innerHTML = data.swap_history.map(swap => {
                    const shortHash = swap.tx_hash.slice(0, 6) + '...' + swap.tx_hash.slice(-4);
                    const timeStr = new Date(swap.timestamp * 1000).toLocaleTimeString();
                    return `
                        <tr>
                            <td><a href="https://otter.pulsechain.com/tx/${swap.tx_hash}" target="_blank" style="color:var(--cyan); text-decoration:none;">${shortHash}</a></td>
                            <td style="font-family:'Share Tech Mono'">${swap.amount0.toFixed(4)}</td>
                            <td style="font-family:'Share Tech Mono'">${swap.amount1.toFixed(4)}</td>
                            <td style="font-family:'Share Tech Mono'; color:var(--green)">$${swap.usd_value.toFixed(2)}</td>
                            <td style="color:var(--text-muted)">${timeStr}</td>
                        </tr>
                    `;
                }).join('');
            }
            
            // Render Chart
            drawChart(data.price_trends);
        }
    } catch (err) {
        console.error("Error fetching pool details:", err);
    }
}

function drawChart(trends) {
    const container = document.getElementById('chart-container');
    container.innerHTML = '';
    
    if (!trends || trends.length === 0) {
        container.innerHTML = `<div class="text-muted">No price trend data available.</div>`;
        return;
    }
    
    // Width and Height of viewBox
    const w = 400;
    const h = 200;
    const padding = 20;
    
    const minTime = Math.min(...trends.map(t => t.timestamp));
    const maxTime = Math.max(...trends.map(t => t.timestamp));
    const minPrice = Math.min(...trends.map(t => t.price));
    const maxPrice = Math.max(...trends.map(t => t.price));
    
    const timeRange = maxTime - minTime || 1;
    const priceRange = maxPrice - minPrice || 1;
    
    // Map data to SVG viewport points
    const points = trends.map(t => {
        const x = padding + ((t.timestamp - minTime) / timeRange) * (w - 2 * padding);
        const y = h - padding - ((t.price - minPrice) / priceRange) * (h - 2 * padding);
        return `${x},${y}`;
    });
    
    const pathD = `M ${points.join(' L ')}`;
    
    const svg = `
        <svg viewBox="0 0 ${w} ${h}" class="chart-svg">
            <!-- Grid Lines -->
            <line x1="${padding}" y1="${padding}" x2="${padding}" y2="${h - padding}" stroke="rgba(255,255,255,0.05)" />
            <line x1="${padding}" y1="${h - padding}" x2="${w - padding}" y2="${h - padding}" stroke="rgba(255,255,255,0.05)" />
            
            <!-- Line Path -->
            <path d="${pathD}" fill="none" stroke="var(--cyan)" stroke-width="2" />
            
            <!-- Data Dots -->
            ${trends.map((t, idx) => {
                const [cx, cy] = points[idx].split(',');
                return `<circle cx="${cx}" cy="${cy}" r="3" fill="var(--magenta)" />`;
            }).join('')}
        </svg>
    `;
    
    container.innerHTML = svg;
}

// Event Listeners
searchInput.addEventListener('input', renderPools);
document.getElementById('btn-refresh').addEventListener('click', fetchPools);
document.getElementById('modal-close-btn').addEventListener('click', () => {
    detailModal.classList.remove('active');
});

// Initial Setup
fetchPools();
setInterval(fetchPools, 5000);
```

---

## 5. Verification Method

To independently verify the implementation after code updates are made:

1. **Start the Web Server**:
   ```bash
   python3 scripts/dashboard_server.py
   ```
2. **Verify API responses match SCOPE.md interface contracts**:
   - Query Pools API:
     ```bash
     curl -s http://localhost:8080/api/nonukes/pools | jq .
     ```
     Check that it returns a JSON object with `"success": true` and a `"pools"` array containing valid objects (address, reserves, swap_count, volume_usd).
   - Query Details API:
     ```bash
     curl -s "http://localhost:8080/api/nonukes/pool_details?address=0x326f7e37bef85c7f74a5a2b8c10b99a47e575cda" | jq .
     ```
     Check that it returns `"success": true` and details including reserves, swap_history and price_trends.
3. **Verify SPA interface loading**:
   - Open `http://localhost:8080/nonukes/` in a web browser.
   - Verify the dark-mode theme, glassmorphic styles, interactive catalog table sorting, group filters, search box, and clicking pool rows to open the detail modal with the SVG chart.
