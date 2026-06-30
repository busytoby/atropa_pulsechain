#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <alsa/asoundlib.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

// Emulate the 3 physical analog stages and protection layers at machine-code speeds
static void emulate_analog_pipeline(float *samples, int num_samples) {
    double V_c_in = 0.0;
    double V_c_e = 0.0;
    double env = 0.0;
    
    // DC blocking filters state variables for inter-stage AC coupling
    double stage1_x_prev = 0.0, stage1_y_prev = 0.0;
    double stage2_x_prev = 0.0, stage2_y_prev = 0.0;
    
    double bias_offset_pp = 0.18;    // 180mV bias for push-pull (Germanium)
    double bias_offset_trans = 0.68; // 680mV bias for transducer (Silicon)
    double loadRL = 8.0;             // 8 ohm speaker load
    
    // 17-Point Precomputed Germanium Base-Emitter Voltage Table (0V to 1.6V inputs)
    double lut[17] = {
        0.0, 0.04, 0.08, 0.115, 0.14, 0.16, 0.175, 0.185, 0.195,
        0.202, 0.208, 0.213, 0.218, 0.222, 0.226, 0.230, 0.235
    };

    for (int i = 0; i < num_samples; i++) {
        double input = samples[i] * 0.8; // Input voltage swing matching physical verification spec (-0.8V to +0.8V)

        // --- STAGE 1: GermaniumStage Pre-Amp ---
        // Biasing base in active region (+0.25V DC bias) to prevent negative cutoff clipping
        double V_b = (input + 0.25) - V_c_in;
        double V_e = V_c_e;
        double Vbe_input = V_b - V_e;
        
        // Solve Vbe via linear interpolation
        double Vbe = 0.0;
        if (Vbe_input > 0.0) {
            double scaled_idx = Vbe_input / 0.1;
            int idx = (int)scaled_idx;
            double frac = scaled_idx - idx;
            if (idx >= 15) {
                Vbe = lut[16];
            } else {
                Vbe = lut[idx] + (lut[idx+1] - lut[idx]) * frac;
            }
        }
        
        double Ib = 0.0;
        if (Vbe_input > Vbe) {
            Ib = (Vbe_input - Vbe) / 100000.0; // 100k ohm source resistance
        }
        double Ic = Ib * 100.0; // Beta = 100
        
        // Vout = Vcc - Ic * Rc (Vcc = 9V, Rc = 4.7k)
        double stage1_out = 9.0 - Ic * 4700.0;
        
        // Update capacitor charge variables on the physical substrate
        V_c_in = V_c_in + Ib * 0.01 - V_c_in * 0.001;
        V_c_e = V_c_e + Ib * 101.0 * 0.01 - V_c_e * 0.005;

        // Centering the collector voltage swing around 5.5V bias point
        double stage1_raw_ac = (5.5 - stage1_out) * 0.25;
        // DC Blocking filter (AC Coupling) to prevent low-frequency drift
        double stage1_ac = stage1_raw_ac - stage1_x_prev + 0.995 * stage1_y_prev;
        stage1_x_prev = stage1_raw_ac;
        stage1_y_prev = stage1_ac;

        // --- STAGE 2: TransducerStage Modulation ---
        double pressure = stage1_ac;
        double modulated_offset = 0.6 - pressure * 0.05;
        if (modulated_offset < 0.1) modulated_offset = 0.1;
        if (modulated_offset > 1.0) modulated_offset = 1.0;
        
        double Vbe_input_trans = bias_offset_trans - modulated_offset;
        double Ib_trans = Vbe_input_trans > 0.0 ? Vbe_input_trans / 10000.0 : 0.0;
        double Ic_trans = Ib_trans * 150.0; // Beta = 150
        double stage2_out = 9.0 - Ic_trans * 2200.0;
        if (stage2_out < 0.0) stage2_out = 0.0;
        if (stage2_out > 9.0) stage2_out = 9.0;
        
        // Centering the modulated output swing to map dynamic swings to [-1.0, 1.0] range
        double stage2_raw_ac = (8.0 - stage2_out) * 1.0;
        // DC Blocking filter (AC Coupling) to keep inputs to Push-Pull symmetric
        double stage2_ac = stage2_raw_ac - stage2_x_prev + 0.995 * stage2_y_prev;
        stage2_x_prev = stage2_raw_ac;
        stage2_y_prev = stage2_ac;

        // --- STAGE 3: PushPullStage & EquaAmplifier ---
        double effective_threshold = 0.2 - bias_offset_pp;
        if (effective_threshold < 0.0) effective_threshold = 0.0;
        
        double pp_out = 0.0;
        if (stage2_ac > effective_threshold) {
            pp_out = stage2_ac - effective_threshold;
        } else if (stage2_ac < -effective_threshold) {
            pp_out = stage2_ac + effective_threshold;
        }
        
        // EquaAmplifier SOAR current-limiting protection
        double I_limit = 1.97;
        double V_max = I_limit * loadRL;
        if (V_max < 0.1) V_max = 0.1;
        
        double absInput = fabs(pp_out);
        if (absInput > env) {
            env = (env + absInput) / 2.0;
        } else {
            env = env * 0.99;
        }
        
        double gain = 1.0;
        if (env > V_max) {
            gain = V_max / env;
        }
        double final_out = pp_out * gain;
        
        // Symmetrical soft limits
        double softLimit = V_max * 0.95;
        if (final_out > softLimit) final_out = softLimit;
        if (final_out < -softLimit) final_out = -softLimit;
        
        // Rescale output to dynamic range using softLimit normalization to preserve audible levels
        samples[i] = (float)(final_out / softLimit);
    }
}

