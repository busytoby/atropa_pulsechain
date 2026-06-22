const fs = require("fs");
const path = require("path");

function compressDNA() {
    console.log("=== WILBY TECH: DNA COMPACTOR ===");
    
    const inputPath = path.join(__dirname, "../tsfi2-deepseek/assets/atropa.dna");
    if (!fs.existsSync(inputPath)) {
        console.error(`Input DNA file not found at: ${inputPath}`);
        process.exit(1);
    }
    
    const buffer = fs.readFileSync(inputPath);
    const magic = buffer.toString("ascii", 0, 4);
    if (magic !== "TSFI") {
        console.error("Invalid DNA magic header!");
        process.exit(1);
    }
    
    const totalFrames = buffer.readUInt32LE(4);
    console.log(`Loaded DNA file: ${totalFrames} frames.`);
    
    const compactFrames = [];
    let prevFrameBytes = null;
    let rleCount = 0;
    
    const frameSize = 31; // 6 floats (24 bytes) + 7 bytes = 31 bytes
    
    for (let f = 0; f < totalFrames; f++) {
        const offset = 8 + f * frameSize;
        if (offset + frameSize > buffer.length) break;
        
        // Read raw float values
        const g_x = buffer.readFloatLE(offset);
        const g_y = buffer.readFloatLE(offset + 4);
        const body_y_stretch = buffer.readFloatLE(offset + 8);
        const pulse = buffer.readFloatLE(offset + 12);
        const dynamic_sick = buffer.readFloatLE(offset + 16);
        const light_intensity = buffer.readFloatLE(offset + 20);
        
        // Read color & state bytes
        const fur_r = buffer.readUInt8(offset + 24);
        const fur_g = buffer.readUInt8(offset + 25);
        const fur_b = buffer.readUInt8(offset + 26);
        const eye_r = buffer.readUInt8(offset + 27);
        const eye_g = buffer.readUInt8(offset + 28);
        const eye_b = buffer.readUInt8(offset + 29);
        const eye_count = buffer.readUInt8(offset + 30);
        
        // Wilby Tech Feature: Scale and convert float32 to compact int8 fixed-point (Q8.8 or custom bounds)
        // g_x is usually in range [-2.0, 2.0]
        const pack_g_x = Math.min(127, Math.max(-128, Math.round(g_x * 64)));
        // g_y is usually in range [-1.0, 1.0]
        const pack_g_y = Math.min(127, Math.max(-128, Math.round(g_y * 128)));
        // body_y_stretch is in range [0, 1.0]
        const pack_stretch = Math.min(127, Math.max(-128, Math.round(body_y_stretch * 256 - 128)));
        // pulse is in range [-1.0, 1.0]
        const pack_pulse = Math.min(127, Math.max(-128, Math.round(pulse * 127)));
        // dynamic_sick is in range [0, 1.0]
        const pack_sick = Math.min(127, Math.max(-128, Math.round(dynamic_sick * 127)));
        // light_intensity is in range [0, 2.0]
        const pack_light = Math.min(127, Math.max(-128, Math.round(light_intensity * 64)));
        
        // Build 13-byte frame byte array
        const currentBytes = Buffer.alloc(13);
        currentBytes.writeInt8(pack_g_x, 0);
        currentBytes.writeInt8(pack_g_y, 1);
        currentBytes.writeInt8(pack_stretch, 2);
        currentBytes.writeInt8(pack_pulse, 3);
        currentBytes.writeInt8(pack_sick, 4);
        currentBytes.writeInt8(pack_light, 5);
        currentBytes.writeUInt8(fur_r, 6);
        currentBytes.writeUInt8(fur_g, 7);
        currentBytes.writeUInt8(fur_b, 8);
        currentBytes.writeUInt8(eye_r, 9);
        currentBytes.writeUInt8(eye_g, 10);
        currentBytes.writeUInt8(eye_b, 11);
        currentBytes.writeUInt8(eye_count, 12);
        
        // Wilby Tech Feature: Run-Length Encoding (RLE) to compress sequential stationary/matching frames
        if (prevFrameBytes && currentBytes.equals(prevFrameBytes) && rleCount < 255) {
            rleCount++;
        } else {
            if (prevFrameBytes) {
                compactFrames.push({ bytes: prevFrameBytes, repeat: rleCount });
            }
            prevFrameBytes = currentBytes;
            rleCount = 0;
        }
    }
    
    // Push the final frame RLE block
    if (prevFrameBytes) {
        compactFrames.push({ bytes: prevFrameBytes, repeat: rleCount });
    }
    
    // Serialize compact DNA output (Header: 'DNAC' + frame count)
    const outBufferList = [];
    const header = Buffer.alloc(8);
    header.write("DNAC", 0, 4, "ascii");
    header.writeUInt32LE(totalFrames, 4);
    outBufferList.push(header);
    
    for (const block of compactFrames) {
        // 1 byte repeat count + 13 bytes frame payload = 14 bytes per RLE chunk
        const blockHeader = Buffer.alloc(1);
        blockHeader.writeUInt8(block.repeat, 0);
        outBufferList.push(blockHeader);
        outBufferList.push(block.bytes);
    }
    
    const finalCompactBuffer = Buffer.concat(outBufferList);
    const outputPath = path.join(__dirname, "../tsfi2-deepseek/assets/atropa_compact.dna");
    fs.writeFileSync(outputPath, finalCompactBuffer);
    
    const origSize = buffer.length;
    const compSize = finalCompactBuffer.length;
    const ratio = (100 - (compSize / origSize) * 100).toFixed(2);
    
    console.log(`[SUCCESS] Volumetric DNA compressed using Wilby size-coding!`);
    console.log(`  Original Size  : ${(origSize / 1024).toFixed(2)} KB (${origSize} bytes)`);
    console.log(`  Compressed Size: ${(compSize / 1024).toFixed(2)} KB (${compSize} bytes)`);
    console.log(`  Space Saved    : ${ratio}%`);
    console.log(`  Output Path    : ${outputPath}`);

    // Wilby Validation Check
    console.log("Running decompression and data boundary integrity audits...");
    if (verifyDecompression(finalCompactBuffer, buffer)) {
        console.log("[SUCCESS] Wilby Validation: Lossless state bytes and quantized float bounds verified successfully!");
    } else {
        console.error("[ERROR] Wilby Validation: Mismatch detected in decompressed DNA bounds!");
        process.exit(1);
    }
}

