#!/usr/bin/env python3
import os
import json
import http.server
import socketserver
import urllib.parse

PORT = 8080
PRICE_CACHE_FILE = "price_cache.json"
UNRESOLVED_FILE = "unresolved_swaps.json"
RESOLVED_FILE = "resolved_swaps.json"

class DashboardHandler(http.server.SimpleHTTPRequestHandler):
    def log_message(self, format, *args):
        # Suppress request logging to keep stderr clean
        pass

    def do_GET(self):
        if self.path == '/api/data':
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            
            prices = {}
            if os.path.exists(PRICE_CACHE_FILE):
                try:
                    with open(PRICE_CACHE_FILE, "r") as f:
                        prices = json.load(f)
                except Exception:
                    pass
                    
            unresolved = []
            if os.path.exists(UNRESOLVED_FILE):
                try:
                    with open(UNRESOLVED_FILE, "r") as f:
                        unresolved = json.load(f)
                except Exception:
                    pass
                    
            resolved = []
            if os.path.exists(RESOLVED_FILE):
                try:
                    with open(RESOLVED_FILE, "r") as f:
                        resolved = json.load(f)
                except Exception:
                    pass
            
            treasury_tokens = {}
            if os.path.exists("treasury_tokens.json"):
                try:
                    with open("treasury_tokens.json", "r") as f:
                        treasury_tokens = json.load(f)
                except Exception:
                    pass
            
            response = {
                "prices": prices,
                "unresolved": unresolved,
                "resolved": resolved,
                "treasury_tokens": treasury_tokens
            }
            self.wfile.write(json.dumps(response, indent=2).encode('utf-8'))
        elif self.path.startswith('/api/ignore'):
            parsed_url = urllib.parse.urlparse(self.path)
            params = urllib.parse.parse_qs(parsed_url.query)
            address = params.get('address', [None])[0]
            ignored_val = params.get('ignored', ['true'])[0].lower() == 'true'
            
            if address:
                address = address.lower()
                tokens = {}
                if os.path.exists("treasury_tokens.json"):
                    try:
                        with open("treasury_tokens.json", "r") as f:
                            tokens = json.load(f)
                    except Exception:
                        pass
                
                if address not in tokens:
                    tokens[address] = {
                        "address": address,
                        "symbol": "UNKNOWN",
                        "name": "Unknown Token",
                        "owner": None,
                        "ignored": ignored_val
                    }
                else:
                    tokens[address]["ignored"] = ignored_val
                
                try:
                    with open("treasury_tokens.json", "w") as f:
                        json.dump(tokens, f, indent=4)
                    self.send_response(200)
                    self.send_header('Content-Type', 'application/json')
                    self.send_header('Access-Control-Allow-Origin', '*')
                    self.end_headers()
                    self.wfile.write(json.dumps({"success": True, "address": address, "ignored": ignored_val}).encode('utf-8'))
                    return
                except Exception as e:
                    print(f"Error saving ignored status: {e}")
            
            self.send_response(400)
            self.end_headers()
            self.wfile.write(b"Invalid request")
        elif self.path == '/' or self.path == '/index.html':
            self.send_response(200)
            self.send_header('Content-Type', 'text/html; charset=utf-8')
            self.end_headers()
            self.wfile.write(HTML_CONTENT.encode('utf-8'))
        else:
            self.send_error(404, "File not found")

