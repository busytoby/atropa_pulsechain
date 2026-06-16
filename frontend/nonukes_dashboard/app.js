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
            
            // Auto-load connected wallet YUE address
            autoLoadYueAddress();
        }
    } catch (err) {
        console.error("Error fetching pool details:", err);
    }
}

async function autoLoadYueAddress() {
    if (!userAccount) {
        if (btnReact) {
            btnReact.disabled = true;
            btnReact.innerText = "Connect Wallet First";
        }
        return;
    }
    try {
        // Chi() -> selector 0x1cb77ea7
        const result = await window.ethereum.request({
            method: 'eth_call',
            params: [{
                from: userAccount,
                to: deployedAddresses.SEI,
                data: '0x1cb77ea7'
            }, 'latest']
        });
        
        if (result && result !== '0x' && result !== '0x' + '0'.repeat(64)) {
            // result is raw bytes containing Yue address slot and LAU address slot
            // parse the Yue address from slot 1
            const cleanResult = result.startsWith('0x') ? result.slice(2) : result;
            const yueAddress = '0x' + cleanResult.slice(24, 64); // parse 20-byte address from padded 32-byte slot
            console.log("Resolved active YUE address from SEI:", yueAddress);
            if (yueInput) {
                yueInput.value = yueAddress;
                localStorage.setItem('yue_address', yueAddress);
            }
            if (btnReact) {
                btnReact.disabled = false;
                btnReact.innerText = "Raise the Bar (React)";
            }
        } else {
            console.log("No active YUE found for:", userAccount);
            if (btnReact) {
                btnReact.disabled = true;
                btnReact.innerText = "YUE Not Deployed (Setup Required)";
            }
        }
    } catch (e) {
        console.error("Failed to query SEI.Chi() for YUE address:", e);
        if (btnReact) {
            btnReact.disabled = true;
            btnReact.innerText = "YUE Not Deployed (Setup Required)";
        }
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

// Global Deployed Addresses
let deployedAddresses = {
    LAUFactory: '0x8013Dd64084e9c9122567563AA86981F4C20576B',
    SEI: '0xfDFB68F5195DF817824Ee881CF63E94402eEc46A',
    CHOA: '0xe4BD72fC5498d94fD5c364015696653DeF6e8F61',
    CHO: '0xCb591105c732C55bc81e109dCAeEd9E136AC6c89'
};

async function fetchConfigAddresses() {
    try {
        const res = await fetch('/api/config');
        const data = await res.json();
        if (data && data.networks && data.networks.localhost) {
            deployedAddresses = { ...deployedAddresses, ...data.networks.localhost };
            console.log("Loaded deployed addresses from config:", deployedAddresses);
            const factoryLbl = document.getElementById('laufactory-address-lbl');
            if (factoryLbl) factoryLbl.innerText = deployedAddresses.LAUFactory;
            if (lauFactoryAddressInput) lauFactoryAddressInput.value = deployedAddresses.LAUFactory;
        }
    } catch (e) {
        console.error("Failed to load backend config addresses, using defaults:", e);
    }
}

// LAU Wizard Elements
const lauAddressInput = document.getElementById('lau-address-input');
const btnLoadLau = document.getElementById('btn-load-lau');
const lauProfileUsername = document.getElementById('lau-profile-username');
const lauNewUsername = document.getElementById('lau-new-username');
const btnUpdateUsername = document.getElementById('btn-update-username');
const btnEnterCho = document.getElementById('btn-enter-cho');
const lauFactoryAddressInput = document.getElementById('lau-factory-address-input');
const lauDeployName = document.getElementById('lau-deploy-name');
const lauDeploySymbol = document.getElementById('lau-deploy-symbol');
const btnDeployLau = document.getElementById('btn-deploy-lau');

const yueDeployName = document.getElementById('yue-deploy-name');
const yueDeploySymbol = document.getElementById('yue-deploy-symbol');
const btnDeployYue = document.getElementById('btn-deploy-yue');
const btnActivatePlay = document.getElementById('btn-activate-play');
const lblStatusComplete = document.getElementById('lbl-status-complete');

const btnWizardConnect = document.getElementById('btn-wizard-connect');
const btnNextToStep4 = document.getElementById('btn-next-to-step-4');
const btnNextToStep5 = document.getElementById('btn-next-to-step-5');

// Active wizard step tracking
let activeStep = 1;

function goToStep(step) {
    activeStep = step;
    for (let i = 1; i <= 5; i++) {
        const tab = document.getElementById(`step-tab-${i}`);
        const panel = document.getElementById(`wizard-step-${i}`);
        if (tab) {
            if (i === step) {
                tab.classList.add('active');
                tab.style.borderColor = 'var(--cyan)';
                tab.style.color = 'var(--cyan)';
            } else if (i < step) {
                tab.classList.remove('active');
                tab.style.borderColor = 'var(--green)';
                tab.style.color = 'var(--green)';
            } else {
                tab.classList.remove('active');
                tab.style.borderColor = 'rgba(255,255,255,0.1)';
                tab.style.color = 'var(--text-muted)';
            }
        }
        if (panel) {
            panel.style.display = (i === step) ? 'block' : 'none';
        }
    }
}
window.goToStep = goToStep;

// LAU history tracking helper
async function updateLauHistoryDisplay() {
    const listContainer = document.getElementById('lau-history-list');
    if (!listContainer) return;
    
    let history = [];
    try {
        history = JSON.parse(localStorage.getItem('lau_history') || '[]');
    } catch(e) {}
    
    // Automatically scan the local chain for all LAU tokens ever created by querying the LAUFactory deployment transactions
    if (window.ethereum) {
        try {
            const factoryAddr = deployedAddresses.LAUFactory ? deployedAddresses.LAUFactory.toLowerCase() : '';
            if (factoryAddr) {
                const blockNumHex = await window.ethereum.request({ method: 'eth_blockNumber' });
                const maxBlock = parseInt(blockNumHex, 16);
                
                // Fetch transaction details of all blocks to recover LAU Factory deploys
                for (let i = 0; i <= maxBlock; i++) {
                    const blockHex = '0x' + i.toString(16);
                    const block = await window.ethereum.request({
                        method: 'eth_getBlockByNumber',
                        params: [blockHex, true]
                    });
                    if (block && block.transactions) {
                        for (const tx of block.transactions) {
                            if (tx.to && tx.to.toLowerCase() === factoryAddr) {
                                const receipt = await window.ethereum.request({
                                    method: 'eth_getTransactionReceipt',
                                    params: [tx.hash]
                                });
                                if (receipt && receipt.logs && receipt.logs.length > 0) {
                                    const log = receipt.logs.find(x => x.address.toLowerCase() !== factoryAddr);
                                    if (log) {
                                        const address = log.address;
                                        if (!history.find(x => x.address.toLowerCase() === address.toLowerCase())) {
                                            // Get the name/username of the deployed token
                                            try {
                                                const nameResult = await window.ethereum.request({
                                                    method: 'eth_call',
                                                    params: [{ to: address, data: '0x06fdde03' }, 'latest']
                                                });
                                                let name = 'LAU Token';
                                                if (nameResult && nameResult !== '0x') {
                                                    const cleanRes = nameResult.startsWith('0x') ? nameResult.slice(2) : nameResult;
                                                    const offset = parseInt(cleanRes.slice(0, 64), 16) * 2;
                                                    const len = parseInt(cleanRes.slice(offset, offset + 64), 16);
                                                    const hexStr = cleanRes.slice(offset + 64, offset + 64 + len * 2);
                                                    let decoded = '';
                                                    for (let j = 0; j < hexStr.length; j += 2) {
                                                        decoded += String.fromCharCode(parseInt(hexStr.slice(j, j + 2), 16));
                                                    }
                                                    if (decoded) name = decoded;
                                                }
                                                history.push({ name, address });
                                            } catch (e) {
                                                history.push({ name: 'LAU Token', address });
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                localStorage.setItem('lau_history', JSON.stringify(history));
            }
        } catch (err) {
            console.error('Auto scan for LAU deployments failed:', err);
        }
    }
    
    if (history.length === 0) {
        listContainer.innerHTML = '<div style="color: var(--text-muted); font-style: italic; padding: 2px 4px; font-size: 0.75rem;">None deployed or imported yet</div>';
        if (lauAddressInput) {
            lauAddressInput.innerHTML = '<option value="">-- No LAU Loaded --</option>';
        }
        return;
    }
    
    listContainer.innerHTML = history.map(item => {
        const shortAddr = item.address.slice(0, 8) + '...' + item.address.slice(-6);
        return `
            <div style="display: flex; justify-content: space-between; align-items: center; background: rgba(255,255,255,0.03); border-radius: 4px; padding: 4px 8px; font-size: 0.8rem; border: 1px solid rgba(255,255,255,0.05);">
                <div style="cursor: pointer; color: var(--cyan); font-family: 'Share Tech Mono'; font-weight: bold;" onclick="selectLauFromHistory('${item.address}')" title="Click to Load LAU">
                    ${item.name} (${shortAddr})
                </div>
                <button onclick="removeLauFromHistory('${item.address}')" style="background: none; border: none; color: var(--magenta); cursor: pointer; font-size: 0.75rem;">✕</button>
            </div>
        `;
    }).join('');
    if (lauAddressInput) {
        const currentVal = localStorage.getItem('lau_address') || '';
        lauAddressInput.innerHTML = '<option value="">-- Select or scan a LAU --</option>' + 
            history.map(item => `<option value="${item.address}">${item.name} (${item.address.slice(0, 8)}...${item.address.slice(-6)})</option>`).join('');
        if (currentVal && history.some(item => item.address.toLowerCase() === currentVal.toLowerCase())) {
            lauAddressInput.value = currentVal;
        } else {
            lauAddressInput.value = '';
        }
    }
}

window.selectLauFromHistory = function(address) {
    if (lauAddressInput) {
        lauAddressInput.value = address;
        localStorage.setItem('lau_address', address);
    }
    loadLauInfo();
};

window.removeLauFromHistory = function(address) {
    let history = [];
    try {
        history = JSON.parse(localStorage.getItem('lau_history') || '[]');
    } catch(e) {}
    history = history.filter(x => x.address !== address);
    localStorage.setItem('lau_history', JSON.stringify(history));
    updateLauHistoryDisplay();
};

function addLauToHistory(name, address) {
    let history = [];
    try {
        history = JSON.parse(localStorage.getItem('lau_history') || '[]');
    } catch(e) {}
    if (!history.find(x => x.address === address)) {
        history.push({ name, address });
        localStorage.setItem('lau_history', JSON.stringify(history));
    }
    updateLauHistoryDisplay();
}

// Load stored values
if (lauAddressInput) {
    lauAddressInput.value = localStorage.getItem('lau_address') || '';
    const onLauAddressChange = (val) => {
        localStorage.setItem('lau_address', val);
        if (val.trim().startsWith('0x') && val.trim().length >= 42) {
            if (btnNextToStep4) btnNextToStep4.disabled = false;
        } else {
            if (btnNextToStep4) btnNextToStep4.disabled = true;
        }
        checkActiveLau();
    };
    lauAddressInput.addEventListener('input', (e) => onLauAddressChange(e.target.value));
    lauAddressInput.addEventListener('change', (e) => onLauAddressChange(e.target.value));
    // Render initial list
    updateLauHistoryDisplay();
}

async function checkActiveLau() {
    if (!userAccount) return;
    try {
        // GetUserTokenAddress(address) -> selector 0xe80c28bf
        const cleanAddr = userAccount.startsWith('0x') ? userAccount.slice(2) : userAccount;
        const paddedAddr = cleanAddr.padStart(64, '0');
        const calldata = '0xe80c28bf' + paddedAddr;

        const result = await window.ethereum.request({
            method: 'eth_call',
            params: [{
                to: deployedAddresses.CHO,
                data: calldata
            }, 'latest']
        });

        const choStatusEl = document.getElementById('cho-entry-status');
        const enterChoBtn = document.getElementById('btn-enter-cho');

        // The result will be a 32-byte word containing the address
        if (result && result !== '0x' && result !== '0x' + '0'.repeat(64)) {
            // Clean the address
            const lauAddress = '0x' + result.slice(-40);
            console.log("Found active LAU address from CHO:", lauAddress);
            
            if (choStatusEl) {
                const shortLau = lauAddress.slice(0, 8) + '...' + lauAddress.slice(-6);
                choStatusEl.innerText = `Entered (LAU: ${shortLau})`;
                choStatusEl.style.color = 'var(--green)';
            }
            if (enterChoBtn) {
                enterChoBtn.style.display = 'none';
            }

            // Check if the loaded address matches, update if not
            const currentLoadedLau = localStorage.getItem('lau_address') || '';
            if (currentLoadedLau.toLowerCase() !== lauAddress.toLowerCase()) {
                if (lauAddressInput) {
                    lauAddressInput.value = lauAddress;
                    localStorage.setItem('lau_address', lauAddress);
                }
                try {
                    const name = document.getElementById("lau-profile-username")?.innerText || "LAU Token";
                    addLauToHistory(name, lauAddress);
                } catch(e) {}
                await loadLauInfo();
            }
            // Automatically advance to Step 4 since they already have a LAU!
            goToStep(4);
        } else {
            console.log("No active LAU address registered on CHO for:", userAccount);
            if (choStatusEl) {
                choStatusEl.innerText = 'Not Entered';
                choStatusEl.style.color = 'var(--magenta)';
            }
            
            // Check if the loaded/selected option in the dropdown is a valid LAU
            const currentVal = lauAddressInput ? lauAddressInput.value : '';
            if (enterChoBtn) {
                if (currentVal && currentVal.startsWith('0x') && currentVal.length >= 42) {
                    enterChoBtn.style.display = 'inline-block';
                } else {
                    enterChoBtn.style.display = 'none';
                }
            }
        }
    } catch (err) {
        console.error("Failed to query CHO for active LAU:", err);
    }
}

async function enterCho() {
    const lauAddress = lauAddressInput.value.trim();
    if (!lauAddress || lauAddress.length < 42 || !lauAddress.startsWith('0x')) {
        alert("Please load or select a valid LAU first.");
        return;
    }
    try {
        // Enter(address) -> selector 0x37a2e752
        const cleanAddr = lauAddress.startsWith('0x') ? lauAddress.slice(2) : lauAddress;
        const paddedAddr = cleanAddr.padStart(64, '0');
        const calldata = '0x37a2e752' + paddedAddr;

        const txHash = await window.ethereum.request({
            method: 'eth_sendTransaction',
            params: [{
                from: userAccount,
                to: deployedAddresses.CHO,
                data: calldata
            }]
        });
        alert(`CHO Entry transaction submitted! Hash: ${txHash}`);
        
        let receipt = null;
        for (let i = 0; i < 20; i++) {
            try {
                receipt = await window.ethereum.request({
                    method: 'eth_getTransactionReceipt',
                    params: [txHash]
                });
                if (receipt && (receipt.blockNumber || receipt.status)) break;
            } catch (e) {}
            await new Promise(r => setTimeout(r, 1000));
        }

        await checkActiveLau();
    } catch (err) {
        console.error("Failed to enter CHO:", err);
        alert(`Failed to enter CHO: ${err.message || err}`);
    }
}

async function checkActiveYue() {
    if (!userAccount) return;
    try {
        const yueStatusEl = document.getElementById('yue-status-lbl');
        const playStatusEl = document.getElementById('play-status-lbl');
        const yueDeployBtn = document.getElementById('btn-deploy-yue');

        // Chi() -> selector 0x1cb77ea7
        const result = await window.ethereum.request({
            method: 'eth_call',
            params: [{
                to: deployedAddresses.SEI,
                data: '0x1cb77ea7'
            }, 'latest']
        });

        if (result && result !== '0x' && result !== '0x' + '0'.repeat(64)) {
            // The result will be two 32-byte words: [Yue address, Lau address]
            const cleanRes = result.startsWith('0x') ? result.slice(2) : result;
            const yueAddress = '0x' + cleanRes.slice(24, 64);
            const lauAddress = '0x' + cleanRes.slice(88, 128);
            
            console.log("Found active YUE address from SEI:", yueAddress, "for LAU:", lauAddress);
            
            localStorage.setItem('yue_address', yueAddress);
            const yueInput = document.getElementById('yue-address-input');
            if (yueInput) yueInput.value = yueAddress;

            if (yueStatusEl) {
                const shortYue = yueAddress.slice(0, 8) + '...' + yueAddress.slice(-6);
                yueStatusEl.innerText = `Started (YUE: ${shortYue})`;
                yueStatusEl.style.color = 'var(--green)';
            }
            if (yueDeployBtn) {
                yueDeployBtn.innerText = "YUE Already Started";
                yueDeployBtn.disabled = true;
            }
            if (btnNextToStep5) btnNextToStep5.disabled = false;

            // Check if Play has been activated (YUE balance > 0)
            // balanceOf(address) -> selector 0x70a08231
            const cleanUserAddr = userAccount.startsWith('0x') ? userAccount.slice(2) : userAccount;
            const paddedUserAddr = cleanUserAddr.padStart(64, '0');
            const balCalldata = '0x70a08231' + paddedUserAddr;

            try {
                const balResult = await window.ethereum.request({
                    method: 'eth_call',
                    params: [{
                        to: yueAddress,
                        data: balCalldata
                    }, 'latest']
                });
                if (balResult && balResult !== '0x') {
                    const balance = parseInt(balResult, 16);
                    if (balance > 0) {
                        if (playStatusEl) {
                            playStatusEl.innerText = `Activated & Minted (${(balance / 10**18).toFixed(2)} YUE)`;
                            playStatusEl.style.color = 'var(--green)';
                        }
                        if (btnActivatePlay) {
                            btnActivatePlay.innerText = "Play Already Active";
                            btnActivatePlay.disabled = true;
                        }
                        if (lblStatusComplete) {
                            lblStatusComplete.innerHTML = `✓ Setup Complete! Ready to Play NoNukes.`;
                        }
                    } else {
                        if (playStatusEl) {
                            playStatusEl.innerText = 'Not Activated';
                            playStatusEl.style.color = 'var(--magenta)';
                        }
                    }
                }
            } catch (balErr) {
                console.error("Failed to query YUE balance:", balErr);
            }

        } else {
            console.log("No active YUE registered on SEI for:", userAccount);
            if (yueStatusEl) {
                yueStatusEl.innerText = 'Not Started';
                yueStatusEl.style.color = 'var(--magenta)';
            }
            if (yueDeployBtn) {
                yueDeployBtn.innerText = "Start YUE Contract";
                yueDeployBtn.disabled = false;
            }
            if (playStatusEl) {
                playStatusEl.innerText = 'Not Activated';
                playStatusEl.style.color = 'var(--magenta)';
            }
        }
    } catch (err) {
        console.error("Failed to query SEI for active YUE:", err);
    }
}

async function connectWallet() {
    if (typeof window.ethereum !== 'undefined') {
        try {
            const accounts = await window.ethereum.request({ method: 'eth_requestAccounts' });
            userAccount = accounts[0];
            btnConnect.innerText = userAccount.slice(0, 6) + '...' + userAccount.slice(-4);
            btnConnect.style.borderColor = 'var(--cyan)';
            btnConnect.style.color = 'var(--cyan)';
            
            // Step 2 Command Update
            const fundCmd = document.getElementById('code-fund-cmd');
            if (fundCmd) {
                fundCmd.innerText = `node scripts/create_mariarahel_lau.js ${userAccount}`;
            }
            
            // Advance Wizard to Step 2
            goToStep(2);
            
            // Scan for all LAU tokens from the factory first to update history list
            await updateLauHistoryDisplay();

            // Check if user has an existing LAU registered in CHO
            await checkActiveLau();

            // Check if user has an existing YUE registered in SEI and played in CHOA
            await checkActiveYue();
            
            // Auto-load LAU if saved (fallback) or auto-detect first ever created LAU from history if not registered in CHO yet
            if (lauAddressInput && lauAddressInput.value) {
                await loadLauInfo();
            } else {
                try {
                    const history = JSON.parse(localStorage.getItem('lau_history') || '[]');
                    if (history && history.length > 0) {
                        const firstLau = history[history.length - 1].address; // get latest created
                        console.log('Auto-detected previously created LAU:', firstLau);
                        if (lauAddressInput) {
                            lauAddressInput.value = firstLau;
                            localStorage.setItem('lau_address', firstLau);
                        }
                        await loadLauInfo();
                    }
                } catch(e) {}
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
        localStorage.setItem('lau_address', lauAddress);
        if (btnNextToStep4) btnNextToStep4.disabled = false;
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
    const factoryAddr = deployedAddresses.LAUFactory;
    const name = lauDeployName.value.trim();
    const symbol = lauDeploySymbol.value.trim();

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

        alert(`LAU Deployment transaction submitted! Hash: ${txHash}

Waiting for transaction confirmation to automatically load address...`);

        let receipt = null;
        for (let i = 0; i < 20; i++) {
            try {
                receipt = await window.ethereum.request({
                    method: 'eth_getTransactionReceipt',
                    params: [txHash]
                });
                if (receipt && (receipt.blockNumber || receipt.status)) break;
            } catch (e) {}
            await new Promise(r => setTimeout(r, 1000));
        }

        if (receipt && receipt.logs && receipt.logs.length > 0) {
            const lauAddress = receipt.logs.find(x => x.address && x.address.toLowerCase() !== factoryAddr.toLowerCase())?.address || receipt.logs[0].address;
            if (lauAddressInput) {
                lauAddressInput.value = lauAddress;
                localStorage.setItem('lau_address', lauAddress);
            // Save newly deployed LAU to history
            addLauToHistory(name, lauAddress);
            }
            await loadLauInfo();
            alert(`LAU contract auto-resolved and loaded successfully!
Address: ${lauAddress}`);
        } else {
            alert(`Transaction confirmed but could not automatically parse address. Please verify logs manually.`);
        }
    } catch (err) {
        console.error("Error deploying LAU:", err);
        alert(`Failed to deploy LAU: ${err.message || err}`);
    }
}

// ABI encoding helper for SEI.Start(address,string,string) -> selector 0x78519019
function encodeStartParams(lauAddr, yueName, yueSymbol) {
    const cleanAddr = lauAddr.startsWith('0x') ? lauAddr.slice(2) : lauAddr;
    const param1Hex = cleanAddr.padStart(64, '0');
    
    // Offset for dynamic string 1 (YueName starts at index 96)
    const offset1 = 96;
    const offset1Hex = offset1.toString(16).padStart(64, '0');
    
    // Encode YueName
    let hex1 = '';
    for (let i = 0; i < yueName.length; i++) {
        hex1 += yueName.charCodeAt(i).toString(16).padStart(2, '0');
    }
    const len1Hex = yueName.length.toString(16).padStart(64, '0');
    const paddedHex1 = hex1.padEnd(Math.ceil(hex1.length / 64) * 64, '0');
    const data1 = len1Hex + paddedHex1;
    
    // Offset for dynamic string 2 (YueSymbol starts after YueName data)
    const offset2 = offset1 + (data1.length / 2);
    const offset2Hex = offset2.toString(16).padStart(64, '0');
    
    // Encode YueSymbol
    let hex2 = '';
    for (let i = 0; i < yueSymbol.length; i++) {
        hex2 += yueSymbol.charCodeAt(i).toString(16).padStart(2, '0');
    }
    const len2Hex = yueSymbol.length.toString(16).padStart(64, '0');
    const paddedHex2 = hex2.padEnd(Math.ceil(hex2.length / 64) * 64, '0');
    const data2 = len2Hex + paddedHex2;
    
    return param1Hex + offset1Hex + offset2Hex + data1 + data2;
}

async function startYue() {
    const lauAddress = lauAddressInput.value.trim();
    const name = yueDeployName.value.trim();
    const symbol = yueDeploySymbol.value.trim();

    if (!lauAddress) {
        alert("No LAU address loaded. Go back to Step 3.");
        return;
    }
    if (!name || !symbol) {
        alert("Please specify a YUE Name and Symbol.");
        return;
    }

    try {
        const encodedParams = encodeStartParams(lauAddress, name, symbol);
        const calldata = '0x78519019' + encodedParams;

        const txHash = await window.ethereum.request({
            method: 'eth_sendTransaction',
            params: [{
                from: userAccount,
                to: deployedAddresses.SEI,
                data: calldata
            }]
        });

        alert(`YUE Deployment (Start) transaction submitted! Hash: ${txHash}`);
        
        // Wait for confirmation
        let receipt = null;
        for (let i = 0; i < 20; i++) {
            try {
                receipt = await window.ethereum.request({
                    method: 'eth_getTransactionReceipt',
                    params: [txHash]
                });
                if (receipt && (receipt.blockNumber || receipt.status)) break;
            } catch (e) {}
            await new Promise(r => setTimeout(r, 1000));
        }

        await checkActiveYue();
    } catch (err) {
        console.error("Error starting YUE:", err);
        alert(`Failed to start YUE: ${err.message || err}`);
    }
}

async function activatePlayAndMint() {
    const lauAddress = lauAddressInput.value.trim();
    if (!lauAddress) {
        alert("LAU address not loaded.");
        return;
    }

    try {
        // Encode Play(address) call (selector 0x74ff4718)
        const cleanAddr = lauAddress.startsWith('0x') ? lauAddress.slice(2) : lauAddress;
        const paddedAddress = cleanAddr.padStart(64, '0');
        const calldata = '0x74ff4718' + paddedAddress;

        const txHash = await window.ethereum.request({
            method: 'eth_sendTransaction',
            params: [{
                from: userAccount,
                to: deployedAddresses.CHOA,
                data: calldata
            }]
        });

        alert(`Play Activated & YUE Minted! Hash: ${txHash}`);
        
        // Wait for confirmation
        let receipt = null;
        for (let i = 0; i < 20; i++) {
            try {
                receipt = await window.ethereum.request({
                    method: 'eth_getTransactionReceipt',
                    params: [txHash]
                });
                if (receipt && (receipt.blockNumber || receipt.status)) break;
            } catch (e) {}
            await new Promise(r => setTimeout(r, 1000));
        }

        await checkActiveYue();
    } catch (err) {
        console.error("Error activating play:", err);
        alert(`Failed to activate play: ${err.message || err}`);
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
if (btnWizardConnect) btnWizardConnect.addEventListener('click', connectWallet);
if (btnReact) btnReact.addEventListener('click', reactYue);
if (btnLoadLau) btnLoadLau.addEventListener('click', loadLauInfo);
if (btnUpdateUsername) btnUpdateUsername.addEventListener('click', updateLauUsername);
if (btnEnterCho) btnEnterCho.addEventListener('click', enterCho);
if (btnDeployLau) btnDeployLau.addEventListener('click', deployNewLau);
if (btnDeployYue) btnDeployYue.addEventListener('click', startYue);
if (btnActivatePlay) btnActivatePlay.addEventListener('click', activatePlayAndMint);

if (btnNextToStep4) {
    btnNextToStep4.addEventListener('click', () => goToStep(4));
    // Enable if there's already a saved LAU
    if (localStorage.getItem('lau_address')) {
        btnNextToStep4.disabled = false;
    }
}
if (btnNextToStep5) {
    btnNextToStep5.addEventListener('click', () => goToStep(5));
}

// Initial Setup
fetchConfigAddresses();
fetchPools();
setInterval(fetchPools, 5000);

