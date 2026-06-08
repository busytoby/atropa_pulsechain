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

            case 0xd6c5268c { // createRoom(uint256,bytes,uint256)
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

            case 0xf1ba03f9 {
                let player := calldataload(4)
                let cmdLen := calldataload(68)
                let firstWord := shr(224, calldataload(100))
                
                let roomId := sload(add(4000000, player))
                if iszero(roomId) { roomId := 1 }

                // Initialize item room placements if not already set up
                if iszero(sload(999999)) {
                    sstore(999999, 1)
                    sstore(add(2000300, 50), 1) // Item 50: Gold Token (starts in Room 1 / lobby)
                    sstore(add(2000300, 51), 2) // Item 51: Keycard (starts in Room 2)
                    sstore(add(2000300, 52), 3) // Item 52: Energy Pack (starts in Room 3)
                }

                let resultPtr := 0x40
                
                switch firstWord
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
                case 0x6c6f6f6b { // "look"
                    let customLen := sload(add(3000000, roomId))
                    if customLen {
                        let wordCount := div(add(customLen, 31), 32)
                        for { let i := 0 } lt(i, 10) { i := add(i, 1) } { // safety cap 10 words
                            if lt(mul(i, 32), customLen) {
                                let val := sload(add(add(3100000, mul(roomId, 100)), i))
                                mstore(add(resultPtr, mul(i, 32)), val)
                            }
                        }
                        resultPtr := add(resultPtr, customLen)
                    }
                    if iszero(customLen) {
                        if eq(roomId, 1) {
                            mstore(resultPtr, 0x596f7520617265207374616e64696e6720696e20746865206c6f6262792e0000) // "You are standing in the lobby."
                            resultPtr := add(resultPtr, 30)
                        }
                        if iszero(eq(roomId, 1)) {
                            mstore(resultPtr, 0x596f752061726520696e20616e20656d70747920726f6f6d2e00000000000000) // "You are in an empty room."
                            resultPtr := add(resultPtr, 25)
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
                    
                    if iszero(count) {
                        mstore(resultPtr, 0x596f757220696e76656e746f727920697320656d7074792e0000000000000000) // "Your inventory is empty."
                        resultPtr := add(resultPtr, 24)
                    }
                    if count {
                        resultPtr := invPtr
                    }
                }
                case 0x7377696e { // "swin" (Swing)
                    mstore(resultPtr, 0x51756173696d6f646f207377696e6773206f6e2074686520726f70653a0a0000) // "Quasimodo swings on the rope:\n"
                    let ptr := add(resultPtr, 29)
                    
                    mstore(ptr, 0x2020743d30733a205b583d20372c20593d20375d0a0000000000000000000000) // "  t=0s: [X= 7, Y= 7]\n"
                    ptr := add(ptr, 21)
                    mstore(ptr, 0x2020743d31733a205b583d20342c20593d20395d0a0000000000000000000000) // "  t=1s: [X= 4, Y= 9]\n"
                    ptr := add(ptr, 21)
                    mstore(ptr, 0x2020743d32733a205b583d2d332c20593d20395d0a0000000000000000000000) // "  t=2s: [X=-3, Y= 9]\n"
                    ptr := add(ptr, 21)
                    mstore(ptr, 0x2020743d33733a205b583d2d372c20593d20375d000000000000000000000000) // "  t=3s: [X=-7, Y= 7]"
                    ptr := add(ptr, 20)
                    
                    resultPtr := ptr
                }
                case 0x7a617000 { // "zap" (Zap)
                    sstore(3000500, 1) // active target flag
                    sstore(3000501, 20) // Cylon target X
                    sstore(3000502, 15) // Cylon target Y
                    sstore(3000503, 0) // Crosshair X
                    sstore(3000504, 0) // Crosshair Y
                    
                    mstore(resultPtr, 0x43796c6f6e205261696465722061707065617273206174205b32302c203135) // "Cylon Raider appears at [20, 15"
                    mstore(add(resultPtr, 31), 0x5d212054797065202761696d205820592720746f2061696d2e00000000000000) // "]! Type 'aim X Y' to aim."
                    resultPtr := add(resultPtr, 56)
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
                        mstore(resultPtr, 0x5a4150212043796c6f6e205261696d65722064657374726f7965642100000000) // "ZAP! Cylon Raider destroyed!"
                        resultPtr := add(resultPtr, 28)
                    }
                    if iszero(hit) {
                        mstore(resultPtr, 0x4d697373212043796c6f6e206973207374696c6c206174205b32302c2031355d) // "Miss! Cylon is still at [20, 15]"
                        resultPtr := add(resultPtr, 32)
                    }
                }
                case 0x73746172 { // "star" (Start game / start)
                    sstore(3000800, 10) // 10 drones
                    sstore(3000801, 20) // player X (center)
                    sstore(3000802, 1)  // player speed level
                    sstore(3000803, 15) // drone X (starts at 15)
                    sstore(3000804, 1)  // stage 1
                    
                    mstore(resultPtr, 0x4d4f544845525348495020696e69746564212031302064726f6e657320616865) // "MOTHERSHIP inited! 10 drones ahe"
                    mstore(add(resultPtr, 32), 0x61642e205479706520277374656572206c6566742f726967687427206f722027) // "ad. Type 'steer left/right' or '"
                    mstore(add(resultPtr, 64), 0x666972652720746f2073686f6f742e0000000000000000000000000000000000) // "fire' to shoot."
                    resultPtr := add(resultPtr, 79)
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
                    let exits := sload(add(3200000, roomId))
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
                    let exits := sload(add(3200000, roomId))
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
                    let exits := sload(add(3200000, roomId))
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
                    let exits := sload(add(3200000, roomId))
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
                if iszero(roomIndex) {
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
                
                // Return empty for other rooms
                mstore(0x20, 0)
                return(0x00, 64)
            }

            default {
                revert(0, 0)
            }

            // --- Interpreter Helper Functions ---
            
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
                        break
                    }
                    dataPtr := add(dataPtr, size)
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
        }
    }
}
