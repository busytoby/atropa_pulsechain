object "MusicMaker" {
    // ========================================================================
    // DEPLOYMENT INITIALIZATION BLOCK (INITCODE)
    // ========================================================================
    code {
        if callvalue() { revert(0, 0) }

        // Copy the core runtime bytecode into memory at slot 0x00
        datacopy(0x00, dataoffset("runtime"), datasize("runtime"))
        
        // Return the runtime bytecode
        return(0x00, datasize("runtime"))
    }
    
    // ========================================================================
    // MAIN PRODUCTION RUNTIME ENVIRONMENT
    // ========================================================================
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))

            // Storage layout:
            // Storage slots map to the virtual SID memory address space.
            // slot = register address (e.g. 54272 for voice 1 frequency low, etc.)

            // ----------------------------------------------------------------
            // METHOD 1: poke(uint16 addr, uint8 val)
            // Selector: 0x86bb605e
            // ----------------------------------------------------------------
            if eq(selector, 0x86bb605e) {
                let addr := shr(240, calldataload(4))
                let val := shr(248, calldataload(36))

                // Limit virtual address space to C64 SID region (54272 to 54300) or PC Speaker (97)
                if and(iszero(eq(addr, 97)), or(lt(addr, 54272), gt(addr, 54300))) { revert(0, 0) }

                sstore(addr, val)

                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 2: peek(uint16 addr) -> uint8
            // Selector: 0x98b0a09e
            // ----------------------------------------------------------------
            if eq(selector, 0x98b0a09e) {
                let addr := shr(240, calldataload(4))

                // Limit virtual address space to C64 SID region (54272 to 54300) or PC Speaker (97)
                if and(iszero(eq(addr, 97)), or(lt(addr, 54272), gt(addr, 54300))) { revert(0, 0) }

                let val := sload(addr)
                mstore(0x00, val)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 3: getVoice1Frequency() -> uint16
            // Selector: 0x112df49e
            // ----------------------------------------------------------------
            if eq(selector, 0x112df49e) {
                let lo := sload(54272) // Voice 1 Freq Low
                let hi := sload(54273) // Voice 1 Freq High
                let freq := or(shl(8, hi), lo)

                mstore(0x00, freq)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 4: getSIDState() -> uint256[25]
            // Selector: 0x361ec891
            // ----------------------------------------------------------------
            if eq(selector, 0x361ec891) {
                for { let i := 0 } lt(i, 25) { i := add(i, 1) } {
                    let val := sload(add(54272, i))
                    mstore(mul(i, 32), val)
                }
                return(0x00, 800)
            }

            // ----------------------------------------------------------------
            // METHOD 5: playNote(uint16 note, uint8 voice)
            // Selector: 0xf63eb9b8
            // ----------------------------------------------------------------
            if eq(selector, 0xf63eb9b8) {
                let note := shr(240, calldataload(4))
                let voice := shr(248, calldataload(36))

                let baseReg := 54272
                if eq(voice, 2) { baseReg := 54279 }
                if eq(voice, 3) { baseReg := 54286 }

                let freq := mul(note, 100)
                let lo := and(freq, 0xff)
                let hi := and(shr(8, freq), 0xff)

                sstore(baseReg, lo)
                sstore(add(baseReg, 1), hi)

                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 6: play4BitSample(uint256 sampleVal)
            // Selector: 0xf35e41d6
            // ----------------------------------------------------------------
            if eq(selector, 0xf35e41d6) {
                let val := calldataload(4)
                sstore(54296, and(val, 0x0F))
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 7: playRealSoundSample(uint256 sampleVal)
            // Selector: 0xe7510ea6
            // ----------------------------------------------------------------
            if eq(selector, 0xe7510ea6) {
                let val := calldataload(4)
                sstore(97, and(val, 0x3F)) // Save 6-bit sample (0-63) in port $61/PIT slot 97
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 8: generateRealSoundClip(uint256 clipId) -> uint256[]
            // Selector: 0x9472ba6f
            // ----------------------------------------------------------------
            if eq(selector, 0x9472ba6f) {
                let clipId := calldataload(4)
                let size := 0
                
                if eq(clipId, 0) {
                    // Clip 0: Golf Swing Whoosh (800 samples)
                    size := 800
                    let seed := 0x12345678
                    for { let i := 0 } lt(i, size) { i := add(i, 1) } {
                        seed := add(mul(seed, 1103515245), 12345)
                        let noise := and(seed, 0x3F)
                        let vol := 0
                        if lt(i, 250) {
                            vol := div(mul(i, 40), 250)
                        }
                        if iszero(lt(i, 250)) {
                            vol := div(mul(sub(size, i), 40), sub(size, 250))
                        }
                        let sample := div(mul(noise, vol), 63)
                        
                        let mAddr := add(0x10000, mul(i, 32))
                        mstore(mAddr, sample)
                        
                        let filterLen := add(1, div(mul(i, 8), size))
                        let sum := 0
                        for { let f := 0 } lt(f, filterLen) { f := add(f, 1) } {
                            let prevIdx := i
                            if gt(i, f) { prevIdx := sub(i, f) }
                            sum := add(sum, mload(add(0x10000, mul(prevIdx, 32))))
                        }
                        sample := div(sum, filterLen)
                        mstore(add(0x40, mul(i, 32)), sample)
                    }
                }
                
                if eq(clipId, 1) {
                    // Clip 1: Golf Tee Hit (800 samples)
                    size := 800
                    let seed := 0xABCDEF12
                    for { let i := 0 } lt(i, size) { i := add(i, 1) } {
                        let sample := 0
                        if lt(i, 80) {
                            let p := mod(i, 2)
                            let vol := div(mul(sub(80, i), 63), 80)
                            if p { sample := vol }
                            seed := add(mul(seed, 1103515245), 12345)
                            let click := div(and(seed, 0x3F), 4)
                            sample := add(sample, click)
                            if gt(sample, 63) { sample := 63 }
                        }
                        if and(iszero(lt(i, 80)), lt(i, 400)) {
                            let p := mod(div(i, 2), 2)
                            let vol := div(mul(sub(400, i), 35), 320)
                            if p { sample := vol }
                        }
                        if and(iszero(lt(i, 400)), lt(i, 800)) {
                            let p := mod(div(i, 4), 2)
                            let vol := div(mul(sub(800, i), 15), 400)
                            if p { sample := vol }
                        }
                        mstore(add(0x40, mul(i, 32)), sample)
                    }
                }
                
                if eq(clipId, 2) {
                    // Clip 2: "Nice shot!" speech (2400 samples)
                    size := 2400
                    let seed := 0x5555AAAA
                    for { let i := 0 } lt(i, size) { i := add(i, 1) } {
                        let sample := 0
                        if lt(i, 400) {
                            let p := mod(div(i, 12), 2)
                            if p { sample := 12 }
                        }
                        if and(iszero(lt(i, 400)), lt(i, 1000)) {
                            let wave1 := mod(div(i, 3), 2)
                            let wave2 := mod(div(i, 2), 2)
                            let vol := 40
                            sample := div(add(mul(wave1, vol), mul(wave2, div(vol, 2))), 2)
                        }
                        if and(iszero(lt(i, 1000)), lt(i, 1400)) {
                            seed := add(mul(seed, 1103515245), 12345)
                            sample := and(seed, 0x1F)
                        }
                        if and(iszero(lt(i, 1400)), lt(i, 1800)) {
                            seed := add(mul(seed, 1103515245), 12345)
                            let n1 := and(seed, 0x1F)
                            seed := add(mul(seed, 1103515245), 12345)
                            let n2 := and(seed, 0x1F)
                            sample := div(add(n1, n2), 2)
                        }
                        if and(iszero(lt(i, 1800)), lt(i, 2200)) {
                            let p := mod(div(i, 5), 2)
                            if p { sample := 35 }
                        }
                        if and(iszero(lt(i, 2200)), lt(i, 2400)) {
                            if lt(sub(i, 2200), 40) {
                                seed := add(mul(seed, 1103515245), 12345)
                                sample := and(seed, 0x15)
                            }
                        }
                        mstore(add(0x40, mul(i, 32)), sample)
                    }
                }
                
                if eq(clipId, 3) {
                    // Clip 3: "Ouch!" speech (1600 samples)
                    size := 1600
                    let seed := 0x99887766
                    for { let i := 0 } lt(i, size) { i := add(i, 1) } {
                        let sample := 0
                        if lt(i, 600) {
                            let period := add(4, div(mul(i, 3), 600))
                            let p := mod(div(i, period), 2)
                            if p { sample := 45 }
                        }
                        if and(iszero(lt(i, 600)), lt(i, 1100)) {
                            let p := mod(div(i, 7), 2)
                            if p { sample := 30 }
                        }
                        if and(iszero(lt(i, 1100)), lt(i, 1600)) {
                            seed := add(mul(seed, 1103515245), 12345)
                            sample := and(seed, 0x27)
                        }
                        mstore(add(0x40, mul(i, 32)), sample)
                    }
                }
                
                if eq(clipId, 4) {
                    // Clip 4: "Welcome back, Tex!" speech (4000 samples)
                    size := 4000
                    let seed := 0x77332211
                    for { let i := 0 } lt(i, size) { i := add(i, 1) } {
                        let sample := 0
                        if lt(i, 600) {
                            if lt(i, 150) {
                                let p := mod(div(i, 8), 2)
                                if p { sample := 30 }
                            }
                            if and(iszero(lt(i, 150)), lt(i, 350)) {
                                let p := mod(div(i, 6), 2)
                                if p { sample := 35 }
                            }
                            if and(iszero(lt(i, 350)), lt(i, 450)) {
                                seed := add(mul(seed, 1103515245), 12345)
                                sample := and(seed, 0x1F)
                            }
                            if iszero(lt(i, 450)) {
                                let p := mod(div(i, 9), 2)
                                if p { sample := 25 }
                            }
                        }
                        if and(iszero(lt(i, 600)), lt(i, 800)) {
                            if lt(i, 650) {
                                let p := mod(div(i, 10), 2)
                                if p { sample := 40 }
                            }
                            if and(iszero(lt(i, 650)), lt(i, 750)) {
                                let p := mod(div(i, 7), 2)
                                if p { sample := 45 }
                            }
                            if iszero(lt(i, 750)) {
                                seed := add(mul(seed, 1103515245), 12345)
                                sample := and(seed, 0x25)
                            }
                        }
                        if and(iszero(lt(i, 900)), lt(i, 1500)) {
                            if lt(i, 950) {
                                seed := add(mul(seed, 1103515245), 12345)
                                sample := and(seed, 0x2C)
                            }
                            if and(iszero(lt(i, 950)), lt(i, 1200)) {
                                let p := mod(div(i, 5), 2)
                                if p { sample := 40 }
                            }
                            if iszero(lt(i, 1200)) {
                                seed := add(mul(seed, 1103515245), 12345)
                                sample := and(seed, 0x35)
                            }
                        }
                        mstore(add(0x40, mul(i, 32)), sample)
                    }
                }
                
                mstore(0x00, 0x20)
                mstore(0x20, size)
                let totalSize := add(0x40, mul(size, 32))
                return(0x00, totalSize)
            }

            revert(0, 0)
        }
    }
}
