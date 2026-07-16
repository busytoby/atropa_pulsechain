object "BtcErc20GasToken" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))
            
            // name() -> returns (string)
            // Selector: 0x06fdde03
            if iszero(sub(selector, 0x06fdde03)) {
                mstore(0, 32)
                mstore(32, 24)
                mstore(64, "Bitcoin Script Gas Token")
                return(0, 96)
            }
            
            // symbol() -> returns (string)
            // Selector: 0x95d89b41
            if iszero(sub(selector, 0x95d89b41)) {
                mstore(0, 32)
                mstore(32, 3)
                mstore(64, "BTC")
                return(0, 96)
            }
            
            // decimals() -> returns (uint8)
            // Selector: 0x313ce567
            if iszero(sub(selector, 0x313ce567)) {
                mstore(0, 8)
                return(0, 32)
            }
            
            // totalSupply() -> returns (uint256)
            // Selector: 0x18160ddd
            if iszero(sub(selector, 0x18160ddd)) {
                mstore(0, 2100000000000000)
                return(0, 32)
            }
            
            // balanceOf(address) -> returns (uint256)
            // Selector: 0x70a08231
            if iszero(sub(selector, 0x70a08231)) {
                let account := calldataload(4)
                mstore(0, sload(get_balance_slot(account)))
                return(0, 32)
            }
            
            // transfer(address, uint256) -> returns (bool)
            // Selector: 0xa9059cbb
            if iszero(sub(selector, 0xa9059cbb)) {
                let to := calldataload(4)
                let amount := calldataload(36)
                
                let from_slot := get_balance_slot(caller())
                let from_bal := sload(from_slot)
                if lt(from_bal, amount) { revert(0, 0) }
                
                sstore(from_slot, sub(from_bal, amount))
                let to_slot := get_balance_slot(to)
                sstore(to_slot, add(sload(to_slot), amount))
                
                // Emit standard ERC-20 Transfer log event (topic hash for Transfer(address,address,uint256))
                log3(0, 0, 0xddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef, caller(), to)
                
                mstore(0, 1)
                return(0, 32)
            }
            
            // approve(address, uint256) -> returns (bool)
            // Selector: 0x095ea7b3
            if iszero(sub(selector, 0x095ea7b3)) {
                let spender := calldataload(4)
                let amount := calldataload(36)
                
                let slot := get_allowance_slot(caller(), spender)
                sstore(slot, amount)
                
                // Approval log
                log3(0, 0, 0x8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b925, caller(), spender)
                
                mstore(0, 1)
                return(0, 32)
            }
            
            // allowance(address, address) -> returns (uint256)
            // Selector: 0xdd62ed3e
            if iszero(sub(selector, 0xdd62ed3e)) {
                let owner := calldataload(4)
                let spender := calldataload(36)
                
                let slot := get_allowance_slot(owner, spender)
                mstore(0, sload(slot))
                return(0, 32)
            }
            
            // transferFrom(address, address, uint256) -> returns (bool)
            // Selector: 0x23b872dd
            if iszero(sub(selector, 0x23b872dd)) {
                let from := calldataload(4)
                let to := calldataload(36)
                let amount := calldataload(68)
                
                let allowance_slot := get_allowance_slot(from, caller())
                let current_allowance := sload(allowance_slot)
                if lt(current_allowance, amount) { revert(0, 0) }
                
                let from_slot := get_balance_slot(from)
                let from_bal := sload(from_slot)
                if lt(from_bal, amount) { revert(0, 0) }
                
                sstore(allowance_slot, sub(current_allowance, amount))
                sstore(from_slot, sub(from_bal, amount))
                
                let to_slot := get_balance_slot(to)
                sstore(to_slot, add(sload(to_slot), amount))
                
                log3(0, 0, 0xddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef, from, to)
                
                mstore(0, 1)
                return(0, 32)
            }
            
            // deposit_utxo(address target, uint256 amount) -> returns (bytes32 utxo_id)
            // Selector: 0xcd6945be
            if iszero(sub(selector, 0xcd6945be)) {
                let target := calldataload(4)
                let amount := calldataload(36)
                
                mstore(0, target)
                mstore(32, amount)
                mstore(64, timestamp())
                let utxo_hash := keccak256(0, 96)
                
                mstore(0, utxo_hash)
                mstore(32, 20)
                sstore(keccak256(0, 64), amount)
                
                mstore(0, utxo_hash)
                mstore(32, 21)
                sstore(keccak256(0, 64), target)
                
                let bal_slot := get_balance_slot(target)
                sstore(bal_slot, add(sload(bal_slot), amount))
                
                mstore(0, utxo_hash)
                return(0, 32)
            }
            
            // spend_utxo(bytes32 utxo_hash, address recipient, uint256 amount, bytes witness_script) -> returns (uint256 success)
            // Selector: 0x48e11a62
            if iszero(sub(selector, 0x48e11a62)) {
                let utxo_hash := calldataload(4)
                let recipient := calldataload(36)
                let amount := calldataload(68)
                
                mstore(0, utxo_hash)
                mstore(32, 20)
                let amt_slot := keccak256(0, 64)
                let utxo_amount := sload(amt_slot)
                if iszero(eq(utxo_amount, amount)) { revert(0, 0) }
                
                mstore(0, utxo_hash)
                mstore(32, 21)
                let own_slot := keccak256(0, 64)
                let owner := sload(own_slot)
                
                let offset := add(4, calldataload(100))
                let len := calldataload(offset)
                let start := add(offset, 32)
                calldatacopy(0x1000, start, len)
                
                let verified := run_btc_verification_loop(len)
                if iszero(verified) { revert(0, 0) }
                
                sstore(amt_slot, 0)
                sstore(own_slot, 0)
                
                let owner_bal_slot := get_balance_slot(owner)
                let owner_bal := sload(owner_bal_slot)
                if lt(owner_bal, amount) { revert(0, 0) }
                sstore(owner_bal_slot, sub(owner_bal, amount))
                
                let rec_bal_slot := get_balance_slot(recipient)
                sstore(rec_bal_slot, add(sload(rec_bal_slot), amount))
                
                mstore(0, 1)
                return(0, 32)
            }
            
            // approve_lau_operator(address operator, uint256 approved) -> returns (uint256 success)
            // Selector: 0xa9c3c1a2
            if iszero(sub(selector, 0xa9c3c1a2)) {
                let operator := calldataload(4)
                let approved := calldataload(36)
                
                mstore(0, operator)
                mstore(32, 12)
                let slot := keccak256(0, 64)
                sstore(slot, approved)
                
                mstore(0, 1)
                return(0, 32)
            }
            
            // nested_transfer(address to, uint256 amount, uint256 btc_gas_fee, bytes witness_script) -> returns (uint256 success)
            // Selector: 0x9fa1cf25
            if iszero(sub(selector, 0x9fa1cf25)) {
                let to := calldataload(4)
                let amount := calldataload(36)
                let btc_gas_fee := calldataload(68)
                
                let total_cost := add(amount, btc_gas_fee)
                let from_slot := get_balance_slot(caller())
                let from_bal := sload(from_slot)
                if lt(from_bal, total_cost) { revert(0, 0) }
                
                // Copy witness script data
                let offset := add(4, calldataload(100))
                let len := calldataload(offset)
                let start := add(offset, 32)
                calldatacopy(0x1000, start, len)
                
                // Execute standard 2-stack validation challenge
                let verified := run_btc_verification_loop(len)
                if iszero(verified) { revert(0, 0) }
                
                // Deduct sender balance and credit recipient nested balance
                sstore(from_slot, sub(from_bal, total_cost))
                let to_slot := get_balance_slot(to)
                sstore(to_slot, add(sload(to_slot), amount))
                
                // STRICT ISOLATION: We modify internal balance sheet memory
                // but do NOT emit log3 Transfer events here (relying solely on BTC ledger tracking).
                
                mstore(0, 1)
                return(0, 32)
            }
            
            // Helper to resolve ERC20 balance storage slot
            function get_balance_slot(account) -> slot {
                mstore(0, account)
                mstore(32, 10)
                slot := keccak256(0, 64)
            }
            
            // Helper to resolve ERC20 allowance storage slot
            function get_allowance_slot(owner, spender) -> slot {
                mstore(0, spender)
                mstore(32, 11)
                let inner := keccak256(0, 64)
                mstore(0, owner)
                mstore(32, inner)
                slot := keccak256(0, 64)
            }
            
            // Verification interpreter loop
            function run_btc_verification_loop(len) -> success {
                let stack_ptr := 0x2000
                let altstack_ptr := 0x2200
                
                let limit := 1000
                mstore(0, caller())
                mstore(32, 12)
                let slot := keccak256(0, 64)
                if sload(slot) {
                    limit := 32000
                }
                
                let ip := 0
                for {} lt(ip, len) {} {
                    let op := byte(0, mload(add(0x1000, ip)))
                    ip := add(ip, 1)
                    
                    let total_elements := add(div(sub(stack_ptr, 0x2000), 32), div(sub(altstack_ptr, 0x2200), 32))
                    if gt(total_elements, limit) { revert(0, 0) }
                    
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
                        if lt(stack_ptr, 0x2040) { revert(0, 0) }
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
                        if lt(stack_ptr, 0x2040) { revert(0, 0) }
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
                        if lt(stack_ptr, 0x2020) { revert(0, 0) }
                        stack_ptr := sub(stack_ptr, 32)
                        let val := mload(stack_ptr)
                        mstore(altstack_ptr, val)
                        altstack_ptr := add(altstack_ptr, 32)
                        continue
                    }
                    
                    // OP_FROMALTSTACK
                    if iszero(sub(op, 0x6c)) {
                        if lt(altstack_ptr, 0x2220) { revert(0, 0) }
                        altstack_ptr := sub(altstack_ptr, 32)
                        let val := mload(altstack_ptr)
                        mstore(stack_ptr, val)
                        stack_ptr := add(stack_ptr, 32)
                        continue
                    }
                    
                    // OP_SHA256
                    if iszero(sub(op, 0xa8)) {
                        if lt(stack_ptr, 0x2020) { revert(0, 0) }
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
                        if lt(stack_ptr, 0x2040) { revert(0, 0) }
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
                
                if lt(stack_ptr, 0x2020) { revert(0, 0) }
                stack_ptr := sub(stack_ptr, 32)
                let final_res := mload(stack_ptr)
                success := iszero(iszero(final_res))
            }
            
            revert(0, 0)
        }
    }
}
