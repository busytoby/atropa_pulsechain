object "NelsonRPN" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))
            
            // execute(bytes expr) -> returns (uint256 x, uint256 y, uint256 z, uint256 t)
            // selector: 0x09c5eabe
            if eq(selector, 0x09c5eabe) {
                let offset := add(4, calldataload(4))
                let len := calldataload(offset)
                let start := add(offset, 32)
                
                // Copy expression to memory at 0x100
                calldatacopy(0x100, start, len)
                
                let x := 0
                let y := 0
                let z := 0
                let t := 0
                let lastX := 0
                let insideNumber := 0
                
                let i := 0
                for {} lt(i, len) {} {
                    let char := byte(0, mload(add(0x100, i)))
                    
                    // Skip spaces
                    if eq(char, 0x20) {
                        insideNumber := 0
                        i := add(i, 1)
                        continue
                    }
                    
                    // Check if it is a digit
                    if and(gt(char, 0x2f), lt(char, 0x3a)) {
                        if iszero(insideNumber) {
                            // Lift stack before starting new number
                            t := z
                            z := y
                            y := x
                            x := 0
                            insideNumber := 1
                        }
                        x := add(mul(x, 10), sub(char, 0x30))
                        i := add(i, 1)
                        continue
                    }
                    
                    // Standard operations
                    switch char
                    
                    // '+' (addition)
                    case 0x2b {
                        insideNumber := 0
                        lastX := x
                        x := add(y, x)
                        y := z
                        z := t
                        i := add(i, 1)
                    }
                    
                    // '-' (subtraction)
                    case 0x2d {
                        insideNumber := 0
                        lastX := x
                        x := sub(y, x)
                        y := z
                        z := t
                        i := add(i, 1)
                    }
                    
                    // '*' (multiplication)
                    case 0x2a {
                        insideNumber := 0
                        lastX := x
                        x := mul(y, x)
                        y := z
                        z := t
                        i := add(i, 1)
                    }
                    
                    // '/' (division)
                    case 0x2f {
                        insideNumber := 0
                        lastX := x
                        if iszero(x) { revert(0, 0) }
                        x := div(y, x)
                        y := z
                        z := t
                        i := add(i, 1)
                    }
                    
                    // 'E' or 'e' (Enter)
                    case 0x45 {
                        insideNumber := 0
                        t := z
                        z := y
                        y := x
                        i := add(i, 1)
                    }
                    case 0x65 {
                        insideNumber := 0
                        t := z
                        z := y
                        y := x
                        i := add(i, 1)
                    }
                    
                    // 'R' or 'r' (Roll down)
                    case 0x52 {
                        insideNumber := 0
                        let temp := x
                        x := y
                        y := z
                        z := t
                        t := temp
                        i := add(i, 1)
                    }
                    case 0x72 {
                        insideNumber := 0
                        let temp := x
                        x := y
                        y := z
                        z := t
                        t := temp
                        i := add(i, 1)
                    }
                    
                    // 'S' or 's' (Swap X and Y)
                    case 0x53 {
                        insideNumber := 0
                        let temp := x
                        x := y
                        y := temp
                        i := add(i, 1)
                    }
                    case 0x73 {
                        insideNumber := 0
                        let temp := x
                        x := y
                        y := temp
                        i := add(i, 1)
                    }
                    
                    // 'C' or 'c' (CLX)
                    case 0x43 {
                        insideNumber := 0
                        x := 0
                        i := add(i, 1)
                    }
                    case 0x63 {
                        insideNumber := 0
                        x := 0
                        i := add(i, 1)
                    }
                    
                    // 'X' or 'x' (Clear Stack)
                    case 0x58 {
                        insideNumber := 0
                        x := 0
                        y := 0
                        z := 0
                        t := 0
                        i := add(i, 1)
                    }
                    case 0x78 {
                        insideNumber := 0
                        x := 0
                        y := 0
                        z := 0
                        t := 0
                        i := add(i, 1)
                    }
                    
                    default {
                        revert(0, 0)
                    }
                }
                
                // Return registers X, Y, Z, T
                mstore(0x00, x)
                mstore(0x20, y)
                mstore(0x40, z)
                mstore(0x60, t)
                return(0x00, 0x80)
            }
            
            revert(0, 0)
        }
    }
}
