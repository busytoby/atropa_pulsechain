/**
 * cartridge_registry.js
 * Handles C64 / Auncient ROM layout parsing and mapper routing.
 */

class CartridgeRegistry {
    constructor() {
        this.mappers = {
            0: (romBytes) => {
                const pokes = [];
                pokes.push({ addr: 53269, val: 15 });
                return { program: Array.from(romBytes), pokes };
            },
            1: (romBytes) => {
                const pokes = [];
                pokes.push({ addr: 53269, val: 7 });
                pokes.push({ addr: 53272, val: 21 });
                return { program: Array.from(romBytes), pokes };
            },
            2: (romBytes) => {
                const pokes = [];
                pokes.push({ addr: 53269, val: 15 });
                return { program: Array.from(romBytes), pokes };
            },
            3: (romBytes) => {
                const banks = [];
                const bankSize = 8192;
                for (let offset = 0; offset < romBytes.length; offset += bankSize) {
                    banks.push(Array.from(romBytes.slice(offset, offset + bankSize)));
                }
                const pokes = [];
                pokes.push({ addr: 53269, val: 15 });
                pokes.push({ addr: 56832, val: 0 });
                return { program: banks[0] || [], pokes, banks };
            },
            4: (romBytes) => {
                const banks = [];
                const bankSize = 16384;
                for (let offset = 0; offset < romBytes.length; offset += bankSize) {
                    banks.push(Array.from(romBytes.slice(offset, offset + bankSize)));
                }
                const pokes = [];
                pokes.push({ addr: 53269, val: 15 });
                pokes.push({ addr: 56834, val: 0 });
                return { program: banks[0] || [], pokes, banks };
            }
        };
    }

    parseCartridge(rawBytes) {
        if (rawBytes.length < 16) return null;
        const magic = String.fromCharCode(...rawBytes.slice(0, 5));
        let mapperId = 0;
        let romStart = 16;
        let name = "Generic ROM";
        
        if (magic === "ROMOX") {
            mapperId = 1;
            name = "Romox Cartridge";
        } else if (magic === "DATAM") {
            mapperId = 2;
            name = "Datamost Cartridge";
        } else if (magic === "MDESK") {
            mapperId = 3;
            name = "Magic Desk Cartridge";
        } else if (magic === "EASYS") {
            mapperId = 4;
            name = "EasyFlash Cartridge";
        }
        
        const mapper = this.mappers[mapperId];
        const mapped = mapper(rawBytes.slice(romStart));
        return {
            name: name,
            program: mapped.program,
            pokes: mapped.pokes,
            banks: mapped.banks
        };
    }
}

window.CartridgeRegistry = CartridgeRegistry;
window.cartridgeRegistry = new CartridgeRegistry();

class CartridgeAudioSynth {
    constructor() {
        this.ctx = null;
    }
    
    init() {
        if (!this.ctx) {
            this.ctx = new (window.AudioContext || window.webkitAudioContext)();
        }
        if (this.ctx && this.ctx.state === "suspended") {
            this.ctx.resume();
        }
    }
    
    playTone(frequency, type, duration, volume = 0.1) {
        try {
            this.init();
            if (!this.ctx) return;
            const osc = this.ctx.createOscillator();
            const gain = this.ctx.createGain();
            
            osc.type = type || "square";
            osc.frequency.setValueAtTime(frequency, this.ctx.currentTime);
            
            gain.gain.setValueAtTime(volume, this.ctx.currentTime);
            gain.gain.exponentialRampToValueAtTime(0.0001, this.ctx.currentTime + duration);
            
            osc.connect(gain);
            gain.connect(this.ctx.destination);
            
            osc.start();
            osc.stop(this.ctx.currentTime + duration);
        } catch (e) {
            console.warn("Audio play failed:", e);
        }
    }
    
    playJump() {
        try {
            this.init();
            if (!this.ctx) return;
            const now = this.ctx.currentTime;
            const osc = this.ctx.createOscillator();
            const gain = this.ctx.createGain();
            
            osc.type = "triangle";
            osc.frequency.setValueAtTime(150, now);
            osc.frequency.exponentialRampToValueAtTime(600, now + 0.15);
            
            gain.gain.setValueAtTime(0.15, now);
            gain.gain.linearRampToValueAtTime(0.01, now + 0.15);
            
            osc.connect(gain);
            gain.connect(this.ctx.destination);
            osc.start();
            osc.stop(now + 0.15);
        } catch (e) {}
    }
    
    playPickup() {
        try {
            this.init();
            if (!this.ctx) return;
            const now = this.ctx.currentTime;
            const osc = this.ctx.createOscillator();
            const gain = this.ctx.createGain();
            
            osc.type = "square";
            osc.frequency.setValueAtTime(523.25, now); // C5
            osc.frequency.setValueAtTime(659.25, now + 0.08); // E5
            osc.frequency.setValueAtTime(783.99, now + 0.16); // G5
            
            gain.gain.setValueAtTime(0.1, now);
            gain.gain.setValueAtTime(0.1, now + 0.16);
            gain.gain.linearRampToValueAtTime(0.001, now + 0.3);
            
            osc.connect(gain);
            gain.connect(this.ctx.destination);
            osc.start();
            osc.stop(now + 0.3);
        } catch (e) {}
    }

    playExplosion() {
        try {
            this.init();
            if (!this.ctx) return;
            const now = this.ctx.currentTime;
            
            const bufferSize = this.ctx.sampleRate * 0.4;
            const buffer = this.ctx.createBuffer(1, bufferSize, this.ctx.sampleRate);
            const data = buffer.getChannelData(0);
            for (let i = 0; i < bufferSize; i++) {
                data[i] = Math.random() * 2 - 1;
            }
            
            const noise = this.ctx.createBufferSource();
            noise.buffer = buffer;
            
            const filter = this.ctx.createBiquadFilter();
            filter.type = "lowpass";
            filter.frequency.setValueAtTime(800, now);
            filter.frequency.exponentialRampToValueAtTime(50, now + 0.4);
            
            const gain = this.ctx.createGain();
            gain.gain.setValueAtTime(0.3, now);
            gain.gain.linearRampToValueAtTime(0.01, now + 0.4);
            
            noise.connect(filter);
            filter.connect(gain);
            gain.connect(this.ctx.destination);
            
            noise.start();
            noise.stop(now + 0.4);
        } catch (e) {}
    }
}

window.CartridgeAudioSynth = CartridgeAudioSynth;
window.cartridgeAudio = new CartridgeAudioSynth();