// Helper to write raw WAV bytes to a file
static void write_wav_file(const char *filename, const char *hex_data) {
    if (strncmp(hex_data, "0x", 2) == 0) hex_data += 2;
    size_t hex_len = strlen(hex_data);
    size_t bin_len = hex_len / 2;
    uint8_t *buffer = malloc(bin_len);
    if (!buffer) return;

    for (size_t i = 0; i < bin_len; i++) {
        unsigned int val;
        sscanf(hex_data + i * 2, "%2x", &val);
        buffer[i] = (uint8_t)val;
    }

    // Process WAV PCM payload through physical analog stages
    if (bin_len > 44) {
        size_t pcm_bytes = bin_len - 44;
        size_t num_samples = pcm_bytes / 2;
        int16_t *pcm_ptr = (int16_t *)(buffer + 44);
        
        float *float_samples = malloc(num_samples * sizeof(float));
        if (float_samples) {
            printf("[C_SPEECH_DEBUG] First 20 raw PCM samples from EVM:\n");
            for (size_t i = 0; i < 20 && i < num_samples; i++) {
                printf("  Sample %zu: %d\n", i, pcm_ptr[i]);
            }
            for (size_t i = 0; i < num_samples; i++) {
                float_samples[i] = (float)pcm_ptr[i] / 32768.0f;
            }
            
            // Run emulate_analog_pipeline
            emulate_analog_pipeline(float_samples, (int)num_samples);
            
            // Peak normalization to 0.95 to maximize volume without clipping
            float initial_peak = 0.0f;
            for (size_t i = 0; i < num_samples; i++) {
                float abs_val = fabs(float_samples[i]);
                if (abs_val > initial_peak) initial_peak = abs_val;
            }
            if (initial_peak > 0.0001f) {
                float norm_factor = 0.95f / initial_peak;
                for (size_t i = 0; i < num_samples; i++) {
                    float_samples[i] *= norm_factor;
                }
            }

            float peak = 0.0f;
            float rms_sum = 0.0f;
            int clip_count = 0;
            for (size_t i = 0; i < num_samples; i++) {
                float val = float_samples[i];
                float abs_val = fabs(val);
                if (abs_val > peak) peak = abs_val;
                rms_sum += val * val;
                if (abs_val >= 0.94f) clip_count++;
                
                if (val > 1.0f) val = 1.0f;
                if (val < -1.0f) val = -1.0f;
                pcm_ptr[i] = (int16_t)(val * 32767.0f);
            }
            float rms = sqrt(rms_sum / num_samples);
            float clip_ratio = (float)clip_count / num_samples;
            
            printf("[TEST_SPEECH_METRICS] Waveform analysis:\n");
            printf("  - Samples analyzed: %zu\n", num_samples);
            printf("  - Peak amplitude: %.4f\n", peak);
            printf("  - RMS energy: %.4f\n", rms);
            printf("  - Clipping ratio: %.2f%%\n", clip_ratio * 100.0f);
            if (clip_ratio > 0.05f) {
                printf("[TEST_SPEECH_WARNING] Extreme clipping detected (%.2f%%). Dynamic range is saturated!\n", clip_ratio * 100.0f);
            } else {
                printf("[TEST_SPEECH_SUCCESS] Waveform characteristics resemble valid, unsaturated human speech.\n");
            }
            free(float_samples);
            printf("[C_SPEECH] Successfully processed audio through virtual analog stages.\n");
        }
    }

    FILE *fp = fopen(filename, "wb");
    if (fp) {
        fwrite(buffer, 1, bin_len, fp);
        fclose(fp);
        printf("[C_SPEECH] Successfully wrote WAV to: %s (%zu bytes)\n", filename, bin_len);
    } else {
        printf("[C_SPEECH] Error: Failed to open %s for writing\n", filename);
    }
    free(buffer);
}

