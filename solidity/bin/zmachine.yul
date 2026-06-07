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
            // 6. parseCommand(address player, bytes cmd) -> 0x3d02a9e3
            // 7. triggerZ6Sound(address musicMaker, uint256 note, uint256 voice) -> 0xb5a0c8d1
            // 8. decryptInvisiclue(address keySystem, address player, uint256 hintId) -> 0x8a92efd4
            
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

            case 0x3d02a9e3 {
                let player := calldataload(4)
                let cmdLen := calldataload(36)
                let firstWord := shr(224, calldataload(68))
                
                let resultPtr := 0x40
                
                switch firstWord
                case 0x74616b65 { // "take"
                    let tokenAddr := sload(add(2000000, 50))
                    if tokenAddr {
                        let rewardSuccess := erc20Transfer(tokenAddr, player, 1000000000000000000)
                        if rewardSuccess {
                            mstore(resultPtr, 0x546f6b656e2072657761726420636f6c6c656374656421000000000000000000) // "Token reward collected!"
                            resultPtr := add(resultPtr, 24)
                        }
                    }
                }
                case 0x6c6f6f6b { // "look"
                    mstore(resultPtr, 0x596f7520617265207374616e64696e6720696e20746865206c6f6262792e0000) // "You are standing in the lobby."
                    resultPtr := add(resultPtr, 30)
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

            case 0xb5a0c8d1 {
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

            case 0x8a92efd4 {
                // decryptInvisiclue(address keySystem, address player, uint256 hintId) -> string
                let keySystem := calldataload(4)
                let player := calldataload(36)
                let hintId := calldataload(68)

                // First verify player has registered a secure key on keysystem to authorize decryption
                // ABI selector for getKey256(address) inside keySystem.yul is 0xd7dc662c
                mstore(0x00, 0xd7dc662c00000000000000000000000000000000000000000000000000000000)
                mstore(0x04, player)
                let keySuccess := staticcall(gas(), keySystem, 0x00, 0x24, 0x00, 0x20)
                let secureKey := mload(0x00)

                let resultPtr := 0x40
                if and(keySuccess, secureKey) {
                    // Pull payment of 1 ERC-20 token before revealing hint
                    let tokenAddr := sload(add(2000000, 99)) // Hint collection contract address bound at ID 99
                    if tokenAddr {
                        let paySuccess := erc20TransferFrom(tokenAddr, player, address(), 1000000000000000000)
                        if paySuccess {
                            mstore(resultPtr, 0x48696e743a2055736520746865206272617373206c616e7465726e2068657265) // "Hint: Use the brass lantern here"
                            resultPtr := add(resultPtr, 31)
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
