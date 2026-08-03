const fs = require("fs");
const path = require("path");
const https = require("https");
const net = require("net");
const { execSync, spawn } = require("child_process");

const STREAM_KEY = "b4p3-xpwp-yrab-9hxt-5ccz";
// Bionika .bio sequencer reader
const bioScorePath = path.join(__dirname, "../assets/bionika/tsfi_rb_score.bio");
const bioScore = JSON.parse(fs.readFileSync(bioScorePath, "utf8"));

// SFT (Swap Flow Telemetry) source
const resolvedSwaps = JSON.parse(fs.readFileSync(path.join(__dirname, "../resolved_swaps.json"), "utf8"));

const SAMPLE_RATE = 44100;
const TCP_PORT = 8089;

const stepDurationSamples = Math.floor(SAMPLE_RATE * (60.0 / bioScore.tempo / 4.0));

let globalSampleCounter = 0;

function queryGoogleTranslateTTS(text) {
    return new Promise((resolve, reject) => {
        const encodedText = encodeURIComponent(text);
        const url = `https://translate.google.com/translate_tts?ie=UTF-8&tl=en&client=tw-ob&q=${encodedText}`;
        const options = {
            headers: {
                "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64)"
            }
        };
        https.get(url, options, (res) => {
            if (res.statusCode !== 200) {
                reject(new Error(`Google Translate TTS returned status ${res.statusCode}`));
                return;
            }
            const chunks = [];
            res.on("data", (c) => chunks.push(c));
            res.on("end", () => resolve(Buffer.concat(chunks)));
        }).on("error", (e) => reject(e));
    });
}


function splitIntoBiteSizedBlocks(text) {
    const sentences = text.match(/[^.!?]+[.!?]+(\s|$)/g) || [text];
    const blocks = [];
    let currentBlock = "";
    for (const sentence of sentences) {
        if ((currentBlock + sentence).length > 140) {
            if (currentBlock.trim().length > 0) {
                blocks.push(currentBlock.trim());
            }
            currentBlock = sentence;
        } else {
            currentBlock += sentence;
        }
    }
    if (currentBlock.trim().length > 0) {
        blocks.push(currentBlock.trim());
    }
    return blocks;
}

const NOTE_FREQS = {
    "C1": 32.70, "Db1": 34.65, "D1": 36.71, "Eb1": 38.89, "E1": 41.20, "F1": 43.65, "Gb1": 46.25, "G1": 49.00, "Ab1": 51.91, "A1": 55.00, "Bb1": 58.27, "B1": 61.74,
    "C2": 65.41, "Db2": 69.30, "D2": 73.42, "Eb2": 77.78, "E2": 82.41, "F2": 87.31, "Gb2": 92.50, "G2": 98.00, "Ab2": 103.83, "A2": 110.00, "Bb2": 116.54, "B2": 123.47,
    "C3": 130.81, "Db3": 138.59, "D3": 146.83, "Eb3": 155.56, "E3": 164.81, "F3": 174.61, "Gb3": 185.00, "G3": 196.00, "Ab3": 207.65, "A3": 220.00, "Bb3": 233.08, "B3": 246.94,
    "C4": 261.63, "Db4": 277.18, "D4": 293.66, "Eb4": 311.13, "E4": 329.63, "F4": 349.23, "Gb4": 369.99, "G4": 392.00, "Ab4": 415.30, "A4": 440.00, "Bb4": 466.16, "B4": 493.88,
    "C5": 523.25, "Db5": 554.37, "D5": 587.33, "Eb5": 622.25, "E5": 659.25, "F5": 698.46, "Gb5": 739.99, "G5": 783.99, "Ab5": 830.61, "A5": 880.00, "Bb5": 932.33, "B5": 987.77
};

let growlPhase = 0;
let leadPhase = 0;
let bassPhase = 0;
let bassCurrentFreq = 130.81;

// 3-voice Chord Pad state
let padPhases = [0, 0, 0];
let padFreqs = [0, 0, 0];
let padGainPrev = 0;

// Vocoder carrier phases
let vocoderPhases = [0, 0, 0];

// Bird call phase
let birdPhase = 0;

let hp_in_prev = 0, hp_out_prev = 0;
const hp_alpha = 0.85;

