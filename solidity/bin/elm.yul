object "EquationalLogicMonitor" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }

    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))

            // Helpers for storage variables
            function getVar(char) -> val {
                val := sload(add(80000, char))
            }
            function setVar(char, val) {
                sstore(add(80000, char), val)
            }

            function isSpace(c) -> space {
                space := or(or(eq(c, 32), eq(c, 9)), or(eq(c, 10), eq(c, 13)))
            }
            function isLetter(c) -> letter {
                letter := and(iszero(lt(c, 65)), iszero(gt(c, 90)))
            }
            function isDigit(c) -> digit {
                digit := and(iszero(lt(c, 48)), iszero(gt(c, 57)))
            }

            // Helper to parse operand (either letter variable or integer)
            // returns (value, nextIdx)
            function parseOperand(startIdx, limit, codeStart) -> val, nextIdx {
                nextIdx := startIdx
                // Skip spaces
                for { } lt(nextIdx, limit) { nextIdx := add(nextIdx, 1) } {
                    let c := byte(0, calldataload(add(codeStart, nextIdx)))
                    if iszero(isSpace(c)) { break }
                }

                if lt(nextIdx, limit) {
                    let c := byte(0, calldataload(add(codeStart, nextIdx)))
                    
                    if isLetter(c) {
                        val := getVar(c)
                        nextIdx := add(nextIdx, 1)
                    }
                    if iszero(isLetter(c)) {
                        if isDigit(c) {
                            val := 0
                            for { } lt(nextIdx, limit) { } {
                                let digitVal := byte(0, calldataload(add(codeStart, nextIdx)))
                                if iszero(isDigit(digitVal)) { break }
                                val := add(mul(val, 10), sub(digitVal, 48))
                                nextIdx := add(nextIdx, 1)
                            }
                        }
                    }
                }
            }

            // 1. getVariable(string name) -> int256
            // Selector: 0x966ac280
            if eq(selector, 0x966ac280) {
                let nameOffset := calldataload(4)
                let nameLen := calldataload(add(4, nameOffset))
                let nameStart := add(36, nameOffset)
                
                let val := 0
                if gt(nameLen, 0) {
                    let char := byte(0, calldataload(nameStart))
                    val := getVar(char)
                }
                mstore(0x00, val)
                return(0x00, 32)
            }

            // 2. evalStatement(string statement) -> (string name, int256 value, bool isEquation)
            // Selector: 0x2a1dd433
            if eq(selector, 0x2a1dd433) {
                let stOffset := calldataload(4)
                let stLen := calldataload(add(4, stOffset))
                let stStart := add(36, stOffset)

                let i := 0
                // Skip spaces
                for { } lt(i, stLen) { i := add(i, 1) } {
                    let c := byte(0, calldataload(add(stStart, i)))
                    if iszero(isSpace(c)) { break }
                }

                let isEq := 0
                let targetVar := 0

                // Check if it is an assignment: e.g. "A ="
                if lt(add(i, 1), stLen) {
                    let c0 := byte(0, calldataload(add(stStart, i)))
                    if isLetter(c0) {
                        let next := add(i, 1)
                        // Skip spaces
                        for { } lt(next, stLen) { next := add(next, 1) } {
                            let c := byte(0, calldataload(add(stStart, next)))
                            if iszero(isSpace(c)) { break }
                        }
                        if lt(next, stLen) {
                            let c := byte(0, calldataload(add(stStart, next)))
                            if eq(c, 61) { // '=' is 61
                                isEq := 1
                                targetVar := c0
                                i := add(next, 1) // skip '='
                            }
                        }
                    }
                }

                // Parse first operand
                let val1 := 0
                let nextIdx := 0
                val1, nextIdx := parseOperand(i, stLen, stStart)
                i := nextIdx

                // Skip spaces
                for { } lt(i, stLen) { i := add(i, 1) } {
                    let c := byte(0, calldataload(add(stStart, i)))
                    if iszero(isSpace(c)) { break }
                }

                let finalVal := val1

                // Check for operator
                if lt(i, stLen) {
                    let op := byte(0, calldataload(add(stStart, i)))
                    
                    if or(or(eq(op, 43), eq(op, 45)), or(eq(op, 42), eq(op, 47))) {
                        // Skip operator
                        i := add(i, 1)
                        // Parse second operand
                        let val2 := 0
                        val2, nextIdx := parseOperand(i, stLen, stStart)
                        i := nextIdx

                        // Evaluate expression
                        if eq(op, 43) { finalVal := add(val1, val2) } // '+'
                        if eq(op, 45) { finalVal := sub(val1, val2) } // '-'
                        if eq(op, 42) { finalVal := mul(val1, val2) } // '*'
                        if eq(op, 47) {
                            if iszero(val2) { revert(0, 0) }
                            finalVal := div(val1, val2)
                        } // '/'
                    }
                }

                // If assignment, save it
                if isEq {
                    setVar(targetVar, finalVal)
                }

                // Return ABI layout:
                // Word 0: Offset to string name (0x60)
                // Word 1: value (finalVal)
                // Word 2: isEquation (isEq)
                // Word 3: string name length
                // Word 4: string name characters (padded to 32 bytes)
                mstore(0x00, 0x60) // Offset to name string
                mstore(0x20, finalVal)
                mstore(0x40, isEq)
                
                if isEq {
                    mstore(0x60, 1) // string length
                    mstore8(0x80, targetVar)
                    return(0x00, 160) // return 5 words (160 bytes)
                }
                if iszero(isEq) {
                    mstore(0x60, 0) // string length
                    return(0x00, 128) // return 4 words (128 bytes)
                }
            }

            revert(0, 0)
        }
    }
}
