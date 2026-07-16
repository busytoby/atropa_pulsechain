object "RamacSystem" {
    code {
        if callvalue() { revert(0, 0) }
        datacopy(0x00, dataoffset("runtime"), datasize("runtime"))
        return(0x00, datasize("runtime"))
    }
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))

            // Helper to parse key and value from command payload starting at offset 68
            // Command layout in bytes: CMD(3 bytes) + space(1 byte) + KEY(until space or null) + space + VAL(until null)
            function getCmdParams(offset, len) -> cmdType, key, value {
                cmdType := shr(232, calldataload(offset)) // 3 bytes command (e.g. WRT, QRY, PRT)
                
                // Parse key (starts at offset + 4)
                let keyStart := add(offset, 4)
                key := calldataload(keyStart)
                
                // For WRT, parse value starting after the first word
                // Assuming key is padded to 32 bytes or separated:
                // Let's simply load value from keyStart + 32
                value := calldataload(add(keyStart, 32))
            }

            // ----------------------------------------------------------------
            // executeInquiry(bytes calldata cmd) -> bytes32 response
            // Selector: 0xe28e404f
            // ----------------------------------------------------------------
            if eq(selector, 0xe28e404f) {
                let cmdLen := calldataload(36)
                let cmdType, key, value := getCmdParams(68, cmdLen)

                // Define fixed cylinder 5 for RAMAC queries
                let cylinder := 5
                
                // WRT Command (ASCII "WRT" = 0x575254)
                if eq(cmdType, 0x575254) {
                    // Compute primary slot: hash(key) % 900
                    mstore(0x00, key)
                    let h := keccak256(0x00, 32)
                    let slot := mod(h, 900) // 45 tracks * 20 sectors

                    // Storage Slot mapping:
                    // base = keccak256(cylinder, slot)
                    mstore(0x00, cylinder)
                    mstore(0x20, slot)
                    let baseSlot := keccak256(0x00, 64)

                    let is_active := sload(add(baseSlot, 3))
                    if iszero(is_active) {
                        sstore(baseSlot, key)
                        sstore(add(baseSlot, 1), value)
                        sstore(add(baseSlot, 2), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) // next = -1
                        sstore(add(baseSlot, 3), 1) // active
                        
                        mstore(0x00, 1) // SUCCESS
                        return(0x00, 32)
                    }

                    // Collision resolution: Find free slot in overflow area (slots 900..999)
                    let found := 0
                    let overflow_slot := 900
                    for {} lt(overflow_slot, 1000) { overflow_slot := add(overflow_slot, 1) } {
                        mstore(0x00, cylinder)
                        mstore(0x20, overflow_slot)
                        let testSlot := keccak256(0x00, 64)
                        let test_active := sload(add(testSlot, 3))
                        if iszero(test_active) {
                            sstore(testSlot, key)
                            sstore(add(testSlot, 1), value)
                            sstore(add(testSlot, 2), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
                            sstore(add(testSlot, 3), 1)
                            
                            // Link previous node in chain to this overflow slot
                            // Traverse to end of chain starting at baseSlot
                            let curr := baseSlot
                            for {} 1 {} {
                                let next_ptr := sload(add(curr, 2))
                                if eq(next_ptr, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) {
                                    sstore(add(curr, 2), overflow_slot)
                                    break
                                }
                                // Compute slot key for next link
                                mstore(0x00, cylinder)
                                mstore(0x20, next_ptr)
                                curr := keccak256(0x00, 64)
                            }

                            found := 1
                            break
                        }
                    }

                    if found {
                        mstore(0x00, 2) // SUCCESS OVERFLOW
                        return(0x00, 32)
                    }
                    revert(0, 0)
                }

                // QRY Command (ASCII "QRY" = 0x515259)
                if eq(cmdType, 0x515259) {
                    mstore(0x00, key)
                    let h := keccak256(0x00, 32)
                    let slot := mod(h, 900)

                    mstore(0x00, cylinder)
                    mstore(0x20, slot)
                    let baseSlot := keccak256(0x00, 64)

                    let curr := baseSlot
                    let found := 0
                    let ret_val := 0

                    for {} 1 {} {
                        let active := sload(add(curr, 3))
                        if iszero(active) { break }
                        let k := sload(curr)
                        if eq(k, key) {
                            ret_val := sload(add(curr, 1))
                            found := 1
                            break
                        }
                        let next_ptr := sload(add(curr, 2))
                        if eq(next_ptr, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) {
                            break
                        }
                        mstore(0x00, cylinder)
                        mstore(0x20, next_ptr)
                        curr := keccak256(0x00, 64)
                    }

                    if found {
                        mstore(0x00, ret_val)
                        return(0x00, 32)
                    }
                    revert(0, 0)
                }

                revert(0, 0)
            }

            // ----------------------------------------------------------------
            // executeALU(bytes calldata program) -> uint256 final_acc1
            // Selector: 0xb1b6081e
            // ----------------------------------------------------------------
            if eq(selector, 0xb1b6081e) {
                let progLen := calldataload(36)
                let numInstructions := div(progLen, 32)
                let dataOffset := 68

                let p_counter := 0
                for {} lt(p_counter, numInstructions) {} {
                    let inst := calldataload(add(dataOffset, mul(p_counter, 32)))
                    let op := byte(0, inst)
                    let dest_acc := byte(1, inst)
                    let src_acc := byte(2, inst)
                    let is_const := byte(3, inst)
                    let val := and(inst, 0x00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff)

                    // Resolve operand value (constant or from register slot 0x1000 + src_acc)
                    let operand := val
                    if iszero(is_const) {
                        operand := sload(add(0x1000, src_acc))
                    }

                    // ADD (1)
                    if eq(op, 1) {
                        let current := sload(add(0x1000, dest_acc))
                        sstore(add(0x1000, dest_acc), add(current, operand))
                        p_counter := add(p_counter, 1)
                    }

                    // SUB (2)
                    if eq(op, 2) {
                        let current := sload(add(0x1000, dest_acc))
                        sstore(add(0x1000, dest_acc), sub(current, operand))
                        p_counter := add(p_counter, 1)
                    }

                    // DIV (3) - RULE 12 COMPLIANT INTERCEPT
                    if eq(op, 3) {
                        let current := sload(add(0x1000, dest_acc))
                        if iszero(operand) {
                            sstore(0x2000, current) // isolation_trap
                            sstore(0x2001, 1) // trap_active
                            revert(0, 0) // Intercept mathematical continuity failure
                        }
                        sstore(add(0x1000, dest_acc), div(current, operand))
                        p_counter := add(p_counter, 1)
                    }

                    // CMP (4)
                    if eq(op, 4) {
                        let current := sload(add(0x1000, dest_acc))
                        let cmp_flag := 0
                        if eq(current, operand) { cmp_flag := 0 }
                        if gt(current, operand) { cmp_flag := 1 }
                        if lt(current, operand) { cmp_flag := 2 }
                        sstore(0x2002, cmp_flag)
                        p_counter := add(p_counter, 1)
                    }

                    // JEQ (5)
                    if eq(op, 5) {
                        let flag := sload(0x2002)
                        if iszero(flag) {
                            p_counter := val // Jump directly to target p_counter instruction index
                        }
                        if flag {
                            p_counter := add(p_counter, 1)
                        }
                    }
                }

                // Return final ACC1 value
                mstore(0x00, sload(0x1001))
                return(0x00, 32)
            }

            revert(0, 0)
        }
    }
}