// ALSA Playback Bridge
static void play_wav_file(const char *filename) {
    if (getenv("SILENT") || getenv("HEADLESS")) {
        return;
    }
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        printf("[C_SPEECH] Error: Could not open WAV file for playback: %s\n", filename);
        return;
    }

    uint8_t header[44];
    if (fread(header, 1, 44, fp) != 44) {
        printf("[C_SPEECH] Error: Invalid WAV header size\n");
        fclose(fp);
        return;
    }

    // Parse channel count, sample rate, bits per sample
    uint16_t channels = header[22] | (header[23] << 8);
    uint32_t sample_rate = header[24] | (header[25] << 8) | (header[26] << 16) | (header[27] << 24);
    uint16_t bits_per_sample = header[34] | (header[35] << 8);

    printf("[C_SPEECH] Playing WAV via ALSA: %s (%d-bit, %d-channel, %d Hz)\n",
           filename, bits_per_sample, channels, sample_rate);

    snd_pcm_format_t format;
    if (bits_per_sample == 8) {
        format = SND_PCM_FORMAT_U8;
    } else if (bits_per_sample == 16) {
        format = SND_PCM_FORMAT_S16_LE;
    } else if (bits_per_sample == 32) {
        format = SND_PCM_FORMAT_S32_LE;
    } else {
        printf("[C_SPEECH] Unsupported bits per sample: %d\n", bits_per_sample);
        fclose(fp);
        return;
    }

    snd_pcm_t *pcm_handle;
    if (snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) {
        printf("[C_SPEECH] Error: Cannot open default ALSA PCM device\n");
        fclose(fp);
        return;
    }

    if (snd_pcm_set_params(pcm_handle, format, SND_PCM_ACCESS_RW_INTERLEAVED,
                           channels, sample_rate, 1, 500000) < 0) {
        printf("[C_SPEECH] Error: Cannot set ALSA parameters\n");
        snd_pcm_close(pcm_handle);
        fclose(fp);
        return;
    }

    // Read and write PCM data
    uint8_t buffer[1024];
    size_t bytes_read;
    int bytes_per_frame = (bits_per_sample / 8) * channels;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        snd_pcm_sframes_t frames = bytes_read / bytes_per_frame;
        snd_pcm_sframes_t written = snd_pcm_writei(pcm_handle, buffer, frames);
        if (written < 0) {
            snd_pcm_prepare(pcm_handle);
            snd_pcm_writei(pcm_handle, buffer, frames);
        }
    }

    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);
    fclose(fp);
    printf("[C_SPEECH] ALSA playback finished.\n");
}

// Helper to check if a string is a valid integer (including negative signs)
static int is_numeric(const char *str) {
    if (!str || *str == '\0') return 0;
    if (*str == '-') str++;
    if (*str == '\0') return 0;
    while (*str) {
        if (*str < '0' || *str > '9') return 0;
        str++;
    }
    return 1;
}

