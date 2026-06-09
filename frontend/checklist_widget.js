(function() {
    // 1. Inject Styles
    const styleEl = document.createElement('style');
    styleEl.textContent = `
        :root {
            --widget-neon-blue: #00f2fe;
            --widget-neon-purple: #4facfe;
            --widget-neon-magenta: #ff007f;
            --widget-bg: rgba(13, 17, 38, 0.95);
            --widget-border: rgba(0, 242, 254, 0.25);
            --widget-glow: 0 0 15px rgba(0, 242, 254, 0.35);
        }

        #checklist-widget-container {
            position: fixed;
            bottom: 25px;
            right: 25px;
            z-index: 10000;
            font-family: 'Rajdhani', sans-serif;
            color: #f0f4f8;
            user-select: none;
        }

        .checklist-trigger {
            width: 60px;
            height: 60px;
            border-radius: 50%;
            background: linear-gradient(135deg, var(--widget-neon-blue), var(--widget-neon-purple));
            border: 2px solid rgba(255,255,255,0.2);
            box-shadow: var(--widget-glow);
            display: flex;
            align-items: center;
            justify-content: center;
            cursor: pointer;
            font-size: 28px;
            transition: all 0.3s cubic-bezier(0.175, 0.885, 0.32, 1.275);
        }

        .checklist-trigger:hover {
            transform: scale(1.1) rotate(5deg);
            box-shadow: 0 0 25px rgba(0, 242, 254, 0.7);
        }

        .checklist-panel {
            position: absolute;
            bottom: 75px;
            right: 0;
            width: 340px;
            max-height: 480px;
            background: var(--widget-bg);
            border: 1px solid var(--widget-border);
            border-radius: 12px;
            box-shadow: var(--widget-glow);
            backdrop-filter: blur(12px);
            display: flex;
            flex-direction: column;
            overflow: hidden;
            transition: all 0.3s ease;
            transform: translateY(20px) scale(0.95);
            opacity: 0;
            pointer-events: none;
        }

        .checklist-panel.active {
            transform: translateY(0) scale(1);
            opacity: 1;
            pointer-events: auto;
        }

        .checklist-header {
            padding: 12px 15px;
            background: rgba(255,255,255,0.03);
            border-bottom: 1px solid var(--widget-border);
            display: flex;
            justify-content: space-between;
            align-items: center;
        }

        .checklist-header h3 {
            font-family: 'Orbitron', sans-serif;
            font-size: 1.05rem;
            letter-spacing: 1px;
            margin: 0;
            background: linear-gradient(to right, var(--widget-neon-blue), var(--widget-neon-purple));
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            text-shadow: 0 0 5px rgba(0,242,254,0.3);
        }

        .checklist-close {
            background: none;
            border: none;
            color: var(--widget-neon-magenta);
            font-size: 1.2rem;
            cursor: pointer;
            transition: transform 0.2s;
        }

        .checklist-close:hover {
            transform: scale(1.2);
        }

        .checklist-list {
            padding: 15px;
            overflow-y: auto;
            flex-grow: 1;
            display: flex;
            flex-direction: column;
            gap: 10px;
            max-height: 250px;
        }

        .checklist-item {
            display: flex;
            align-items: center;
            gap: 10px;
            padding: 8px 10px;
            background: rgba(255, 255, 255, 0.02);
            border: 1px solid rgba(255, 255, 255, 0.05);
            border-radius: 6px;
            transition: all 0.2s ease;
        }

        .checklist-item:hover {
            background: rgba(0, 242, 254, 0.05);
            border-color: rgba(0, 242, 254, 0.2);
        }

        .checklist-item.checked {
            opacity: 0.65;
        }

        .checklist-checkbox {
            appearance: none;
            width: 18px;
            height: 18px;
            border: 1px solid var(--widget-neon-blue);
            border-radius: 4px;
            background: transparent;
            cursor: pointer;
            position: relative;
            outline: none;
            transition: all 0.2s;
        }

        .checklist-checkbox:checked {
            background: var(--widget-neon-blue);
            box-shadow: 0 0 8px var(--widget-neon-blue);
        }

        .checklist-checkbox:checked::after {
            content: '✓';
            color: #000;
            font-size: 14px;
            font-weight: bold;
            position: absolute;
            top: -1px;
            left: 3px;
        }

        .checklist-badge {
            font-size: 0.75rem;
            text-transform: uppercase;
            font-weight: bold;
            color: var(--widget-neon-purple);
            border: 1px solid var(--widget-neon-purple);
            padding: 2px 6px;
            border-radius: 4px;
            max-width: 80px;
            overflow: hidden;
            text-overflow: ellipsis;
            white-space: nowrap;
        }

        .checklist-text {
            font-size: 0.95rem;
            flex-grow: 1;
            word-break: break-word;
        }

        .checklist-item.checked .checklist-text {
            text-decoration: line-through;
            color: var(--text-secondary);
        }

        .checklist-delete {
            background: none;
            border: none;
            color: rgba(255, 255, 255, 0.3);
            cursor: pointer;
            font-size: 1rem;
            transition: color 0.2s;
        }

        .checklist-delete:hover {
            color: var(--widget-neon-magenta);
        }

        .checklist-form {
            padding: 15px;
            border-top: 1px solid var(--widget-border);
            display: flex;
            flex-direction: column;
            gap: 10px;
            background: rgba(0, 0, 0, 0.2);
        }

        .checklist-input-group {
            display: flex;
            gap: 8px;
        }

        .checklist-input {
            background: rgba(0, 0, 0, 0.4);
            border: 1px solid rgba(255, 255, 255, 0.1);
            color: #fff;
            border-radius: 6px;
            padding: 8px 10px;
            font-family: inherit;
            font-size: 0.9rem;
            outline: none;
            transition: border-color 0.2s;
        }

        .checklist-input:focus {
            border-color: var(--widget-neon-blue);
        }

        .checklist-btn {
            background: linear-gradient(135deg, var(--widget-neon-blue), var(--widget-neon-purple));
            color: #000;
            border: none;
            border-radius: 6px;
            padding: 8px 12px;
            cursor: pointer;
            font-family: inherit;
            font-weight: bold;
            font-size: 0.9rem;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            transition: all 0.2s;
        }

        .checklist-btn:hover {
            box-shadow: 0 0 10px rgba(0, 242, 254, 0.5);
            transform: translateY(-1px);
        }
    `;
    document.head.appendChild(styleEl);

    // 2. Inject DOM Elements
    const container = document.createElement('div');
    container.id = 'checklist-widget-container';
    container.innerHTML = `
        <div class="checklist-trigger" title="Checklist Hub">☑️</div>
        <div class="checklist-panel">
            <div class="checklist-header">
                <h3>System Checklist</h3>
                <button class="checklist-close">×</button>
            </div>
            <div class="checklist-list" id="checklist-widget-items"></div>
            <div class="checklist-form">
                <div class="checklist-input-group">
                    <input type="text" id="checklist-widget-desc" class="checklist-input" placeholder="New task description..." style="flex-grow: 1;">
                    <input type="text" id="checklist-widget-cat" class="checklist-input" placeholder="Tag" style="width: 80px;">
                </div>
                <button class="checklist-btn" id="checklist-widget-add">Add Task</button>
            </div>
        </div>
    `;
    document.body.appendChild(container);

    // 3. Logic & Event Handling
    const trigger = container.querySelector('.checklist-trigger');
    const panel = container.querySelector('.checklist-panel');
    const closeBtn = container.querySelector('.checklist-close');
    const listContainer = container.querySelector('#checklist-widget-items');
    const descInput = container.querySelector('#checklist-widget-desc');
    const catInput = container.querySelector('#checklist-widget-cat');
    const addBtn = container.querySelector('#checklist-widget-add');

    // Load Items from LocalStorage
    let checklist = [];
    const saved = localStorage.getItem('universal_checklist');
    if (saved) {
        checklist = JSON.parse(saved);
    } else {
        // Default items
        checklist = [
            { checked: false, text: "Verify local RPC node connectivity", category: "System" },
            { checked: true, text: "Explore Ahoy! Magazine retro tools", category: "General" },
            { checked: false, text: "Check on-chain contract deployments", category: "Contracts" }
        ];
        saveToStorage();
    }

    function saveToStorage() {
        localStorage.setItem('universal_checklist', JSON.stringify(checklist));
    }

    function renderChecklist() {
        listContainer.innerHTML = '';
        if (checklist.length === 0) {
            listContainer.innerHTML = '<div style="text-align: center; color: var(--text-secondary); font-size: 0.95rem; margin-top: 20px;">No pending tasks.</div>';
            return;
        }
        
        checklist.forEach((item, index) => {
            const itemEl = document.createElement('div');
            itemEl.className = `checklist-item ${item.checked ? 'checked' : ''}`;
            itemEl.innerHTML = `
                <input type="checkbox" class="checklist-checkbox" ${item.checked ? 'checked' : ''}>
                <span class="checklist-badge">${item.category || 'General'}</span>
                <span class="checklist-text">${item.text}</span>
                <button class="checklist-delete">×</button>
            `;
            
            // Toggle event
            const checkbox = itemEl.querySelector('.checklist-checkbox');
            checkbox.addEventListener('change', () => {
                item.checked = checkbox.checked;
                if (item.checked) {
                    itemEl.classList.add('checked');
                } else {
                    itemEl.classList.remove('checked');
                }
                saveToStorage();
            });

            // Delete event
            const deleteBtn = itemEl.querySelector('.checklist-delete');
            deleteBtn.addEventListener('click', () => {
                checklist.splice(index, 1);
                saveToStorage();
                renderChecklist();
            });

            listContainer.appendChild(itemEl);
        });
    }

    // Toggle Panel Open/Close
    trigger.addEventListener('click', () => {
        panel.classList.toggle('active');
        renderChecklist();
    });

    closeBtn.addEventListener('click', () => {
        panel.classList.remove('active');
    });

    // Add Task Handler
    addBtn.addEventListener('click', () => {
        const text = descInput.value.trim();
        const category = catInput.value.trim() || 'General';
        if (!text) return;

        checklist.push({ checked: false, text, category });
        saveToStorage();
        renderChecklist();

        descInput.value = '';
        catInput.value = '';
    });

    descInput.addEventListener('keydown', (e) => {
        if (e.key === 'Enter') {
            addBtn.click();
        }
    });

    renderChecklist();
})();
