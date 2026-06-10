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

            case 0x7e1ef7e9 { // bindParserAddress(address)
                sstore(2600000, calldataload(4))
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
                let parserAddr := sload(2600000)
                if iszero(parserAddr) { revert(0, 0) }
                calldatacopy(0, 0, calldatasize())
                let success := delegatecall(gas(), parserAddr, 0, calldatasize(), 0, 0)
                returndatacopy(0, 0, returndatasize())
                if success { return(0, returndatasize()) }
                revert(0, returndatasize())
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
