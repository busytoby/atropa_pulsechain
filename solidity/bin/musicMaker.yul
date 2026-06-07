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

                // Limit virtual SID address space to C64 SID region (54272 to 54300)
                if or(lt(addr, 54272), gt(addr, 54300)) { revert(0, 0) }

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

                if or(lt(addr, 54272), gt(addr, 54300)) { revert(0, 0) }

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

            revert(0, 0)
        }
    }
}
