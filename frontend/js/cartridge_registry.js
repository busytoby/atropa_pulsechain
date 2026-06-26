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
    
    playArpeggio(notes, type = "square", duration = 0.5, speed = 0.08, volume = 0.08, pattern = "up", adsr = null) {
        try {
            this.init();
            if (!this.ctx || !notes || notes.length === 0) return;
            const now = this.ctx.currentTime;
            const osc = this.ctx.createOscillator();
            const gain = this.ctx.createGain();
            
            osc.type = type;
            
            let activeNotes = [...notes];
            if (pattern === "down") {
                activeNotes.reverse();
            } else if (pattern === "updown") {
                activeNotes = [...notes, ...[...notes].reverse().slice(1, -1)];
            }
            
            let time = now;
            let noteIdx = 0;
            while (time < now + duration) {
                let freq;
                if (pattern === "random") {
                    freq = activeNotes[Math.floor(Math.random() * activeNotes.length)];
                } else {
                    freq = activeNotes[noteIdx % activeNotes.length];
                }
                osc.frequency.setValueAtTime(freq, time);
                time += speed;
                noteIdx++;
            }
            
            if (adsr) {
                const a = adsr.attack || 0.01;
                const d = adsr.decay || 0.05;
                const s = adsr.sustain || 0.7;
                const r = adsr.release || 0.15;
                
                const peakVolume = volume;
                const sustainVolume = volume * s;
                
                gain.gain.setValueAtTime(0.0001, now);
                gain.gain.linearRampToValueAtTime(peakVolume, now + a);
                gain.gain.exponentialRampToValueAtTime(Math.max(0.0001, sustainVolume), now + a + d);
                gain.gain.setValueAtTime(Math.max(0.0001, sustainVolume), now + duration - r);
                gain.gain.exponentialRampToValueAtTime(0.0001, now + duration);
            } else {
                gain.gain.setValueAtTime(volume, now);
                gain.gain.exponentialRampToValueAtTime(0.0001, now + duration);
            }
            
            osc.connect(gain);
            gain.connect(this.ctx.destination);
            
            osc.start();
            osc.stop(now + duration);
        } catch (e) {
            console.warn("Arpeggio playback failed:", e);
        }
    }

    playMajorArpeggio(rootFreq, type = "square", duration = 0.5, speed = 0.08, volume = 0.08) {
        const notes = [rootFreq, rootFreq * 1.25, rootFreq * 1.5, rootFreq * 2.0];
        this.playArpeggio(notes, type, duration, speed, volume);
    }

    playMinorArpeggio(rootFreq, type = "square", duration = 0.5, speed = 0.08, volume = 0.08) {
        const notes = [rootFreq, rootFreq * 1.2, rootFreq * 1.5, rootFreq * 2.0];
        this.playArpeggio(notes, type, duration, speed, volume);
    }

    playDiminishedArpeggio(rootFreq, type = "square", duration = 0.5, speed = 0.08, volume = 0.08) {
        const notes = [rootFreq, rootFreq * 1.2, rootFreq * 1.414, rootFreq * 2.0];
        this.playArpeggio(notes, type, duration, speed, volume);
    }

    playAugmentedArpeggio(rootFreq, type = "square", duration = 0.5, speed = 0.08, volume = 0.08) {
        const notes = [rootFreq, rootFreq * 1.25, rootFreq * 1.5625, rootFreq * 2.0];
        this.playArpeggio(notes, type, duration, speed, volume);
    }

    playSus2Arpeggio(rootFreq, type = "square", duration = 0.5, speed = 0.08, volume = 0.08) {
        const notes = [rootFreq, rootFreq * 1.125, rootFreq * 1.5, rootFreq * 2.0];
        this.playArpeggio(notes, type, duration, speed, volume);
    }

    playSus4Arpeggio(rootFreq, type = "square", duration = 0.5, speed = 0.08, volume = 0.08) {
        const notes = [rootFreq, rootFreq * 1.333, rootFreq * 1.5, rootFreq * 2.0];
        this.playArpeggio(notes, type, duration, speed, volume);
    }

    playMaj7Arpeggio(rootFreq, type = "square", duration = 0.5, speed = 0.08, volume = 0.08) {
        const notes = [rootFreq, rootFreq * 1.25, rootFreq * 1.5, rootFreq * 1.875, rootFreq * 2.0];
        this.playArpeggio(notes, type, duration, speed, volume);
    }

    playMin7Arpeggio(rootFreq, type = "square", duration = 0.5, speed = 0.08, volume = 0.08) {
        const notes = [rootFreq, rootFreq * 1.2, rootFreq * 1.5, rootFreq * 1.8, rootFreq * 2.0];
        this.playArpeggio(notes, type, duration, speed, volume);
    }

    playDom7Arpeggio(rootFreq, type = "square", duration = 0.5, speed = 0.08, volume = 0.08) {
        const notes = [rootFreq, rootFreq * 1.25, rootFreq * 1.5, rootFreq * 1.777, rootFreq * 2.0];
        this.playArpeggio(notes, type, duration, speed, volume);
    }

    playHalfDim7Arpeggio(rootFreq, type = "square", duration = 0.5, speed = 0.08, volume = 0.08) {
        const notes = [rootFreq, rootFreq * 1.2, rootFreq * 1.414, rootFreq * 1.8, rootFreq * 2.0];
        this.playArpeggio(notes, type, duration, speed, volume);
    }

    playDim7Arpeggio(rootFreq, type = "square", duration = 0.5, speed = 0.08, volume = 0.08) {
        const notes = [rootFreq, rootFreq * 1.2, rootFreq * 1.414, rootFreq * 1.682, rootFreq * 2.0];
        this.playArpeggio(notes, type, duration, speed, volume);
    }
    
    playPickup() {
        this.playMajorArpeggio(523.25, "square", 0.3, 0.08, 0.1);
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

