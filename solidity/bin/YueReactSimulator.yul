object "YueReactSimulator" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))
            
            // React(address Qing) -> returns (uint256)
            // Selector signature: 0x7ac9e05f
            if eq(selector, 0x7ac9e05f) {
                let qing := calldataload(4)
                let qing_clean := and(qing, 0xffffffffffffffffffffffffffffffffffffffff)
                
                let omicron := 0
                let omega := 0
                
                // Read Fomalhaute and Fornax balances from storage using the caller's address mapping key
                // Storage Slot 50 holds Fomalhaute balances: mapping(address => uint256)
                mstore(0, origin())
                mstore(32, 50)
                let fom_bal_slot := keccak256(0, 64)
                let fom_bal := sload(fom_bal_slot)
                
                // Storage Slot 51 holds Fornax balances: mapping(address => uint256)
                mstore(0, origin())
                mstore(32, 51)
                let fornax_bal_slot := keccak256(0, 64)
                let fornax_bal := sload(fornax_bal_slot)
                
                // Fomalhaute SHIO contract address
                if or(eq(qing_clean, 0x00000000000000000000000000000000f0ba1ab0), eq(qing_clean, 0x203e366a1821570b2f84ff5ae8b3bdeb48dc4fa1)) {
                    if gt(fom_bal, 0) {
                        // Formula: base coordinates dynamically scale with balance
                        omicron := div(fom_bal, 10) // Fomalhaut B Flare Epibar
                        omega := div(fom_bal, 3)    // Fomalhaut A Dominance Hypobar
                    }
                }
                
                // Fornax SHIO contract address
                if eq(qing_clean, 0x00000000000000000000000000000000f074a1e0) {
                    if gt(fornax_bal, 0) {
                        omicron := div(fornax_bal, 20) // Epibar
                        omega := div(fornax_bal, 5)    // Hypobar
                    }
                }

                // URGENT Qing address (0x0000000000000000000000000000000000009999)
                if eq(qing_clean, 0x0000000000000000000000000000000000009999) {
                    mstore(0, qing_clean)
                    mstore(32, 52)
                    let urg_bal := sload(keccak256(0, 64))
                    if gt(urg_bal, 0) {
                        omicron := div(urg_bal, 2)  // Epibar
                        omega := div(urg_bal, 1)    // Hypobar
                    }
                }
                
                // Store Hypobar (slot 11)
                mstore(0, qing_clean)
                mstore(32, 11)
                let hypo_slot := keccak256(0, 64)
                let current_hypo := sload(hypo_slot)
                sstore(hypo_slot, add(current_hypo, omega))
                
                // Store Epibar (slot 12)
                mstore(32, 12)
                let epi_slot := keccak256(0, 64)
                let current_epi := sload(epi_slot)
                sstore(epi_slot, add(current_epi, omicron))
                
                mstore(0, 100)
                return(0, 32)
            }

            // Bar(address Qing) -> returns (uint256 Hypogram, uint256 Epigram)
            // Selector signature: 0x0ebc8577
            if eq(selector, 0x0ebc8577) {
                let qing := calldataload(4)
                let qing_clean := and(qing, 0xffffffffffffffffffffffffffffffffffffffff)
                
                // Get Hypobar (slot 11)
                mstore(0, qing_clean)
                mstore(32, 11)
                let hypo_slot := keccak256(0, 64)
                let hypo_val := sload(hypo_slot)
                
                // Get Epibar (slot 12)
                mstore(32, 12)
                let epi_slot := keccak256(0, 64)
                let epi_val := sload(epi_slot)
                
                mstore(0, hypo_val)
                mstore(32, epi_val)
                return(0, 64)
            }

            // balanceOf(address account) -> returns (uint256)
            // Selector signature: 0x70a08231
            if eq(selector, 0x70a08231) {
                let account := calldataload(4)
                let account_clean := and(account, 0xffffffffffffffffffffffffffffffffffffffff)
                
                // Check Fomalhaute balance (slot 50)
                mstore(0, account_clean)
                mstore(32, 50)
                let fom_bal := sload(keccak256(0, 64))
                
                // Check Fornax balance (slot 51)
                mstore(32, 51)
                let fornax_bal := sload(keccak256(0, 64))

                // Check URGENT balance (slot 52)
                mstore(32, 52)
                let urg_bal := sload(keccak256(0, 64))
                
                let total_bal := add(add(fom_bal, fornax_bal), urg_bal)
                mstore(0, total_bal)
                return(0, 32)
            }
            
            // setBalance(address account, uint256 amount, uint256 slot_id) -> custom test helper
            // Selector signature: 0xf221a28a
            if eq(selector, 0xf221a28a) {
                let account := calldataload(4)
                let amount := calldataload(36)
                let slot_id := calldataload(68)
                let account_clean := and(account, 0xffffffffffffffffffffffffffffffffffffffff)
                
                mstore(0, account_clean)
                mstore(32, slot_id)
                sstore(keccak256(0, 64), amount)
                return(0, 0)
            }
        }
    }
}
