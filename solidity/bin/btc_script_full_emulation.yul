object "BtcScriptFullEmulation" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))
            
            // balance_of(address account) -> returns (uint256 balance)
            // Selector: 0x70a08231
            if iszero(sub(selector, 0x70a08231)) {
                let account := calldataload(4)
                let slot := get_balance_slot(account)
                mstore(0, sload(slot))
                return(0, 32)
            }
            
            // deposit_utxo(address target, uint256 amount) -> returns (bytes32 utxo_id)
            // Selector: 0xcd6945be
            if iszero(sub(selector, 0xcd6945be)) {
                let target := calldataload(4)
                let amount := calldataload(36)
                
                // Generate a pseudo-UTXO hash based on target, amount, and chain state parameters
                mstore(0, target)
                mstore(32, amount)
                mstore(64, timestamp())
                let utxo_hash := keccak256(0, 96)
                
                // Store UTXO amount at slot: hash(utxo_hash, 0)
                mstore(0, utxo_hash)
                mstore(32, 0)
                let amount_slot := keccak256(0, 64)
                sstore(amount_slot, amount)
                
                // Store UTXO owner at slot: hash(utxo_hash, 1)
                mstore(0, utxo_hash)
                mstore(32, 1)
                let owner_slot := keccak256(0, 64)
                sstore(owner_slot, target)
                
                // Increase target overall balance
                let balance_slot := get_balance_slot(target)
                sstore(balance_slot, add(sload(balance_slot), amount))
                
                mstore(0, utxo_hash)
                return(0, 32)
            }
            
            // spend_utxo(bytes32 utxo_hash, address recipient, uint256 amount, bytes witness_script) -> returns (uint256 success)
            // Selector: 0x48e11a62
            if iszero(sub(selector, 0x48e11a62)) {
                let utxo_hash := calldataload(4)
                let recipient := calldataload(36)
                let amount := calldataload(68)
                
                // Verify UTXO exists and matches amount
                mstore(0, utxo_hash)
                mstore(32, 0)
                let amount_slot := keccak256(0, 64)
                let utxo_amount := sload(amount_slot)
                if iszero(eq(utxo_amount, amount)) { revert(0, 0) }
                
                // Fetch UTXO owner
                mstore(0, utxo_hash)
                mstore(32, 1)
                let owner_slot := keccak256(0, 64)
                let owner := sload(owner_slot)
                
                // Load witness script payload
                let offset := add(4, calldataload(100))
                let len := calldataload(offset)
                let start := add(offset, 32)
                
                // Copy witness script to memory at 0x1000
                calldatacopy(0x1000, start, len)
                
                // Execute standard 2-stack verification challenge
                let verified := run_btc_verification_loop(len)
                if iszero(verified) { revert(0, 0) }
                
                // Nullify/Consume UTXO to prevent double spending
                sstore(amount_slot, 0)
                sstore(owner_slot, 0)
                
                // Shift balance to recipient
                let owner_bal_slot := get_balance_slot(owner)
                sstore(owner_bal_slot, sub(sload(owner_bal_slot), amount))
                
                let rec_bal_slot := get_balance_slot(recipient)
                sstore(rec_bal_slot, add(sload(rec_bal_slot), amount))
                
                mstore(0, 1) // return success (true)
                return(0, 32)
            }
            
            // Helper function to resolve balance slots
            function get_balance_slot(account) -> slot {
                mstore(0, account)
                mstore(32, 9) // Storage offset prefix 9 for balance mappings
                slot := keccak256(0, 64)
            }
            
            // Verification interpreter loop
            function run_btc_verification_loop(len) -> success {
                let stack_ptr := 0x2000
                let altstack_ptr := 0x2200
                
                let ip := 0
                for {} lt(ip, len) {} {
                    let op := byte(0, mload(add(0x1000, ip)))
                    ip := add(ip, 1)
                    
                    // OP_PUSH
                    if iszero(sub(op, 0x01)) {
                        let val := shr(224, mload(add(0x1000, ip)))
                        ip := add(ip, 4)
                        mstore(stack_ptr, val)
                        stack_ptr := add(stack_ptr, 32)
                        continue
                    }
                    
                    // OP_ADD
                    if iszero(sub(op, 0x93)) {
                        stack_ptr := sub(stack_ptr, 32)
                        let b := mload(stack_ptr)
                        stack_ptr := sub(stack_ptr, 32)
                        let a := mload(stack_ptr)
                        mstore(stack_ptr, add(a, b))
                        stack_ptr := add(stack_ptr, 32)
                        continue
                    }
                    
                    // OP_SUB
                    if iszero(sub(op, 0x94)) {
                        stack_ptr := sub(stack_ptr, 32)
                        let b := mload(stack_ptr)
                        stack_ptr := sub(stack_ptr, 32)
                        let a := mload(stack_ptr)
                        mstore(stack_ptr, sub(a, b))
                        stack_ptr := add(stack_ptr, 32)
                        continue
                    }
                    
                    // OP_TOALTSTACK
                    if iszero(sub(op, 0x6b)) {
                        stack_ptr := sub(stack_ptr, 32)
                        let val := mload(stack_ptr)
                        mstore(altstack_ptr, val)
                        altstack_ptr := add(altstack_ptr, 32)
                        continue
                    }
                    
                    // OP_FROMALTSTACK
                    if iszero(sub(op, 0x6c)) {
                        altstack_ptr := sub(altstack_ptr, 32)
                        let val := mload(altstack_ptr)
                        mstore(stack_ptr, val)
                        stack_ptr := add(stack_ptr, 32)
                        continue
                    }
                    
                    // OP_SHA256
                    if iszero(sub(op, 0xa8)) {
                        stack_ptr := sub(stack_ptr, 32)
                        let val := mload(stack_ptr)
                        mstore(0x3000, val)
                        let success_call := staticcall(gas(), 2, 0x3000, 32, stack_ptr, 32)
                        if iszero(success_call) { revert(0, 0) }
                        stack_ptr := add(stack_ptr, 32)
                        continue
                    }
                    
                    // OP_EQUALVERIFY
                    if iszero(sub(op, 0x88)) {
                        stack_ptr := sub(stack_ptr, 32)
                        let b := mload(stack_ptr)
                        stack_ptr := sub(stack_ptr, 32)
                        let a := mload(stack_ptr)
                        if iszero(eq(a, b)) { revert(0, 0) }
                        continue
                    }
                    
                    // OP_HALT
                    if iszero(op) {
                        break
                    }
                    
                    revert(0, 0)
                }
                
                // Return verification success if stack leaves 1 (True)
                stack_ptr := sub(stack_ptr, 32)
                let final_res := mload(stack_ptr)
                success := eq(final_res, 1)
            }
            
            revert(0, 0)
        }
    }
}
