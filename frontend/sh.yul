/*
 * Unix 1 Thompson Shell (sh) - EVM-Native Yul Implementation
 * 
 * Interprets command strings and forwards requests to Unix1Utils:
 * - "ls" -> calls ls()
 * - "cat <filename>" -> calls cat(nameHash)
 * - "rm <filename>" -> calls rm(nameHash)
 * - "write <filename> <word0> <word1>" -> calls write_file(nameHash, word0, word1)
 */

object "Unix1Shell" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    
    object "runtime" {
        code {
            switch shr(224, calldataload(0))
            
            // 0x3a4f6cf4: run_command(address utilsContract, bytes cmd) -> returns raw bytes from utility
            case 0x3a4f6cf4 {
                let utilsContract := calldataload(4)
                
                // Get offset and length of command bytes from calldata
                let cmdOffset := add(4, calldataload(36))
                let cmdLen := calldataload(cmdOffset)
                let cmdStart := add(cmdOffset, 32)
                
                // Copy command to memory at 0x100
                calldatacopy(0x100, cmdStart, cmdLen)
                
                // Find first space to isolate the command name
                let cmdWordEnd := findChar(0x100, cmdLen, 0x20) // space
                let cmdName := getCommandHash(0x100, cmdWordEnd)
                
                switch cmdName
                
                // "ls" (hash: 0x6c730000...)
                case 0x6c73000000000000000000000000000000000000000000000000000000000000 {
                    // Call ls() -> selector 0x8ab095c3
                    mstore(0x00, 0x8ab095c300000000000000000000000000000000000000000000000000000000)
                    let success := call(gas(), utilsContract, 0, 0x00, 4, 0x00, 0)
                    let size := returndatasize()
                    returndatacopy(0x00, 0, size)
                    return(0x00, size)
                }
                
                // "cat" (hash: 0x63617400...)
                case 0x6361740000000000000000000000000000000000000000000000000000000000 {
                    // Parse target filename hash
                    let argStart := add(add(0x100, cmdWordEnd), 1)
                    let argLen := sub(cmdLen, add(cmdWordEnd, 1))
                    let filenameHash := getCommandHash(argStart, argLen)
                    
                    // Call cat(nameHash) -> selector 0x24249a2a
                    mstore(0x00, 0x24249a2a00000000000000000000000000000000000000000000000000000000)
                    mstore(0x04, filenameHash)
                    let success := call(gas(), utilsContract, 0, 0x00, 36, 0x00, 0)
                    let size := returndatasize()
                    returndatacopy(0x00, 0, size)
                    return(0x00, size)
                }
                
                // "rm" (hash: 0x726d0000...)
                case 0x726d000000000000000000000000000000000000000000000000000000000000 {
                    let argStart := add(add(0x100, cmdWordEnd), 1)
                    let argLen := sub(cmdLen, add(cmdWordEnd, 1))
                    let filenameHash := getCommandHash(argStart, argLen)
                    
                    // Call rm(nameHash) -> selector 0x0f2fa44a
                    mstore(0x00, 0x0f2fa44a00000000000000000000000000000000000000000000000000000000)
                    mstore(0x04, filenameHash)
                    let success := call(gas(), utilsContract, 0, 0x00, 36, 0x00, 0)
                    let size := returndatasize()
                    returndatacopy(0x00, 0, size)
                    return(0x00, size)
                }
                
                default {
                    revert(0, 0)
                }
            }
            
            default {
                revert(0, 0)
            }
            
            /*
             * Helpers
             */
            function findChar(start, len, targetChar) -> index {
                index := len
                for { let i := 0 } lt(i, len) { i := add(i, 1) } {
                    let char := byte(0, mload(add(start, i)))
                    if eq(char, targetChar) {
                        index := i
                        break
                    }
                }
            }
            
            function getCommandHash(start, len) -> h {
                let temp := 0
                for { let i := 0 } lt(i, len) { i := add(i, 1) } {
                    let char := byte(0, mload(add(start, i)))
                    temp := or(temp, shl(sub(248, mul(i, 8)), char))
                }
                h := temp
            }
        }
    }
}
