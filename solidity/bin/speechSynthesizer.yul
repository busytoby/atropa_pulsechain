object "SpeechSynthesizer" {
    code {
        // Initcode: Copy runtime code to memory and return it for deployment
        datacopy(0x00, dataoffset("runtime"), datasize("runtime"))
        return(0x00, datasize("runtime"))
    }

    object "runtime" {
        code {
            // Revert on call values
            if callvalue() { revert(0, 0) }

            // Ensure calldatasize is at least 4 bytes
            if lt(calldatasize(), 4) { revert(0, 0) }

            let selector := shr(224, calldataload(0))

            // Helper to get current CPU address
            function getCpuAddress() -> cpuAddr {
                cpuAddr := sload(0)
                if iszero(cpuAddr) {
                    cpuAddr := 0x270049e5af030c305Fe92C3af096A5E3445B7fb7 // Default hardcoded localhost CPU
                }
            }

            // Helper to call peekUser(address user, uint256 addr) -> uint256
            function peekUser(cpu, user, addr) -> val {
                mstore(0x1000, shl(224, 0x0ccd522c)) // selector for peekUser(address,uint256)
                mstore(0x1004, user)
                mstore(0x1024, addr)
                let ok := staticcall(gas(), cpu, 0x1000, 68, 0x2000, 32)
                if iszero(ok) { revert(0, 0) }
                val := mload(0x2000)
            }

            // Helper to call pokeUser(address user, uint256 addr, uint256 val) -> uint256
            function pokeUser(cpu, user, addr, val) {
                mstore(0x1000, shl(224, 0xa7260f02)) // selector for pokeUser(address,uint256,uint256)
                mstore(0x1004, user)
                mstore(0x1024, addr)
                mstore(0x1044, val)
                let ok := call(gas(), cpu, 0, 0x1000, 100, 0x2000, 32)
                if iszero(ok) { revert(0, 0) }
            }

            // ----------------------------------------------------------------
            // Method: bindCpuAddress(address cpu) -> bool
            // Selector: 0x678575ea
            // ----------------------------------------------------------------
            if eq(selector, 0x678575ea) {
                let cpu := and(calldataload(4), 0xffffffffffffffffffffffffffffffffffffffff)
                sstore(0, cpu)
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method: getSpeechState() -> (uint256, uint256, uint256, uint256)
            // Selector: 0x8040caa2
            // ----------------------------------------------------------------
            if eq(selector, 0x8040caa2) {
                let cpu := getCpuAddress()
                let callerAddr := caller()
                let p := peekUser(cpu, callerAddr, 54784)
                let inf := peekUser(cpu, callerAddr, 54785)
                let busy := peekUser(cpu, callerAddr, 54786)
                let counter := peekUser(cpu, callerAddr, 54787)
                
                mstore(0x00, p)
                mstore(0x20, inf)
                mstore(0x40, busy)
                mstore(0x60, counter)
                return(0x00, 128)
            }

            // ----------------------------------------------------------------
            // Method: writePhoneme(uint256 phoneme, uint256 inflection) -> uint256
            // Selector: 0xe435320e
            // ----------------------------------------------------------------
            if eq(selector, 0xe435320e) {
                let cpu := getCpuAddress()
                let callerAddr := caller()
                let phoneme := calldataload(4)
                let inflection := calldataload(36)
                
                pokeUser(cpu, callerAddr, 54785, inflection)
                pokeUser(cpu, callerAddr, 54784, phoneme)
                
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method: clearBusy() -> uint256
            // Selector: 0x761abbb0
            // ----------------------------------------------------------------
            if eq(selector, 0x761abbb0) {
                let cpu := getCpuAddress()
                let callerAddr := caller()
                pokeUser(cpu, callerAddr, 54786, 0)
                
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method: getTMS5220State() -> (uint256, uint256, uint256)
            // Selector: 0xada6ddc2
            // ----------------------------------------------------------------
            if eq(selector, 0xada6ddc2) {
                let cpu := getCpuAddress()
                let callerAddr := caller()
                let cmd := peekUser(cpu, callerAddr, 54792)
                let status := peekUser(cpu, callerAddr, 54794)
                let fifoLen := peekUser(cpu, callerAddr, 54795)
                
                mstore(0x00, cmd)
                mstore(0x20, status)
                mstore(0x40, fifoLen)
                return(0x00, 96)
            }

            // ----------------------------------------------------------------
            // Method: writeTMS5220Command(uint256 command) -> uint256
            // Selector: 0x285a5991
            // ----------------------------------------------------------------
            if eq(selector, 0x285a5991) {
                let cpu := getCpuAddress()
                let callerAddr := caller()
                let command := calldataload(4)
                pokeUser(cpu, callerAddr, 54792, command)
                
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method: writeTMS5220Data(uint256 data) -> uint256
            // Selector: 0x5dcd4af2
            // ----------------------------------------------------------------
            if eq(selector, 0x5dcd4af2) {
                let cpu := getCpuAddress()
                let callerAddr := caller()
                let data := calldataload(4)
                pokeUser(cpu, callerAddr, 54793, data)
                
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method: bindMusicMaker(address musicMaker) -> bool
            // Selector: 0x665d2526
            // ----------------------------------------------------------------
            if eq(selector, 0x665d2526) {
                let musicMaker := and(calldataload(4), 0xffffffffffffffffffffffffffffffffffffffff)
                sstore(1, musicMaker)
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method: writeReflectionCoefficients(uint256[9] coefficients, uint256 pitch, uint256 energy) -> bool
            // Selector: 0xbd7856a3
            // ----------------------------------------------------------------
            if eq(selector, 0xbd7856a3) {
                let cpu := getCpuAddress()
                let callerAddr := caller()
                
                // Poke 9 reflection coefficients to addresses 54800 to 54808
                for { let i := 0 } lt(i, 9) { i := add(i, 1) } {
                    let coef := calldataload(add(4, mul(i, 32)))
                    pokeUser(cpu, callerAddr, add(54800, i), coef)
                }

                let pitch := calldataload(292)
                let energy := calldataload(324)

                pokeUser(cpu, callerAddr, 54809, pitch)
                pokeUser(cpu, callerAddr, 54810, energy)

                // If musicMaker address is set, poke frequency registers to voice 1
                let musicMaker := sload(1)
                if iszero(iszero(musicMaker)) {
                    let freq := mul(pitch, 10)
                    let lo := and(freq, 0xff)
                    let hi := and(shr(8, freq), 0xff)
                    
                    // Poke Voice 1 Freq Low
                    mstore(0x1100, shl(224, 0x86bb605e))
                    mstore(0x1104, 54272)
                    mstore(0x1124, lo)
                    let dummy := call(gas(), musicMaker, 0, 0x1100, 68, 0, 0)
                    if iszero(dummy) {
                        returndatacopy(0, 0, returndatasize())
                        revert(0, returndatasize())
                    }
                    
                    // Poke Voice 1 Freq High
                    mstore(0x1100, shl(224, 0x86bb605e))
                    mstore(0x1104, 54273)
                    mstore(0x1124, hi)
                    dummy := call(gas(), musicMaker, 0, 0x1100, 68, 0, 0)
                    if iszero(dummy) {
                        returndatacopy(0, 0, returndatasize())
                        revert(0, returndatasize())
                    }
                    
                    // Poke Voice 1 Control (Gate + Sawtooth = 17)
                    mstore(0x1100, shl(224, 0x86bb605e))
                    mstore(0x1104, 54276)
                    mstore(0x1124, 17)
                    dummy := call(gas(), musicMaker, 0, 0x1100, 68, 0, 0)
                    if iszero(dummy) {
                        returndatacopy(0, 0, returndatasize())
                        revert(0, returndatasize())
                    }
                }

                mstore(0x00, 1)
                return(0x00, 32)
            }

            revert(0, 0)
        }
    }
}
