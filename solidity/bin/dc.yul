/*
 * Unix 1 Desk Calculator (dc) - EVM-Native Yul Implementation
 * 
 * Implements a postfix (RPN) stack-based arithmetic calculator.
 * Supports space-separated integers and basic operators (+, -, *, /).
 */

object "Unix1DC" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    
    object "runtime" {
        code {
            switch shr(224, calldataload(0))
            
            // 0x5b3f2ea0: execute_dc(bytes expr) -> returns uint256 (result on top of stack)
            case 0x5b3f2ea0 {
                let offset := add(4, calldataload(4))
                let len := calldataload(offset)
                let start := add(offset, 32)
                
                // Copy expression to memory at 0x100
                calldatacopy(0x100, start, len)
                
                // Stack starts at memory offset 0x1000
                let stackPtr := 0x1000
                
                let i := 0
                for {} lt(i, len) {} {
                    let char := byte(0, mload(add(0x100, i)))
                    
                    // Skip spaces
                    if eq(char, 0x20) {
                        i := add(i, 1)
                        continue
                    }
                    
                    // Check if it is an operator
                    switch char
                    
                    // '+' (addition)
                    case 0x2b {
                        stackPtr := sub(stackPtr, 32)
                        let a := mload(stackPtr)
                        stackPtr := sub(stackPtr, 32)
                        let b := mload(stackPtr)
                        mstore(stackPtr, add(b, a))
                        stackPtr := add(stackPtr, 32)
                        i := add(i, 1)
                    }
                    
                    // '-' (subtraction)
                    case 0x2d {
                        stackPtr := sub(stackPtr, 32)
                        let a := mload(stackPtr)
                        stackPtr := sub(stackPtr, 32)
                        let b := mload(stackPtr)
                        mstore(stackPtr, sub(b, a))
                        stackPtr := add(stackPtr, 32)
                        i := add(i, 1)
                    }
                    
                    // '*' (multiplication)
                    case 0x2a {
                        stackPtr := sub(stackPtr, 32)
                        let a := mload(stackPtr)
                        stackPtr := sub(stackPtr, 32)
                        let b := mload(stackPtr)
                        mstore(stackPtr, mul(b, a))
                        stackPtr := add(stackPtr, 32)
                        i := add(i, 1)
                    }
                    
                    // '/' (division)
                    case 0x2f {
                        stackPtr := sub(stackPtr, 32)
                        let a := mload(stackPtr)
                        stackPtr := sub(stackPtr, 32)
                        let b := mload(stackPtr)
                        if iszero(a) { revert(0, 0) }
                        mstore(stackPtr, div(b, a))
                        stackPtr := add(stackPtr, 32)
                        i := add(i, 1)
                    }
                    
                    // Assume it's a number
                    default {
                        let val := 0
                        let digitCount := 0
                        for {} lt(add(i, digitCount), len) {} {
                            let digit := byte(0, mload(add(0x100, add(i, digitCount))))
                            if lt(digit, 0x30) { break }
                            if gt(digit, 0x39) { break }
                            val := add(mul(val, 10), sub(digit, 0x30))
                            digitCount := add(digitCount, 1)
                        }
                        
                        if iszero(digitCount) {
                            revert(0, 0) // Invalid character
                        }
                        
                        mstore(stackPtr, val)
                        stackPtr := add(stackPtr, 32)
                        i := add(i, digitCount)
                    }
                }
                
                // Return top value on the stack
                let finalVal := mload(sub(stackPtr, 32))
                mstore(0x00, finalVal)
                return(0x00, 0x20)
            }
            
            default {
                revert(0, 0)
            }
        }
    }
}