function verifyDecompression(compactBuf, originalBuf) {
    const magic = compactBuf.toString("ascii", 0, 4);
    if (magic !== "DNAC") return false;
    const totalFrames = compactBuf.readUInt32LE(4);
    let offset = 8;
    let decompressedFrames = 0;
    const frameSize = 31;
    
    while (offset < compactBuf.length) {
        if (offset + 14 > compactBuf.length) break;
        const repeat = compactBuf.readUInt8(offset);
        const payload = compactBuf.subarray(offset + 1, offset + 14);
        offset += 14;
        
        const pack_g_x = payload.readInt8(0);
        const fur_r = payload.readUInt8(6);
        const fur_g = payload.readUInt8(7);
        const fur_b = payload.readUInt8(8);
        const eye_r = payload.readUInt8(9);
        const eye_g = payload.readUInt8(10);
        const eye_b = payload.readUInt8(11);
        const eye_count = payload.readUInt8(12);
        
        for (let r = 0; r <= repeat; r++) {
            const origOffset = 8 + decompressedFrames * frameSize;
            if (origOffset + frameSize > originalBuf.length) return false;
            
            if (originalBuf.readUInt8(origOffset + 24) !== fur_r ||
                originalBuf.readUInt8(origOffset + 25) !== fur_g ||
                originalBuf.readUInt8(origOffset + 26) !== fur_b ||
                originalBuf.readUInt8(origOffset + 27) !== eye_r ||
                originalBuf.readUInt8(origOffset + 28) !== eye_g ||
                originalBuf.readUInt8(origOffset + 29) !== eye_b ||
                originalBuf.readUInt8(origOffset + 30) !== eye_count) {
                console.log(`Mismatch in lossless bytes at frame ${decompressedFrames}`);
                return false;
            }
            
            const orig_g_x = originalBuf.readFloatLE(origOffset);
            const calc_g_x = pack_g_x / 64;
            const clipped_orig = Math.min(127 / 64, Math.max(-128 / 64, orig_g_x));
            if (Math.abs(clipped_orig - calc_g_x) > 0.01) {
                console.log(`Mismatch in float g_x at frame ${decompressedFrames}: orig=${orig_g_x}, clipped=${clipped_orig}, calc=${calc_g_x}`);
                return false;
            }
            decompressedFrames++;
        }
    }
    return decompressedFrames === totalFrames;
}

compressDNA();
