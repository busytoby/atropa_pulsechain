object "MurogaEngelbart" {
    // ========================================================================
    // DEPLOYMENT INITIALIZATION BLOCK (INITCODE)
    // ========================================================================
    code {
        if callvalue() { revert(0, 0) }

        // Copy runtime bytecode into memory at slot 0x00
        datacopy(0x00, dataoffset("runtime"), datasize("runtime"))
        
        // Return runtime bytecode
        return(0x00, datasize("runtime"))
    }
    
    // ========================================================================
    // MAIN PRODUCTION RUNTIME ENVIRONMENT
    // ========================================================================
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))
            
            // Selector 0x11110000: muroga_threshold_gate
            // Inputs:
            // - calldataload(4): offset of inputs array
            // - calldataload(36): offset of weights array
            // - calldataload(68): count of elements
            // - calldataload(100): threshold T
            if eq(selector, 0x11110000) {
                let inputs_offset := add(4, calldataload(4))
                let weights_offset := add(4, calldataload(36))
                let count := calldataload(68)
                let threshold := calldataload(100)
                
                let weighted_sum := 0
                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    let input_val := calldataload(add(inputs_offset, mul(i, 32)))
                    let weight_val := calldataload(add(weights_offset, mul(i, 32)))
                    weighted_sum := add(weighted_sum, mul(input_val, weight_val))
                }
                
                let output := 0
                if gte(weighted_sum, threshold) {
                    output := 1
                }
                
                mstore(0x00, output)
                return(0x00, 32)
            }
            
            // Selector 0x22220000: peek_a_boo_card_match
            // Inputs:
            // - calldataload(4): offset of card_a array
            // - calldataload(36): offset of card_b array
            // - calldataload(68): word_count
            if eq(selector, 0x22220000) {
                let card_a_offset := add(4, calldataload(4))
                let card_b_offset := add(4, calldataload(36))
                let word_count := calldataload(68)
                
                let holes_count := 0
                for { let i := 0 } lt(i, word_count) { i := add(i, 1) } {
                    let word_a := calldataload(add(card_a_offset, mul(i, 32)))
                    let word_b := calldataload(add(card_b_offset, mul(i, 32)))
                    let match_word := and(word_a, word_b)
                    
                    // Count set bits in match_word
                    for {} gt(match_word, 0) {} {
                        if and(match_word, 1) {
                            holes_count := add(holes_count, 1)
                        }
                        match_word := shr(1, match_word)
                    }
                }
                
                mstore(0x00, holes_count)
                return(0x00, 32)
            }
            
            // Selector 0x33330000: muroga_parametron_majority
            // Inputs:
            // - calldataload(4): phase_in_1
            // - calldataload(36): phase_in_2
            // - calldataload(68): phase_in_3
            if eq(selector, 0x33330000) {
                let p1 := calldataload(4)
                let p2 := calldataload(36)
                let p3 := calldataload(68)
                
                let sum := add(add(p1, p2), p3)
                let output := 0
                if gte(sum, 2) {
                    output := 1
                }
                
                mstore(0x00, output)
                return(0x00, 32)
            }
            
            revert(0, 0)
        }
    }
}