HTML_CONTENT = """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>PulseChain Price Oracle Dashboard</title>
    <link href="https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;600;800&display=swap" rel="stylesheet">
    <style>
        :root {
            --bg-color: #0b0f19;
            --panel-bg: rgba(17, 25, 40, 0.75);
            --panel-border: rgba(255, 255, 255, 0.08);
            --text-color: #f3f4f6;
            --text-muted: #9ca3af;
            --primary: #8b5cf6;
            --primary-glow: rgba(139, 92, 246, 0.4);
            --success: #10b981;
            --success-glow: rgba(16, 185, 129, 0.3);
            --warning: #f59e0b;
            --danger: #ef4444;
            --glow-color: rgba(99, 102, 241, 0.15);
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }

        body {
            font-family: 'Outfit', sans-serif;
            background-color: var(--bg-color);
            color: var(--text-color);
            background-image: 
                radial-gradient(at 0% 0%, rgba(99, 102, 241, 0.15) 0px, transparent 50%),
                radial-gradient(at 100% 100%, rgba(139, 92, 246, 0.12) 0px, transparent 50%),
                radial-gradient(at 50% 50%, rgba(16, 185, 129, 0.05) 0px, transparent 50%);
            background-attachment: fixed;
            min-height: 100vh;
            padding: 2rem;
            overflow-x: hidden;
        }

        .header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 2.5rem;
            padding: 1.5rem;
            background: var(--panel-bg);
            backdrop-filter: blur(16px);
            border: 1px solid var(--panel-border);
            border-radius: 20px;
            box-shadow: 0 8px 32px 0 rgba(0, 0, 0, 0.3);
        }

        .logo-section {
            display: flex;
            align-items: center;
            gap: 1rem;
        }

        .pulse-indicator {
            width: 14px;
            height: 14px;
            background-color: var(--success);
            border-radius: 50%;
            position: relative;
            box-shadow: 0 0 10px var(--success);
            animation: pulse-ring 2s infinite;
        }

        @keyframes pulse-ring {
            0% {
                transform: scale(0.95);
                box-shadow: 0 0 0 0 rgba(16, 185, 129, 0.7);
            }
            70% {
                transform: scale(1);
                box-shadow: 0 0 0 10px rgba(16, 185, 129, 0);
            }
            100% {
                transform: scale(0.95);
                box-shadow: 0 0 0 0 rgba(16, 185, 129, 0);
            }
        }

        .title {
            font-size: 1.8rem;
            font-weight: 800;
            background: linear-gradient(135deg, #a78bfa, #8b5cf6, #34d399);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }

        .subtitle {
            font-size: 0.9rem;
            color: var(--text-muted);
            margin-top: 0.2rem;
        }

        .status-badge {
            background: rgba(16, 185, 129, 0.1);
            color: var(--success);
            border: 1px solid rgba(16, 185, 129, 0.2);
            padding: 0.4rem 1rem;
            border-radius: 30px;
            font-size: 0.85rem;
            font-weight: 600;
            display: flex;
            align-items: center;
            gap: 0.5rem;
        }

        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 2rem;
            margin-bottom: 2rem;
        }

        .card {
            background: var(--panel-bg);
            backdrop-filter: blur(16px);
            border: 1px solid var(--panel-border);
            border-radius: 20px;
            padding: 1.5rem;
            box-shadow: 0 8px 32px 0 rgba(0, 0, 0, 0.2);
            transition: transform 0.3s ease, border-color 0.3s ease;
        }

        .card:hover {
            transform: translateY(-4px);
            border-color: rgba(139, 92, 246, 0.25);
        }

        .card-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 1.2rem;
            border-bottom: 1px solid rgba(255, 255, 255, 0.05);
            padding-bottom: 0.8rem;
        }

        .card-title {
            font-size: 1.1rem;
            font-weight: 600;
            color: #fff;
            display: flex;
            align-items: center;
            gap: 0.6rem;
        }

        .stats-num {
            font-size: 2.5rem;
            font-weight: 800;
            margin: 0.5rem 0;
            background: linear-gradient(135deg, #fff, var(--text-muted));
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }

        .stats-label {
            font-size: 0.85rem;
            color: var(--text-muted);
        }

        /* Lists and Tables */
        .list-container {
            max-height: 400px;
            overflow-y: auto;
            scrollbar-width: thin;
            scrollbar-color: rgba(255, 255, 255, 0.1) transparent;
        }

        .list-container::-webkit-scrollbar {
            width: 6px;
        }

        .list-container::-webkit-scrollbar-thumb {
            background: rgba(255, 255, 255, 0.1);
            border-radius: 4px;
        }

        .table {
            width: 100%;
            border-collapse: collapse;
        }

        .table th, .table td {
            text-align: left;
            padding: 0.8rem 1rem;
            font-size: 0.9rem;
        }

        .table th {
            color: var(--text-muted);
            font-weight: 600;
            border-bottom: 1px solid rgba(255, 255, 255, 0.05);
        }

        .table tr:not(:last-child) {
            border-bottom: 1px solid rgba(255, 255, 255, 0.03);
        }

        .table tr:hover {
            background: rgba(255, 255, 255, 0.02);
        }

        .address-link {
            color: var(--primary);
            text-decoration: none;
            font-family: monospace;
            transition: color 0.2s ease;
        }

        .address-link:hover {
            color: #a78bfa;
            text-decoration: underline;
        }

        .badge {
            padding: 0.2rem 0.6rem;
            border-radius: 6px;
            font-size: 0.75rem;
            font-weight: 600;
        }

        .badge-resolved {
            background: rgba(16, 185, 129, 0.15);
            color: var(--success);
        }

        .badge-unresolved {
            background: rgba(245, 158, 11, 0.15);
            color: var(--warning);
        }

        .search-bar {
            width: 100%;
            background: rgba(255, 255, 255, 0.04);
            border: 1px solid var(--panel-border);
            padding: 0.8rem 1.2rem;
            border-radius: 12px;
            color: #fff;
            font-family: inherit;
            font-size: 0.9rem;
            margin-bottom: 1rem;
            outline: none;
            transition: border-color 0.3s ease;
        }

        .search-bar:focus {
            border-color: var(--primary);
            background: rgba(255, 255, 255, 0.06);
        }

        .token-symbol-glow {
            font-weight: 800;
            color: #34d399;
            text-shadow: 0 0 10px rgba(52, 211, 153, 0.2);
        }

        /* Timeline styles */
        .timeline-item {
            padding: 1rem;
            border-left: 2px solid var(--primary);
            margin-left: 0.5rem;
            position: relative;
            background: rgba(255, 255, 255, 0.02);
            border-radius: 0 12px 12px 0;
            margin-bottom: 1rem;
        }

        .timeline-item::before {
            content: '';
            width: 10px;
            height: 10px;
            background: var(--primary);
            border-radius: 50%;
            position: absolute;
            left: -6px;
            top: 1.4rem;
            box-shadow: 0 0 8px var(--primary);
        }

        .timeline-time {
            font-size: 0.75rem;
            color: var(--text-muted);
            margin-bottom: 0.4rem;
        }

        .timeline-title {
            font-size: 0.95rem;
            font-weight: 600;
            margin-bottom: 0.3rem;
            display: flex;
            align-items: center;
            justify-content: space-between;
        }

        .timeline-val {
            color: var(--success);
            font-weight: 700;
        }

        .timeline-details {
            font-size: 0.85rem;
            color: var(--text-muted);
        }

        .btn {
            padding: 0.3rem 0.8rem;
            border-radius: 8px;
            font-size: 0.8rem;
            font-weight: 600;
            cursor: pointer;
            border: 1px solid transparent;
            transition: all 0.2s ease;
        }
        .btn-ignore {
            background: rgba(239, 68, 68, 0.15);
            color: var(--danger);
            border-color: rgba(239, 68, 68, 0.25);
        }
        .btn-ignore:hover {
            background: rgba(239, 68, 68, 0.25);
            box-shadow: 0 0 8px rgba(239, 68, 68, 0.2);
        }
        .btn-track {
            background: rgba(16, 185, 129, 0.15);
            color: var(--success);
            border-color: rgba(16, 185, 129, 0.25);
        }
        .btn-track:hover {
            background: rgba(16, 185, 129, 0.25);
            box-shadow: 0 0 8px rgba(16, 185, 129, 0.2);
        }
    </style>
</head>
<body>
    <div class="header">
        <div class="logo-section">
            <div class="pulse-indicator"></div>
            <div>
                <h1 class="title">PulseChain Price Oracle</h1>
                <div class="subtitle">Real-Time PulseX Liquidity & Swap Resolver</div>
            </div>
        </div>
        <div class="status-badge">
            <span style="display:inline-block; width:8px; height:8px; background-color:var(--success); border-radius:50%"></span>
            Monitoring Block Logs
        </div>
    </div>

    <div class="grid">
        <!-- Stats Card 1 -->
        <div class="card">
            <div class="card-title">💵 Market Coverage</div>
            <div class="stats-num" id="stat-tokens-count">0</div>
            <div class="stats-label">Unique Currencies Tracked and Priced in USD</div>
        </div>
        <!-- Stats Card 2 -->
        <div class="card">
            <div class="card-title">⏳ Resolution Queue</div>
            <div class="stats-num" id="stat-unresolved-count" style="color: var(--warning)">0</div>
            <div class="stats-label">Unresolved Trades Awaiting Price Paths</div>
        </div>
        <!-- Stats Card 3 -->
        <div class="card">
            <div class="card-title">🚀 Swaps Resolved</div>
            <div class="stats-num" id="stat-resolved-count" style="color: var(--success)">0</div>
            <div class="stats-label">Total Swap Paths Successfully Coordinated</div>
        </div>
    </div>

    <div class="grid" style="grid-template-columns: 2fr 1.3fr">
        <!-- Main Price Catalog -->
        <div class="card">
            <div class="card-header">
                <div class="card-title">📋 Live USD Token Catalog</div>
                <div class="badge badge-resolved" id="catalog-count">0 Tokens</div>
            </div>
            <input type="text" id="catalog-search" class="search-bar" placeholder="Filter by Name, Symbol, or Address...">
            <div class="list-container" style="max-height: 480px">
                <table class="table" id="catalog-table">
                    <thead>
                        <tr>
                            <th onclick="setSort('symbol')" style="cursor: pointer; user-select: none;">Token<span id="sort-symbol"> ⇅</span></th>
                            <th onclick="setSort('address')" style="cursor: pointer; user-select: none;">Address<span id="sort-address"> ⇅</span></th>
                            <th onclick="setSort('price')" style="cursor: pointer; user-select: none;">USD Price<span id="sort-price"> ⇅</span></th>
                            <th onclick="setSort('volume')" style="cursor: pointer; user-select: none;">Traded Volume<span id="sort-volume" style="color: var(--primary)"> ▼</span></th>
                            <th onclick="setSort('amount')" style="cursor: pointer; user-select: none;">Amount for $1.00 USD<span id="sort-amount"> ⇅</span></th>
                        </tr>
                    </thead>
                    <tbody>
                        <tr>
                            <td colspan="5" style="text-align: center; color: var(--text-muted)">Loading oracle prices...</td>
                        </tr>
                    </tbody>
                </table>
            </div>
        </div>

        <!-- Recent Realtime Resolutions -->
        <div class="card">
            <div class="card-header">
                <div class="card-title">⚡ Real-Time Resolution Activity</div>
            </div>
            <div class="list-container" id="timeline-list" style="max-height: 540px">
                <div style="text-align: center; color: var(--text-muted); padding: 2rem">Waiting for events...</div>
            </div>
        </div>
    </div>

    <div class="grid" style="grid-template-columns: 1.2fr 1fr">
        <!-- Unresolved Swaps Table -->
        <div class="card">
            <div class="card-header">
                <div class="card-title" style="color: var(--warning)">⏳ Pending Resolution Queue</div>
                <div class="badge badge-unresolved" id="pending-count">0 Swap events</div>
            </div>
            <div class="list-container" style="max-height: 300px">
                <table class="table" id="unresolved-table">
                    <thead>
                        <tr>
                            <th>Tx Hash</th>
                            <th>From Token</th>
                            <th>To Token</th>
                            <th>Swap Amount</th>
                            <th>Timestamp</th>
                        </tr>
                    </thead>
                    <tbody>
                        <tr>
                            <td colspan="5" style="text-align: center; color: var(--text-muted)">No pending swaps in queue.</td>
                        </tr>
                    </tbody>
                </table>
            </div>
        </div>

        <!-- Treasury Token Registry Card -->
        <div class="card">
            <div class="card-header">
                <div class="card-title" style="color: #fbbf24">👑 Treasury Token Registry</div>
                <div class="badge badge-resolved" id="treasury-tokens-count" style="background: rgba(251, 191, 36, 0.15); color: #fbbf24">0 Minted</div>
            </div>
            <div style="margin-bottom: 0.8rem; font-size: 0.85rem; color: var(--text-muted)">
                Active tracking of standard tokens minted by minters. Ignored tokens are hidden from the price catalog.
            </div>
            <div class="list-container" style="max-height: 300px">
                <table class="table" id="treasury-registry-table">
                    <thead>
                        <tr>
                            <th>Token</th>
                            <th>Owner Address</th>
                            <th>Action</th>
                        </tr>
                    </thead>
                    <tbody>
                        <tr>
                            <td colspan="3" style="text-align: center; color: var(--text-muted)">No treasury tokens identified yet.</td>
                        </tr>
                    </tbody>
                </table>
            </div>
        </div>
    </div>

    <script>
        let cachedData = { prices: {}, unresolved: [], resolved: [] };

        async function fetchData() {
            try {
                const response = await fetch('/api/data');
                if (!response.ok) return;
                const data = await response.json();
                
                cachedData = data;
                updateDashboard();
            } catch (err) {
                console.error("Failed fetching data:", err);
            }
        }

        function formatAddress(addr) {
            if (!addr) return '';
            return addr.slice(0, 6) + '...' + addr.slice(-4);
        }

        function updateDashboard() {
            // Update Stats
            const priceEntries = Object.entries(cachedData.prices);
            document.getElementById('stat-tokens-count').innerText = priceEntries.length;
            document.getElementById('stat-unresolved-count').innerText = cachedData.unresolved.length;
            document.getElementById('stat-resolved-count').innerText = cachedData.resolved.length;
            document.getElementById('catalog-count').innerText = `${priceEntries.length} Tokens`;
            document.getElementById('pending-count').innerText = `${cachedData.unresolved.length} Swap events`;

            // Update Price Catalog
            renderCatalog();

            // Update Realtime Timeline
            const timeline = document.getElementById('timeline-list');
            if (cachedData.resolved.length === 0) {
                timeline.innerHTML = '<div style="text-align: center; color: var(--text-muted); padding: 2rem">No resolved swap events recorded.</div>';
            } else {
                timeline.innerHTML = cachedData.resolved.slice().reverse().map(item => {
                    const t = new Date(item.timestamp * 1000).toLocaleTimeString();
                    return `
                        <div class="timeline-item">
                            <div class="timeline-time">${t}</div>
                            <div class="timeline-title">
                                <span>${item.token0.symbol} ➡️ ${item.token1.symbol}</span>
                                <span class="timeline-val">$${parseFloat(item.usd_value).toFixed(2)} USD</span>
                            </div>
                            <div class="timeline-details">
                                Tx: <a class="address-link" target="_blank" href="https://otter.pulsechain.com/tx/0x${item.tx_hash}">${formatAddress(item.tx_hash)}</a><br>
                                Amount: ${parseFloat(item.amount0).toLocaleString()} ${item.token0.symbol} ➡️ ${parseFloat(item.amount1).toLocaleString()} ${item.token1.symbol}
                            </div>
                        </div>
                    `;
                }).join('');
            }

            // Update Unresolved Queue Table
            const unresolvedBody = document.querySelector('#unresolved-table tbody');
            if (cachedData.unresolved.length === 0) {
                unresolvedBody.innerHTML = '<tr><td colspan="5" style="text-align: center; color: var(--text-muted)">No pending swaps in queue.</td></tr>';
            } else {
                unresolvedBody.innerHTML = cachedData.unresolved.map(item => {
                    const t = new Date(item.timestamp * 1000).toLocaleTimeString();
                    return `
                        <tr>
                            <td><a class="address-link" target="_blank" href="https://otter.pulsechain.com/tx/0x${item.tx_hash}">${formatAddress(item.tx_hash)}</a></td>
                            <td><span class="token-symbol-glow">${item.token0.symbol}</span> <span style="font-size:0.8rem; color:var(--text-muted)">(${item.token0.name})</span></td>
                            <td><span class="token-symbol-glow">${item.token1.symbol}</span> <span style="font-size:0.8rem; color:var(--text-muted)">(${item.token1.name})</span></td>
                            <td>${parseFloat(item.amount0).toLocaleString()} ➡️ ${parseFloat(item.amount1).toLocaleString()}</td>
                            <td>${t}</td>
                        </tr>
                    `;
                }).join('');
            }

            // Update Treasury Token Registry Table
            const treasuryBody = document.querySelector('#treasury-registry-table tbody');
            const treasuryEntries = Object.entries(cachedData.treasury_tokens || {});
            
            // Update treasury count badge
            document.getElementById('treasury-tokens-count').innerText = `${treasuryEntries.length} Minted`;

            if (treasuryEntries.length === 0) {
                treasuryBody.innerHTML = '<tr><td colspan="3" style="text-align: center; color: var(--text-muted)">No treasury tokens identified yet.</td></tr>';
            } else {
                treasuryBody.innerHTML = treasuryEntries.map(([addr, token]) => {
                    const isIgnored = token.ignored === true;
                    const name = token.name || 'Unknown';
                    const symbol = token.symbol || 'UNKNOWN';
                    
                    // Display gold crown next to tracked treasury tokens, cross out ignored ones
                    const displaySymbol = isIgnored ? `<del style="color: var(--text-muted)">${symbol}</del>` : `👑 <span class="token-symbol-glow" style="color: #fbbf24">${symbol}</span>`;
                    const displayName = isIgnored ? `<del style="color: var(--text-muted)">${name}</del>` : name;
                    
                    const buttonText = isIgnored ? "Track" : "Ignore";
                    const buttonClass = isIgnored ? "btn-track" : "btn-ignore";
                    const nextIgnored = !isIgnored;
                    
                    return `
                        <tr>
                            <td>
                                <div>${displaySymbol}</div>
                                <div style="font-size:0.75rem; color:var(--text-muted)">${displayName}</div>
                            </td>
                            <td><a class="address-link" target="_blank" href="https://otter.pulsechain.com/address/${addr}">${formatAddress(addr)}</a></td>
                            <td>
                                <button class="btn ${buttonClass}" onclick="toggleIgnoreToken('${addr}', ${nextIgnored})">${buttonText}</button>
                            </td>
                        </tr>
                    `;
                }).join('');
            }
        }

        let currentSort = { column: 'volume', direction: 'desc' };

        function setSort(column) {
            if (currentSort.column === column) {
                currentSort.direction = currentSort.direction === 'asc' ? 'desc' : 'asc';
            } else {
                currentSort.column = column;
                currentSort.direction = 'desc';
            }
            updateSortIndicators();
            renderCatalog();
        }

        function updateSortIndicators() {
            const columns = ['symbol', 'address', 'price', 'volume', 'amount'];
            columns.forEach(col => {
                const el = document.getElementById(`sort-${col}`);
                if (!el) return;
                if (currentSort.column === col) {
                    el.innerText = currentSort.direction === 'asc' ? ' ▲' : ' ▼';
                    el.style.color = 'var(--primary)';
                } else {
                    el.innerText = ' ⇅';
                    el.style.color = 'var(--text-muted)';
                }
            });
        }

        function renderCatalog() {
            const searchVal = document.getElementById('catalog-search').value.toLowerCase();
            const catalogBody = document.querySelector('#catalog-table tbody');
            const priceEntries = Object.entries(cachedData.prices);

            // Calculate volumes for each token address
            const volumes = {};
            if (cachedData.resolved) {
                cachedData.resolved.forEach(swap => {
                    const addr0 = swap.token0.address.toLowerCase();
                    const addr1 = swap.token1.address.toLowerCase();
                    const val = parseFloat(swap.usd_value) || 0;
                    volumes[addr0] = (volumes[addr0] || 0) + val;
                    volumes[addr1] = (volumes[addr1] || 0) + val;
                });
            }

            const filtered = priceEntries.filter(([addr, val]) => {
                const addrLower = addr.toLowerCase();
                if (cachedData.treasury_tokens && cachedData.treasury_tokens[addrLower] && cachedData.treasury_tokens[addrLower].ignored) {
                    return false;
                }
                return addrLower.includes(searchVal) || 
                       (val && val.symbol && val.symbol.toLowerCase().includes(searchVal)) ||
                       (val && val.name && val.name.toLowerCase().includes(searchVal));
            });

            // Sort logic: Treasury tokens first, then sorted by chosen column
            filtered.sort((a, b) => {
                const addrA = a[0].toLowerCase();
                const addrB = b[0].toLowerCase();
                
                const isTreasuryA = !!(cachedData.treasury_tokens && cachedData.treasury_tokens[addrA] && !cachedData.treasury_tokens[addrA].ignored);
                const isTreasuryB = !!(cachedData.treasury_tokens && cachedData.treasury_tokens[addrB] && !cachedData.treasury_tokens[addrB].ignored);
                
                if (isTreasuryA && !isTreasuryB) return -1;
                if (!isTreasuryA && isTreasuryB) return 1;
                
                let valA, valB;
                const priceA = parseFloat(typeof a[1] === 'object' ? a[1].price : a[1]) || 0;
                const priceB = parseFloat(typeof b[1] === 'object' ? b[1].price : b[1]) || 0;
                
                if (currentSort.column === 'price') {
                    valA = priceA;
                    valB = priceB;
                } else if (currentSort.column === 'volume') {
                    valA = volumes[addrA] || 0;
                    valB = volumes[addrB] || 0;
                } else if (currentSort.column === 'amount') {
                    valA = priceA > 0 ? (1 / priceA) : 0;
                    valB = priceB > 0 ? (1 / priceB) : 0;
                } else if (currentSort.column === 'symbol') {
                    const matchA = cachedData.resolved.find(r => r.token0.address === a[0]) || cachedData.resolved.find(r => r.token1.address === a[0]);
                    const symA = (matchA ? (matchA.token0.address === a[0] ? matchA.token0.symbol : matchA.token1.symbol) : ((a[1] && a[1].symbol) || '')).toLowerCase();
                    
                    const matchB = cachedData.resolved.find(r => r.token0.address === b[0]) || cachedData.resolved.find(r => r.token1.address === b[0]);
                    const symB = (matchB ? (matchB.token0.address === b[0] ? matchB.token0.symbol : matchB.token1.symbol) : ((b[1] && b[1].symbol) || '')).toLowerCase();
                    
                    return currentSort.direction === 'asc' ? symA.localeCompare(symB) : symB.localeCompare(symA);
                } else if (currentSort.column === 'address') {
                    return currentSort.direction === 'asc' ? addrA.localeCompare(addrB) : addrB.localeCompare(addrA);
                }
                
                if (currentSort.direction === 'asc') {
                    return valA - valB;
                } else {
                    return valB - valA;
                }
            });

            if (filtered.length === 0) {
                catalogBody.innerHTML = '<tr><td colspan="5" style="text-align: center; color: var(--text-muted)">No matching tokens found.</td></tr>';
                return;
            }

            catalogBody.innerHTML = filtered.map(([addr, val]) => {
                let name = "Unknown Token";
                let symbol = "UNKNOWN";
                
                if (val && typeof val === 'object') {
                    if (val.name) name = val.name;
                    if (val.symbol) symbol = val.symbol;
                }
                
                // If it's seed stablecoins, display metadata or defaults
                if (addr.includes("15d38573d2feeb82e7ad5187ab8c1d52810b1f07")) { name = "USD Coin from Ethereum"; symbol = "USDC"; }
                else if (addr.includes("a0b86991c6218b36c1d19d4a2e9eb0ce3606eb48")) { name = "USD Coin"; symbol = "USDC"; }
                else if (addr.includes("0xcb81b54a05e0547d2d08c4a9e273a7d4c72b9eb")) { name = "Tether USD"; symbol = "USDT"; }
                else if (addr.includes("dac17f958d2ee523a2206206994597c13d831ec7")) { name = "Tether USD"; symbol = "USDT"; }
                else if (addr.includes("efd766ccb8c15e5e9f813af7b2809857baa53a1f")) { name = "Dai Stablecoin"; symbol = "DAI"; }
                else if (addr.includes("6b175474e89094c44da98b954eedeac495271d0f")) { name = "Dai Stablecoin"; symbol = "DAI"; }
                
                // If the cache contains price object with metadata
                let price = typeof val === 'object' ? val.price : val;
                
                // Search resolved/unresolved to match symbol/name if it was a dynamically resolved token
                const matchResolved = cachedData.resolved.find(r => r.token0.address === addr) || 
                                      cachedData.resolved.find(r => r.token1.address === addr);
                if (matchResolved) {
                    const match = matchResolved.token0.address === addr ? matchResolved.token0 : matchResolved.token1;
                    name = match.name;
                    symbol = match.symbol;
                }

                let isTreasury = false;
                let treasuryOwner = null;
                const addrLower = addr.toLowerCase();
                if (cachedData.treasury_tokens && cachedData.treasury_tokens[addrLower]) {
                    isTreasury = true;
                    treasuryOwner = cachedData.treasury_tokens[addrLower].owner;
                } else if (val && typeof val === 'object') {
                    if (val.is_treasury) isTreasury = val.is_treasury;
                    if (val.treasury_owner) treasuryOwner = val.treasury_owner;
                }

                let priceFloat = parseFloat(price);
                let amountForOneUsd = priceFloat > 0 ? (1 / priceFloat).toLocaleString(undefined, {minimumFractionDigits: 4, maximumFractionDigits: 8}) : 'N/A';
                
                const vol = volumes[addrLower] || 0;
                const volStr = vol > 0 ? `$${vol.toLocaleString(undefined, {minimumFractionDigits: 2, maximumFractionDigits: 2})}` : '$0.00';

                let symbolHtml = isTreasury ? `<span class="token-symbol-glow" style="color: #fbbf24; text-shadow: 0 0 10px rgba(251, 191, 36, 0.4)">👑 ${symbol}</span>` : `<span class="token-symbol-glow">${symbol}</span>`;
                let nameHtml = isTreasury ? `<div style="font-size:0.75rem; color:var(--text-muted)">${name} <span style="color:#fbbf24; font-size:0.7rem; font-weight:600" title="Treasury Owner: ${treasuryOwner}">(Treasury)</span></div>` : `<div style="font-size:0.75rem; color:var(--text-muted)">${name}</div>`;

                return `
                    <tr style="${isTreasury ? 'background: rgba(251, 191, 36, 0.02); border-left: 3px solid #fbbf24;' : ''}">
                        <td>
                            ${symbolHtml}
                            ${nameHtml}
                        </td>
                        <td><a class="address-link" target="_blank" href="https://otter.pulsechain.com/address/${addr}">${formatAddress(addr)}</a></td>
                        <td style="font-family: monospace; font-weight: 700; color: #fff">$${priceFloat.toFixed(8)}</td>
                        <td style="font-family: monospace; font-weight: 700; color: #a78bfa">${volStr}</td>
                        <td style="font-family: monospace; font-weight: 700; color: #34d399">${amountForOneUsd} ${symbol}</td>
                    </tr>
                `;
            }).join('');
        }

        async function toggleIgnoreToken(address, ignored) {
            try {
                const response = await fetch(`/api/ignore?address=${encodeURIComponent(address)}&ignored=${ignored}`);
                if (response.ok) {
                    fetchData();
                } else {
                    console.error("Failed to toggle ignore status");
                }
            } catch (err) {
                console.error("Error toggling ignore status:", err);
            }
        }

        document.getElementById('catalog-search').addEventListener('input', renderCatalog);

        // Fetch updates every 2 seconds
        fetchData();
        setInterval(fetchData, 2000);
    </script>
</body>
</html>
"""

if __name__ == "__main__":
    handler = DashboardHandler
    socketserver.TCPServer.allow_reuse_address = True
    with socketserver.TCPServer(("", PORT), handler) as httpd:
        print(f"Dashboard web server active on http://localhost:{PORT}")
        httpd.serve_forever()