let lp_growl_prev = 0;

let currentStepDurationSamples = stepDurationSamples;
let currentFreqMultiplier = 1.0;

let filter_v0 = 0.0;
let filter_v1 = 0.0;

let currentDispensation = 0;
let persistentTempo = 60;
let persistentTranspose = 0;
let persistentDispensation = 0;


const DISPENSATION_NAMES = [
    "Classic Chiptune",
    "Deep Growler",
    "Avian Sanctuary",
    "Lush Chimes",
    "Cosmic Ambient"
];

function applyResonantLPF(sample, cutoffHz, resonance) {
    const f = 2.0 * Math.sin(Math.PI * cutoffHz / SAMPLE_RATE);
    const q = 1.0 - resonance;
    filter_v0 = (1.0 - f * q) * filter_v0 - f * filter_v1 + f * sample;
    filter_v1 = (1.0 - f * q) * filter_v1 + f * filter_v0;
    return filter_v1;
}

function generateMusicSample(pattern, stepIndex, stepSampleIdx, stepAge) {
    let drumL = 0.0, drumR = 0.0;

    const hasDrums = (currentDispensation !== 4);
    const softDrums = (currentDispensation === 2);

    // Kick
    if (hasDrums && pattern.drums.kick[stepIndex] === 1 && stepSampleIdx < currentStepDurationSamples * 0.7) {
        const kickFreq = 150.0 * Math.exp(-stepAge / 0.04) + 45.0;
        const vol = softDrums ? 0.6 : 1.2;
        const kickVal = Math.sin(2 * Math.PI * kickFreq * stepAge) * Math.exp(-stepAge / 0.12) * vol;
        drumL += kickVal;
        drumR += kickVal;
    }

    // Snare
    if (hasDrums && !softDrums && pattern.drums.snare[stepIndex] === 1 && stepAge < 0.25) {
        const rattle = (Math.random() * 2 - 1) * Math.exp(-stepAge / 0.12) * 0.4;
        const tone = Math.sin(2 * Math.PI * 180 * stepAge) * Math.exp(-stepAge / 0.06) * 0.4;
        const snareVal = (tone + rattle) * 0.75;
        drumL += snareVal;
        drumR += snareVal;
    }

    // Rimshot
    if (hasDrums && pattern.drums.rimshot && pattern.drums.rimshot[stepIndex] > 0 && stepAge < 0.15) {
        const rimFreq = 1500.0 * Math.exp(-stepAge / 0.006) + 340.0;
        const vol = softDrums ? 0.25 : 0.42;
        const rimVal = Math.sin(2 * Math.PI * rimFreq * stepAge) * Math.exp(-stepAge / 0.02) * vol * pattern.drums.rimshot[stepIndex];
        drumL += rimVal * 0.55;
        drumR += rimVal * 0.45;
    }

    // Hihats
    if (hasDrums && pattern.drums.closed_hh[stepIndex] > 0 && stepAge < 0.06) {
        const noise = Math.random() * 2 - 1;
        const filteredHh = hp_alpha * (hp_out_prev + noise - hp_in_prev);
        hp_in_prev = noise;
        hp_out_prev = filteredHh;
        const vol = softDrums ? 0.06 : 0.15;
        const hhVal = filteredHh * Math.exp(-stepAge / 0.02) * vol * pattern.drums.closed_hh[stepIndex];
        drumL += hhVal * 0.7;
        drumR += hhVal * 0.3;
    }
    if (hasDrums && pattern.drums.open_hh[stepIndex] > 0 && stepAge < 0.3) {
        const noise = Math.random() * 2 - 1;
        const filteredHh = hp_alpha * (hp_out_prev + noise - hp_in_prev);
        hp_in_prev = noise;
        hp_out_prev = filteredHh;
        const vol = softDrums ? 0.05 : 0.15;
        const hhVal = filteredHh * Math.exp(-stepAge / 0.15) * vol * pattern.drums.open_hh[stepIndex];
        drumL += hhVal * 0.6;
        drumR += hhVal * 0.4;
    }

    if (hasDrums && !softDrums && pattern.drums.clap[stepIndex] > 0 && stepAge < 0.3) {
        const noise = Math.random() * 2 - 1;
        const clapVal = noise * Math.exp(-stepAge / 0.08) * 0.35 * pattern.drums.clap[stepIndex];
        drumL += clapVal * 0.3;
        drumR += clapVal * 0.7;
    }

    // Bass
    const bassNote = pattern.bass.sequence[stepIndex];
    const bassTargetFreq = (bassNote && NOTE_FREQS[bassNote]) ? (NOTE_FREQS[bassNote] * currentFreqMultiplier) : 0.0;
    const isAccent = pattern.bass.accents[stepIndex];
    const isSlide = pattern.bass.slides[stepIndex];

    if (bassTargetFreq > 0.0) {
        const slideSpeed = isSlide ? 0.08 : 0.006;
        bassCurrentFreq += (bassTargetFreq - bassCurrentFreq) * slideSpeed;
    }

    bassPhase += bassCurrentFreq / SAMPLE_RATE;
    if (bassPhase > 1.0) bassPhase -= 2.0;
    
    let bassSample = Math.sin(2 * Math.PI * bassPhase) * 0.6;
    if (isAccent) bassSample *= 1.35;
    if (currentDispensation === 4) bassSample *= 0.4;

    // Sub Growl
    const growlNote = pattern.sub_growl.sequence[stepIndex];
    const growlFreq = (growlNote && NOTE_FREQS[growlNote]) ? (NOTE_FREQS[growlNote] * currentFreqMultiplier) : 65.41;
    let growlGain = pattern.sub_growl.gain[stepIndex] || 0.28;
    if (currentDispensation === 1) growlGain *= 1.5;

    growlPhase += growlFreq * 2.0 / SAMPLE_RATE;
    if (growlPhase > 1.0) growlPhase -= 2.0;
    
    let growlSample = Math.tanh(growlPhase * 0.8) * growlGain;
    lp_growl_prev += 0.04 * (growlSample - lp_growl_prev);
    growlSample = lp_growl_prev;

    let leadSampleL = 0.0;
    let leadSampleR = 0.0;

    const timeSec = globalSampleCounter / SAMPLE_RATE;

    if (currentDispensation === 2) {
        const arpNote = pattern.arpeggiator_filter.sequence[stepIndex];
        if (arpNote && arpNote !== "0" && stepAge < 0.2) {
            const birdFreq = 2600.0 * Math.exp(-stepAge / 0.04) + 1200.0 * Math.sin(2.0 * Math.PI * 190.0 * stepAge);
            birdPhase += birdFreq / SAMPLE_RATE;
            const birdVal = Math.sin(2.0 * Math.PI * birdPhase) * Math.exp(-stepAge / 0.12) * 0.18;
            const panL = 0.4 + 0.4 * Math.sin(2.0 * Math.PI * 0.8 * timeSec);
            leadSampleL = birdVal * panL;
            leadSampleR = birdVal * (1.0 - panL);
        }
    } else if (currentDispensation === 3) {
        const arpNote = pattern.arpeggiator_filter.sequence[stepIndex];
        const arpFreq = (arpNote && NOTE_FREQS[arpNote]) ? (NOTE_FREQS[arpNote] * currentFreqMultiplier) : 523.25;
        if (arpNote && arpNote !== "0" && stepAge < 0.35) {
            leadPhase += arpFreq / SAMPLE_RATE;
            const modSample = Math.sin(2.0 * Math.PI * leadPhase * 2.01) * 2.5 * Math.exp(-stepAge / 0.08);
            const bellVal = Math.sin(2.0 * Math.PI * leadPhase + modSample) * Math.exp(-stepAge / 0.22) * 0.15;
            const panL = 0.5 + 0.35 * Math.sin(2.0 * Math.PI * 0.15 * timeSec);
            leadSampleL = bellVal * panL;
            leadSampleR = bellVal * (1.0 - panL);
        }
    } else if (currentDispensation === 0 || currentDispensation === 1) {
        const arpNote = pattern.arpeggiator_filter.sequence[stepIndex];
        const arpFreq = (arpNote && NOTE_FREQS[arpNote]) ? (NOTE_FREQS[arpNote] * currentFreqMultiplier) : 523.25;
        const arpGate = pattern.arpeggiator_filter.gate[stepIndex];

        leadPhase += arpFreq * 2.0 / SAMPLE_RATE;
        if (leadPhase > 1.0) leadPhase -= 2.0;
        
        const leadEnv = arpGate ? Math.exp(-(stepSampleIdx / SAMPLE_RATE) / 0.18) : 0.0;
        let leadRaw = leadPhase * leadEnv * 0.15;

        const lfoVal = Math.sin(2 * Math.PI * 0.2 * timeSec);
        const cutoffHz = 550.0 + lfoVal * 300.0;
        const rawLeadFiltered = applyResonantLPF(leadRaw, cutoffHz, 0.55);

        const panL = 0.5 + 0.35 * Math.sin(2.0 * Math.PI * 0.15 * timeSec);
        leadSampleL = rawLeadFiltered * panL;
        leadSampleR = rawLeadFiltered * (1.0 - panL);
    }

    // Lush Chord Pad (3-voice string polyphony)
    let padSample = 0.0;
    const hasPads = (currentDispensation === 2 || currentDispensation === 3 || currentDispensation === 4);
    if (hasPads && pattern.pad) {
        const padNotes = pattern.pad.sequence[stepIndex];
        if (Array.isArray(padNotes)) {
            padFreqs = padNotes.map(n => NOTE_FREQS[n] * currentFreqMultiplier || 0);
        }
        
        let padRawAccum = 0.0;
        for (let v = 0; v < 3; v++) {
            if (padFreqs[v] > 0) {
                padPhases[v] += padFreqs[v] / SAMPLE_RATE;
                if (padPhases[v] > 1.0) padPhases[v] -= 2.0;
                padRawAccum += Math.sin(2 * Math.PI * padPhases[v]);
            }
        }
        
        const padTargetGain = (Array.isArray(padNotes)) ? pattern.pad.gain : pattern.pad.gain * 0.5;
        padGainPrev += (padTargetGain - padGainPrev) * 0.00015;
        padSample = (padRawAccum / 3.0) * padGainPrev;
    }

    globalSampleCounter++;
    
    return {
        left: drumL + bassSample + growlSample + leadSampleL + padSample,
        right: drumR + bassSample + growlSample + leadSampleR + padSample
    };
}

