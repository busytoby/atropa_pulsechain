object "BtcRailsVM" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))
            
            // execute(bytes bytecode, uint256 cycles) -> returns (uint256 pc, uint256 sp, uint256 asp, uint256 halted)
            // Selector: 0x22137682
            if iszero(sub(selector, 0x22137682)) {
                let offset := add(4, calldataload(4))
                let len := calldataload(offset)
                let start := add(offset, 32)
                let cycles := calldataload(36)
                
                // Copy bytecode to memory at 0x1000
                calldatacopy(0x1000, start, len)
                
                // Set free memory pointer to 0x1100 (after bytecode area)
                mstore(0x40, 0x1100)
                
                // Dynamically allocate start boundaries for stack and altstack
                let stack_base := allocate_mem(32000)
                let altstack_base := allocate_mem(32000)
                
                // Virtual registers setup
                let regPC := 0
                let regSP := stack_base
                let regASP := altstack_base
                let regHalted := 0
                
                // Resolve stack limit: standard is 1000; LAU operators are allowed up to 32000
                let limit := 1000
                if is_lau_operator(caller()) {
                    limit := 32000
                }
                
                for { let cyc := 0 } and(and(lt(cyc, cycles), lt(regPC, len)), iszero(regHalted)) { cyc := add(cyc, 1) } {
                    let op := byte(0, mload(add(0x1000, regPC)))
                    regPC := add(regPC, 1)
                    
                    // Track combined elements count
                    let elements_count := add(div(sub(regSP, stack_base), 32), div(sub(regASP, altstack_base), 32))
                    if gt(elements_count, limit) {
                        regHalted := 1
                        break
                    }
                    
                    switch op
                    case 1 { // PUSH (read next 4 bytes as 32-bit int)
                        if lt(len, add(regPC, 4)) {
                            regHalted := 1
                            break
                        }
                        let val := shr(224, mload(add(0x1000, regPC)))
                        regPC := add(regPC, 4)
                        
                        mstore(regSP, val)
                        regSP := add(regSP, 32)
                    }
                    case 2 { // ADD
                        if lt(regSP, add(stack_base, 64)) {
                            regHalted := 1
                            break
                        }
                        regSP := sub(regSP, 32)
                        let b := mload(regSP)
                        regSP := sub(regSP, 32)
                        let a := mload(regSP)
                        mstore(regSP, add(a, b))
                        regSP := add(regSP, 32)
                    }
                    case 3 { // SUB
                        if lt(regSP, add(stack_base, 64)) {
                            regHalted := 1
                            break
                        }
                        regSP := sub(regSP, 32)
                        let b := mload(regSP)
                        regSP := sub(regSP, 32)
                        let a := mload(regSP)
                        mstore(regSP, sub(a, b))
                        regSP := add(regSP, 32)
                    }
                    case 4 { // TOALTSTACK
                        if lt(regSP, add(stack_base, 32)) {
                            regHalted := 1
                            break
                        }
                        regSP := sub(regSP, 32)
                        let val := mload(regSP)
                        mstore(regASP, val)
                        regASP := add(regASP, 32)
                    }
                    case 5 { // FROMALTSTACK
                        if lt(regASP, add(altstack_base, 32)) {
                            regHalted := 1
                            break
                        }
                        regASP := sub(regASP, 32)
                        let val := mload(regASP)
                        mstore(regSP, val)
                        regSP := add(regSP, 32)
                    }
                    case 6 { // HALT
                        regHalted := 1
                    }
                    case 7 { // VERIFY_ACH_ROUTING
                         if lt(regSP, add(stack_base, 32)) {
                             regHalted := 1
                             break
                         }
                         regSP := sub(regSP, 32)
                         let val := mload(regSP)
                         let temp := val
                         let d9 := mod(temp, 10)
                         temp := div(temp, 10)
                         let d8 := mod(temp, 10)
                         temp := div(temp, 10)
                         let d7 := mod(temp, 10)
                         temp := div(temp, 10)
                         let d6 := mod(temp, 10)
                         temp := div(temp, 10)
                         let d5 := mod(temp, 10)
                         temp := div(temp, 10)
                         let d4 := mod(temp, 10)
                         temp := div(temp, 10)
                         let d3 := mod(temp, 10)
                         temp := div(temp, 10)
                         let d2 := mod(temp, 10)
                         temp := div(temp, 10)
                         let d1 := mod(temp, 10)
                         let sum := add(
                             add(mul(3, add(add(d1, d4), d7)), mul(7, add(add(d2, d5), d8))),
                             add(add(d3, d6), d9)
                         )
                         let is_valid := 0
                         if iszero(mod(sum, 10)) {
                             is_valid := 1
                         }
                         mstore(regSP, is_valid)
                         regSP := add(regSP, 32)
                    }
                    default {
                        regHalted := 1
                    }
                }
                
                // Return final state registers
                mstore(0, regPC)
                mstore(32, regSP)
                mstore(64, regASP)
                mstore(96, regHalted)
                return(0, 128)
            }
            
            // peek(uint256 offset) -> returns (uint256 value)
            // Selector: 0x0f882f07
            if iszero(sub(selector, 0x0f882f07)) {
                let offset := calldataload(4)
                mstore(0, sload(offset))
                return(0, 32)
            }
            
            // poke(uint256 offset, uint256 value) -> returns (uint256 success)
            // Selector: 0xba972e3a
            if iszero(sub(selector, 0xba972e3a)) {
                let offset := calldataload(4)
                let value := calldataload(36)
                sstore(offset, value)
                mstore(0, 1)
                return(0, 32)
            }
            
            // approve_lau_operator(address operator, uint256 approved) -> returns (uint256 success)
            // Selector: 0xa9c3c1a2
            if iszero(sub(selector, 0xa9c3c1a2)) {
                let operator := calldataload(4)
                let approved := calldataload(36)
                
                mstore(0, operator)
                mstore(32, 12) // Prefix 12 for LAU operator approvals
                let slot := keccak256(0, 64)
                sstore(slot, approved)
                
                mstore(0, 1)
                return(0, 32)
            }
            
            // Helper function to allocate dynamic memory space
            function allocate_mem(size) -> addr {
                addr := mload(0x40)
                mstore(0x40, add(addr, size))
            }
            
            // Helper function to verify LAU operator approval
            function is_lau_operator(operator) -> res {
                mstore(0, operator)
                mstore(32, 12)
                let slot := keccak256(0, 64)
                res := sload(slot)
            }
            
            revert(0, 0)
        }
    }
}
