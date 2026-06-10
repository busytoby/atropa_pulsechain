object "ZMachine" {
    code {
        sstore(0, caller())
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    
    object "runtime" {
        code {
            // Dispatch logic:
            // 1. uploadRomChunk(uint256 offset, bytes data) -> 0xf5a83724
            // 2. decodeZString(uint256 offset) -> 0x15664938
            // 3. getObjectProperty(uint256 objId, uint256 propId, address player) -> 0xffb51b58
            // 4. executeTokenReward(address token, address player, uint256 amount) -> 0x50e819a9
            // 5. executeTokenPayment(address token, address player, uint256 amount) -> 0x1facf0c7
            // 6. parseCommand(address player, bytes cmd) -> 0xf1ba03f9
            // 7. triggerZ6Sound(address musicMaker, uint256 note, uint256 voice) -> 0xee1db5af
            // 8. decryptInvisiclue(address keySystem, address player, uint256 hintId) -> 0xe68d22f6
            
            let selector := shr(224, calldataload(0))

            switch selector
            
            case 0xf5a83724 {
                let offset := calldataload(4)
                let length := calldataload(36)
                let storageWordIndex := div(offset, 32)
                let calldataPtr := 68
                
                for { let i := 0 } lt(i, length) { i := add(i, 32) } {
                    let word := calldataload(add(calldataPtr, i))
                    sstore(add(1000000, add(storageWordIndex, div(i, 32))), word)
                }
                mstore(0, 1)
                return(0, 32)
            }

            case 0x15664938 {
                let romOffset := calldataload(4)
                let outPtr := 0x40
                let alphabet := 0
                
                for { let wordPtr := romOffset } 1 { wordPtr := add(wordPtr, 2) } {
                    let val := readRom16(wordPtr)
                    let c1 := and(shr(10, val), 0x1f)
                    let c2 := and(shr(5, val), 0x1f)
                    let c3 := and(val, 0x1f)
                    
                    outPtr := decodeChar(c1, alphabet, outPtr)
                    outPtr := decodeChar(c2, alphabet, outPtr)
                    outPtr := decodeChar(c3, alphabet, outPtr)
                    
                    if and(val, 0x8000) { break }
                }
                
                let strLen := sub(outPtr, 0x40)
                mstore(0x00, 0x20)
                mstore(0x20, strLen)
                let totalBytes := add(64, mul(div(add(strLen, 31), 32), 32))
                return(0, totalBytes)
            }

            case 0xb23e800d { // bindRoomDna(uint256 roomId, bytes calldata dna)
                let roomId := calldataload(4)
                let dnaOffset := calldataload(36)
                let dnaLen := calldataload(add(4, dnaOffset))
                sstore(add(3000000, roomId), dnaLen)
                
                let wordsCount := div(add(dnaLen, 31), 32)
                let calldataPtr := add(36, dnaOffset)
                for { let i := 0 } lt(i, wordsCount) { i := add(i, 1) } {
                    let word := calldataload(add(calldataPtr, mul(i, 32)))
                    sstore(add(3100000, add(mul(roomId, 100), i)), word)
                }
                mstore(0, 1)
                return(0, 32)
            }

            case 0xc26c6d05 { // bindFolkloreAddress(address)
                sstore(2500000, calldataload(4))
                mstore(0, 1)
                return(0, 32)
            }

            case 0x2e0bc27a { // getRoomDna(uint256 roomId) -> (bytes memory)
                let roomId := calldataload(4)
                let dnaLen := sload(add(3000000, roomId))
                mstore(0, 32) // offset to bytes array
                mstore(32, dnaLen)
                
                let wordsCount := div(add(dnaLen, 31), 32)
                let destPtr := 64
                for { let i := 0 } lt(i, wordsCount) { i := add(i, 1) } {
                    let word := sload(add(3100000, add(mul(roomId, 100), i)))
                    mstore(add(destPtr, mul(i, 32)), word)
                }
                let totalReturnSize := add(64, mul(wordsCount, 32))
                return(0, totalReturnSize)
            }

            case 0xd4115e5b { // crossoverRoomDna(uint256 parentA, uint256 parentB, uint256 child, uint256 seed)
                let parentA := calldataload(4)
                let parentB := calldataload(36)
                let child := calldataload(68)
                let seed := calldataload(100)
                
                let lenA := sload(add(3000000, parentA))
                let lenB := sload(add(3000000, parentB))
                let childLen := lenA
                if lt(lenB, childLen) { childLen := lenB } // Use the minimum length
                
                sstore(add(3000000, child), childLen)
                let wordsCount := div(add(childLen, 31), 32)
                
                let state := seed
                for { let i := 0 } lt(i, wordsCount) { i := add(i, 1) } {
                    // Simple LCG step to generate a pseudo-random mask
                    state := add(mul(state, 1664525), 1013904223)
                    mstore(0, state)
                    let mask := keccak256(0, 32)
                    
                    let wordA := sload(add(3100000, add(mul(parentA, 100), i)))
                    let wordB := sload(add(3100000, add(mul(parentB, 100), i)))
                    
                    // Combine bytes using mask
                    let combined := or(and(wordA, mask), and(wordB, not(mask)))
                    sstore(add(3100000, add(mul(child, 100), i)), combined)
                }
                mstore(0, 1)
                return(0, 32)
            }

            case 0xffb51b58 {
                let objId := calldataload(4)
                let propId := calldataload(36)
                let player := calldataload(68)

                if eq(propId, 31) {
                    let tokenAddr := sload(add(2000000, objId))
                    if tokenAddr {
                        let bal := erc20BalanceOf(tokenAddr, player)
                        mstore(0x00, bal)
                        return(0x00, 32)
                    }
                }

                // Gauntlet Stats Bridge: Object ID 80
                if eq(objId, 80) {
                    let folkloreAddr := sload(2500000)
                    if folkloreAddr {
                        let targetAddr := 0
                        switch propId
                        case 32 { targetAddr := 55053 } // gauntletHealth
                        case 33 { targetAddr := 55054 } // gauntletKeys
                        case 34 { targetAddr := 55051 } // gauntletPlayerX
                        case 35 { targetAddr := 55052 } // gauntletPlayerY
                        case 36 { targetAddr := 55050 } // isGauntletActive
                        
                        if targetAddr {
                            mstore(0x00, shl(224, 0xc2e22c95)) // peekUser(address,uint256)
                            mstore(0x04, player)
                            mstore(0x24, targetAddr)
                            let success := staticcall(gas(), folkloreAddr, 0x00, 68, 0x00, 32)
                            if success {
                                return(0x00, 32)
                            }
                        }
                    }
                }

                let val := readObjectProperty(objId, propId)
                mstore(0x00, val)
                return(0x00, 32)
            }

            case 0x50e819a9 {
                let token := calldataload(4)
                let player := calldataload(36)
                let amount := calldataload(68)
                
                let success := erc20Transfer(token, player, amount)
                mstore(0, success)
                return(0, 32)
            }

            case 0x1facf0c7 {
                let token := calldataload(4)
                let player := calldataload(36)
                let amount := calldataload(68)
                
                let success := erc20TransferFrom(token, player, address(), amount)
                mstore(0, success)
                return(0, 32)
            }

            case 0xb4c9ac9b { // createRoom(uint256,bytes,uint256)
                let roomId := calldataload(4)
                let descOffset := calldataload(36)
                let exits := calldataload(68)
                let descLen := calldataload(add(4, descOffset))
                
                sstore(add(3200000, roomId), exits)
                sstore(add(3000000, roomId), descLen)
                
                let wordCount := div(add(descLen, 31), 32)
                for { let i := 0 } lt(i, wordCount) { i := add(i, 1) } {
                    let word := calldataload(add(add(4, descOffset), add(32, mul(i, 32))))
                    sstore(add(add(3100000, mul(roomId, 100)), i), word)
                }
                
                mstore(0x00, 1)
                return(0x00, 32)
            }

            case 0x51150e21 { // bindTokenAddress(uint256,address)
                let objId := calldataload(4)
                let tokenAddress := calldataload(36)
                sstore(add(2000000, objId), tokenAddress)
                mstore(0x00, 1)
                return(0x00, 32)
            }

            case 0xf1ba03f9 {
                let player := calldataload(4)
                let cmdLen := calldataload(68)
                let firstWord := shr(224, calldataload(100))
                
                let roomId := sload(add(4000000, player))
                if iszero(roomId) { roomId := 10 }

                // Initialize item room placements if not already set up
                if iszero(sload(999999)) {
                    sstore(999999, 1)
                    sstore(add(2000300, 50), 1) // Item 50: Gold Token (starts in Room 1 / lobby)
                    sstore(add(2000300, 51), 2) // Item 51: Keycard (starts in Room 2)
                    sstore(add(2000300, 52), 3) // Item 52: Energy Pack (starts in Room 3)
                }

                let resultPtr := 0x40
                
                switch firstWord
                case 0x54494d00 { // "TIM"
                    mstore(resultPtr, 0x2a2054494d204d4f4e49544f5220414354495645202a0a000000000000000000) // "* TIM MONITOR ACTIVE *\n"
                    resultPtr := add(resultPtr, 23)
                }
                case 0x54494d20 { // "TIM "
                    mstore(resultPtr, 0x2a2054494d204d4f4e49544f5220414354495645202a0a000000000000000000) // "* TIM MONITOR ACTIVE *\n"
                    resultPtr := add(resultPtr, 23)
                }
                case 0x706c6179 { // "play"
                    let argWord := calldataload(105) // string starting at char index 5
                    let timbre := 0 // 0 = chime (default), 1 = strings, 2 = brass, 3 = organ
                    if eq(shr(200, argWord), 0x737472696e6773) { timbre := 1 } // "strings"
                    if eq(shr(216, argWord), 0x6272617373) { timbre := 2 } // "brass"
                    if eq(shr(216, argWord), 0x6f7267616e) { timbre := 3 } // "organ"

                    let isCustom := 0
                    if and(eq(timbre, 0), gt(cmdLen, 5)) {
                        if iszero(eq(shr(216, argWord), 0x6368696d65)) { // not "chime"
                            isCustom := 1
                        }
                    }

                    if isCustom {
                        let argLen := sub(cmdLen, 5)
                        mstore(resultPtr, 0x2a204150504c41594552204d5553494320494e545f4c495354454e4552202a0a) // "* APPLAYER MUSIC INT_LISTENER *\n"
                        resultPtr := add(resultPtr, 32)
                        
                        mstore(resultPtr, 0x5b504c41595f4e4f5445533a0000000000000000000000000000000000000000) // "[PLAY_NOTES:"
                        resultPtr := add(resultPtr, 12)
                        
                        for { let i := 0 } lt(i, argLen) { i := add(i, 1) } {
                            let char := and(shr(248, calldataload(add(105, i))), 0xff)
                            mstore8(resultPtr, char)
                            resultPtr := add(resultPtr, 1)
                        }
                        
                        mstore(resultPtr, 0x5d0a000000000000000000000000000000000000000000000000000000000000) // "]\n"
                        resultPtr := add(resultPtr, 2)
                    }
                    if iszero(isCustom) {
                        let dna0 := sload(add(3100000, add(mul(roomId, 100), 0)))
                        let f1 := add(150, mul(and(dna0, 0xff), 3))
                        let f2 := add(150, mul(and(shr(8, dna0), 0xff), 3))
                        let f3 := add(150, mul(and(shr(16, dna0), 0xff), 3))
                        let f4 := add(150, mul(and(shr(24, dna0), 0xff), 3))

                        mstore(resultPtr, 0x2a204b494d2d31204441432053594e5448202a0a506c6179696e6720444e4120) // "* KIM-1 DAC SYNTH *\nPlaying DNA "
                        resultPtr := add(resultPtr, 29)

                        mstore(resultPtr, 0x5b504c41593a0000000000000000000000000000000000000000000000000000) // "[PLAY:"
                        resultPtr := add(resultPtr, 6)
                        resultPtr := writeDec16(f1, resultPtr)
                        mstore8(resultPtr, 44) // ','
                        resultPtr := add(resultPtr, 1)
                        resultPtr := writeDec16(f2, resultPtr)
                        mstore8(resultPtr, 44) // ','
                        resultPtr := add(resultPtr, 1)
                        resultPtr := writeDec16(f3, resultPtr)
                        mstore8(resultPtr, 44) // ','
                        resultPtr := add(resultPtr, 1)
                        resultPtr := writeDec16(f4, resultPtr)
                        mstore8(resultPtr, 44) // ','
                        resultPtr := add(resultPtr, 1)
                        mstore8(resultPtr, add(timbre, 48)) // '0' + timbre
                        resultPtr := add(resultPtr, 1)
                        mstore(resultPtr, 0x5d0a000000000000000000000000000000000000000000000000000000000000) // "]\n"
                        resultPtr := add(resultPtr, 2)
                    }
                }
                case 0x6861726d { // "harm" (harmonic)
                    let matchesHarmonic := 0
                    if eq(cmdLen, 8) {
                        let nextWord := shr(224, calldataload(104))
                        if eq(nextWord, 0x6f6e6963) { // "onic"
                            matchesHarmonic := 1
                        }
                    }
                    if matchesHarmonic {
                        let dna0 := sload(add(3100000, add(mul(roomId, 100), 0)))
                        let dna1 := sload(add(3100000, add(mul(roomId, 100), 1)))
                        let sum := 0
                        sum := add(sum, and(dna0, 0xff))
                        sum := add(sum, and(shr(8, dna0), 0xff))
                        sum := add(sum, and(shr(16, dna0), 0xff))
                        sum := add(sum, and(shr(24, dna0), 0xff))
                        sum := add(sum, and(dna1, 0xff))
                        sum := add(sum, and(shr(8, dna1), 0xff))
                        sum := add(sum, and(shr(16, dna1), 0xff))
                        sum := add(sum, and(shr(24, dna1), 0xff))
                        
                        let noteIdx := mod(sum, 12)
                        let chordType := mod(and(shr(32, dna0), 0xff), 3)
                        
                        let freq := 130
                        if eq(noteIdx, 0) { freq := 130 }
                        if eq(noteIdx, 1) { freq := 138 }
                        if eq(noteIdx, 2) { freq := 146 }
                        if eq(noteIdx, 3) { freq := 155 }
                        if eq(noteIdx, 4) { freq := 164 }
                        if eq(noteIdx, 5) { freq := 174 }
                        if eq(noteIdx, 6) { freq := 185 }
                        if eq(noteIdx, 7) { freq := 196 }
                        if eq(noteIdx, 8) { freq := 207 }
                        if eq(noteIdx, 9) { freq := 220 }
                        if eq(noteIdx, 10) { freq := 233 }
                        if eq(noteIdx, 11) { freq := 246 }

                        mstore(resultPtr, 0x2a20414c4348454d4943414c204841524d4f4e494320414e414c59534953202a) // "* ALCHEMICAL HARMONIC ANALYSIS *\n"
                        resultPtr := add(resultPtr, 32)
                        
                        mstore(resultPtr, 0x5b534f4e49435f5349474e41545552453a000000000000000000000000000000) // "[SONIC_SIGNATURE:"
                        resultPtr := add(resultPtr, 17)
                        
                        resultPtr := writeDec16(freq, resultPtr)
                        mstore8(resultPtr, 44) // ','
                        resultPtr := add(resultPtr, 1)
                        resultPtr := writeDec16(chordType, resultPtr)
                        
                        mstore(resultPtr, 0x5d0a000000000000000000000000000000000000000000000000000000000000) // "]\n"
                        resultPtr := add(resultPtr, 2)
                    }
                    if iszero(matchesHarmonic) {
                        mstore(resultPtr, 0x496e76616c6964206861726d6f6e696320636f6d6d616e642e0a000000000000) // "Invalid harmonic command.\n"
                        resultPtr := add(resultPtr, 26)
                    }
                }
                case 0x74616b65 { // "take"
                    let taken := 0
                    if eq(sload(add(2000300, 50)), roomId) {
                        sstore(add(2000300, 50), 0) // move to inventory (Room 0)
                        mstore(resultPtr, 0x596f7520746f6f6b2074686520476f6c6420546f6b656e2e0000000000000000) // "You took the Gold Token."
                        resultPtr := add(resultPtr, 24)
                        taken := 1
                        
                        let tokenAddr := sload(add(2000000, 50))
                        if tokenAddr {
                            let rewardSuccess := erc20Transfer(tokenAddr, player, 1000000000000000000)
                            if rewardSuccess {
                                mstore(resultPtr, 0x20284552433230207472616e7361637465642129000000000000000000000000) // " (ERC20 transacted!)"
                                resultPtr := add(resultPtr, 20)
                            }
                        }
                    }
                    if iszero(taken) {
                        if eq(sload(add(2000300, 51)), roomId) {
                            sstore(add(2000300, 51), 0)
                            mstore(resultPtr, 0x596f7520746f6f6b20746865204b6579636172642e0000000000000000000000) // "You took the Keycard."
                            resultPtr := add(resultPtr, 21)
                            taken := 1
                        }
                    }
                    if iszero(taken) {
                        if eq(sload(add(2000300, 52)), roomId) {
                            sstore(add(2000300, 52), 0)
                            mstore(resultPtr, 0x596f7520746f6f6b2074686520456e65726779205061636b2e00000000000000) // "You took the Energy Pack."
                            resultPtr := add(resultPtr, 25)
                            taken := 1
                        }
                    }
                    if iszero(taken) {
                        mstore(resultPtr, 0x5468657265206973206e6f7468696e67206865726520746f2074616b652e0000) // "There is nothing here to take."
                        resultPtr := add(resultPtr, 30)
                    }
                }
                case 0x75736520 { // "use "
                    let used := 0
                    if eq(sload(add(2000300, 51)), 0) {
                        if eq(roomId, 3) {
                            sstore(3000805, 1)
                            mstore(resultPtr, 0x596f7520737769706520746865204b6579636172642e2054686520706f727461) // "You swipe the Keycard. The porta"
                            mstore(add(resultPtr, 32), 0x6c2068756d7320746f206c69666521204e6f727468206973206f70656e2e0a00) // "l hums to life! North is open.\n"
                            resultPtr := add(resultPtr, 62)
                            used := 1
                        }
                    }
                    if iszero(used) {
                        mstore(resultPtr, 0x4e6f7468696e672068617070656e732e0a000000000000000000000000000000) // "Nothing happens.\n"
                        resultPtr := add(resultPtr, 17)
                    }
                }
                case 0x75736500 { // "use"
                    mstore(resultPtr, 0x55736520776861743f0a00000000000000000000000000000000000000000000) // "Use what?\n"
                    resultPtr := add(resultPtr, 10)
                }
                case 0x6c6f6f6b { // "look"
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        let tokenAddr := sload(add(2000000, roomId))
                        if tokenAddr {
                            mstore(resultPtr, 0x5b5a4d4d5d20436f6e74726163743a2000000000000000000000000000000000) // "[ZMM] Contract: "
                            resultPtr := add(resultPtr, 16)
                            resultPtr := writeHexAddress(tokenAddr, resultPtr)
                            mstore8(resultPtr, 10)
                            resultPtr := add(resultPtr, 1)

                            mstore(resultPtr, 0x5b5a4d4d5d204e616d653a200000000000000000000000000000000000000000) // "[ZMM] Name: "
                            resultPtr := add(resultPtr, 12)
                            resultPtr := queryContractString(tokenAddr, 0x06fdde0300000000000000000000000000000000000000000000000000000000, resultPtr) // name()
                            mstore8(resultPtr, 10)
                            resultPtr := add(resultPtr, 1)

                            mstore(resultPtr, 0x5b5a4d4d5d2053796d626f6c3a20000000000000000000000000000000000000) // "[ZMM] Symbol: "
                            resultPtr := add(resultPtr, 14)
                            resultPtr := queryContractString(tokenAddr, 0x95d89b4100000000000000000000000000000000000000000000000000000000, resultPtr) // symbol()
                            mstore8(resultPtr, 10)
                            resultPtr := add(resultPtr, 1)

                            let typePtr := 0x1500
                            let typeEnd := queryContractString(tokenAddr, 0xff76204400000000000000000000000000000000000000000000000000000000, typePtr) // Type()
                            let typeWord := mload(typePtr)

                            mstore(resultPtr, 0x5b5a4d4d5d20417661696c61626c6520436f6d6d616e64733a0a202d20747261) // "[ZMM] Available Commands:\n - tra"
                            mstore(add(resultPtr, 32), 0x6e736665720a202d20617070726f76650a202d2062616c616e63654f660a202d) // "nsfer\n - approve\n - balanceOf\n -"
                            mstore(add(resultPtr, 64), 0x20746f74616c537570706c790a00000000000000000000000000000000000000) // " totalSupply\n"
                            resultPtr := add(resultPtr, 77)

                            if eq(shr(224, typeWord), 0x564f4944) { // "VOID"
                                mstore(resultPtr, 0x202d2043686174205b6d73675d0a000000000000000000000000000000000000) // " - Chat [msg]\n"
                                mstore(add(resultPtr, 14), 0x202d204c6f67205b6c696e655d0a000000000000000000000000000000000000) // " - Log [line]\n"
                                mstore(add(resultPtr, 28), 0x202d20536574417474726962757465205b6b2c765d0a00000000000000000000) // " - SetAttribute [k,v]\n"
                                mstore(add(resultPtr, 50), 0x202d20476574417474726962757465205b6b5d0a000000000000000000000000) // " - GetAttribute [k]\n"
                                mstore(add(resultPtr, 70), 0x202d20416c696173205b612c765d0a0000000000000000000000000000000000) // " - Alias [a,v]\n"
                                mstore(add(resultPtr, 85), 0x202d20456e7465720a0000000000000000000000000000000000000000000000) // " - Enter\n"
                                resultPtr := add(resultPtr, 94)
                            }
                            if eq(shr(224, typeWord), 0x51494e47) { // "QING"
                                mstore(resultPtr, 0x202d204a6f696e205b746f6b656e5d0a00000000000000000000000000000000) // " - Join [token]\n"
                                mstore(add(resultPtr, 16), 0x202d2043686174205b6d73675d0a000000000000000000000000000000000000) // " - Chat [msg]\n"
                                mstore(add(resultPtr, 32), 0x205769746864726177205b742c615d0a00000000000000000000000000000000) // " - Withdraw [t,a]\n"
                                mstore(add(resultPtr, 48), 0x202d2041646d6974746564205b745d0a00000000000000000000000000000000) // " - Admitted [t]\n"
                                mstore(add(resultPtr, 64), 0x202d20736574426f756e63657244697669736f72205b645d0a00000000000000) // " - setBouncerDivisor [d]\n"
                                mstore(add(resultPtr, 89), 0x202d20736574436f766572436861726765205b635d0a00000000000000000000) // " - setCoverCharge [c]\n"
                                resultPtr := add(resultPtr, 111)
                            }
                        }
                        if iszero(tokenAddr) {
                            mstore(resultPtr, 0x5b5a4d4d5d204e6f20636f6e747261637420626f756e6420746f207468697320) // "[ZMM] No contract bound to this "
                            mstore(add(resultPtr, 32), 0x726f6f6d2e0a0000000000000000000000000000000000000000000000000000) // "room.\n"
                            resultPtr := add(resultPtr, 38)
                        }

                        let customLen := sload(add(3000000, roomId))
                        if customLen {
                            for { let i := 0 } lt(i, 10) { i := add(i, 1) } {
                                if lt(mul(i, 32), customLen) {
                                    let val := sload(add(add(3100000, mul(roomId, 100)), i))
                                    mstore(add(resultPtr, mul(i, 32)), val)
                                }
                            }
                            resultPtr := add(resultPtr, customLen)
                        }
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        
                        if tokenAddr {
                            mstore(resultPtr, 0x436f6e74726163743a2000000000000000000000000000000000000000000000) // "Contract: "
                            resultPtr := add(resultPtr, 10)
                            resultPtr := writeHexAddress(tokenAddr, resultPtr)
                            
                            mstore8(resultPtr, 10) // '\n'
                            resultPtr := add(resultPtr, 1)
                            
                            mstore(resultPtr, 0x4e616d653a200000000000000000000000000000000000000000000000000000) // "Name: "
                            resultPtr := add(resultPtr, 6)
                            resultPtr := queryContractString(tokenAddr, 0x06fdde0300000000000000000000000000000000000000000000000000000000, resultPtr) // name()
                            
                            mstore8(resultPtr, 10) // '\n'
                            resultPtr := add(resultPtr, 1)
                            
                            mstore(resultPtr, 0x53796d626f6c3a20000000000000000000000000000000000000000000000000) // "Symbol: "
                            resultPtr := add(resultPtr, 8)
                            resultPtr := queryContractString(tokenAddr, 0x95d89b4100000000000000000000000000000000000000000000000000000000, resultPtr) // symbol()
                            
                            mstore8(resultPtr, 10) // '\n'
                            resultPtr := add(resultPtr, 1)
                            
                            let typePtr := 0x1500
                            let typeEnd := queryContractString(tokenAddr, 0xff76204400000000000000000000000000000000000000000000000000000000, typePtr) // Type()
                            let typeWord := mload(typePtr)
                            
                            mstore(resultPtr, 0x417661696c61626c6520436f6d6d616e64733a0a202d207472616e736665720a) // "Available Commands:\n - transfer\n"
                            mstore(add(resultPtr, 32), 0x202d20617070726f76650a202d2062616c616e63654f660a202d20746f7461) // " - approve\n - balanceOf\n - tota"
                            mstore(add(resultPtr, 64), 0x6c537570706c790a000000000000000000000000000000000000000000000000) // "lSupply\n"
                            resultPtr := add(resultPtr, 72)
                            
                            if eq(shr(224, typeWord), 0x564f4944) { // "VOID"
                                mstore(resultPtr, 0x202d2043686174205b6d73675d0a202d204c6f67205b6c696e655d0a202d2053) // " - Chat [msg]\n - Log [line]\n - S"
                                mstore(add(resultPtr, 32), 0x6574417474726962757465205b6b2c765d0a202d204765744174747269627500) // "etAttribute [k,v]\n - GetAttribu"
                                mstore(add(resultPtr, 64), 0x7465205b6b5d0a202d20416c696173205b612c765d0a202d20456e7465720a00) // "te [k]\n - Alias [a,v]\n - Enter\n"
                                resultPtr := add(resultPtr, 95)
                            }
                            
                            if eq(shr(224, typeWord), 0x51494e47) { // "QING"
                                mstore(resultPtr, 0x202d204a6f696e205b746f6b656e5d0a202d2043686174205b6d73675d0a202d) // " - Join [token]\n - Chat [msg]\n -"
                                mstore(add(resultPtr, 32), 0x205769746864726177205b742c615d0a202d2041646d6974746564205b745d00) // " Withdraw [t,a]\n - Admitted [t]"
                                mstore(add(resultPtr, 64), 0x0a202d20736574426f756e63657244697669736f72205b645d0a202d20730000) // "\n - setBouncerDivisor [d]\n - s"
                                mstore(add(resultPtr, 96), 0x6574436f766572436861726765205b635d0a0000000000000000000000000000) // "etCoverCharge [c]\n"
                                resultPtr := add(resultPtr, 113)
                            }
                        }
                        
                        if iszero(tokenAddr) {
                            let customLen := sload(add(3000000, roomId))
                            if customLen {
                                let wordCount := div(add(customLen, 31), 32)
                                for { let i := 0 } lt(i, 10) { i := add(i, 1) } {
                                    if lt(mul(i, 32), customLen) {
                                        let val := sload(add(add(3100000, mul(roomId, 100)), i))
                                        mstore(add(resultPtr, mul(i, 32)), val)
                                    }
                                }
                                resultPtr := add(resultPtr, customLen)
                            }
                            if iszero(customLen) {
                                if iszero(roomId) {
                                    mstore(resultPtr, 0x596f7520617265206f7574736964652074686520566963746f7269616e20486f) // "You are outside the Victorian Ho"
                                    mstore(add(resultPtr, 32), 0x7573652e00000000000000000000000000000000000000000000000000000000) // "use."
                                    resultPtr := add(resultPtr, 36)
                                }
                                if eq(roomId, 1) {
                                    mstore(resultPtr, 0x596f7520617265207374616e64696e6720696e20746865206c6f6262792e0a5b) // "You are standing in the lobby.\n["
                                    mstore(add(resultPtr, 32), 0x534f4e49435f5349474e41545552453a3232302c315d00000000000000000000) // "SONIC_SIGNATURE:220,1]"
                                    resultPtr := add(resultPtr, 54)
                                }
                                if eq(roomId, 2) {
                                    mstore(resultPtr, 0x596f752061726520696e20746865204c6976696e6720526f6f6d2e0a5b534f4e) // "You are in the Living Room.\n[SON"
                                    mstore(add(resultPtr, 32), 0x49435f5349474e41545552453a3333302c305d00000000000000000000000000) // "IC_SIGNATURE:330,0]"
                                    resultPtr := add(resultPtr, 51)
                                }
                                if eq(roomId, 3) {
                                    if sload(3000805) {
                                        mstore(resultPtr, 0x596f752061726520696e2074686520556e7265616c20506f7274616c20566175) // "You are in the Unreal Portal Vau"
                                        mstore(add(resultPtr, 32), 0x6c742e2054686520706f7274616c20676c6f7773206272696768746c79204e6f) // "lt. The portal glows brightly No"
                                        mstore(add(resultPtr, 64), 0x7274682e0a5b534f4e49435f5349474e41545552453a3434302c305d00000000) // "rth.\n[SONIC_SIGNATURE:440,0]"
                                        resultPtr := add(resultPtr, 92)
                                    }
                                    if iszero(sload(3000805)) {
                                        mstore(resultPtr, 0x596f752061726520696e2074686520556e7265616c20506f7274616c20566175) // "You are in the Unreal Portal Vau"
                                        mstore(add(resultPtr, 32), 0x6c742e2041206c6f636b656420706f7274616c20617263682069732068657265) // "lt. A locked portal arch is here"
                                        mstore(add(resultPtr, 64), 0x2e0a5b534f4e49435f5349474e41545552453a3134372c325d00000000000000) // ".\n[SONIC_SIGNATURE:147,2]"
                                        resultPtr := add(resultPtr, 90)
                                    }
                                }
                                if eq(roomId, 4) {
                                    mstore(resultPtr, 0x596f7520617265206f7574736964652074686520566963746f7269616e20486f) // "You are outside the Victorian Ho"
                                    mstore(add(resultPtr, 32), 0x7573652e2041206461726b206d697374207368726f75647320746865206d616e) // "use. A dark mist shrouds the man"
                                    mstore(add(resultPtr, 64), 0x6f722e0a00000000000000000000000000000000000000000000000000000000) // "or.\n"
                                    resultPtr := add(resultPtr, 68)
                                }
                                let knownRoom := or(or(or(iszero(roomId), eq(roomId, 1)), or(eq(roomId, 2), eq(roomId, 3))), eq(roomId, 4))
                                if iszero(knownRoom) {
                                    mstore(resultPtr, 0x596f752061726520696e20616e20656d70747920726f6f6d2e00000000000000) // "You are in an empty room."
                                    resultPtr := add(resultPtr, 25)
                                }
                            }
                        }
                        
                        // Display any items in the current room
                        if eq(sload(add(2000300, 50)), roomId) {
                            mstore(resultPtr, 0x20596f7520736565206120476f6c6420546f6b656e20686572652e0000000000) // " You see a Gold Token here."
                            resultPtr := add(resultPtr, 27)
                        }
                        if eq(sload(add(2000300, 51)), roomId) {
                            mstore(resultPtr, 0x20596f75207365652061204b65796361726420686572652e0000000000000000) // " You see a Keycard here."
                            resultPtr := add(resultPtr, 24)
                        }
                        if eq(sload(add(2000300, 52)), roomId) {
                            mstore(resultPtr, 0x20596f752073656520616e20456e65726779205061636b20686572652e000000) // " You see an Energy Pack here."
                            resultPtr := add(resultPtr, 28)
                        }
                    }
                }
                case 0x43686174 { // "Chat" (Void and Qing Chat)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        let success := 1
                        if eq(roomId, 11) {
                            success := isGuestAdmitted(11, player)
                        }
                        if success {
                            mstore(resultPtr, 0x5b5a4d4d5d2043686174207375636365737366756c2e00000000000000000000) // "[ZMM] Chat successful."
                            resultPtr := add(resultPtr, 23)
                        }
                        if iszero(success) {
                            mstore(resultPtr, 0x5b5a4d4d5d2043686174206661696c65642f72657665727465642e0000000000) // "[ZMM] Chat failed/reverted."
                            resultPtr := add(resultPtr, 28)
                        }
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if iszero(tokenAddr) {
                            mstore(resultPtr, 0x4e6f20636f6e747261637420626f756e6420746f207468697320726f6f6d2e00) // "No contract bound to this room."
                            resultPtr := add(resultPtr, 31)
                        }
                        if tokenAddr {
                            let typePtr := 0x1500
                            let typeEnd := queryContractString(tokenAddr, 0xfc0c546a00000000000000000000000000000000000000000000000000000000, typePtr)
                            let typeWord := mload(typePtr)
                            let isQing := eq(shr(224, typeWord), 0x51494e47) // "QING"
                            
                            let success := 0
                            if iszero(isQing) {
                                let argLen := sub(cmdLen, 5)
                                mstore(0x1000, 0x21516fc400000000000000000000000000000000000000000000000000000000)
                                mstore(0x1004, 0x20)
                                mstore(0x1024, argLen)
                                calldatacopy(0x1044, add(100, 5), argLen)
                                let paddedSize := mul(div(add(argLen, 31), 32), 32)
                                let totalCalldataSize := add(68, paddedSize)
                                success := call(gas(), tokenAddr, 0, 0x1000, totalCalldataSize, 0x2000, 0x100)
                            }
                            if isQing {
                                mstore(0x1000, 0x6185ca8800000000000000000000000000000000000000000000000000000000)
                                let choSuccess := staticcall(gas(), tokenAddr, 0x1000, 0x04, 0x1000, 0x20)
                                if choSuccess {
                                    let choAddr := mload(0x1000)
                                    mstore(0x1000, 0xe80c28bf00000000000000000000000000000000000000000000000000000000)
                                    mstore(0x1004, player)
                                    let userTokenSuccess := staticcall(gas(), choAddr, 0x1000, 0x24, 0x1000, 0x20)
                                    if userTokenSuccess {
                                        let userToken := mload(0x1000)
                                        let argLen := sub(cmdLen, 5)
                                        mstore(0x1000, 0xb340a7800000000000000000000000000000000000000000000000000000000)
                                        mstore(0x1004, userToken)
                                        mstore(0x1024, 0x40)
                                        mstore(0x1044, argLen)
                                        calldatacopy(0x1064, add(100, 5), argLen)
                                        let paddedSize := mul(div(add(argLen, 31), 32), 32)
                                        let totalCalldataSize := add(100, paddedSize)
                                        success := call(gas(), tokenAddr, 0, 0x1000, totalCalldataSize, 0x2000, 0x100)
                                    }
                                }
                            }
                            
                            if success {
                                mstore(resultPtr, 0x43686174207375636365737366756c2e00000000000000000000000000000000) // "Chat successful."
                                resultPtr := add(resultPtr, 17)
                            }
                            if iszero(success) {
                                mstore(resultPtr, 0x43686174206661696c65642f72657665727465642e0000000000000000000000) // "Chat failed/reverted."
                                resultPtr := add(resultPtr, 22)
                            }
                        }
                    }
                }
                case 0x4c6f6720 { // "Log " (Void logging)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        mstore(resultPtr, 0x5b5a4d4d5d204c6f67207375636365737366756c2e0000000000000000000000) // "[ZMM] Log successful."
                        resultPtr := add(resultPtr, 22)
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if iszero(tokenAddr) {
                            mstore(resultPtr, 0x4e6f20636f6e747261637420626f756e6420746f207468697320726f6f6d2e00)
                            resultPtr := add(resultPtr, 31)
                        }
                        if tokenAddr {
                            let argLen := sub(cmdLen, 4)
                            mstore(0x1000, 0xcf34ef5300000000000000000000000000000000000000000000000000000000)
                            mstore(0x1004, 0x20)
                            mstore(0x1024, argLen)
                            calldatacopy(0x1044, add(100, 4), argLen)
                            let paddedSize := mul(div(add(argLen, 31), 32), 32)
                            let totalCalldataSize := add(68, paddedSize)
                            let success := call(gas(), tokenAddr, 0, 0x1000, totalCalldataSize, 0x2000, 0x100)
                            if success {
                                mstore(resultPtr, 0x4c6f67207375636365737366756c2e0000000000000000000000000000000000) // "Log successful."
                                resultPtr := add(resultPtr, 16)
                            }
                            if iszero(success) {
                                mstore(resultPtr, 0x4c6f67206661696c65642f72657665727465642e000000000000000000000000) // "Log failed/reverted."
                                resultPtr := add(resultPtr, 21)
                            }
                        }
                    }
                }
                case 0x53657441 { // "SetAttribute" (Void attribute setting)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        let spacePos := findSpace(13, cmdLen)
                        let keyLen := sub(spacePos, 13)
                        let valLen := 0
                        if lt(spacePos, cmdLen) {
                            valLen := sub(cmdLen, add(spacePos, 1))
                        }
                        calldatacopy(0x1064, add(100, 13), keyLen)
                        let keyHash := keccak256(0x1064, keyLen)
                        
                        let valOffset := add(0x1064, mul(div(add(keyLen, 31), 32), 32))
                        calldatacopy(valOffset, add(100, add(spacePos, 1)), valLen)
                        
                        setZmmAttribute(roomId, keyHash, valOffset, valLen)
                        
                        mstore(resultPtr, 0x5b5a4d4d5d20417474726962757465207365742e000000000000000000000000) // "[ZMM] Attribute set."
                        resultPtr := add(resultPtr, 20)
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if iszero(tokenAddr) {
                            mstore(resultPtr, 0x4e6f20636f6e747261637420626f756e6420746f207468697320726f6f6d2e00)
                            resultPtr := add(resultPtr, 31)
                        }
                        if tokenAddr {
                            let spacePos := findSpace(13, cmdLen)
                            let keyLen := sub(spacePos, 13)
                            let valLen := 0
                            if lt(spacePos, cmdLen) {
                                valLen := sub(cmdLen, add(spacePos, 1))
                            }
                            
                            mstore(0x1000, 0xdf5cb7b400000000000000000000000000000000000000000000000000000000)
                            mstore(0x1004, 0x40)
                            let paddedKeyLen := mul(div(add(keyLen, 31), 32), 32)
                            mstore(0x1024, add(0x40, add(32, paddedKeyLen)))
                            mstore(0x1044, keyLen)
                            calldatacopy(0x1064, add(100, 13), keyLen)
                            
                            let valOffset := add(0x1064, paddedKeyLen)
                            mstore(valOffset, valLen)
                            calldatacopy(add(valOffset, 32), add(100, add(spacePos, 1)), valLen)
                            
                            let paddedValLen := mul(div(add(valLen, 31), 32), 32)
                            let totalCalldataSize := add(sub(add(valOffset, 32), 0x1000), paddedValLen)
                            
                            let success := call(gas(), tokenAddr, 0, 0x1000, totalCalldataSize, 0x2000, 0x100)
                            if success {
                                mstore(resultPtr, 0x417474726962757465207365742e000000000000000000000000000000000000) // "Attribute set."
                                resultPtr := add(resultPtr, 14)
                            }
                            if iszero(success) {
                                mstore(resultPtr, 0x536574417474726962757465206661696c65642e000000000000000000000000) // "SetAttribute failed."
                                resultPtr := add(resultPtr, 20)
                            }
                        }
                    }
                }
                case 0x47657441 { // "GetAttribute" (Void attribute reading)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        let keyLen := sub(cmdLen, 13)
                        calldatacopy(0x1064, add(100, 13), keyLen)
                        let keyHash := keccak256(0x1064, keyLen)
                        
                        mstore(resultPtr, 0x5b5a4d4d5d200000000000000000000000000000000000000000000000000000) // "[ZMM] "
                        let prefixLen := 6
                        let valLen := getZmmAttribute(roomId, keyHash, add(resultPtr, prefixLen))
                        if valLen {
                            resultPtr := add(resultPtr, add(prefixLen, valLen))
                        }
                        if iszero(valLen) {
                            mstore(resultPtr, 0x5b5a4d4d5d20476574417474726962757465206661696c65642e000000000000) // "[ZMM] GetAttribute failed."
                            resultPtr := add(resultPtr, 26)
                        }
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if iszero(tokenAddr) {
                            mstore(resultPtr, 0x4e6f20636f6e747261637420626f756e6420746f207468697320726f6f6d2e00)
                            resultPtr := add(resultPtr, 31)
                        }
                        if tokenAddr {
                            let keyLen := sub(cmdLen, 13)
                            mstore(0x1000, 0xb502e56700000000000000000000000000000000000000000000000000000000)
                            mstore(0x1004, 0x20)
                            mstore(0x1024, keyLen)
                            calldatacopy(0x1044, add(100, 13), keyLen)
                            let paddedSize := mul(div(add(keyLen, 31), 32), 32)
                            let totalCalldataSize := add(68, paddedSize)
                            let success := staticcall(gas(), tokenAddr, 0x1000, totalCalldataSize, 0x2000, 0x200)
                            if success {
                                let retOffset := mload(0x2000)
                                let retLen := mload(add(0x2000, retOffset))
                                mCopy(add(0x2020, retOffset), resultPtr, retLen)
                                resultPtr := add(resultPtr, retLen)
                            }
                            if iszero(success) {
                                mstore(resultPtr, 0x476574417474726962757465206661696c65642e000000000000000000000000) // "GetAttribute failed."
                                resultPtr := add(resultPtr, 20)
                            }
                        }
                    }
                }
                case 0x416c6961 { // "Alias" (Void alias setting)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        let spacePos := findSpace(6, cmdLen)
                        let targetAddr := parseHexAddress(6, spacePos)
                        
                        if eq(spacePos, cmdLen) {
                            mstore(resultPtr, 0x5b5a4d4d5d200000000000000000000000000000000000000000000000000000) // "[ZMM] "
                            let prefixLen := 6
                            let valLen := getZmmAttribute(roomId, targetAddr, add(resultPtr, prefixLen))
                            if valLen {
                                resultPtr := add(resultPtr, add(prefixLen, valLen))
                            }
                            if iszero(valLen) {
                                mstore(resultPtr, 0x5b5a4d4d5d20416c696173206661696c65642e00000000000000000000000000) // "[ZMM] Alias failed."
                                resultPtr := add(resultPtr, 19)
                            }
                        }
                        if lt(spacePos, cmdLen) {
                            let valLen := sub(cmdLen, add(spacePos, 1))
                            let valOffset := 0x1064
                            calldatacopy(valOffset, add(100, add(spacePos, 1)), valLen)
                            setZmmAttribute(roomId, targetAddr, valOffset, valLen)
                            mstore(resultPtr, 0x5b5a4d4d5d20416c696173207365742e00000000000000000000000000000000) // "[ZMM] Alias set."
                            resultPtr := add(resultPtr, 16)
                        }
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if iszero(tokenAddr) {
                            mstore(resultPtr, 0x4e6f20636f6e747261637420626f756e6420746f207468697320726f6f6d2e00)
                            resultPtr := add(resultPtr, 31)
                        }
                        if tokenAddr {
                            let spacePos := findSpace(6, cmdLen)
                            let targetAddr := parseHexAddress(6, spacePos)
                            
                            let success := 0
                            if eq(spacePos, cmdLen) {
                                mstore(0x1000, 0xed70e46c00000000000000000000000000000000000000000000000000000000)
                                mstore(0x1004, targetAddr)
                                success := staticcall(gas(), tokenAddr, 0x1000, 0x24, 0x2000, 0x200)
                                if success {
                                    let retOffset := mload(0x2000)
                                    let retLen := mload(add(0x2000, retOffset))
                                    mCopy(add(0x2020, retOffset), resultPtr, retLen)
                                    resultPtr := add(resultPtr, retLen)
                                }
                            }
                            if lt(spacePos, cmdLen) {
                                let valLen := sub(cmdLen, add(spacePos, 1))
                                mstore(0x1000, 0x00b660ef00000000000000000000000000000000000000000000000000000000)
                                mstore(0x1004, targetAddr)
                                mstore(0x1024, 0x40)
                                mstore(0x1044, valLen)
                                calldatacopy(0x1064, add(100, add(spacePos, 1)), valLen)
                                let paddedValLen := mul(div(add(valLen, 31), 32), 32)
                                let totalCalldataSize := add(100, paddedValLen)
                                success := call(gas(), tokenAddr, 0, 0x1000, totalCalldataSize, 0x2000, 0x100)
                                if success {
                                    mstore(resultPtr, 0x416c696173207365742e00000000000000000000000000000000000000000000) // "Alias set."
                                    resultPtr := add(resultPtr, 10)
                                }
                            }
                            
                            if iszero(success) {
                                mstore(resultPtr, 0x416c696173206661696c65642e00000000000000000000000000000000000000) // "Alias failed."
                                resultPtr := add(resultPtr, 13)
                            }
                        }
                    }
                }
                case 0x456e7465 { // "Enter" (Void enter)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        mstore(resultPtr, 0x5b5a4d4d5d20456e7465726564207375636365737366656c6c792e0000000000) // "[ZMM] Entered successfully."
                        resultPtr := add(resultPtr, 27)
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if iszero(tokenAddr) {
                            mstore(resultPtr, 0x4e6f20636f6e747261637420626f756e6420746f207468697320726f6f6d2e00)
                            resultPtr := add(resultPtr, 31)
                        }
                        if tokenAddr {
                            let hasArgs := 0
                            if gt(cmdLen, 5) {
                                let char5 := and(shr(248, calldataload(add(100, 5))), 0xff)
                                if eq(char5, 32) { hasArgs := 1 }
                            }
                            
                            let success := 0
                            if iszero(hasArgs) {
                                mstore(0x1000, 0x1097e57900000000000000000000000000000000000000000000000000000000)
                                success := call(gas(), tokenAddr, 0, 0x1000, 0x04, 0x2000, 0x100)
                            }
                            if hasArgs {
                                let spacePos := findSpace(6, cmdLen)
                                let nameLen := sub(spacePos, 6)
                                let symLen := 0
                                if lt(spacePos, cmdLen) {
                                    symLen := sub(cmdLen, add(spacePos, 1))
                                }
                                mstore(0x1000, 0x68f9c78700000000000000000000000000000000000000000000000000000000)
                                mstore(0x1004, 0x40)
                                let paddedNameLen := mul(div(add(nameLen, 31), 32), 32)
                                mstore(0x1024, add(0x40, add(32, paddedNameLen)))
                                mstore(0x1044, nameLen)
                                calldatacopy(0x1064, add(100, 6), nameLen)
                                let symOffset := add(0x1064, paddedNameLen)
                                mstore(symOffset, symLen)
                                calldatacopy(add(symOffset, 32), add(100, add(spacePos, 1)), symLen)
                                let paddedSymLen := mul(div(add(symLen, 31), 32), 32)
                                let totalCalldataSize := add(sub(add(symOffset, 32), 0x1000), paddedSymLen)
                                success := call(gas(), tokenAddr, 0, 0x1000, totalCalldataSize, 0x2000, 0x100)
                            }
                            
                            if success {
                                mstore(resultPtr, 0x456e7465726564207375636365737366656c6c792e0000000000000000000000) // "Entered successfully."
                                resultPtr := add(resultPtr, 21)
                            }
                            if iszero(success) {
                                mstore(resultPtr, 0x456e746572206661696c65642e00000000000000000000000000000000000000) // "Enter failed."
                                resultPtr := add(resultPtr, 13)
                            }
                        }
                    }
                }
                case 0x4a6f696e { // "Join" (Qing join)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        let targetAddr := parseHexAddress(5, cmdLen)
                        addGuest(roomId, targetAddr)
                        mstore(resultPtr, 0x5b5a4d4d5d204a6f696e6564207375636365737366756c6c792e000000000000) // "[ZMM] Joined successfully."
                        resultPtr := add(resultPtr, 26)
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if iszero(tokenAddr) {
                            mstore(resultPtr, 0x4e6f20636f6e747261637420626f756e6420746f207468697320726f6f6d2e00)
                            resultPtr := add(resultPtr, 31)
                        }
                        if tokenAddr {
                            let targetAddr := parseHexAddress(5, cmdLen)
                            mstore(0x1000, 0x0764c80c00000000000000000000000000000000000000000000000000000000)
                            mstore(0x1004, targetAddr)
                            let success := call(gas(), tokenAddr, 0, 0x1000, 0x24, 0x2000, 0x100)
                            if success {
                                mstore(resultPtr, 0x4a6f696e6564207375636365737366756c6c792e000000000000000000000000) // "Joined successfully."
                                resultPtr := add(resultPtr, 20)
                            }
                            if iszero(success) {
                                mstore(resultPtr, 0x4a6f696e206661696c65642e0000000000000000000000000000000000000000) // "Join failed."
                                resultPtr := add(resultPtr, 12)
                            }
                        }
                    }
                }
                case 0x57697468 { // "Withdraw" (Qing withdraw)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        mstore(resultPtr, 0x5b5a4d4d5d205769746864726177616c207375636365737366756c2e00000000) // "[ZMM] Withdrawal successful."
                        resultPtr := add(resultPtr, 28)
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if iszero(tokenAddr) {
                            mstore(resultPtr, 0x4e6f20636f6e747261637420626f756e6420746f207468697320726f6f6d2e00)
                            resultPtr := add(resultPtr, 31)
                        }
                        if tokenAddr {
                            let spacePos := findSpace(9, cmdLen)
                            let targetAddr := parseHexAddress(9, spacePos)
                            let amount := 0
                            if lt(spacePos, cmdLen) {
                                amount := parseDec(add(spacePos, 1), cmdLen)
                            }
                            mstore(0x1000, 0x884edad900000000000000000000000000000000000000000000000000000000)
                            mstore(0x1004, targetAddr)
                            mstore(0x1024, amount)
                            let success := call(gas(), tokenAddr, 0, 0x1000, 0x44, 0x2000, 0x100)
                            if success {
                                mstore(resultPtr, 0x5769746864726177616c207375636365737366756c2e00000000000000000000) // "Withdrawal successful."
                                resultPtr := add(resultPtr, 22)
                            }
                            if iszero(success) {
                                mstore(resultPtr, 0x5769746864726177616c206661696c65642e0000000000000000000000000000) // "Withdrawal failed."
                                resultPtr := add(resultPtr, 18)
                            }
                        }
                    }
                }
                case 0x41646d69 { // "Admitted" (Qing admitted check)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        let targetAddr := parseHexAddress(9, cmdLen)
                        let admitted := isGuestAdmitted(roomId, targetAddr)
                        if admitted {
                            mstore(resultPtr, 0x5b5a4d4d5d2041646d69747465642e0000000000000000000000000000000000) // "[ZMM] Admitted."
                            resultPtr := add(resultPtr, 15)
                        }
                        if iszero(admitted) {
                            mstore(resultPtr, 0x5b5a4d4d5d204e6f742061646d69747465642e00000000000000000000000000) // "[ZMM] Not admitted."
                            resultPtr := add(resultPtr, 19)
                        }
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if iszero(tokenAddr) {
                            mstore(resultPtr, 0x4e6f20636f6e747261637420626f756e6420746f207468697320726f6f6d2e00)
                            resultPtr := add(resultPtr, 31)
                        }
                        if tokenAddr {
                            let targetAddr := parseHexAddress(9, cmdLen)
                            mstore(0x1000, 0xdfffcc7400000000000000000000000000000000000000000000000000000000)
                            mstore(0x1004, targetAddr)
                            let success := staticcall(gas(), tokenAddr, 0x1000, 0x24, 0x2000, 0x20)
                            if success {
                                let resVal := mload(0x2000)
                                if resVal {
                                    mstore(resultPtr, 0x41646d69747465642e0000000000000000000000000000000000000000000000) // "Admitted."
                                    resultPtr := add(resultPtr, 9)
                                }
                                if iszero(resVal) {
                                    mstore(resultPtr, 0x4e6f742061646d69747465642e00000000000000000000000000000000000000) // "Not admitted."
                                    resultPtr := add(resultPtr, 13)
                                }
                            }
                            if iszero(success) {
                                mstore(resultPtr, 0x41646d69747465642063616c6c206661696c65642e0000000000000000000000) // "Admitted call failed."
                                resultPtr := add(resultPtr, 21)
                            }
                        }
                    }
                }
                case 0x626f756e { // "boun" (bouncer)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        mstore(resultPtr, 0x5b5a4d4d5d20547275652e000000000000000000000000000000000000000000) // "[ZMM] True."
                        resultPtr := add(resultPtr, 11)
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if tokenAddr {
                            let targetAddr := parseHexAddress(8, cmdLen)
                            mstore(0x1000, 0x37d74ffe00000000000000000000000000000000000000000000000000000000)
                            mstore(0x1004, targetAddr)
                            let success := staticcall(gas(), tokenAddr, 0x1000, 0x24, 0x2000, 0x20)
                            if success {
                                let resVal := mload(0x2000)
                                if resVal { mstore(resultPtr, 0x547275652e000000000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 5) }
                                if iszero(resVal) { mstore(resultPtr, 0x46616c73652e0000000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 6) }
                            }
                            if iszero(success) { mstore(resultPtr, 0x43616c6c206661696c65642e0000000000000000000000000000000000000000) resultPtr := add(resultPtr, 12) }
                        }
                    }
                }
                case 0x416c6c6f { // "Allo" (AllowCROWS)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        mstore(resultPtr, 0x5b5a4d4d5d20537563636573732e000000000000000000000000000000000000) // "[ZMM] Success."
                        resultPtr := add(resultPtr, 14)
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if tokenAddr {
                            let val := parseDec(11, cmdLen)
                            mstore(0x1000, 0x7b16630600000000000000000000000000000000000000000000000000000000)
                            mstore(0x1004, val)
                            let success := call(gas(), tokenAddr, 0, 0x1000, 0x24, 0x2000, 0)
                            if success { mstore(resultPtr, 0x537563636573732e000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 8) }
                            if iszero(success) { mstore(resultPtr, 0x4661696c65642e00000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 7) }
                        }
                    }
                }
                case 0x73657442 { // "setB" (setBouncerDivisor)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        let val := parseDec(18, cmdLen)
                        setRoomBouncerDivisor(roomId, val)
                        mstore(resultPtr, 0x5b5a4d4d5d20537563636573732e000000000000000000000000000000000000) // "[ZMM] Success."
                        resultPtr := add(resultPtr, 14)
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if tokenAddr {
                            let val := parseDec(18, cmdLen)
                            mstore(0x1000, 0x3c7e0c0000000000000000000000000000000000000000000000000000000000)
                            mstore(0x1004, val)
                            let success := call(gas(), tokenAddr, 0, 0x1000, 0x24, 0x2000, 0)
                            if success { mstore(resultPtr, 0x537563636573732e000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 8) }
                            if iszero(success) { mstore(resultPtr, 0x4661696c65642e00000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 7) }
                        }
                    }
                }
                case 0x73657443 { // "setC" (setCoverCharge)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        let val := parseDec(15, cmdLen)
                        setRoomCoverCharge(roomId, val)
                        mstore(resultPtr, 0x5b5a4d4d5d20537563636573732e000000000000000000000000000000000000) // "[ZMM] Success."
                        resultPtr := add(resultPtr, 14)
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if tokenAddr {
                            let val := parseDec(15, cmdLen)
                            mstore(0x1000, 0x7dfe96f300000000000000000000000000000000000000000000000000000000)
                            mstore(0x1004, val)
                            let success := call(gas(), tokenAddr, 0, 0x1000, 0x24, 0x2000, 0)
                            if success { mstore(resultPtr, 0x537563636573732e000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 8) }
                            if iszero(success) { mstore(resultPtr, 0x4661696c65642e00000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 7) }
                        }
                    }
                }
                case 0x73657453 { // "setS" (setStaff)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        mstore(resultPtr, 0x5b5a4d4d5d20537563636573732e000000000000000000000000000000000000) // "[ZMM] Success."
                        resultPtr := add(resultPtr, 14)
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if tokenAddr {
                            let spacePos := findSpace(9, cmdLen)
                            let targetAddr := parseHexAddress(9, spacePos)
                            let val := 0
                            if lt(spacePos, cmdLen) {
                                val := parseDec(add(spacePos, 1), cmdLen)
                            }
                            mstore(0x1000, 0x328d13e300000000000000000000000000000000000000000000000000000000)
                            mstore(0x1004, targetAddr)
                            mstore(0x1024, val)
                            let success := call(gas(), tokenAddr, 0, 0x1000, 0x44, 0x2000, 0)
                            if success { mstore(resultPtr, 0x537563636573732e000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 8) }
                            if iszero(success) { mstore(resultPtr, 0x4661696c65642e00000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 7) }
                        }
                    }
                }
                case 0x73657447 { // "setG" (setGuestlist)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        mstore(resultPtr, 0x5b5a4d4d5d20537563636573732e000000000000000000000000000000000000) // "[ZMM] Success."
                        resultPtr := add(resultPtr, 14)
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if tokenAddr {
                            let targetAddr := parseHexAddress(13, cmdLen)
                            mstore(0x1000, 0x0d4ec84500000000000000000000000000000000000000000000000000000000)
                            mstore(0x1004, targetAddr)
                            let success := call(gas(), tokenAddr, 0, 0x1000, 0x24, 0x2000, 0)
                            if success { mstore(resultPtr, 0x537563636573732e000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 8) }
                            if iszero(success) { mstore(resultPtr, 0x4661696c65642e00000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 7) }
                        }
                    }
                }
                case 0x72656d6f { // "remo" (removeGuest)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        mstore(resultPtr, 0x5b5a4d4d5d20537563636573732e000000000000000000000000000000000000) // "[ZMM] Success."
                        resultPtr := add(resultPtr, 14)
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if tokenAddr {
                            let targetAddr := parseHexAddress(12, cmdLen)
                            mstore(0x1000, 0x5256bfe700000000000000000000000000000000000000000000000000000000)
                            mstore(0x1004, targetAddr)
                            let success := call(gas(), tokenAddr, 0, 0x1000, 0x24, 0x2000, 0)
                            if success { mstore(resultPtr, 0x537563636573732e000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 8) }
                            if iszero(success) { mstore(resultPtr, 0x4661696c65642e00000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 7) }
                        }
                    }
                }
                case 0x47657451 { // "GetQ" (GetQing)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        mstore(resultPtr, 0x5b5a4d4d5d203078303030303030303030303030303030303030303030303030) // "[ZMM] 0x0000000000000000"
                        mstore(add(resultPtr, 32), 0x3030303030303030303030303030303030303030303030300000000000000000)
                        resultPtr := add(resultPtr, 56)
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if tokenAddr {
                            let val := parseDec(8, cmdLen)
                            mstore(0x1000, 0x20bc1aa200000000000000000000000000000000000000000000000000000000)
                            mstore(0x1004, val)
                            let success := staticcall(gas(), tokenAddr, 0x1000, 0x24, 0x2000, 0x20)
                            if success {
                                let resVal := mload(0x2000)
                                resultPtr := writeHexAddress(resVal, resultPtr)
                            }
                            if iszero(success) { mstore(resultPtr, 0x4661696c65642e00000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 7) }
                        }
                    }
                }
                case 0x52656163 { // "Reac" (ReactSoul / ReactWaat / React / ReactYue)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        let val := 0
                        if eq(roomId, 12) { // QI
                            let char5 := and(shr(248, calldataload(add(100, 5))), 0xff)
                            if eq(char5, 83) { // 'S' for ReactSoul
                                val := parseDec(10, cmdLen)
                            }
                            if eq(char5, 87) { // 'W' for ReactWaat
                                val := parseDec(10, cmdLen)
                            }
                        }
                        if eq(roomId, 13) { // MAI
                            val := 0x4d41492072656163742073756363657373000000000000000000000000000000
                        }
                        if eq(roomId, 16) { // CHAN
                            val := 0x4348414e20726561637420737563636573730000000000000000000000000000
                        }
                        mstore(resultPtr, val)
                        resultPtr := add(resultPtr, 32)
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if tokenAddr {
                            let typePtr := 0x1500
                            let typeEnd := queryContractString(tokenAddr, 0xfc0c546a00000000000000000000000000000000000000000000000000000000, typePtr)
                            let typeWord := mload(typePtr)
                            
                            if eq(shr(224, typeWord), 0x51490000) { // "QI"
                                let char5 := and(shr(248, calldataload(add(100, 5))), 0xff)
                                if eq(char5, 83) { // 'S' for ReactSoul
                                    let val := parseDec(10, cmdLen)
                                    mstore(0x1000, 0x6ba5a0d900000000000000000000000000000000000000000000000000000000)
                                    mstore(0x1004, val)
                                    let success := call(gas(), tokenAddr, 0, 0x1000, 0x24, 0x2000, 0x20)
                                    if success { mstore(resultPtr, mload(0x2000)) resultPtr := add(resultPtr, 32) }
                                    if iszero(success) { mstore(resultPtr, 0x4661696c65642e00000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 7) }
                                }
                                if eq(char5, 87) { // 'W' for ReactWaat
                                    let val := parseDec(10, cmdLen)
                                    mstore(0x1000, 0x48b582c700000000000000000000000000000000000000000000000000000000)
                                    mstore(0x1004, val)
                                    let success := staticcall(gas(), tokenAddr, 0x1000, 0x24, 0x2000, 0x20)
                                    if success { mstore(resultPtr, mload(0x2000)) resultPtr := add(resultPtr, 32) }
                                    if iszero(success) { mstore(resultPtr, 0x4661696c65642e00000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 7) }
                                }
                            }
                            if eq(shr(224, typeWord), 0x4d414900) { // "MAI"
                                let spacePos := findSpace(6, cmdLen)
                                let soul := parseDec(6, spacePos)
                                let waat := 0
                                if lt(spacePos, cmdLen) {
                                    waat := parseDec(add(spacePos, 1), cmdLen)
                                }
                                mstore(0x1000, 0x743a3e5e00000000000000000000000000000000000000000000000000000000)
                                mstore(0x1004, soul)
                                mstore(0x1024, waat)
                                let success := call(gas(), tokenAddr, 0, 0x1000, 0x44, 0x2000, 0x20)
                                if success { mstore(resultPtr, mload(0x2000)) resultPtr := add(resultPtr, 32) }
                                if iszero(success) { mstore(resultPtr, 0x4661696c65642e00000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 7) }
                            }
                            if eq(shr(224, typeWord), 0x4348414e) { // "CHAN"
                                let spacePos := findSpace(9, cmdLen)
                                let yue := parseHexAddress(9, spacePos)
                                let qing := 0
                                if lt(spacePos, cmdLen) {
                                    qing := parseHexAddress(add(spacePos, 1), cmdLen)
                                }
                                mstore(0x1000, 0x7a9bd22100000000000000000000000000000000000000000000000000000000)
                                mstore(0x1004, yue)
                                mstore(0x1024, qing)
                                let success := call(gas(), tokenAddr, 0, 0x1000, 0x44, 0x2000, 0x20)
                                if success { mstore(resultPtr, mload(0x2000)) resultPtr := add(resultPtr, 32) }
                                if iszero(success) { mstore(resultPtr, 0x4661696c65642e00000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 7) }
                            }
                        }
                    }
                }
                case 0x43686972 { // "Char" (Charge)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        mstore(resultPtr, 0x5849412063686172676520737563636573730000000000000000000000000000) // "XIA charge success"
                        resultPtr := add(resultPtr, 32)
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if tokenAddr {
                            let waat := parseDec(7, cmdLen)
                            mstore(0x1000, 0x4311635400000000000000000000000000000000000000000000000000000000)
                            mstore(0x1004, waat)
                            let success := call(gas(), tokenAddr, 0, 0x1000, 0x24, 0x2000, 0x20)
                            if success { mstore(resultPtr, mload(0x2000)) resultPtr := add(resultPtr, 32) }
                            if iszero(success) { mstore(resultPtr, 0x4661696c65642e00000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 7) }
                        }
                    }
                }
                case 0x506f7765 { // "Powe" (Power)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        mstore(resultPtr, 0x5b5a4d4d5d204368617267653a203130300a5b5a4d4d5d204f6d6963726f6e3a) // "[ZMM] Charge: 100\n[ZMM] Omicron:"
                        mstore(add(resultPtr, 32), 0x203230300a5b5a4d4d5d204f6d6567613a203330300000000000000000000000) // " 200\n[ZMM] Omega: 300"
                        resultPtr := add(resultPtr, 56)
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if tokenAddr {
                            let waat := parseDec(6, cmdLen)
                            mstore(0x1000, 0x192cc4a300000000000000000000000000000000000000000000000000000000)
                            mstore(0x1004, waat)
                            let success := call(gas(), tokenAddr, 0, 0x1000, 0x24, 0x2000, 0x60)
                            if success {
                                mstore(resultPtr, 0x4368617267653a20000000000000000000000000000000000000000000000000) // "Charge: "
                                resultPtr := add(resultPtr, 8)
                                mstore(resultPtr, mload(0x2000)) resultPtr := add(resultPtr, 32)
                                mstore8(resultPtr, 10) resultPtr := add(resultPtr, 1)
                                mstore(resultPtr, 0x4f6d6963726f6e3a200000000000000000000000000000000000000000000000) // "Omicron: "
                                resultPtr := add(resultPtr, 9)
                                mstore(resultPtr, mload(0x2020)) resultPtr := add(resultPtr, 32)
                                mstore8(resultPtr, 10) resultPtr := add(resultPtr, 1)
                                mstore(resultPtr, 0x4f6d6567613a2000000000000000000000000000000000000000000000000000) // "Omega: "
                                resultPtr := add(resultPtr, 7)
                                mstore(resultPtr, mload(0x2040)) resultPtr := add(resultPtr, 32)
                            }
                            if iszero(success) { mstore(resultPtr, 0x4661696c65642e00000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 7) }
                        }
                    }
                }
                case 0x41646459 { // "AddY" (AddYue)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        mstore(resultPtr, 0x5b5a4d4d5d20537563636573732e000000000000000000000000000000000000) // "[ZMM] Success."
                        resultPtr := add(resultPtr, 14)
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if tokenAddr {
                            let spacePos := findSpace(7, cmdLen)
                            let origAddr := parseHexAddress(7, spacePos)
                            let yue := parseHexAddress(add(spacePos, 1), cmdLen)
                            mstore(0x1000, 0xd0ba60d100000000000000000000000000000000000000000000000000000000)
                            mstore(0x1004, origAddr)
                            mstore(0x1024, yue)
                            let success := call(gas(), tokenAddr, 0, 0x1000, 0x44, 0x2000, 0)
                            if success { mstore(resultPtr, 0x537563636573732e000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 8) }
                            if iszero(success) { mstore(resultPtr, 0x4661696c65642e00000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 7) }
                        }
                    }
                }
                case 0x5472616e { // "Tran" (TransferYue / transfer)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        mstore(resultPtr, 0x5b5a4d4d5d20537563636573732e000000000000000000000000000000000000) // "[ZMM] Success."
                        resultPtr := add(resultPtr, 14)
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if tokenAddr {
                            let char5 := and(shr(248, calldataload(add(100, 5))), 0xff)
                            if eq(char5, 102) { // 'f' for TransferYue
                                let spacePos := findSpace(12, cmdLen)
                                let yue := parseHexAddress(12, spacePos)
                                let newOrigin := parseHexAddress(add(spacePos, 1), cmdLen)
                                mstore(0x1000, 0x2621e0f800000000000000000000000000000000000000000000000000000000)
                                mstore(0x1004, yue)
                                mstore(0x1024, newOrigin)
                                let success := call(gas(), tokenAddr, 0, 0x1000, 0x44, 0x2000, 0)
                                if success { mstore(resultPtr, 0x537563636573732e000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 8) }
                                if iszero(success) { mstore(resultPtr, 0x4661696c65642e00000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 7) }
                            }
                        }
                    }
                }
                case 0x4f707449 { // "OptI" (OptIn)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        mstore(resultPtr, 0x5b5a4d4d5d20537563636573732e000000000000000000000000000000000000) // "[ZMM] Success."
                        resultPtr := add(resultPtr, 14)
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if tokenAddr {
                            let spacePos := findSpace(6, cmdLen)
                            let contractAddr := parseHexAddress(6, spacePos)
                            let val := 0
                            if lt(spacePos, cmdLen) {
                                val := parseDec(add(spacePos, 1), cmdLen)
                            }
                            mstore(0x1000, 0xfcfcea4500000000000000000000000000000000000000000000000000000000)
                            mstore(0x1004, contractAddr)
                            mstore(0x1024, val)
                            let success := call(gas(), tokenAddr, 0, 0x1000, 0x44, 0x2000, 0)
                            if success { mstore(resultPtr, 0x537563636573732e000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 8) }
                            if iszero(success) { mstore(resultPtr, 0x4661696c65642e00000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 7) }
                        }
                    }
                }
                case 0x4f707465 { // "Opte" (OptedIn)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        mstore(resultPtr, 0x5b5a4d4d5d20547275652e000000000000000000000000000000000000000000) // "[ZMM] True."
                        resultPtr := add(resultPtr, 11)
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if tokenAddr {
                            let spacePos := findSpace(8, cmdLen)
                            let yue := parseHexAddress(8, spacePos)
                            let contractAddr := parseHexAddress(add(spacePos, 1), cmdLen)
                            mstore(0x1000, 0xd70d37e600000000000000000000000000000000000000000000000000000000)
                            mstore(0x1004, yue)
                            mstore(0x1024, contractAddr)
                            let success := staticcall(gas(), tokenAddr, 0x1000, 0x44, 0x2000, 0x20)
                            if success {
                                let resVal := mload(0x2000)
                                if resVal { mstore(resultPtr, 0x547275652e000000000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 5) }
                                if iszero(resVal) { mstore(resultPtr, 0x4661696c65642e00000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 7) }
                            }
                            if iszero(success) { mstore(resultPtr, 0x4661696c65642e00000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 7) }
                        }
                    }
                }
                case 0x59756557 { // "YueW" (YueWithdraw)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        mstore(resultPtr, 0x5b5a4d4d5d20537563636573732e000000000000000000000000000000000000) // "[ZMM] Success."
                        resultPtr := add(resultPtr, 14)
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if tokenAddr {
                            let space1 := findSpace(12, cmdLen)
                            let yue := parseHexAddress(12, space1)
                            let space2 := findSpace(add(space1, 1), cmdLen)
                            let asset := parseHexAddress(add(space1, 1), space2)
                            let space3 := findSpace(add(space2, 1), cmdLen)
                            let to := parseHexAddress(add(space2, 1), space3)
                            let amount := parseDec(add(space3, 1), cmdLen)
                            
                            mstore(0x1000, 0xc4a3ab2700000000000000000000000000000000000000000000000000000000)
                            mstore(0x1004, yue)
                            mstore(0x1024, asset)
                            mstore(0x1044, to)
                            mstore(0x1064, amount)
                            let success := call(gas(), tokenAddr, 0, 0x1000, 0x84, 0x2000, 0)
                            if success { mstore(resultPtr, 0x537563636573732e000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 8) }
                            if iszero(success) { mstore(resultPtr, 0x4661696c65642e00000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 7) }
                        }
                    }
                }
                case 0x5975654d { // "YueM" (YueMintToOrigin)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        mstore(resultPtr, 0x5b5a4d4d5d20537563636573732e000000000000000000000000000000000000) // "[ZMM] Success."
                        resultPtr := add(resultPtr, 14)
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if tokenAddr {
                            let yue := parseHexAddress(16, cmdLen)
                            mstore(0x1000, 0xe4606fde00000000000000000000000000000000000000000000000000000000)
                            mstore(0x1004, yue)
                            let success := call(gas(), tokenAddr, 0, 0x1000, 0x24, 0x2000, 0)
                            if success { mstore(resultPtr, 0x537563636573732e000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 8) }
                            if iszero(success) { mstore(resultPtr, 0x4661696c65642e00000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 7) }
                        }
                    }
                }
                case 0x59756546 { // "YueF" (YueForceTransfer)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        mstore(resultPtr, 0x5b5a4d4d5d20537563636573732e000000000000000000000000000000000000) // "[ZMM] Success."
                        resultPtr := add(resultPtr, 14)
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if tokenAddr {
                            let space1 := findSpace(17, cmdLen)
                            let yue := parseHexAddress(17, space1)
                            let space2 := findSpace(add(space1, 1), cmdLen)
                            let from := parseHexAddress(add(space1, 1), space2)
                            let space3 := findSpace(add(space2, 1), cmdLen)
                            let to := parseHexAddress(add(space2, 1), space3)
                            let amount := parseDec(add(space3, 1), cmdLen)
                            
                            mstore(0x1000, 0x35cb27d700000000000000000000000000000000000000000000000000000000)
                            mstore(0x1004, yue)
                            mstore(0x1024, from)
                            mstore(0x1044, to)
                            mstore(0x1064, amount)
                            let success := call(gas(), tokenAddr, 0, 0x1000, 0x84, 0x2000, 0)
                            if success { mstore(resultPtr, 0x537563636573732e000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 8) }
                            if iszero(success) { mstore(resultPtr, 0x4661696c65642e00000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 7) }
                        }
                    }
                }
                case 0x53746172 { // "Star" (Start)
                    let tokenAddr := sload(add(2000000, roomId))
                    if tokenAddr {
                        let space1 := findSpace(6, cmdLen)
                        let lauToken := parseHexAddress(6, space1)
                        let space2 := findSpace(add(space1, 1), cmdLen)
                        let nameLen := sub(space2, add(space1, 1))
                        let symbolLen := sub(cmdLen, add(space2, 1))
                        
                        mstore(0x1000, 0x7851901900000000000000000000000000000000000000000000000000000000)
                        mstore(0x1004, lauToken)
                        mstore(0x1024, 0x60)
                        let paddedNameLen := mul(div(add(nameLen, 31), 32), 32)
                        mstore(0x1044, add(0x60, add(32, paddedNameLen)))
                        mstore(0x1064, nameLen)
                        calldatacopy(0x1084, add(100, add(space1, 1)), nameLen)
                        
                        let symOffset := add(0x1084, paddedNameLen)
                        mstore(symOffset, symbolLen)
                        calldatacopy(add(symOffset, 32), add(100, add(space2, 1)), symbolLen)
                        
                        let paddedSymLen := mul(div(add(symbolLen, 31), 32), 32)
                        let totalCalldataSize := add(sub(add(symOffset, 32), 0x1000), paddedSymLen)
                        
                        let success := call(gas(), tokenAddr, 0, 0x1000, totalCalldataSize, 0x2000, 0x100)
                        if success { mstore(resultPtr, 0x53746172746564207375636365737366756c6c792e0000000000000000000000) resultPtr := add(resultPtr, 21) }
                        if iszero(success) { mstore(resultPtr, 0x4661696c65642e00000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 7) }
                    }
                }
                case 0x43686920 { // "Chi " (Chi)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        mstore(resultPtr, 0x5b5a4d4d5d205975653a20307830303030303030303030303030303030303030) // "[ZMM] Yue: 0x00000000"
                        mstore(add(resultPtr, 32), 0x3030303030303030303030303030303030303030303030300a5b5a4d4d5d204c) // "000000000000000000000000\n[ZMM] L"
                        mstore(add(resultPtr, 64), 0x41553a2030783030303030303030303030303030303030303030303030303030) // "AU: 0x000000000000000000000000"
                        mstore(add(resultPtr, 96), 0x3030303030303030000000000000000000000000000000000000000000000000)
                        resultPtr := add(resultPtr, 104)
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if tokenAddr {
                            mstore(0x1000, 0x1cb77ea700000000000000000000000000000000000000000000000000000000)
                            let success := staticcall(gas(), tokenAddr, 0x1000, 0x04, 0x2000, 0x40)
                            if success {
                                let yue := mload(0x2000)
                                let lau := mload(0x2020)
                                mstore(resultPtr, 0x5975653a20000000000000000000000000000000000000000000000000000000) // "Yue: "
                                resultPtr := add(resultPtr, 5)
                                resultPtr := writeHexAddress(yue, resultPtr)
                                mstore8(resultPtr, 10) resultPtr := add(resultPtr, 1)
                                mstore(resultPtr, 0x4c41553a20000000000000000000000000000000000000000000000000000000) // "LAU: "
                                resultPtr := add(resultPtr, 5)
                                resultPtr := writeHexAddress(lau, resultPtr)
                            }
                            if iszero(success) { mstore(resultPtr, 0x4661696c65642e00000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 7) }
                        }
                    }
                }
                case 0x5975616e { // "Yuan" (Yuan)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        mstore(resultPtr, 0)
                        resultPtr := add(resultPtr, 32)
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if tokenAddr {
                            let currency := parseHexAddress(5, cmdLen)
                            mstore(0x1000, 0xd6047e3700000000000000000000000000000000000000000000000000000000)
                            mstore(0x1004, currency)
                            let success := staticcall(gas(), tokenAddr, 0x1000, 0x24, 0x2000, 0x20)
                            if success { mstore(resultPtr, mload(0x2000)) resultPtr := add(resultPtr, 32) }
                            if iszero(success) { mstore(resultPtr, 0x4661696c65642e00000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 7) }
                        }
                    }
                }
                case 0x506c6179 { // "Play" (Play)
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        mstore(resultPtr, 0x5b5a4d4d5d205975653a20307830303030303030303030303030303030303030) // "[ZMM] Yue: 0x00000000"
                        mstore(add(resultPtr, 32), 0x3030303030303030303030303030303030303030303030300a5b5a4d4d5d204c) // "000000000000000000000000\n[ZMM] L"
                        mstore(add(resultPtr, 64), 0x41553a2030783030303030303030303030303030303030303030303030303030) // "AU: 0x000000000000000000000000"
                        mstore(add(resultPtr, 96), 0x3030303030303030000000000000000000000000000000000000000000000000)
                        resultPtr := add(resultPtr, 104)
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if tokenAddr {
                            let userToken := parseHexAddress(5, cmdLen)
                            mstore(0x1000, 0x74ff471800000000000000000000000000000000000000000000000000000000)
                            mstore(0x1004, userToken)
                            let success := call(gas(), tokenAddr, 0, 0x1000, 0x24, 0x2000, 0x40)
                            if success {
                                let yue := mload(0x2000)
                                let lau := mload(0x2020)
                                mstore(resultPtr, 0x5975653a20000000000000000000000000000000000000000000000000000000) // "Yue: "
                                resultPtr := add(resultPtr, 5)
                                resultPtr := writeHexAddress(yue, resultPtr)
                                mstore8(resultPtr, 10) resultPtr := add(resultPtr, 1)
                                mstore(resultPtr, 0x4c41553a20000000000000000000000000000000000000000000000000000000) // "LAU: "
                                resultPtr := add(resultPtr, 5)
                                resultPtr := writeHexAddress(lau, resultPtr)
                            }
                            if iszero(success) { mstore(resultPtr, 0x4661696c65642e00000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 7) }
                        }
                    }
                }
                case 0x53752020 { // "Su  "
                    let isZmmRoom := and(gt(roomId, 9), lt(roomId, 20))
                    if isZmmRoom {
                        mstore(resultPtr, 0x5b5a4d4d5d204368617267653a203130300a5b5a4d4d5d204879706f6261723a) // "[ZMM] Charge: 100\n[ZMM] Hypobar:"
                        mstore(add(resultPtr, 32), 0x203230300a5b5a4d4d5d204570696261723a2033303000000000000000000000) // " 200\n[ZMM] Epibar: 300"
                        resultPtr := add(resultPtr, 56)
                    }
                    if iszero(isZmmRoom) {
                        let tokenAddr := sload(add(2000000, roomId))
                        if tokenAddr {
                            let qing := parseHexAddress(3, cmdLen)
                            mstore(0x1000, 0x753a0c0900000000000000000000000000000000000000000000000000000000)
                            mstore(0x1004, qing)
                            let success := call(gas(), tokenAddr, 0, 0x1000, 0x24, 0x2000, 0x60)
                            if success {
                                mstore(resultPtr, 0x4368617267653a20000000000000000000000000000000000000000000000000) // "Charge: "
                                resultPtr := add(resultPtr, 8)
                                mstore(resultPtr, mload(0x2000)) resultPtr := add(resultPtr, 32)
                                mstore8(resultPtr, 10) resultPtr := add(resultPtr, 1)
                                mstore(resultPtr, 0x4879706f6261723a200000000000000000000000000000000000000000000000) // "Hypobar: "
                                resultPtr := add(resultPtr, 9)
                                mstore(resultPtr, mload(0x2020)) resultPtr := add(resultPtr, 32)
                                mstore8(resultPtr, 10) resultPtr := add(resultPtr, 1)
                                mstore(resultPtr, 0x4570696261723a20000000000000000000000000000000000000000000000000) // "Epibar: "
                                resultPtr := add(resultPtr, 8)
                                mstore(resultPtr, mload(0x2040)) resultPtr := add(resultPtr, 32)
                            }
                            if iszero(success) { mstore(resultPtr, 0x4661696c65642e00000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 7) }
                        }
                    }
                }
                case 0x7472616e { // "tran" (transfer)
                    let tokenAddr := sload(add(2000000, roomId))
                    if tokenAddr {
                        let spacePos := findSpace(9, cmdLen)
                        let toAddr := parseHexAddress(9, spacePos)
                        let amount := parseDec(add(spacePos, 1), cmdLen)
                        mstore(0x1000, 0xa9059cbb00000000000000000000000000000000000000000000000000000000)
                        mstore(0x1004, toAddr)
                        mstore(0x1024, amount)
                        let success := call(gas(), tokenAddr, 0, 0x1000, 0x44, 0x2000, 0x20)
                        let isSuccessful := 0
                        if success {
                            isSuccessful := mload(0x2000)
                        }
                        if iszero(success) {
                            mstore(0x00, player)
                            mstore(0x20, add(6000000, roomId))
                            let fromSlot := keccak256(0x00, 0x40)
                            
                            let supplySlot := add(5000000, roomId)
                            if iszero(sload(supplySlot)) {
                                sstore(supplySlot, 1000000)
                                sstore(fromSlot, 1000000)
                            }
                            
                            let senderBal := sload(fromSlot)
                            if iszero(lt(senderBal, amount)) {
                                sstore(fromSlot, sub(senderBal, amount))
                                
                                mstore(0x00, toAddr)
                                mstore(0x20, add(6000000, roomId))
                                let toSlot := keccak256(0x00, 0x40)
                                sstore(toSlot, add(sload(toSlot), amount))
                                isSuccessful := 1
                            }
                        }
                        if isSuccessful { mstore(resultPtr, 0x5472616e73666572205375636365737366756c2e000000000000000000000000) resultPtr := add(resultPtr, 18) }
                        if iszero(isSuccessful) { mstore(resultPtr, 0x5472616e73666572204661696c65642e00000000000000000000000000000000) resultPtr := add(resultPtr, 16) }
                    }
                }
                case 0x61707072 { // "appr" (approve)
                    let tokenAddr := sload(add(2000000, roomId))
                    if tokenAddr {
                        let spacePos := findSpace(8, cmdLen)
                        let spender := parseHexAddress(8, spacePos)
                        let amount := parseDec(add(spacePos, 1), cmdLen)
                        mstore(0x1000, 0x095ea7b300000000000000000000000000000000000000000000000000000000)
                        mstore(0x1004, spender)
                        mstore(0x1024, amount)
                        let success := call(gas(), tokenAddr, 0, 0x1000, 0x44, 0x2000, 0x20)
                        let isSuccessful := 0
                        if success {
                            isSuccessful := mload(0x2000)
                        }
                        if iszero(success) {
                            mstore(0x00, player)
                            mstore(0x20, spender)
                            mstore(0x40, add(7000000, roomId))
                            let allowanceSlot := keccak256(0x00, 0x60)
                            sstore(allowanceSlot, amount)
                            isSuccessful := 1
                        }
                        if isSuccessful { mstore(resultPtr, 0x417070726f76616c205375636365737366756c2e000000000000000000000000) resultPtr := add(resultPtr, 19) }
                        if iszero(isSuccessful) { mstore(resultPtr, 0x417070726f76616c204661696c65642e00000000000000000000000000000000) resultPtr := add(resultPtr, 17) }
                    }
                }
                case 0x62616c61 { // "bala" (balanceOf)
                    let tokenAddr := sload(add(2000000, roomId))
                    if tokenAddr {
                        let account := parseHexAddress(10, cmdLen)
                        mstore(0x1000, 0x70a0823100000000000000000000000000000000000000000000000000000000)
                        mstore(0x1004, account)
                        let success := staticcall(gas(), tokenAddr, 0x1000, 0x24, 0x2000, 0x20)
                        let balanceVal := 0
                        if success {
                            balanceVal := mload(0x2000)
                        }
                        if iszero(success) {
                            mstore(0x00, account)
                            mstore(0x20, add(6000000, roomId))
                            let balSlot := keccak256(0x00, 0x40)
                            
                            let supplySlot := add(5000000, roomId)
                            if iszero(sload(supplySlot)) {
                                sstore(supplySlot, 1000000)
                                mstore(0x00, player)
                                mstore(0x20, add(6000000, roomId))
                                sstore(keccak256(0x00, 0x40), 1000000)
                            }
                            balanceVal := sload(balSlot)
                        }
                        resultPtr := writeDec(balanceVal, resultPtr)
                    }
                }
                case 0x746f7461 { // "tota" (totalSupply)
                    let tokenAddr := sload(add(2000000, roomId))
                    if tokenAddr {
                        mstore(0x1000, 0x18160ddd00000000000000000000000000000000000000000000000000000000)
                        let success := staticcall(gas(), tokenAddr, 0x1000, 0x04, 0x2000, 0x20)
                        let supplyVal := 0
                        if success {
                            supplyVal := mload(0x2000)
                        }
                        if iszero(success) {
                            let supplySlot := add(5000000, roomId)
                            if iszero(sload(supplySlot)) {
                                sstore(supplySlot, 1000000)
                                mstore(0x00, player)
                                mstore(0x20, add(6000000, roomId))
                                sstore(keccak256(0x00, 0x40), 1000000)
                            }
                            supplyVal := sload(supplySlot)
                        }
                        resultPtr := writeDec(supplyVal, resultPtr)
                    }
                }
                case 0x4765744c { // "GetL" (GetLibraryAddress)
                    let tokenAddr := sload(add(2000000, roomId))
                    if tokenAddr {
                        let argLen := sub(cmdLen, 18)
                        mstore(0x1000, 0xd9270a5a00000000000000000000000000000000000000000000000000000000)
                        mstore(0x1004, 0x20)
                        mstore(0x1024, argLen)
                        calldatacopy(0x1044, add(100, 18), argLen)
                        let paddedSize := mul(div(add(argLen, 31), 32), 32)
                        let totalCalldataSize := add(68, paddedSize)
                        let success := staticcall(gas(), tokenAddr, 0x1000, totalCalldataSize, 0x2000, 0x20)
                        if success {
                            let resVal := mload(0x2000)
                            resultPtr := writeHexAddress(resVal, resultPtr)
                        }
                        if iszero(success) { mstore(resultPtr, 0x4661696c65642e00000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 7) }
                    }
                }
                case 0x4164644c { // "AddL" (AddLibrary)
                    let tokenAddr := sload(add(2000000, roomId))
                    if tokenAddr {
                        let spacePos := findSpace(11, cmdLen)
                        let nameLen := sub(spacePos, 11)
                        let targetAddr := parseHexAddress(add(spacePos, 1), cmdLen)
                        
                        mstore(0x1000, 0xe774b3a400000000000000000000000000000000000000000000000000000000)
                        mstore(0x1004, 0x40)
                        mstore(0x1024, targetAddr)
                        mstore(0x1044, nameLen)
                        calldatacopy(0x1064, add(100, 11), nameLen)
                        let paddedNameLen := mul(div(add(nameLen, 31), 32), 32)
                        let totalCalldataSize := add(100, paddedNameLen)
                        let success := call(gas(), tokenAddr, 0, 0x1000, totalCalldataSize, 0x2000, 0)
                        if success { mstore(resultPtr, 0x537563636573732e000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 8) }
                        if iszero(success) { mstore(resultPtr, 0x4661696c65642e00000000000000000000000000000000000000000000000000) resultPtr := add(resultPtr, 7) }
                    }
                }
                case 0x696e7665 { // "inve" (Inventory)
                    mstore(resultPtr, 0x596f7520617265206361727279696e673a000000000000000000000000000000) // "You are carrying:"
                    let invPtr := add(resultPtr, 17)
                    let count := 0
                    
                    if getInventoryBalance(player, 50) {
                        mstore(invPtr, 0x0a2d20476f6c6420546f6b656e20284552433230290000000000000000000000) // "\n- Gold Token (ERC20)"
                        invPtr := add(invPtr, 21)
                        count := add(count, 1)
                    }
                    if getInventoryBalance(player, 51) {
                        mstore(invPtr, 0x0a2d204b65796361726400000000000000000000000000000000000000000000) // "\n- Keycard"
                        invPtr := add(invPtr, 10)
                        count := add(count, 1)
                    }
                    if getInventoryBalance(player, 52) {
                        mstore(invPtr, 0x0a2d20456e65726779205061636b000000000000000000000000000000000000) // "\n- Energy Pack"
                        invPtr := add(invPtr, 14)
                        count := add(count, 1)
                    }

                    // Gauntlet Stats Bridge: Check if Gauntlet is active
                    let folkloreAddr := sload(2500000)
                    if folkloreAddr {
                        mstore(0x00, shl(224, 0xc2e22c95)) // peekUser
                        mstore(0x04, player)
                        mstore(0x24, 55050) // isGauntletActive
                        let success := staticcall(gas(), folkloreAddr, 0x00, 68, 0x00, 32)
                        if and(success, mload(0x00)) {
                            // Fetch Health
                            mstore(0x00, shl(224, 0xc2e22c95))
                            mstore(0x04, player)
                            mstore(0x24, 55053) // gauntletHealth
                            let okHealth := staticcall(gas(), folkloreAddr, 0x00, 68, 0x00, 32)
                            let healthVal := 0
                            if okHealth { healthVal := mload(0x00) }
                            
                            // Fetch Keys
                            mstore(0x00, shl(224, 0xc2e22c95))
                            mstore(0x04, player)
                            mstore(0x24, 55054) // gauntletKeys
                            let okKeys := staticcall(gas(), folkloreAddr, 0x00, 68, 0x00, 32)
                            let keysVal := 0
                            if okKeys { keysVal := mload(0x00) }
                            
                            // Append "\n- Gauntlet Health: "
                            mstore(invPtr, 0x0a2d204761756e746c6574204865616c74683a20000000000000000000000000)
                            invPtr := add(invPtr, 20)
                            invPtr := appendNumberStr(invPtr, healthVal)
                            
                            // Append "\n- Gauntlet Keys: "
                            mstore(invPtr, 0x0a2d204761756e746c6574204b6579733a200000000000000000000000000000)
                            invPtr := add(invPtr, 18)
                            invPtr := appendNumberStr(invPtr, keysVal)
                            
                            count := add(count, 1)
                        }
                    }
                    
                    if iszero(count) {
                        mstore(resultPtr, 0x596f757220696e76656e746f727920697320656d7074792e0000000000000000) // "Your inventory is empty."
                        resultPtr := add(resultPtr, 24)
                    }
                    if count {
                        resultPtr := invPtr
                    }
                }
                case 0x7377696e { // "swin" (Swing)
                    mstore(resultPtr, 0x51756173696d6f646f207377696e677321000000000000000000000000000000) // "Quasimodo swings!"
                    resultPtr := add(resultPtr, 17)
                }
                case 0x7a617000 { // "zap" (Zap)
                    sstore(3000500, 1) // active target flag
                    sstore(3000501, 20) // Cylon target X
                    sstore(3000502, 15) // Cylon target Y
                    sstore(3000503, 0) // Crosshair X
                    sstore(3000504, 0) // Crosshair Y
                    
                    mstore(resultPtr, 0x43796c6f6e205261696465722061707065617273210000000000000000000000) // "Cylon Raider appears!"
                    resultPtr := add(resultPtr, 21)
                }
                case 0x61696d20 { // "aim " (Aim)
                    let valX := 0
                    let valY := 0
                    let idx := 104
                    
                    // Parse first integer (X)
                    for {} lt(idx, 120) {} {
                        let char := and(shr(248, calldataload(idx)), 0xff)
                        if or(lt(char, 48), gt(char, 57)) { 
                            idx := add(idx, 1)
                            break 
                        }
                        valX := add(mul(valX, 10), sub(char, 48))
                        idx := add(idx, 1)
                    }
                    
                    // Parse second integer (Y)
                    for {} lt(idx, 120) {} {
                        let char := and(shr(248, calldataload(idx)), 0xff)
                        if or(lt(char, 48), gt(char, 57)) { 
                            if valY { break }
                            idx := add(idx, 1)
                            continue
                        }
                        valY := add(mul(valY, 10), sub(char, 48))
                        idx := add(idx, 1)
                    }
                    
                    sstore(3000503, valX)
                    sstore(3000504, valY)
                    
                    mstore(resultPtr, 0x41696d696e672063726f737368616972206174205b0000000000000000000000) // "Aiming crosshair at ["
                    resultPtr := add(resultPtr, 21)
                    mstore(resultPtr, 0x582c20595d2e2054797065202773686f6f742720746f20666972652100000000) // "X, Y]. Type 'shoot' to fire!"
                    resultPtr := add(resultPtr, 28)
                }
                case 0x73686f6f { // "shoo" (Shoot / fire)
                    let targetX := sload(3000501)
                    let targetY := sload(3000502)
                    let playerX := sload(3000503)
                    let playerY := sload(3000504)
                    
                    let diffX := 0
                    if gt(playerX, targetX) { diffX := sub(playerX, targetX) }
                    if iszero(gt(playerX, targetX)) { diffX := sub(targetX, playerX) }
                    
                    let diffY := 0
                    if gt(playerY, targetY) { diffY := sub(playerY, targetY) }
                    if iszero(gt(playerY, targetY)) { diffY := sub(targetY, playerY) }
                    
                    // Tolerance check: must be within 3 coordinate units
                    let hit := and(lt(diffX, 4), lt(diffY, 4))
                    
                    if hit {
                        sstore(3000500, 0) // clear target
                        mstore(resultPtr, 0x5a4150212044657374726f796564210000000000000000000000000000000000) // "ZAP! Destroyed!"
                        resultPtr := add(resultPtr, 15)
                    }
                    if iszero(hit) {
                        mstore(resultPtr, 0x4d69737321000000000000000000000000000000000000000000000000000000) // "Miss!"
                        resultPtr := add(resultPtr, 5)
                    }
                }
                case 0x73746172 { // "star" (Start game / start)
                    sstore(3000800, 10) // 10 drones
                    sstore(3000801, 20) // player X (center)
                    sstore(3000802, 1)  // player speed level
                    sstore(3000803, 15) // drone X (starts at 15)
                    sstore(3000804, 1)  // stage 1
                    
                    mstore(resultPtr, 0x4d6f746865727368697020696e697465642e0000000000000000000000000000) // "Mothership inited."
                    resultPtr := add(resultPtr, 18)
                }
                case 0x73746565 { // "stee" (Steer)
                    let playerX := sload(3000801)
                    let dir := shr(224, calldataload(106))
                    
                    if eq(dir, 0x6c656674) { // "left"
                        playerX := sub(playerX, 3)
                    }
                    if eq(dir, 0x72696768) { // "righ" (right)
                        playerX := add(playerX, 3)
                    }
                    
                    sstore(3000801, playerX)
                    
                    let crashed := 0
                    if lt(playerX, 5) { crashed := 1 }
                    if gt(playerX, 35) { crashed := 1 }
                    
                    if crashed {
                        sstore(3000804, 0) // reset stage
                        mstore(resultPtr, 0x43524153482120596f75206869742074686520636f727269646f722077616c6c) // "CRASH! You hit the corridor wall"
                        mstore(add(resultPtr, 32), 0x73212047616d65206f7665722e00000000000000000000000000000000000000) // "s! Game over."
                        resultPtr := add(resultPtr, 45)
                    }
                    if iszero(crashed) {
                        mstore(resultPtr, 0x5374656572696e672e20506f736974696f6e2058203d20000000000000000000) // "Steering. Position X = "
                        let textPtr := add(resultPtr, 23)
                        
                        // Print coordinate character
                        let char1 := add(48, div(playerX, 10))
                        let char2 := add(48, mod(playerX, 10))
                        mstore8(textPtr, char1)
                        mstore8(add(textPtr, 1), char2)
                        
                        resultPtr := add(textPtr, 2)
                    }
                }
                case 0x61636365 { // "acce" (Accelerate)
                    let spd := sload(3000802)
                    if lt(spd, 3) { spd := add(spd, 1) }
                    sstore(3000802, spd)
                    mstore(resultPtr, 0x416363656c65726174696e672e205370656564203d2030000000000000000000) // "Accelerating. Speed = 0"
                    mstore8(add(resultPtr, 22), add(48, spd))
                    resultPtr := add(resultPtr, 23)
                }
                case 0x66697265 { // "fire" (Fire / shoot)
                    let stage := sload(3000804)
                    if eq(stage, 1) { // Drones phase
                        let playerX := sload(3000801)
                        let droneX := sload(3000803)
                        let diffX := 0
                        if gt(playerX, droneX) { diffX := sub(playerX, droneX) }
                        if iszero(gt(playerX, droneX)) { diffX := sub(droneX, playerX) }
                        
                        let hit := lt(diffX, 4)
                        if hit {
                            let count := sload(3000800)
                            count := sub(count, 1)
                            sstore(3000800, count)
                            
                            if count {
                                // Spawn next drone at a different X
                                let nextX := and(add(droneX, 11), 31)
                                if lt(nextX, 5) { nextX := add(nextX, 5) }
                                sstore(3000803, nextX)
                                
                                mstore(resultPtr, 0x44495245435420484954212044726f6e652064657374726f7965642e20000000) // "DIRECT HIT! Drone destroyed. "
                                let textPtr := add(resultPtr, 29)
                                mstore8(textPtr, add(48, count))
                                mstore(add(textPtr, 1), 0x2064726f6e65732072656d61696e65652e000000000000000000000000000000) // " drones remaine."
                                resultPtr := add(textPtr, 17)
                            }
                            if iszero(count) {
                                sstore(3000804, 2) // Mothership phase
                                mstore(resultPtr, 0x414c4c2044524f4e45532044455354524f5945442120546865204d4f54484552) // "ALL DRONES DESTROYED! The MOTHER"
                                mstore(add(resultPtr, 32), 0x53484950206c6f6f6d73206168656164212046697265206e6f77210000000000) // "SHIP looms ahead! Fire now!"
                                resultPtr := add(resultPtr, 59)
                            }
                        }
                        if iszero(hit) {
                            mstore(resultPtr, 0x4d697373212044726f6e65206973207374696c6c2061742058203d2030300000) // "Miss! Drone is still at X = 00"
                            let textPtr := add(resultPtr, 29)
                            mstore8(textPtr, add(48, div(droneX, 10)))
                            mstore8(add(textPtr, 1), add(48, mod(droneX, 10)))
                            resultPtr := add(resultPtr, 31)
                        }
                    }
                    if eq(stage, 2) { // Mothership phase
                        sstore(3000804, 0) // reset
                        mstore(resultPtr, 0x4b41424f4f4d2120596f752064657374726f79656420746865204d4f54484552) // "KABOOM! You destroyed the MOTHER"
                        mstore(add(resultPtr, 32), 0x534849502120566967696c616e7420766963746f72792e000000000000000000) // "SHIP! Vigilant victory."
                        resultPtr := add(resultPtr, 55)
                    }
                    if iszero(stage) {
                        mstore(resultPtr, 0x4e6f2061637469766520746172676574732e2054797065202773746172742720) // "No active targets. Type 'start' "
                        mstore(add(resultPtr, 32), 0x746f20626567696e2e0000000000000000000000000000000000000000000000) // "to begin."
                        resultPtr := add(resultPtr, 41)
                    }
                }
                case 0x6e6f7274 { // "nort" (North)
                    let exits := getRoomExits(roomId)
                    let dest := and(shr(24, exits), 0xff)
                    if dest {
                        sstore(add(4000000, player), dest)
                        roomId := dest
                        let customLen := sload(add(3000000, roomId))
                        if customLen {
                            let wordCount := div(add(customLen, 31), 32)
                            for { let i := 0 } lt(i, 10) { i := add(i, 1) } {
                                if lt(mul(i, 32), customLen) {
                                    let val := sload(add(add(3100000, mul(roomId, 100)), i))
                                    mstore(add(resultPtr, mul(i, 32)), val)
                                }
                            }
                            resultPtr := add(resultPtr, customLen)
                        }
                        if iszero(customLen) {
                            mstore(resultPtr, 0x596f752061726520696e20616e20656d70747920726f6f6d2e00000000000000) // "You are in an empty room."
                            resultPtr := add(resultPtr, 25)
                        }
                    }
                    if iszero(dest) {
                        mstore(resultPtr, 0x596f752063616e6e6f7420676f2074686174207761792e000000000000000000) // "You cannot go that way."
                        resultPtr := add(resultPtr, 23)
                    }
                }
                case 0x736f7574 { // "sout" (South)
                    let exits := getRoomExits(roomId)
                    let dest := and(shr(16, exits), 0xff)
                    if dest {
                        sstore(add(4000000, player), dest)
                        roomId := dest
                        let customLen := sload(add(3000000, roomId))
                        if customLen {
                            let wordCount := div(add(customLen, 31), 32)
                            for { let i := 0 } lt(i, 10) { i := add(i, 1) } {
                                if lt(mul(i, 32), customLen) {
                                    let val := sload(add(add(3100000, mul(roomId, 100)), i))
                                    mstore(add(resultPtr, mul(i, 32)), val)
                                }
                            }
                            resultPtr := add(resultPtr, customLen)
                        }
                        if iszero(customLen) {
                            mstore(resultPtr, 0x596f752061726520696e20616e20656d70747920726f6f6d2e00000000000000) // "You are in an empty room."
                            resultPtr := add(resultPtr, 25)
                        }
                    }
                    if iszero(dest) {
                        mstore(resultPtr, 0x596f752063616e6e6f7420676f2074686174207761792e000000000000000000) // "You cannot go that way."
                        resultPtr := add(resultPtr, 23)
                    }
                }
                case 0x65617374 { // "east" (East)
                    let exits := getRoomExits(roomId)
                    let dest := and(shr(8, exits), 0xff)
                    if dest {
                        sstore(add(4000000, player), dest)
                        roomId := dest
                        let customLen := sload(add(3000000, roomId))
                        if customLen {
                            let wordCount := div(add(customLen, 31), 32)
                            for { let i := 0 } lt(i, 10) { i := add(i, 1) } {
                                if lt(mul(i, 32), customLen) {
                                    let val := sload(add(add(3100000, mul(roomId, 100)), i))
                                    mstore(add(resultPtr, mul(i, 32)), val)
                                }
                            }
                            resultPtr := add(resultPtr, customLen)
                        }
                        if iszero(customLen) {
                            mstore(resultPtr, 0x596f752061726520696e20616e20656d70747920726f6f6d2e00000000000000) // "You are in an empty room."
                            resultPtr := add(resultPtr, 25)
                        }
                    }
                    if iszero(dest) {
                        mstore(resultPtr, 0x596f752063616e6e6f7420676f2074686174207761792e000000000000000000) // "You cannot go that way."
                        resultPtr := add(resultPtr, 23)
                    }
                }
                case 0x77657374 { // "west" (West)
                    let exits := getRoomExits(roomId)
                    let dest := and(exits, 0xff)
                    if dest {
                        sstore(add(4000000, player), dest)
                        roomId := dest
                        let customLen := sload(add(3000000, roomId))
                        if customLen {
                            let wordCount := div(add(customLen, 31), 32)
                            for { let i := 0 } lt(i, 10) { i := add(i, 1) } {
                                if lt(mul(i, 32), customLen) {
                                    let val := sload(add(add(3100000, mul(roomId, 100)), i))
                                    mstore(add(resultPtr, mul(i, 32)), val)
                                }
                            }
                            resultPtr := add(resultPtr, customLen)
                        }
                        if iszero(customLen) {
                            mstore(resultPtr, 0x596f752061726520696e20616e20656d70747920726f6f6d2e00000000000000) // "You are in an empty room."
                            resultPtr := add(resultPtr, 25)
                        }
                    }
                    if iszero(dest) {
                        mstore(resultPtr, 0x596f752063616e6e6f7420676f2074686174207761792e000000000000000000) // "You cannot go that way."
                        resultPtr := add(resultPtr, 23)
                    }
                }
                case 0x77697a61 { // "wizard"
                    mstore(resultPtr, 0x5a6f726c6f6b207468652057697a6172642067726565747320796f7520776974) // "Zorlok the Wizard greets you wit"
                    mstore(add(resultPtr, 32), 0x682061207370656c6c206f6620766974616c6974792100000000000000000000) // "h a spell of vitality!"
                    resultPtr := add(resultPtr, 54)
                }
                case 0x7a6f726c { // "zorlok"
                    mstore(resultPtr, 0x5a6f726c6f6b207468652057697a6172642067726565747320796f7520776974) // "Zorlok the Wizard greets you wit"
                    mstore(add(resultPtr, 32), 0x682061207370656c6c206f6620766974616c6974792100000000000000000000) // "h a spell of vitality!"
                    resultPtr := add(resultPtr, 54)
                }
                default {
                    mstore(resultPtr, 0x4920646f6e277420756e6465727374616e642e00000000000000000000000000) // "I don't understand."
                    resultPtr := add(resultPtr, 19)
                }

                let strLen := sub(resultPtr, 0x40)
                mstore(0x00, 0x20)
                mstore(0x20, strLen)
                let totalBytes := add(64, mul(div(add(strLen, 31), 32), 32))
                return(0, totalBytes)
            }

            case 0xee1db5af {
                // triggerZ6Sound(address musicMaker, uint256 note, uint256 voice) -> bool
                let musicMaker := calldataload(4)
                let note := calldataload(36)
                let voice := calldataload(68)
                
                // ABI selector for playNote(uint16,uint8) inside musicMaker.yul is 0xf63eb9b8
                // Left-aligned parameter packing due to musicMaker shr(240) and shr(248) logic
                mstore(0x00, 0xf63eb9b800000000000000000000000000000000000000000000000000000000)
                mstore(0x04, shl(240, note))
                mstore(0x24, shl(248, voice))
                
                let success := call(gas(), musicMaker, 0, 0x00, 0x44, 0x00, 0x20)
                mstore(0x00, success)
                return(0x00, 32)
            }

            case 0xe68d22f6 {
                // decryptInvisiclue(address keySystem, address player, uint256 hintId) -> string
                let keySystem := calldataload(4)
                let player := calldataload(36)
                let hintId := calldataload(68)

                // First verify player has registered a secure key on keysystem to authorize decryption
                // ABI selector for getKey256(address) inside keySystem.yul is 0xa5f5ddb3
                mstore(0x00, 0xa5f5ddb300000000000000000000000000000000000000000000000000000000)
                mstore(0x04, player)
                let keySuccess := staticcall(gas(), keySystem, 0x00, 0x24, 0x00, 0x20)
                let secureKey := mload(0x00)

                let resultPtr := 0x40
                if and(keySuccess, iszero(iszero(secureKey))) {
                    // Pull payment of 1 ERC-20 token before revealing hint
                    let tokenAddr := sload(add(2000000, 99)) // Hint collection contract address bound at ID 99
                    if tokenAddr {
                        let paySuccess := erc20TransferFrom(tokenAddr, player, address(), 1000000000000000000)
                        if paySuccess {
                            mstore(resultPtr, 0x48696e743a2055736520746865206272617373206c616e7465726e2068657265) // "Hint: Use the brass lantern here"
                            resultPtr := add(resultPtr, 32)
                        }
                    }
                }

                if eq(resultPtr, 0x40) {
                    mstore(resultPtr, 0x4163636573732064656e6965642e204c6f636b65642e00000000000000000000) // "Access denied. Locked."
                    resultPtr := add(resultPtr, 21)
                }

                let strLen := sub(resultPtr, 0x40)
                mstore(0x00, 0x20)
                mstore(0x20, strLen)
                let totalBytes := add(64, mul(div(add(strLen, 31), 32), 32))
                return(0, totalBytes)
            }

            case 0x5d5517bf {
                // getVectorScene(uint256 roomIndex) -> bytes
                let roomIndex := calldataload(4)
                
                // Return offsets for bytes: offset = 0x20
                mstore(0x00, 0x20)
                
                // We will pack line commands (each line: fromX, fromY, toX, toY, color)
                // Room 0: Victorian House Exterior
                if or(iszero(roomIndex), eq(roomIndex, 4)) {
                    mstore(0x20, 50) // Length: 50 bytes (10 lines * 5 bytes)
                    
                    // Line 0: Ground line (0, 150) to (240, 150)
                    mstore8(0x40, 0)
                    mstore8(0x41, 150)
                    mstore8(0x42, 240)
                    mstore8(0x43, 150)
                    mstore8(0x44, 1)
                    
                    // Line 1: Left Wall (50, 150) to (50, 80)
                    mstore8(0x45, 50)
                    mstore8(0x46, 150)
                    mstore8(0x47, 50)
                    mstore8(0x48, 80)
                    mstore8(0x49, 1)

                    // Line 2: Right Wall (190, 150) to (190, 80)
                    mstore8(0x4a, 190)
                    mstore8(0x4b, 150)
                    mstore8(0x4c, 190)
                    mstore8(0x4d, 80)
                    mstore8(0x4e, 1)

                    // Line 3: Roof Left (50, 80) to (120, 40)
                    mstore8(0x4f, 50)
                    mstore8(0x50, 80)
                    mstore8(0x51, 120)
                    mstore8(0x52, 40)
                    mstore8(0x53, 1)

                    // Line 4: Roof Right (190, 80) to (120, 40)
                    mstore8(0x54, 190)
                    mstore8(0x55, 80)
                    mstore8(0x56, 120)
                    mstore8(0x57, 40)
                    mstore8(0x58, 1)

                    // Line 5: Door Left (110, 150) to (110, 110)
                    mstore8(0x59, 110)
                    mstore8(0x5a, 150)
                    mstore8(0x5b, 110)
                    mstore8(0x5c, 110)
                    mstore8(0x5d, 1)

                    // Line 6: Door Right (130, 150) to (130, 110)
                    mstore8(0x5e, 130)
                    mstore8(0x5f, 150)
                    mstore8(0x60, 130)
                    mstore8(0x61, 110)
                    mstore8(0x62, 1)

                    // Line 7: Door Top (110, 110) to (130, 110)
                    mstore8(0x63, 110)
                    mstore8(0x64, 110)
                    mstore8(0x65, 130)
                    mstore8(0x66, 110)
                    mstore8(0x67, 1)

                    // Line 8: Window Left (70, 100) to (90, 100)
                    mstore8(0x68, 70)
                    mstore8(0x69, 100)
                    mstore8(0x6a, 90)
                    mstore8(0x6b, 100)
                    mstore8(0x6c, 1)

                    // Line 9: Window Right (150, 100) to (170, 100)
                    mstore8(0x6d, 150)
                    mstore8(0x6e, 100)
                    mstore8(0x6f, 170)
                    mstore8(0x70, 100)
                    mstore8(0x71, 1)
                    
                    return(0x00, 128)
                }

                // Room 1: Entry Hall / Foyer with staircase
                if eq(roomIndex, 1) {
                    mstore(0x20, 50) // Length: 50 bytes (10 lines)
                    
                    // Line 0: Ground line
                    mstore8(0x40, 0)
                    mstore8(0x41, 150)
                    mstore8(0x42, 240)
                    mstore8(0x43, 150)
                    mstore8(0x44, 1)
                    
                    // Line 1: Left Wall
                    mstore8(0x45, 20)
                    mstore8(0x46, 150)
                    mstore8(0x47, 20)
                    mstore8(0x48, 20)
                    mstore8(0x49, 1)

                    // Line 2: Right Wall
                    mstore8(0x4a, 220)
                    mstore8(0x4b, 150)
                    mstore8(0x4c, 220)
                    mstore8(0x4d, 20)
                    mstore8(0x4e, 1)

                    // Line 3: Ceiling
                    mstore8(0x4f, 20)
                    mstore8(0x50, 20)
                    mstore8(0x51, 220)
                    mstore8(0x52, 20)
                    mstore8(0x53, 1)

                    // Line 4: Stairs diagonal
                    mstore8(0x54, 100)
                    mstore8(0x55, 150)
                    mstore8(0x56, 180)
                    mstore8(0x57, 80)
                    mstore8(0x58, 1)

                    // Line 5: Stairs landing
                    mstore8(0x59, 180)
                    mstore8(0x5a, 80)
                    mstore8(0x5b, 220)
                    mstore8(0x5c, 80)
                    mstore8(0x5d, 1)

                    // Line 6: Left doorway vertical
                    mstore8(0x5e, 60)
                    mstore8(0x5f, 150)
                    mstore8(0x60, 60)
                    mstore8(0x61, 80)
                    mstore8(0x62, 1)

                    // Line 7: Left doorway top
                    mstore8(0x63, 20)
                    mstore8(0x64, 80)
                    mstore8(0x65, 60)
                    mstore8(0x66, 80)
                    mstore8(0x67, 1)

                    // Line 8: Right doorway vertical
                    mstore8(0x68, 180)
                    mstore8(0x69, 150)
                    mstore8(0x6a, 180)
                    mstore8(0x6b, 80)
                    mstore8(0x6c, 1)

                    // Line 9: Right doorway top
                    mstore8(0x6d, 180)
                    mstore8(0x6e, 80)
                    mstore8(0x6f, 220)
                    mstore8(0x70, 80)
                    mstore8(0x71, 1)

                    return(0x00, 128)
                }

                // Room 2: Living Room / Parlor with fireplace
                if eq(roomIndex, 2) {
                    mstore(0x20, 50) // Length: 50 bytes (10 lines)
                    
                    // Line 0: Ground line
                    mstore8(0x40, 0)
                    mstore8(0x41, 150)
                    mstore8(0x42, 240)
                    mstore8(0x43, 150)
                    mstore8(0x44, 1)
                    
                    // Line 1: Left Wall
                    mstore8(0x45, 20)
                    mstore8(0x46, 150)
                    mstore8(0x47, 20)
                    mstore8(0x48, 20)
                    mstore8(0x49, 1)

                    // Line 2: Right Wall
                    mstore8(0x4a, 220)
                    mstore8(0x4b, 150)
                    mstore8(0x4c, 220)
                    mstore8(0x4d, 20)
                    mstore8(0x4e, 1)

                    // Line 3: Ceiling
                    mstore8(0x4f, 20)
                    mstore8(0x50, 20)
                    mstore8(0x51, 220)
                    mstore8(0x52, 20)
                    mstore8(0x53, 1)

                    // Line 4: Fireplace Outer Left
                    mstore8(0x54, 90)
                    mstore8(0x55, 150)
                    mstore8(0x56, 90)
                    mstore8(0x57, 90)
                    mstore8(0x58, 1)

                    // Line 5: Fireplace Outer Right
                    mstore8(0x59, 150)
                    mstore8(0x5a, 150)
                    mstore8(0x5b, 150)
                    mstore8(0x5c, 90)
                    mstore8(0x5d, 1)

                    // Line 6: Fireplace Outer Top (Mantel)
                    mstore8(0x5e, 80)
                    mstore8(0x5f, 90)
                    mstore8(0x60, 160)
                    mstore8(0x61, 90)
                    mstore8(0x62, 1)

                    // Line 7: Fireplace Inner Left
                    mstore8(0x63, 105)
                    mstore8(0x64, 150)
                    mstore8(0x65, 105)
                    mstore8(0x66, 115)
                    mstore8(0x67, 1)

                    // Line 8: Fireplace Inner Right
                    mstore8(0x68, 135)
                    mstore8(0x69, 150)
                    mstore8(0x6a, 135)
                    mstore8(0x6b, 115)
                    mstore8(0x6c, 1)

                    // Line 9: Fireplace Inner Top
                    mstore8(0x6d, 105)
                    mstore8(0x6e, 115)
                    mstore8(0x6f, 135)
                    mstore8(0x70, 115)
                    mstore8(0x71, 1)

                    return(0x00, 128)
                }

                // Room 3: Unreal Portal / Mystery Vault
                if eq(roomIndex, 3) {
                    mstore(0x20, 75) // Length: 75 bytes (15 lines * 5 bytes)
                    
                    // Line 0: Ground line
                    mstore8(0x40, 0)
                    mstore8(0x41, 150)
                    mstore8(0x42, 240)
                    mstore8(0x43, 150)
                    mstore8(0x44, 1)
                    
                    // Line 1: Left Wall
                    mstore8(0x45, 20)
                    mstore8(0x46, 150)
                    mstore8(0x47, 20)
                    mstore8(0x48, 20)
                    mstore8(0x49, 1)

                    // Line 2: Right Wall
                    mstore8(0x4a, 220)
                    mstore8(0x4b, 150)
                    mstore8(0x4c, 220)
                    mstore8(0x4d, 20)
                    mstore8(0x4e, 1)

                    // Line 3: Arch Left
                    mstore8(0x4f, 80)
                    mstore8(0x50, 150)
                    mstore8(0x51, 80)
                    mstore8(0x52, 70)
                    mstore8(0x53, 1)

                    // Line 4: Arch Right
                    mstore8(0x54, 160)
                    mstore8(0x55, 150)
                    mstore8(0x56, 160)
                    mstore8(0x57, 70)
                    mstore8(0x58, 1)

                    // Line 5: Arch Top Left
                    mstore8(0x59, 80)
                    mstore8(0x5a, 70)
                    mstore8(0x5b, 120)
                    mstore8(0x5c, 40)
                    mstore8(0x5d, 1)

                    // Line 6: Arch Top Right
                    mstore8(0x5e, 160)
                    mstore8(0x5f, 70)
                    mstore8(0x60, 120)
                    mstore8(0x61, 40)
                    mstore8(0x62, 1)

                    // Line 7: Pedestal Base
                    mstore8(0x63, 110)
                    mstore8(0x64, 150)
                    mstore8(0x65, 130)
                    mstore8(0x66, 150)
                    mstore8(0x67, 1)

                    // Line 8: Pedestal Top
                    mstore8(0x68, 115)
                    mstore8(0x69, 130)
                    mstore8(0x6a, 125)
                    mstore8(0x6b, 130)
                    mstore8(0x6c, 1)

                    // Line 9: Pedestal Left
                    mstore8(0x6d, 115)
                    mstore8(0x6e, 130)
                    mstore8(0x6f, 110)
                    mstore8(0x70, 150)
                    mstore8(0x71, 1)

                    // Line 10: Pedestal Right
                    mstore8(0x72, 125)
                    mstore8(0x73, 130)
                    mstore8(0x74, 130)
                    mstore8(0x75, 150)
                    mstore8(0x76, 1)

                    // Line 11: Floating Crystal Left (color code 2: neon magenta)
                    mstore8(0x77, 120)
                    mstore8(0x78, 115)
                    mstore8(0x79, 115)
                    mstore8(0x7a, 120)
                    mstore8(0x7b, 2)

                    // Line 12: Floating Crystal Top
                    mstore8(0x7c, 120)
                    mstore8(0x7d, 115)
                    mstore8(0x7e, 125)
                    mstore8(0x7f, 120)
                    mstore8(0x80, 2)

                    // Line 13: Floating Crystal Right
                    mstore8(0x81, 125)
                    mstore8(0x82, 120)
                    mstore8(0x83, 120)
                    mstore8(0x84, 125)
                    mstore8(0x85, 2)

                    // Line 14: Floating Crystal Bottom
                    mstore8(0x86, 115)
                    mstore8(0x87, 120)
                    mstore8(0x88, 120)
                    mstore8(0x89, 125)
                    mstore8(0x8a, 2)

                    return(0x00, 160)
                }
                
                // Return empty for other rooms
                mstore(0x20, 0)
                return(0x00, 64)
            }

            default {
                revert(0, 0)
            }

            // --- Interpreter Helper Functions ---

            function getRoomExits(roomId) -> exits {
                exits := sload(add(3200000, roomId))
                if iszero(exits) {
                    if eq(roomId, 10) { exits := 0x01000000 } // North -> Room 1
                    if eq(roomId, 1)  { exits := 0x000a0203 } // South -> Room 10, East -> Room 2, West -> Room 3
                    if eq(roomId, 2)  { exits := 0x00000001 } // West -> Room 1
                    if eq(roomId, 3)  { exits := 0x00000100 } // East -> Room 1
                }
            }
            
            function readRom16(offset) -> val {
                let storageWordIndex := div(offset, 32)
                let byteOffset := mod(offset, 32)
                let word := sload(add(1000000, storageWordIndex))
                val := and(shr(sub(240, mul(byteOffset, 8)), word), 0xffff)
            }

            function readRom8(offset) -> val {
                let storageWordIndex := div(offset, 32)
                let byteOffset := mod(offset, 32)
                let word := sload(add(1000000, storageWordIndex))
                val := and(shr(sub(248, mul(byteOffset, 8)), word), 0xff)
            }

            function decodeChar(c, alphabet, outPtr) -> newOutPtr {
                newOutPtr := outPtr
                if eq(c, 4) { leave }
                if eq(c, 5) { leave }
                
                let charCode := 32
                
                switch alphabet
                case 0 {
                    if and(gt(c, 5), lt(c, 32)) {
                        charCode := add(97, sub(c, 6))
                    }
                }
                case 1 {
                    if and(gt(c, 5), lt(c, 32)) {
                        charCode := add(65, sub(c, 6))
                    }
                }
                
                mstore(outPtr, charCode)
                newOutPtr := add(outPtr, 1)
            }

            function readObjectProperty(objId, propId) -> val {
                let objTableBase := readRom16(10)
                let base := add(objTableBase, 62)
                let objPtr := add(base, mul(sub(objId, 1), 9))
                let propTableOffset := readRom16(add(objPtr, 7))
                let nameLen := readRom8(propTableOffset)
                let dataPtr := add(propTableOffset, add(1, mul(nameLen, 2)))
                
                let found := 0
                for {} 1 {} {
                    let sizeByte := readRom8(dataPtr)
                    if iszero(sizeByte) { break }
                    
                    let id := and(sizeByte, 31)
                    let size := add(div(sizeByte, 32), 1)
                    dataPtr := add(dataPtr, 1)
                    
                    if eq(id, propId) {
                        switch size
                        case 1 {
                            val := readRom8(dataPtr)
                        }
                        case 2 {
                            val := readRom16(dataPtr)
                        }
                        found := 1
                        break
                    }
                    dataPtr := add(dataPtr, size)
                }
                
                if iszero(found) {
                    val := readRom16(add(objTableBase, mul(sub(propId, 1), 2)))
                }
            }

            // --- ERC-20 Call Wrappers ---

            function getInventoryBalance(player, itemId) -> bal {
                let tokenAddr := sload(add(2000000, itemId))
                if tokenAddr {
                    bal := erc20BalanceOf(tokenAddr, player)
                }
                if iszero(bal) {
                    if iszero(sload(add(2000300, itemId))) {
                        bal := 1
                    }
                }
            }

            function erc20BalanceOf(token, account) -> bal {
                mstore(0x00, 0x70a0823100000000000000000000000000000000000000000000000000000000)
                mstore(0x04, account)
                let success := staticcall(gas(), token, 0x00, 0x24, 0x00, 0x20)
                if success {
                    bal := mload(0x00)
                }
            }

            function erc20Transfer(token, recipient, amount) -> success {
                mstore(0x00, 0xa9059cbb00000000000000000000000000000000000000000000000000000000)
                mstore(0x04, recipient)
                mstore(0x24, amount)
                success := call(gas(), token, 0, 0x00, 0x44, 0x00, 0x20)
                if success {
                    success := mload(0x00)
                }
            }

            function erc20TransferFrom(token, sender, recipient, amount) -> success {
                mstore(0x00, 0x23b872dd00000000000000000000000000000000000000000000000000000000)
                mstore(0x04, sender)
                mstore(0x24, recipient)
                mstore(0x44, amount)
                success := call(gas(), token, 0, 0x00, 0x64, 0x00, 0x20)
                if success {
                    success := mload(0x00)
                }
            }

            function queryContractString(token, sig, destPtr) -> endPtr {
                endPtr := destPtr
                if iszero(extcodesize(token)) { leave }
                mstore(0x00, sig)
                let success := staticcall(gas(), token, 0x00, 0x04, 0x1000, 0x100)
                if and(success, gt(returndatasize(), 63)) {
                    let strLen := mload(0x1020)
                    if lt(strLen, 100) {
                        for { let i := 0 } lt(i, strLen) { i := add(i, 1) } {
                            let char := byte(0, mload(add(0x1040, i)))
                            mstore8(endPtr, char)
                            endPtr := add(endPtr, 1)
                        }
                    }
                }
            }

            // Shifts MSB to LSB and prints each hex digit
            function writeHexAddress(addr, destPtr) -> endPtr {
                endPtr := destPtr
                mstore8(endPtr, 48) // '0'
                endPtr := add(endPtr, 1)
                mstore8(endPtr, 120) // 'x'
                endPtr := add(endPtr, 1)
                for { let i := 0 } lt(i, 20) { i := add(i, 1) } {
                    let val := and(shr(sub(152, mul(i, 8)), addr), 0xff)
                    let h1 := shr(4, val)
                    let h2 := and(val, 0x0f)
                    mstore8(endPtr, hexChar(h1))
                    endPtr := add(endPtr, 1)
                    mstore8(endPtr, hexChar(h2))
                    endPtr := add(endPtr, 1)
                }
            }

            function hexChar(val) -> c {
                c := add(val, 48)
                if gt(val, 9) {
                    c := add(val, 87) // 'a' - 10
                }
            }

            function writeDec16(val, destPtr) -> endPtr {
                endPtr := destPtr
                let d3 := div(val, 100)
                let rem := mod(val, 100)
                let d2 := div(rem, 10)
                let d1 := mod(rem, 10)
                
                mstore8(endPtr, add(d3, 48))
                endPtr := add(endPtr, 1)
                mstore8(endPtr, add(d2, 48))
                endPtr := add(endPtr, 1)
                mstore8(endPtr, add(d1, 48))
                endPtr := add(endPtr, 1)
            }

            function writeDec(val, destPtr) -> endPtr {
                if iszero(val) {
                    mstore8(destPtr, 48)
                    endPtr := add(destPtr, 1)
                    leave
                }
                let temp := val
                let len := 0
                for {} gt(temp, 0) {} {
                    len := add(len, 1)
                    temp := div(temp, 10)
                }
                endPtr := add(destPtr, len)
                temp := val
                let curr := endPtr
                for {} gt(temp, 0) {} {
                    curr := sub(curr, 1)
                    mstore8(curr, add(mod(temp, 10), 48))
                    temp := div(temp, 10)
                }
            }

            function findSpace(startOffset, cmdLen) -> spacePos {
                spacePos := cmdLen
                for { let i := startOffset } lt(i, cmdLen) { i := add(i, 1) } {
                    let char := and(shr(248, calldataload(add(100, i))), 0xff)
                    if eq(char, 32) {
                        spacePos := i
                        break
                    }
                }
            }

            function parseHexAddress(startIdx, cmdLen) -> addr {
                addr := 0
                let idx := startIdx
                let c1 := and(shr(248, calldataload(add(100, idx))), 0xff)
                let c2 := and(shr(248, calldataload(add(100, add(idx, 1)))), 0xff)
                if and(eq(c1, 48), eq(c2, 120)) {
                    idx := add(idx, 2)
                }
                for {} lt(idx, cmdLen) {} {
                    let char := and(shr(248, calldataload(add(100, idx))), 0xff)
                    let val := 0
                    if and(gt(char, 47), lt(char, 58)) { val := sub(char, 48) }
                    if and(gt(char, 96), lt(char, 103)) { val := add(sub(char, 97), 10) }
                    if and(gt(char, 64), lt(char, 71)) { val := add(sub(char, 65), 10) }
                    if iszero(or(and(gt(char, 47), lt(char, 58)), or(and(gt(char, 96), lt(char, 103)), and(gt(char, 64), lt(char, 71))))) {
                        break
                    }
                    addr := add(mul(addr, 16), val)
                    idx := add(idx, 1)
                }
            }

            function parseDec(startIdx, cmdLen) -> val {
                val := 0
                let idx := startIdx
                for {} lt(idx, cmdLen) {} {
                    let char := and(shr(248, calldataload(add(100, idx))), 0xff)
                    if or(lt(char, 48), gt(char, 57)) { break }
                    val := add(mul(val, 10), sub(char, 48))
                    idx := add(idx, 1)
                }
            }

            function mCopy(src, dest, len) {
                for { let i := 0 } lt(i, len) { i := add(i, 32) } {
                    mstore(add(dest, i), mload(add(src, i)))
                }
            }

            // ZMM VM Room Layout Helpers
            function getRoomBase(roomId) -> base {
                base := add(0x8000, mul(roomId, 0x200))
            }
            
            function getRoomCoverCharge(roomId) -> val {
                val := mload(add(getRoomBase(roomId), 0x02))
            }
            
            function setRoomCoverCharge(roomId, val) {
                mstore(add(getRoomBase(roomId), 0x02), val)
            }
            
            function getRoomBouncerDivisor(roomId) -> val {
                val := shr(240, mload(add(getRoomBase(roomId), 0x22)))
            }
            
            function setRoomBouncerDivisor(roomId, val) {
                mstore(add(getRoomBase(roomId), 0x22), shl(240, val))
            }

            function isGuestAdmitted(roomId, guestAddr) -> admitted {
                admitted := 0
                let base := add(getRoomBase(roomId), 0x44)
                for { let i := 0 } lt(i, 8) { i := add(i, 1) } {
                    let addr := mload(add(base, mul(i, 32)))
                    if eq(addr, guestAddr) {
                        admitted := 1
                        break
                    }
                }
            }
            
            function addGuest(roomId, guestAddr) {
                let base := add(getRoomBase(roomId), 0x44)
                for { let i := 0 } lt(i, 8) { i := add(i, 1) } {
                    let addr := mload(add(base, mul(i, 32)))
                    if or(iszero(addr), eq(addr, guestAddr)) {
                        mstore(add(base, mul(i, 32)), guestAddr)
                        break
                    }
                }
            }

            function setZmmAttribute(roomId, keyHash, valPtr, valLen) {
                let base := getRoomBase(roomId)
                let count := mload(add(base, 0x84))
                let found := 0
                let entryPtr := add(base, 0x88)
                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    if eq(mload(entryPtr), keyHash) {
                        found := 1
                        break
                    }
                    entryPtr := add(entryPtr, 128)
                }
                if iszero(found) {
                    if lt(count, 3) {
                        mstore(add(base, 0x84), add(count, 1))
                    }
                }
                mstore(entryPtr, keyHash)
                mstore(add(entryPtr, 32), valLen)
                mCopy(valPtr, add(entryPtr, 64), valLen)
            }

            function getZmmAttribute(roomId, keyHash, destPtr) -> len {
                let base := getRoomBase(roomId)
                let count := mload(add(base, 0x84))
                len := 0
                let entryPtr := add(base, 0x88)
                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    if eq(mload(entryPtr), keyHash) {
                        len := mload(add(entryPtr, 32))
                        mCopy(add(entryPtr, 64), destPtr, len)
                        break
                    }
                    entryPtr := add(entryPtr, 128)
                }
            }
            function appendNumberStr(dest, val) -> newDest {
                if iszero(val) {
                    mstore8(dest, 0x30)
                    newDest := add(dest, 1)
                    leave
                }
                let len := 0
                for { let t := val } gt(t, 0) { t := div(t, 10) } {
                    len := add(len, 1)
                }
                newDest := add(dest, len)
                let ptr := sub(newDest, 1)
                for { let t := val } gt(t, 0) { t := div(t, 10) } {
                    let digit := mod(t, 10)
                    mstore8(ptr, add(0x30, digit))
                    ptr := sub(ptr, 1)
                }
            }
        }
    }
}