const shuffleArray = (array) => {
    for (let i = array.length - 1; i > 0; i--) {
        const j = Math.floor(Math.random() * (i + 1));
        [array[i], array[j]] = [array[j], array[i]];
    }
    return array;
};

function getStringSeed(str) {
    let hash = 0;
    for (let i = 0; i < str.length; i++) {
        hash = (hash * 31 + str.charCodeAt(i)) | 0;
    }
    return Math.abs(hash);
}

const ttsQueue = [];
let queueActive = true;

async function startPrefetchWorker(loreFiles) {
    let fileIdx = 0;
    let shuffledLores = shuffleArray([...loreFiles]);
    
    while (queueActive) {
        if (ttsQueue.length >= 3) {
            await new Promise(r => setTimeout(r, 500));
            continue;
        }
        
        if (fileIdx >= shuffledLores.length) {
            shuffledLores = shuffleArray([...loreFiles]);
            fileIdx = 0;
        }
        
        const lorePath = shuffledLores[fileIdx];
        fileIdx++;
        
        try {
            const content = fs.readFileSync(lorePath, "utf8");
            
            const paragraphs = content
                .split("\n\n")
                .map(p => p.trim())
                .filter(p => p.length > 0 && !p.startsWith("```"));
                
            const textBlocks = [];
            for (const p of paragraphs) {
                const subBlocks = splitIntoBiteSizedBlocks(p);
                textBlocks.push(...subBlocks);
            }
                
            for (let i = 0; i < textBlocks.length; i++) {
                const textBlock = textBlocks[i];
                
                while (ttsQueue.length >= 3 && queueActive) {
                    await new Promise(r => setTimeout(r, 200));
                }
                if (!queueActive) break;
                
                const blockId = `${Date.now()}_${Math.random().toString(36).substr(2, 5)}`;
                const tempChunkPath = `/tmp/tsfi_chunk_${blockId}.mp3`;
                const tempPcmPath = `/tmp/tsfi_chunk_${blockId}.pcm`;
                
                if (fs.existsSync("/tmp/tts_disable")) {
                    // Generate 1.5 seconds of silence (44100 * 2 bytes * 1.5 = 132300 bytes)
                    const silence = Buffer.alloc(132300);
                    fs.writeFileSync(tempPcmPath, silence);
                } else {
                    const audioData = await queryGoogleTranslateTTS(textBlock);
                    fs.writeFileSync(tempChunkPath, audioData);
                    execSync(`ffmpeg -y -i "${tempChunkPath}" -f s16le -ac 1 -ar 44100 "${tempPcmPath}" 2>/dev/null`);
                    fs.unlinkSync(tempChunkPath);
                }
                
                ttsQueue.push({
                    textBlock,
                    pcmPath: tempPcmPath,
                    lorePath,
                    blockIndex: i,
                    totalBlocks: textBlocks.length
                });
            }
        } catch (err) {
            console.error("[PREFETCH WORKER ERROR]:", err.message);
            await new Promise(r => setTimeout(r, 3000));
        }
    }
}

