object "AuntSallyParser" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))
            
            // evaluate(bytes expr) -> returns (int256 result)
            // selector: 0x1658af37
            if eq(selector, 0x1658af37) {
                let offset := add(4, calldataload(4))
                let len := calldataload(offset)
                let start := add(offset, 32)
                
                // Copy expression to memory at 0x1000
                calldatacopy(0x1000, start, len)
                
                // Stacks:
                // valStack: stores int256 values. Ptr starts at 0x2000
                // opStack: stores uint256 operator chars. Ptr starts at 0x3000
                let valPtr := 0x2000
                let opPtr := 0x3000
                
                function valPush(val) {
                    mstore(mload(0x40), val) // Use 0x40 to temp save, wait let's just write to ptr
                }
                
                // Let's implement stack helpers inline or with functions passing ptrs
                // Since Yul functions cannot access outer variables, we pass/return ptrs:
                function pushVal(ptr, val) -> newPtr {
                    mstore(ptr, val)
                    newPtr := add(ptr, 32)
                }
                
                function popVal(ptr) -> newPtr, val {
                    newPtr := sub(ptr, 32)
                    val := mload(newPtr)
                }
                
                function pushOp(ptr, op) -> newPtr {
                    mstore(ptr, op)
                    newPtr := add(ptr, 32)
                }
                
                function popOp(ptr) -> newPtr, op {
                    newPtr := sub(ptr, 32)
                    op := mload(newPtr)
                }
                
                function getPrecedence(op) -> prec {
                    prec := 0
                    if eq(op, 0x2b) { prec := 1 } // '+'
                    if eq(op, 0x2d) { prec := 1 } // '-'
                    if eq(op, 0x2a) { prec := 2 } // '*'
                    if eq(op, 0x2f) { prec := 2 } // '/'
                }
                
                function applyOp(op, a, b) -> res {
                    switch op
                    case 0x2b { res := add(a, b) } // '+'
                    case 0x2d { res := sub(a, b) } // '-'
                    case 0x2a { res := mul(a, b) } // '*'
                    case 0x2f {
                        if iszero(b) { revert(0, 0) }
                        res := div(a, b)
                    } // '/'
                    default { revert(0, 0) }
                }
                
                let i := 0
                for {} lt(i, len) {} {
                    let char := byte(0, mload(add(0x1000, i)))
                    
                    // Skip spaces
                    if eq(char, 0x20) {
                        i := add(i, 1)
                        continue
                    }
                    
                    // Read digit
                    if and(gt(char, 0x2f), lt(char, 0x3a)) {
                        let val := 0
                        for {} and(lt(i, len), and(gt(char, 0x2f), lt(char, 0x3a))) {} {
                            val := add(mul(val, 10), sub(char, 0x30))
                            i := add(i, 1)
                            if lt(i, len) {
                                char := byte(0, mload(add(0x1000, i)))
                            }
                        }
                        valPtr := pushVal(valPtr, val)
                        continue
                    }
                    
                    // Left parenthesis
                    if eq(char, 0x28) {
                        opPtr := pushOp(opPtr, char)
                        i := add(i, 1)
                        continue
                    }
                    
                    // Right parenthesis
                    if eq(char, 0x29) {
                        for {} gt(opPtr, 0x3000) {} {
                            let nextOpPtr := 0
                            let op := 0
                            nextOpPtr, op := popOp(opPtr)
                            if eq(op, 0x28) { // '('
                                opPtr := nextOpPtr
                                break
                            }
                            
                            // Pop operands and apply
                            let nextValPtr := 0
                            let b := 0
                            let a := 0
                            nextValPtr, b := popVal(valPtr)
                            nextValPtr, a := popVal(nextValPtr)
                            let res := applyOp(op, a, b)
                            valPtr := pushVal(nextValPtr, res)
                            opPtr := nextOpPtr
                        }
                        i := add(i, 1)
                        continue
                    }
                    
                    // Standard Operators
                    let isOp := or(or(eq(char, 0x2b), eq(char, 0x2d)), or(eq(char, 0x2a), eq(char, 0x2f)))
                    if isOp {
                        for {} gt(opPtr, 0x3000) {} {
                            let topOp := mload(sub(opPtr, 32))
                            if lt(getPrecedence(topOp), getPrecedence(char)) {
                                break
                            }
                            
                            let nextOpPtr := 0
                            let op := 0
                            nextOpPtr, op := popOp(opPtr)
                            
                            let nextValPtr := 0
                            let b := 0
                            let a := 0
                            nextValPtr, b := popVal(valPtr)
                            nextValPtr, a := popVal(nextValPtr)
                            let res := applyOp(op, a, b)
                            valPtr := pushVal(nextValPtr, res)
                            opPtr := nextOpPtr
                        }
                        opPtr := pushOp(opPtr, char)
                        i := add(i, 1)
                        continue
                    }
                    
                    // Unsupported character
                    revert(0, 0)
                }
                
                // Process remaining operators
                for {} gt(opPtr, 0x3000) {} {
                    let nextOpPtr := 0
                    let op := 0
                    nextOpPtr, op := popOp(opPtr)
                    
                    let nextValPtr := 0
                    let b := 0
                    let a := 0
                    nextValPtr, b := popVal(valPtr)
                    nextValPtr, a := popVal(nextValPtr)
                    let res := applyOp(op, a, b)
                    valPtr := pushVal(nextValPtr, res)
                    opPtr := nextOpPtr
                }
                
                // Return final result
                let finalVal := mload(sub(valPtr, 32))
                mstore(0x00, finalVal)
                return(0x00, 32)
            }
            
            revert(0, 0)
        }
    }
}
