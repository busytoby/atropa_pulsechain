function escapeHtml(str) {
    if (!str) return '';
    return str.toString().replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;").replace(/"/g, "&quot;").replace(/'/g, "&#039;");
}

let pools = [];
let groups = new Set();
let currentGroup = 'ALL';
let currentSort = { column: 'volume_usd', direction: 'desc' };
let selectedAddress = null;
let currentPartnerAddress = null;

// Initialize Elements
const searchInput = document.getElementById('pool-search');
const groupFiltersContainer = document.getElementById('group-filters');
const poolTableBody = document.querySelector('#pool-table tbody');
const detailModal = document.getElementById('detail-modal');

async function fetchPools() {
    try {
        const res = await fetch('/api/nonukes/pools');
        const data = await res.json();
        if (data && data.success) {
            pools = data.pools;
            extractGroups();
            renderStats();
            renderGroupFilters();
            renderPools();
        } else {
            pools = [];
            extractGroups();
            renderStats();
            renderGroupFilters();
            renderPools();
        }
    } catch (err) {
        console.error("Error fetching pools:", err);
        pools = [];
        extractGroups();
        renderStats();
        renderGroupFilters();
        renderPools();
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
        html += `<button class="filter-chip ${currentGroup === g ? 'active' : ''}" onclick="setGroupFilter(this.getAttribute('data-group'))" data-group="${escapeHtml(g)}">${escapeHtml(g)}</button>`;
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
        const matchSearch = p.symbol.toLowerCase().includes(searchVal) || p.address.toLowerCase().includes(searchVal);
        return matchGroup && matchSearch;
    });
    
    // Sort
    filtered.sort((a, b) => {
        let valA, valB;
        if (currentSort.column === 'reserves') {
            valA = a.reserves ? ((a.reserves.token0 || 0) + (a.reserves.token1 || 0)) : 0;
            valB = b.reserves ? ((b.reserves.token0 || 0) + (b.reserves.token1 || 0)) : 0;
        } else {
            valA = a[currentSort.column];
            valB = b[currentSort.column];
        }
        
        if (typeof valA === 'string') {
            valB = valB || '';
            return currentSort.direction === 'asc' ? valA.localeCompare(valB) : valB.localeCompare(valA);
        } else {
            valA = valA || 0;
            valB = valB || 0;
            return currentSort.direction === 'asc' ? valA - valB : valB - valA;
        }
    });
    
    if (filtered.length === 0) {
        poolTableBody.innerHTML = `<tr><td colspan="6" class="text-center text-muted" style="text-align: center; padding: 2rem;">No pools matching current filters.</td></tr>`;
        return;
    }
    
    poolTableBody.innerHTML = filtered.map(p => {
        const volStr = `$${(p.volume_usd || 0).toLocaleString(undefined, {minimumFractionDigits: 2, maximumFractionDigits: 2})}`;
        const formatAddr = p.address.slice(0, 6) + '...' + p.address.slice(-4);
        const token0Val = p.reserves && typeof p.reserves.token0 === 'number' ? p.reserves.token0 : 0;
        const token1Val = p.reserves && typeof p.reserves.token1 === 'number' ? p.reserves.token1 : 0;
        return `
            <tr onclick="openPoolDetails(this.getAttribute('data-address'))" data-address="${escapeHtml(p.address)}">
                <td>
                    <div style="font-weight: 700; color: #fff;">${escapeHtml(p.symbol)}</div>
                    <div style="font-size:0.75rem; color:var(--text-muted); font-family:'Share Tech Mono'">${escapeHtml(formatAddr)}</div>
                </td>
                <td><span class="btn" style="padding: 0.15rem 0.5rem; font-size:0.7rem; border-color:${p.version === 'V1' ? 'var(--cyan)' : 'var(--magenta)'}">${escapeHtml(p.version)}</span></td>
                <td style="color:var(--magenta); font-weight:600">${escapeHtml(p.target_group)}</td>
                <td style="font-family:'Share Tech Mono'">${token0Val.toFixed(2)} / ${token1Val.toFixed(2)}</td>
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
            currentPartnerAddress = data.partner_address;
            document.getElementById('modal-title').innerText = `${data.reserves.token1_symbol} / ${data.reserves.token0_symbol} Details`;
            
            // Render reserves
            document.getElementById('res0-lbl').innerText = data.reserves.token0_symbol;
            document.getElementById('res0-val').innerText = Number(data.reserves.token0).toFixed(4);
            document.getElementById('res1-lbl').innerText = data.reserves.token1_symbol;
            document.getElementById('res1-val').innerText = Number(data.reserves.token1).toFixed(4);
            
            // Render swaps
            const historyBody = document.querySelector('#history-table tbody');
            if (!data.swap_history || data.swap_history.length === 0) {
                historyBody.innerHTML = `<tr><td colspan="5" class="text-center text-muted" style="text-align: center; padding: 1rem;">No swaps recorded for this pool.</td></tr>`;
            } else {
                historyBody.innerHTML = data.swap_history.map(swap => {
                    const shortHash = swap.tx_hash.slice(0, 6) + '...' + swap.tx_hash.slice(-4);
                    const timeStr = new Date(swap.timestamp * 1000).toLocaleTimeString();
                    return `
                        <tr>
                            <td><a href="https://otter.pulsechain.com/tx/${escapeHtml(swap.tx_hash)}" target="_blank" style="color:var(--cyan); text-decoration:none;">${escapeHtml(shortHash)}</a></td>
                            <td style="font-family:'Share Tech Mono'">${Number(swap.amount0).toFixed(4)}</td>
                            <td style="font-family:'Share Tech Mono'">${Number(swap.amount1).toFixed(4)}</td>
                            <td style="font-family:'Share Tech Mono'; color:var(--green)">$${Number(swap.usd_value).toFixed(2)}</td>
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
    
    if (!trends || trends.length === 0 || typeof trends[0] !== 'object') {
        container.innerHTML = `<div class="text-muted">No price trend data available.</div>`;
        return;
    }
    
    // Width and Height of viewBox
    const w = 400;
    const h = 200;
    const padding = 20;
    
    let minTime = Infinity, maxTime = -Infinity;
    let minPrice = Infinity, maxPrice = -Infinity;
    trends.forEach(t => {
        if (t.timestamp < minTime) minTime = t.timestamp;
        if (t.timestamp > maxTime) maxTime = t.timestamp;
        if (t.price < minPrice) minPrice = t.price;
        if (t.price > maxPrice) maxPrice = t.price;
    });
    
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

// Expose functions to window for onclick handlers in index.html
window.setGroupFilter = setGroupFilter;
window.changeSort = changeSort;
window.openPoolDetails = openPoolDetails;

// MetaMask Integration
let userAccount = null;

const btnConnect = document.getElementById('btn-connect-wallet');
const yueInput = document.getElementById('yue-address-input');
const btnReact = document.getElementById('btn-react-yue');

// Load saved YUE Address from localStorage if available
if (yueInput) {
    yueInput.value = localStorage.getItem('yue_address') || '';
    yueInput.addEventListener('input', (e) => {
        localStorage.setItem('yue_address', e.target.value);
    });
}

// Helper for decoding ABI strings
function decodeAbiString(hex) {
    if (!hex || hex === '0x' || hex.length < 130) return '';
    try {
        const len = parseInt(hex.slice(66, 130), 16);
        const strHex = hex.slice(130, 130 + len * 2);
        let str = '';
        for (let i = 0; i < strHex.length; i += 2) {
            str += String.fromCharCode(parseInt(strHex.substr(i, 2), 16));
        }
        return str;
    } catch (e) {
        return '';
    }
}

// Helper for encoding single ABI string parameter
function encodeAbiString(str) {
    const offset = '0000000000000000000000000000000000000000000000000000000000000020';
    let hexStr = '';
    for (let i = 0; i < str.length; i++) {
        hexStr += str.charCodeAt(i).toString(16).padStart(2, '0');
    }
    const lenHex = str.length.toString(16).padStart(64, '0');
    const paddedHexStr = hexStr.padEnd(Math.ceil(hexStr.length / 64) * 64, '0');
    return offset + lenHex + paddedHexStr;
}

// Helper for encoding two ABI string parameters
function encodeAbiTwoStrings(str1, str2) {
    const offset1Hex = '0000000000000000000000000000000000000000000000000000000000000040';
    
    let hex1 = '';
    for (let i = 0; i < str1.length; i++) {
        hex1 += str1.charCodeAt(i).toString(16).padStart(2, '0');
    }
    const len1Hex = str1.length.toString(16).padStart(64, '0');
    const paddedHex1 = hex1.padEnd(Math.ceil(hex1.length / 64) * 64, '0');
    const data1 = len1Hex + paddedHex1;
    
    const offset2 = 64 + (data1.length / 2);
    const offset2Hex = offset2.toString(16).padStart(64, '0');
    
    let hex2 = '';
    for (let i = 0; i < str2.length; i++) {
        hex2 += str2.charCodeAt(i).toString(16).padStart(2, '0');
    }
    const len2Hex = str2.length.toString(16).padStart(64, '0');
    const paddedHex2 = hex2.padEnd(Math.ceil(hex2.length / 64) * 64, '0');
    const data2 = len2Hex + paddedHex2;
    
    return offset1Hex + offset2Hex + data1 + data2;
}

// LAU Elements
const lauAddressInput = document.getElementById('lau-address-input');
const btnLoadLau = document.getElementById('btn-load-lau');
const lauProfileUsername = document.getElementById('lau-profile-username');
const lauNewUsername = document.getElementById('lau-new-username');
const btnUpdateUsername = document.getElementById('btn-update-username');
const lauFactoryAddressInput = document.getElementById('lau-factory-address-input');
const lauDeployName = document.getElementById('lau-deploy-name');
const lauDeploySymbol = document.getElementById('lau-deploy-symbol');
const btnDeployLau = document.getElementById('btn-deploy-lau');
const lauDisconnected = document.getElementById('lau-disconnected');
const lauConnected = document.getElementById('lau-connected');

// Load stored values
if (lauAddressInput) {
    lauAddressInput.value = localStorage.getItem('lau_address') || '';
    lauAddressInput.addEventListener('input', (e) => {
        localStorage.setItem('lau_address', e.target.value);
    });
}
if (lauFactoryAddressInput) {
    lauFactoryAddressInput.value = localStorage.getItem('laufactory_address') || '0x8013Dd64084e9c9122567563AA86981F4C20576B';
    lauFactoryAddressInput.addEventListener('input', (e) => {
        localStorage.setItem('laufactory_address', e.target.value);
        document.getElementById('laufactory-address-lbl').innerText = e.target.value;
    });
    document.getElementById('laufactory-address-lbl').innerText = lauFactoryAddressInput.value;
}

async function connectWallet() {
    if (typeof window.ethereum !== 'undefined') {
        try {
            const accounts = await window.ethereum.request({ method: 'eth_requestAccounts' });
            userAccount = accounts[0];
            btnConnect.innerText = userAccount.slice(0, 6) + '...' + userAccount.slice(-4);
            btnConnect.style.borderColor = 'var(--cyan)';
            btnConnect.style.color = 'var(--cyan)';
            
            // Toggle LAU interface
            if (lauDisconnected) lauDisconnected.style.display = 'none';
            if (lauConnected) lauConnected.style.display = 'grid';
            
            // Auto-load LAU if saved
            if (lauAddressInput && lauAddressInput.value) {
                loadLauInfo();
            }
        } catch (err) {
            console.error("User rejected wallet connection", err);
        }
    } else {
        alert("MetaMask is not installed. Please install it to interact with PulseChain.");
    }
}

async function loadLauInfo() {
    const lauAddress = lauAddressInput.value.trim();
    if (!lauAddress || lauAddress.length < 42 || !lauAddress.startsWith('0x')) {
        alert("Please enter a valid LAU contract address.");
        return;
    }

    try {
        // Query Username view function (selector 0xc584460d)
        const result = await window.ethereum.request({
            method: 'eth_call',
            params: [{
                to: lauAddress,
                data: '0xc584460d'
            }, 'latest']
        });
        
        const username = decodeAbiString(result);
        lauProfileUsername.innerText = username || 'No Username Set';
    } catch (err) {
        console.error("Error reading LAU contract:", err);
        lauProfileUsername.innerText = "Error Loading Contract";
    }
}

async function updateLauUsername() {
    const lauAddress = lauAddressInput.value.trim();
    const newUsername = lauNewUsername.value.trim();
    
    if (!lauAddress || lauAddress.length < 42 || !lauAddress.startsWith('0x')) {
        alert("Please enter a valid LAU contract address.");
        return;
    }
    if (!newUsername) {
        alert("Please enter a new username.");
        return;
    }

    try {
        // Encode Username(string) call (selector 0xc6bf2b65)
        const encodedStr = encodeAbiString(newUsername);
        const calldata = '0xc6bf2b65' + encodedStr;

        const txHash = await window.ethereum.request({
            method: 'eth_sendTransaction',
            params: [{
                from: userAccount,
                to: lauAddress,
                data: calldata
            }]
        });

        alert(`Transaction submitted! Hash: ${txHash}`);
    } catch (err) {
        console.error("Error updating username:", err);
        alert(`Failed to update username: ${err.message || err}`);
    }
}

async function deployNewLau() {
    const factoryAddr = lauFactoryAddressInput.value.trim();
    const name = lauDeployName.value.trim();
    const symbol = lauDeploySymbol.value.trim();

    if (!factoryAddr || factoryAddr.length < 42 || !factoryAddr.startsWith('0x')) {
        alert("Please enter a valid LAUFactory address.");
        return;
    }
    if (!name || !symbol) {
        alert("Please specify a Name and Symbol.");
        return;
    }

    try {
        // Encode New(string,string) call (selector 0x8c6dc5ec)
        const encodedParams = encodeAbiTwoStrings(name, symbol);
        const calldata = '0x8c6dc5ec' + encodedParams;

        const txHash = await window.ethereum.request({
            method: 'eth_sendTransaction',
            params: [{
                from: userAccount,
                to: factoryAddr,
                data: calldata
            }]
        });

        alert(`LAU Deployment transaction submitted! Hash: ${txHash}`);
    } catch (err) {
        console.error("Error deploying LAU:", err);
        alert(`Failed to deploy LAU: ${err.message || err}`);
    }
}

async function reactYue() {
    if (!userAccount) {
        alert("Please connect your MetaMask wallet first.");
        return;
    }
    const yueAddress = yueInput.value.trim();
    if (!yueAddress || yueAddress.length < 42 || !yueAddress.startsWith('0x')) {
        alert("Please enter a valid YUE contract address.");
        return;
    }
    if (!currentPartnerAddress) {
        alert("No partner (Qing) token address found for this pool.");
        return;
    }

    try {
        // Send transaction to YUE.React(Qing) using MetaMask
        // React(address) method signature: React(address) -> 0x6bc32fe2
        // selector for React(address): 0x6bc32fe2 + 32 bytes address padded
        const cleanPartnerAddr = currentPartnerAddress.startsWith('0x') ? currentPartnerAddress.slice(2) : currentPartnerAddress;
        const paddedAddress = cleanPartnerAddr.padStart(64, '0');
        const calldata = '0x6bc32fe2' + paddedAddress;

        const txParams = {
            from: userAccount,
            to: yueAddress,
            data: calldata
        };

        const txHash = await window.ethereum.request({
            method: 'eth_sendTransaction',
            params: [txParams],
        });

        alert(`Transaction submitted! Hash: ${txHash}`);
    } catch (err) {
        console.error("Transaction failed", err);
        alert(`Transaction failed: ${err.message || err}`);
    }
}

if (btnConnect) btnConnect.addEventListener('click', connectWallet);
if (btnReact) btnReact.addEventListener('click', reactYue);
if (btnLoadLau) btnLoadLau.addEventListener('click', loadLauInfo);
if (btnUpdateUsername) btnUpdateUsername.addEventListener('click', updateLauUsername);
if (btnDeployLau) btnDeployLau.addEventListener('click', deployNewLau);

// Initial Setup
fetchPools();
setInterval(fetchPools, 5000);
