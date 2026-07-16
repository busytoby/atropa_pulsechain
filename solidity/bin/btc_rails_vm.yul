object "BtcRailsVM" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))
            
            // execute(bytes bytecode) -> returns (uint256 stack_count, uint256 altstack_count, int256[] memory_dump)
            // Selector: 0x9c3f25c7
            if iszero(sub(selector, 0x9c3f25c7)) {
                let offset := add(4, calldataload(4))
                let len := calldataload(offset)
                let start := add(offset, 32)
                
                // Copy expression/bytecode payload to memory at 0x1000
                calldatacopy(0x1000, start, len)
                
                // Set stack pointers in memory
                let stack_ptr := 0x2000
                let altstack_ptr := 0x2200
                
                let ip := 0
                for {} lt(ip, len) {} {
                    let op := byte(0, mload(add(0x1000, ip)))
                    ip := add(ip, 1)
                    
                    switch op
                    case 1 { // PUSH (read next 4 bytes as 32-bit int)
                        let val := shr(224, mload(add(0x1000, ip)))
                        ip := add(ip, 4)
                        
                        mstore(stack_ptr, val)
                        stack_ptr := add(stack_ptr, 32)
                    }
                    case 2 { // ADD
                        stack_ptr := sub(stack_ptr, 32)
                        let b := mload(stack_ptr)
                        stack_ptr := sub(stack_ptr, 32)
                        let a := mload(stack_ptr)
                        mstore(stack_ptr, add(a, b))
                        stack_ptr := add(stack_ptr, 32)
                    }
                    case 3 { // SUB
                        stack_ptr := sub(stack_ptr, 32)
                        let b := mload(stack_ptr)
                        stack_ptr := sub(stack_ptr, 32)
                        let a := mload(stack_ptr)
                        mstore(stack_ptr, sub(a, b))
                        stack_ptr := add(stack_ptr, 32)
                    }
                    case 4 { // TOALTSTACK
                        stack_ptr := sub(stack_ptr, 32)
                        let val := mload(stack_ptr)
                        mstore(altstack_ptr, val)
                        altstack_ptr := add(altstack_ptr, 32)
                    }
                    case 5 { // FROMALTSTACK
                        altstack_ptr := sub(altstack_ptr, 32)
                        let val := mload(altstack_ptr)
                        mstore(stack_ptr, val)
                        stack_ptr := add(stack_ptr, 32)
                    }
                    case 6 { // HALT
                        break
                    }
                    default {
                        revert(0, 0)
                    }
                }
                
                // Format return output
                let stack_count := div(sub(stack_ptr, 0x2000), 32)
                let altstack_count := div(sub(altstack_ptr, 0x2200), 32)
                
                let ret_offset := 0x3000
                mstore(ret_offset, stack_count)
                mstore(add(ret_offset, 32), altstack_count)
                
                let out_ptr := add(ret_offset, 64)
                let src := 0x2000
                for {} lt(src, stack_ptr) {} {
                    mstore(out_ptr, mload(src))
                    out_ptr := add(out_ptr, 32)
                    src := add(src, 32)
                }
                src := 0x2200
                for {} lt(src, altstack_ptr) {} {
                    mstore(out_ptr, mload(src))
                    out_ptr := add(out_ptr, 32)
                    src := add(src, 32)
                }
                
                return(ret_offset, sub(out_ptr, ret_offset))
            }
            revert(0, 0)
        }
    }
}