const FRAME_SIZE = 1280 * 692 * 3;
let lastCompleteFrame = Buffer.alloc(FRAME_SIZE);
let frameAccumulator = Buffer.alloc(0);
let gapInterval = null;
let ffmpegStdin = null;

function startGapFiller() {
    if (gapInterval) return;
    gapInterval = setInterval(() => {
        if (ffmpegStdin && !ffmpegStdin.destroyed) {
            ffmpegStdin.write(lastCompleteFrame);
        }
    }, 1000 / 24);
}

function stopGapFiller() {
    if (gapInterval) {
        clearInterval(gapInterval);
        gapInterval = null;
    }
}

async function main() {
    console.log("=== COMPILING 3D STAR WARS SCROLLER RENDERER ===");
    const srcC = path.join(__dirname, "../tsfi2-deepseek/tools/tsfi_starwars_scroller.c");
    const binC = path.join(__dirname, "../tsfi2-deepseek/tools/tsfi_starwars_scroller");
    execSync(`gcc -O3 "${srcC}" -o "${binC}" -lm`);
    console.log("✓ Compiled successfully.");

    const loreDir = path.join(__dirname, "../lore");
    const getLoreFiles = () => {
        return fs.readdirSync(loreDir)
            .filter(f => f.endsWith(".md"))
            .map(f => path.join(loreDir, f));
    };

    const loreFiles = getLoreFiles();
    console.log(`Discovered ${loreFiles.length} lore files for dynamic random streaming.`);

    let audioSocket = null;
    const server = net.createServer((socket) => {
        console.log("[TCP] ffmpeg audio channel connected.");
        audioSocket = socket;
        socket.on("error", (err) => {
            console.error("[TCP] Audio socket error:", err.message);
        });
    });

    server.listen(TCP_PORT, "127.0.0.1", () => {
        console.log(`[TCP] Audio loopback server listening on 127.0.0.1:${TCP_PORT}`);
    });

    const rtmpUrl = `rtmp://a.rtmp.youtube.com/live2/${STREAM_KEY}`;
    console.log(`[STREAM] Launching ffmpeg to RTMP target: rtmp://a.rtmp.youtube.com/live2/****`);

    const ffmpeg = spawn("ffmpeg", [
        "-y",
        "-re",
        "-f", "rawvideo",
        "-vcodec", "rawvideo",
        "-s", "1280x692",
        "-pix_fmt", "rgb24",
        "-r", "24",
        "-i", "-",
        "-re",
        "-f", "s16le",
        "-ac", "2",
        "-ar", "44100",
        "-i", `tcp://127.0.0.1:${TCP_PORT}`,
        "-c:v", "libx264",
        "-preset", "veryfast",
        "-pix_fmt", "yuv420p",
        "-g", "48",
        "-keyint_min", "48",
        "-c:a", "aac",
        "-b:a", "192k",
        "-ar", "44100",
        "-ac", "2",
        "-f", "flv",
        rtmpUrl
    ]);

    ffmpegStdin = ffmpeg.stdin;

    startGapFiller();

    ffmpeg.stderr.on("data", (data) => {
        if (data.toString().includes("frame=")) {
            process.stdout.write(`\r[FFMPEG STREAM] ${data.toString().trim()}`);
        }
    });

    ffmpeg.on("close", (code) => {
        console.log(`\n[STREAM] ffmpeg closed with code ${code}`);
        queueActive = false;
        stopGapFiller();
        server.close();
        process.exit(code);
    });

    await new Promise(r => setTimeout(r, 2000));

    startPrefetchWorker(loreFiles);

    let musicTimeStartSec = 0.0;

    const delaySamples = Math.floor(SAMPLE_RATE * 0.35);
    const delayBufferL = new Float32Array(delaySamples);
    const delayBufferR = new Float32Array(delaySamples);
    let delayIdx = 0;

    // Chime & pitch tracking variables
    let speechEnvelope = 0.0;
    let voiceArpPhase = 0.0;
    let voiceArpFreq = 0.0;
    let voiceArpEnv = 0.0;
    let voiceArpIndex = 0;
    const voiceTriggerThreshold = 0.035;
    let lastTriggerSample = 0;

    const ZCR_WINDOW = 512;
    const speechWindow = new Float32Array(ZCR_WINDOW);
    let speechWindowIdx = 0;

    while (true) {
        while (ttsQueue.length === 0) {
            await new Promise(r => setTimeout(r, 100));
        }

        const job = ttsQueue.shift();
        const { textBlock, pcmPath, lorePath, blockIndex, totalBlocks } = job;

        console.log(`\n=============================================================`);
        console.log(`[LORE FILE] Streaming: ${path.basename(lorePath)}`);
        console.log(`=============================================================\n`);
        console.log(`\n--- [Block ${blockIndex + 1}/${totalBlocks}] processing ---`);
        console.log(`Text: "${textBlock.substring(0, 100)}..."`);

        try {
            const pcmBuffer = fs.readFileSync(pcmPath);
            fs.unlinkSync(pcmPath);

            const numSamples = pcmBuffer.length / 2;
            const duration = numSamples / SAMPLE_RATE;
            const totalFrames = Math.floor(duration * 24) + 96;

            const seed = getStringSeed(textBlock);

            // Slowly transition musical parameters to form a long-running, continuous theme
            if (Math.random() < 0.15) {
                persistentTempo = 58 + (seed % 9);
                persistentTranspose = [0, 2, -3, 5, -5][seed % 5];
                persistentDispensation = seed % 5;
            }

            const blockTranspose = persistentTranspose;
            const blockTempo = persistentTempo;
            
            currentFreqMultiplier = Math.pow(2, blockTranspose / 12.0);
            currentStepDurationSamples = Math.floor(SAMPLE_RATE * (60.0 / blockTempo / 4.0));
            currentDispensation = persistentDispensation;


            const colorScheme = seed % 5;
            const shapeType = seed % 3;

            console.log(`[GENERATIVE MODEL] Seed: ${seed} | Key Transpose: ${blockTranspose} st | Tempo: ${blockTempo} BPM | Color Scheme: ${colorScheme} | Shape: ${shapeType} | Dispensation: ${DISPENSATION_NAMES[currentDispensation]}`);
            console.log(`[TTS] Loaded Pre-fetched Speech: ${duration.toFixed(2)}s | Total Video Frames (inc Intro): ${totalFrames}`);

            const tempMd = `/tmp/tsfi_block_run.md`;
            fs.writeFileSync(tempMd, textBlock);

            const introSamples = SAMPLE_RATE * 4;
            const totalSamples = introSamples + numSamples;
            const mixedAudio = Buffer.alloc(totalSamples * 4);

            const arrangement = bioScore.arrangement;
            const patterns = bioScore.patterns;
            const samplesPerPattern = currentStepDurationSamples * 16;
            const totalPatternsDuration = samplesPerPattern * arrangement.length;

            for (let s = 0; s < totalSamples; s++) {
                let speechVal = 0.0;
                if (s >= introSamples) {
                    const speechIdx = s - introSamples;
                    speechVal = pcmBuffer.readInt16LE(speechIdx * 2) / 32768.0;
                }

                speechWindow[speechWindowIdx] = speechVal;
                speechWindowIdx = (speechWindowIdx + 1) % ZCR_WINDOW;

                speechEnvelope = speechEnvelope * 0.995 + Math.abs(speechVal) * 0.005;

                const timeSec = globalSampleCounter / SAMPLE_RATE;
                // Wow & flutter speed/frequency drift
                const wow = 0.0016 * Math.sin(2.0 * Math.PI * 4.3 * timeSec);
                const flutter = 0.0006 * Math.sin(2.0 * Math.PI * 19.2 * timeSec);
                currentFreqMultiplier = Math.pow(2, blockTranspose / 12.0) * (1.0 + wow + flutter);

                const currentSampleOffset = globalSampleCounter % totalPatternsDuration;
                const arrangementIdx = Math.floor(currentSampleOffset / samplesPerPattern);
                const patternName = arrangement[arrangementIdx];
                const pattern = patterns[patternName];
                const stepIndex = Math.floor((currentSampleOffset % samplesPerPattern) / currentStepDurationSamples);
                const stepSampleIdx = globalSampleCounter % currentStepDurationSamples;
                const stepAge = stepSampleIdx / SAMPLE_RATE;

                const padNotes = pattern.pad ? pattern.pad.sequence[stepIndex] : null;
                const activeChord = Array.isArray(padNotes) ? padNotes : ["C3", "Eb3", "G3"];

                let estimatedPitchHz = 0;
                if (s % 256 === 0) {
                    let crossings = 0;
                    for (let i = 1; i < ZCR_WINDOW; i++) {
                        const prev = speechWindow[(speechWindowIdx + i - 1) % ZCR_WINDOW];
                        const curr = speechWindow[(speechWindowIdx + i) % ZCR_WINDOW];
                        if ((prev >= 0 && curr < 0) || (prev < 0 && curr >= 0)) {
                            crossings++;
                        }
                    }
                    const freq = (crossings * SAMPLE_RATE) / (2.0 * ZCR_WINDOW);
                    if (freq >= 90.0 && freq <= 380.0) {
                        estimatedPitchHz = freq;
                    }
                }

                let snappedPitchFreq = 0;
                if (estimatedPitchHz > 0) {
                    let closestFreq = 0;
                    let minDiff = Infinity;
                    for (const note of activeChord) {
                        const noteFreq = NOTE_FREQS[note] * currentFreqMultiplier;
                        if (noteFreq > 0) {
                            let octaveFreq = noteFreq;
                            while (octaveFreq < 90.0) octaveFreq *= 2.0;
                            while (octaveFreq > 380.0) octaveFreq /= 2.0;
                            
                            const diff = Math.abs(octaveFreq - estimatedPitchHz);
                            if (diff < minDiff) {
                                minDiff = diff;
                                closestFreq = octaveFreq;
                            }
                        }
                    }
                    if (closestFreq > 0) {
                        snappedPitchFreq = closestFreq;
                    }
                }

                let targetPadFreqs;
                if (snappedPitchFreq > 0) {
                    targetPadFreqs = [
                        snappedPitchFreq,
                        snappedPitchFreq * 1.5,
                        snappedPitchFreq * 1.2
                    ];
                } else {
                    targetPadFreqs = activeChord.map(n => NOTE_FREQS[n] * currentFreqMultiplier || 0);
                }

                for (let v = 0; v < 3; v++) {
                    if (targetPadFreqs[v] > 0) {
                        vocoderPhases[v] += targetPadFreqs[v] / SAMPLE_RATE;
                        if (vocoderPhases[v] > 1.0) vocoderPhases[v] -= 2.0;
                    }
                }

                let vocoderVal = 0.0;
                for (let v = 0; v < 3; v++) {
                    if (targetPadFreqs[v] > 0) {
                        vocoderVal += speechVal * Math.sin(2.0 * Math.PI * vocoderPhases[v]);
                    }
                }
                vocoderVal = (vocoderVal / 3.0) * 0.58;

                if (snappedPitchFreq > 0 && speechEnvelope > voiceTriggerThreshold && (s - lastTriggerSample) > SAMPLE_RATE * 0.14) {
                    lastTriggerSample = s;
                    voiceArpFreq = snappedPitchFreq * 2.0;
                    voiceArpEnv = 1.0;
                }

                const music = generateMusicSample(pattern, stepIndex, stepSampleIdx, stepAge);

                let voiceChime = 0.0;
                if (voiceArpEnv > 0.001) {
                    voiceArpPhase += voiceArpFreq / SAMPLE_RATE;
                    if (voiceArpPhase > 1.0) voiceArpPhase -= 2.0;
                    
                    // Smooth linear attack-fade (8ms) to prevent clicks
                    const ageSamples = s - lastTriggerSample;
                    const attackScale = Math.min(1.0, ageSamples / (SAMPLE_RATE * 0.008));
                    
                    voiceArpEnv *= 0.9997;
                    const mod = Math.sin(2.0 * Math.PI * voiceArpPhase * 1.5) * 1.2 * voiceArpEnv;
                    voiceChime = Math.sin(2.0 * Math.PI * voiceArpPhase + mod) * voiceArpEnv * attackScale * 0.065;
                }

                const echoL = delayBufferL[delayIdx];
                const echoR = delayBufferR[delayIdx];

                // Projector gate mechanical hum (speed scaled with active block tempo)
                const humFreq = 72.0 * (blockTempo / 60.0);
                const projHum = Math.sin(2.0 * Math.PI * humFreq * timeSec) * 0.008 + (Math.random() * 2 - 1) * 0.002;

                // Binaural spatial panning for reactive chimes (circular left-right sweep)
                const chimePanL = 0.5 + 0.45 * Math.sin(2.0 * Math.PI * 0.4 * timeSec);
                const chimePanR = 1.0 - chimePanL;

                // Mix in panned chimes, vocals, and projector noise
                let leftVal = speechVal * 0.72 + vocoderVal * 0.38 + music.left * 0.35 + (voiceChime * chimePanL) * 0.90 + echoL * 0.16 + projHum;
                let rightVal = speechVal * 0.72 + vocoderVal * 0.38 + music.right * 0.35 + (voiceChime * chimePanR) * 0.90 + echoR * 0.16 + projHum;

                // Soft dynamic compression (tanh) to prevent clipping click/pop distortion
                leftVal = Math.tanh(leftVal);
                rightVal = Math.tanh(rightVal);

                // Update cross-delay buffers using dynamically compressed levels
                delayBufferL[delayIdx] = rightVal * 0.12 + echoR * 0.18;
                delayBufferR[delayIdx] = leftVal * 0.12 + echoL * 0.18;

                delayIdx = (delayIdx + 1) % delaySamples;

                mixedAudio.writeInt16LE(Math.floor(leftVal * 32767), s * 4);
                mixedAudio.writeInt16LE(Math.floor(rightVal * 32767), s * 4 + 2);
            }

            if (audioSocket) {
                audioSocket.write(mixedAudio);
            }

            stopGapFiller();

            // Resolve current SFT telemetry entry using seed
            const activeSwap = resolvedSwaps[seed % resolvedSwaps.length];
            const usdVal = activeSwap.usd_value || 0.0;
            const tokenSym = `${activeSwap.token0.symbol}/${activeSwap.token1.symbol}`;
            const txHash = activeSwap.tx_hash || "00000";

            const scroller = spawn(binC, [
                tempMd,
                String(totalFrames),
                path.basename(lorePath),
                String(musicTimeStartSec),
                String(colorScheme),
                String(shapeType),
                String(blockTempo),
                String(usdVal),
                tokenSym,
                txHash
            ]);

            scroller.stdout.on("data", (chunk) => {
                if (ffmpegStdin && !ffmpegStdin.destroyed) {
                    ffmpegStdin.write(chunk);
                }

                frameAccumulator = Buffer.concat([frameAccumulator, chunk]);
                while (frameAccumulator.length >= FRAME_SIZE) {
                    frameAccumulator.copy(lastCompleteFrame, 0, 0, FRAME_SIZE);
                    frameAccumulator = frameAccumulator.slice(FRAME_SIZE);
                }
            });

            await new Promise((resolve) => {
                scroller.on("close", () => {
                    resolve();
                });
            });

            startGapFiller();

            musicTimeStartSec += totalFrames / 24.0;
            fs.unlinkSync(tempMd);

            console.log(`✓ Block completed rendering.`);
        } catch (err) {
            console.error(`[ERROR] Block playback failed:`, err.message);
        }
    }
}

main().catch(console.error);
