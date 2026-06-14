# ChatTTS Integration Design Plan

ChatTTS is a generative speech model optimized for conversational scenarios. It excels at synthesizing natural dialogue, including non-verbal sounds (laughter, sighs, pauses) and variable prosody. This design plan outlines how to integrate ChatTTS capabilities into the **Atropa TTS & Vocoder Lab**.

---

## 1. System Architecture

To avoid running heavy neural Transformer models directly on the EVM / C controller (which is gas and resource-prohibitive), we propose a **hybrid client-server architecture**:

```mermaid
graph LR
    UI[Atropa Web Dashboard] -->|1. Text + Tags: e.g. hello [laughter]| API[Local Python FastAPI Bridge]
    API -->|2. Inference| ChatTTS[ChatTTS Transformer Engine]
    ChatTTS -->|3. Generate Waveform & Mel| API
    API -->|4. Return Quantized Mel/LPC Frames| UI
    UI -->|5. Playback or Stream to EVM| EVM[SpeechSynthesizer Contract]
```

---

## 2. Parsing Conversational Markers (G2P Tags)

ChatTTS allows control over conversational nuances using embedded tags. We can update our G2P parser to handle these markers:

| Tag | Intended Effect | LPC/SID Emulation Mapping |
| :--- | :--- | :--- |
| `[laughter]` | Short chuckle / laughter burst | Modulates excitation source with chaotic white noise and rapid pitch spikes. |
| `[sigh]` | Soft breath release | Forces unvoiced aspiration noise (low energy, high bandwidth). |
| `[uv_break]` | Short silence or gulp | Inserts a brief `PA0` (pause) frame into the phoneme chain. |

---

## 3. Python FastAPI Execution Bridge (`scripts/chattts_server.py`)

A lightweight wrapper running ChatTTS locally to generate Mel spectrograms and audio tokens:

```python
from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
import ChatTTS
import torch
import soundfile as sf
import io

app = FastAPI(title="Atropa ChatTTS Bridge")

# Initialize ChatTTS engine
chat = ChatTTS.Chat()
chat.load_models(source="local", local_path="./models")

class TTSRequest(BaseModel):
    text: str
    ref_speaker_seed: int = 42
    temperature: float = 0.3
    oral: float = 0.3  # controls frequency of laughter/sighs

@app.post("/api/synthesize")
async def synthesize(req: TTSRequest):
    try:
        # Run inference
        wavs = chat.infer([req.text], params_refine_text={'prompt': '[oral_2][laugh_2]'}, 
                          params_infer_code={'spk_emb': chat.sample_random_speaker(seed=req.ref_speaker_seed)})
        
        # Save output audio to buffer
        buffer = io.BytesIO()
        sf.write(buffer, wavs[0][0], 24000, format='WAV')
        buffer.seek(0)
        
        # Optionally extract Mel frames to return to the dashboard
        return {
            "status": "success",
            "audio_length": len(wavs[0][0]),
            "sample_rate": 24000
        }
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))
```

---

## 4. Frontend Dashboard Integration

1. **Tag Buttons**: Add quick-action conversational buttons (`Insert [laughter]`, `Insert [sigh]`, `Insert [uv_break]`) next to the main text area.
2. **Audio Streaming**: Stream the high-fidelity wav outputs directly into the visualizer panels, comparing the results against our LPC-16 vocoder using `scripts/voice_aligner.py`.