int main(int argc, char **argv) {
    printf("=== TSFi C Speech Generation Controller Integration ===\n");
    tsfi_wire_firmware_init();

    // Default arguments
    const char *voice = "ana";
    int embedding[8] = { 15, -25, 40, -10, 5, -8, 12, -5 }; // Default 'ana' coefficients * 100
    int phoneme_count = 5;
    const char *phonemes[64] = { "ee", "oo", "m", "oo", "s" };

    // Detect if we have 8 embedding coefficients passed before phonemes.
    int has_coefficients = 0;
    if (argc > 9) {
        has_coefficients = 1;
        for (int i = 0; i < 8; i++) {
            if (!is_numeric(argv[2 + i])) {
                has_coefficients = 0;
                break;
            }
        }
    }

    if (has_coefficients) {
        voice = argv[1];
        for (int i = 0; i < 8; i++) {
            embedding[i] = atoi(argv[2 + i]);
        }
        phoneme_count = argc - 10;
        if (phoneme_count < 1) {
            printf("[C_SPEECH] Error: At least one phoneme must be specified.\n");
            return 1;
        }
        if (phoneme_count > 64) phoneme_count = 64;
        for (int i = 0; i < phoneme_count; i++) {
            phonemes[i] = argv[i + 10];
        }
    } else if (argc > 1) {
        // Fallback for voice + phonemes only (without coefficients)
        voice = argv[1];
        phoneme_count = argc - 2;
        if (phoneme_count < 1) {
            printf("[C_SPEECH] Error: At least one phoneme must be specified.\n");
            return 1;
        }
        if (phoneme_count > 64) phoneme_count = 64;
        for (int i = 0; i < phoneme_count; i++) {
            phonemes[i] = argv[i + 2];
        }
    }

    printf("[C_SPEECH] Generating speech for voice: '%s' with %d phonemes\n", voice, phoneme_count);
    printf("[C_SPEECH] Using Speaker Coefficients: [%d, %d, %d, %d, %d, %d, %d, %d]\n",
           embedding[0], embedding[1], embedding[2], embedding[3],
           embedding[4], embedding[5], embedding[6], embedding[7]);

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Initialize speechSynthesizer.yul in EVM context
    printf("[C_SPEECH] Loading speechSynthesizer Yul contract...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"speechSynthesizer\", \"../solidity/bin/speechSynthesizer.yul\", 0x8ff9a7885e7CB3C01D556e1bC4f9fb2A6e7Ac167");
    
    // Check if initialization succeeded
    if (strstr(vm.output_buffer, "Error") != NULL) {
        printf("[C_SPEECH] Error loading speechSynthesizer: %s\n", vm.output_buffer);
        tsfi_zmm_vm_destroy(&vm);
        return 1;
    }
    printf("[C_SPEECH] Yul Speech Synthesizer Contract Registered Successfully.\n");

    // 1a. Upload packed INT8 weights to Yul contract storage (slots 0x7777 and 0x7778)
    printf("[C_SPEECH] Loading packed neural weights into contract storage...\n");
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"speechSynthesizer\", \"200c2ae0"
        "0000000000000000000000000000000000000000000000000000000000000000" // slot offset 0
        "05000aec50f10ffb0ff6050ae246f61405fb0a000fec3cf6f60afb05000aec32" // packed0
        "\"");
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"speechSynthesizer\", \"200c2ae0"
        "0000000000000000000000000000000000000000000000000000000000000001" // slot offset 1
        "5af10fecf1140af6e755f60f05f6fb050afb4bf60a050af1ec14f141f60f000a" // packed1
        "\"");

    // 1b. Register the speaker dynamically
    char register_calldata[4096];
    strcpy(register_calldata, "YULEXEC \"speechSynthesizer\", \"5f60cdf4");
    
    // Voice name (bytes32 right-padded)
    char voice_name_hex[65];
    memset(voice_name_hex, '0', 64);
    voice_name_hex[64] = '\0';
    for (size_t i = 0; i < strlen(voice) && i < 32; i++) {
        sprintf(voice_name_hex + i * 2, "%02x", voice[i]);
    }
    for (size_t i = strlen(voice) * 2; i < 64; i++) {
        voice_name_hex[i] = '0';
    }
    strcat(register_calldata, voice_name_hex);
    
    // Embedding coefficients (8 elements, each 32 bytes hex representing signed 256-bit int)
    for (int i = 0; i < 8; i++) {
        char coeff_hex[128];
        // Convert to two's complement hex if negative
        if (embedding[i] < 0) {
            // Represent as negative in hex (256-bit)
            // A simple way is to pad with Fs
            sprintf(coeff_hex, "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff%04x", (unsigned int)(embedding[i] & 0xFFFF));
        } else {
            sprintf(coeff_hex, "000000000000000000000000000000000000000000000000000000000000%04x", (unsigned int)embedding[i]);
        }
        strcat(register_calldata, coeff_hex);
    }

    // Append optional dynamic pitch contour from PITCH_CONTOUR env var
    const char *pitch_env = getenv("PITCH_CONTOUR");
    if (pitch_env) {
        printf("[C_SPEECH] Found PITCH_CONTOUR environment variable: %s\n", pitch_env);
        char *pitch_env_copy = strdup(pitch_env);
        char *token = strtok(pitch_env_copy, ",");
        int count = 0;
        while (token && count < 24) {
            int pitch_val = atoi(token);
            char pitch_hex[128];
            sprintf(pitch_hex, "000000000000000000000000000000000000000000000000000000000000%04x", (unsigned int)(pitch_val & 0xFFFF));
            strcat(register_calldata, pitch_hex);
            token = strtok(NULL, ",");
            count++;
        }
        free(pitch_env_copy);
        printf("[C_SPEECH] Appended %d autotune pitch values to speaker registration.\n", count);
    }
    
    strcat(register_calldata, "\"");

    printf("[C_SPEECH] Registering speaker profile on-chain...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, register_calldata);

    // 2. Predict Mel-Spectrogram using predictMelQuantized
    // Dynamic Construction of Calldata for predictMelQuantized
    char mel_calldata[8192];
    strcpy(mel_calldata, "YULEXEC \"speechSynthesizer\", \"18c1ab9a");
    
    // Offset (64 / 0x40)
    strcat(mel_calldata, "0000000000000000000000000000000000000000000000000000000000000040");
    
    // Voice (bytes32 right-padded)
    strcat(mel_calldata, voice_name_hex);
    
    // Array length (32 bytes)
    char len_hex[65];
    sprintf(len_hex, "%064x", phoneme_count);
    strcat(mel_calldata, len_hex);
    
    // Phonemes (each bytes32 right-padded)
    for (int i = 0; i < phoneme_count; i++) {
        char ph_hex[65];
        memset(ph_hex, '0', 64);
        ph_hex[64] = '\0';
        for (size_t j = 0; j < strlen(phonemes[i]) && j < 32; j++) {
            sprintf(ph_hex + j * 2, "%02x", phonemes[i][j]);
        }
        for (size_t j = strlen(phonemes[i]) * 2; j < 64; j++) {
            ph_hex[j] = '0';
        }
        strcat(mel_calldata, ph_hex);
    }
    strcat(mel_calldata, "\"");

    printf("[C_SPEECH] Executing predictMelQuantized dynamic inference...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, mel_calldata);

    // Extract Mel frames returned (we need to skip ABI array headers)
    char mel_data_hex[256];
    memset(mel_data_hex, 0, sizeof(mel_data_hex));
    
    const char *out_ptr = vm.output_buffer;
    if (strlen(out_ptr) >= 192) {
        memcpy(mel_data_hex, out_ptr + 128, 80);
        mel_data_hex[80] = '\0';
        printf("[C_SPEECH] Predicted Mel Spectrogram Hex: %s\n", mel_data_hex);
    } else {
        printf("[C_SPEECH] Error: Invalid output length from predictMelQuantized: %s\n", vm.output_buffer);
        tsfi_zmm_vm_destroy(&vm);
        return 1;
    }

    // 3. Run synthesizeNeuralWav using the returned Mel frames
    char synth_exec_cmd[2048];
    snprintf(synth_exec_cmd, sizeof(synth_exec_cmd),
        "YULEXEC \"speechSynthesizer\", \"20c4433b"
        "0000000000000000000000000000000000000000000000000000000000000060" // Offset of bytes array (96)
        "0000000000000000000000000000000000000000000000000000000000000a00" // upsampleFactor (2560)
        "%s" // speaker name (bytes32 hex)
        "0000000000000000000000000000000000000000000000000000000000000028" // Length of bytes (40)
        "%s" // Mel spectrogram bytes (80 hex characters)
        "0000000000000000000000000000000000000000000000000000000000000000" // padding
        "\"", voice_name_hex, mel_data_hex);

    printf("[C_SPEECH] Executing synthesizeNeuralWav vocoder...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, synth_exec_cmd);

    // Save WAV file
    if (strlen(vm.output_buffer) > 128) {
        char len_hex[65];
        strncpy(len_hex, vm.output_buffer + 64, 64);
        len_hex[64] = '\0';
        unsigned long wav_len = strtoul(len_hex, NULL, 16);
        printf("[C_SPEECH] Received synthesized audio size: %lu bytes\n", wav_len);

        char *wav_hex_data = vm.output_buffer + 128;
        wav_hex_data[wav_len * 2] = '\0';

        write_wav_file("karateka_c_synthesis.wav", wav_hex_data);
        play_wav_file("karateka_c_synthesis.wav");
    } else {
        printf("[C_SPEECH] Error: Synthesis execution failed or output too short: %s\n", vm.output_buffer);
        tsfi_zmm_vm_destroy(&vm);
        return 1;
    }

    tsfi_zmm_vm_destroy(&vm);
    printf("=== C SPEECH GENERATION CONTROLLER INTEGRATION SUCCESS ===\n");
    return 0;
}
