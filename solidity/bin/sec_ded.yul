object "SecDedECC" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))
            
            // encode_ecc(uint256) -> uint256
            // selector: 0x51c72054
            if eq(selector, 0x51c72054) {
                let data := calldataload(4)
                let ecc_word := encode(data)
                mstore(0x00, ecc_word)
                return(0x00, 32)
            }
            
            // decode_ecc(uint256) -> uint256, uint256
            // returns (corrected_data, error_code)
            // selector: 0x900f0aa0
            if eq(selector, 0x900f0aa0) {
                let word := calldataload(4)
                let corrected, err := decode(word)
                mstore(0x00, corrected)
                mstore(0x20, err)
                return(0x00, 64)
            }
            
            revert(0, 0)
            
            // Helper functions
            function get_data_pos(i) -> pos {
                let data_idx := 0
                let candidate := 3
                for {} 1 {} {
                    // Check if candidate is power of 2: (candidate & (candidate - 1)) == 0
                    if iszero(and(candidate, sub(candidate, 1))) {
                        candidate := add(candidate, 1)
                        continue
                    }
                    if eq(data_idx, i) {
                        pos := candidate
                        break
                    }
                    data_idx := add(data_idx, 1)
                    candidate := add(candidate, 1)
                }
            }
            
            function encode(data) -> ecc_word {
                ecc_word := 0
                // Place data bits
                for { let i := 0 } lt(i, 56) { i := add(i, 1) } {
                    let bit := and(shr(i, data), 1)
                    if bit {
                        ecc_word := or(ecc_word, shl(get_data_pos(i), 1))
                    }
                }
                
                // Calculate parity bits
                for { let j := 0 } lt(j, 6) { j := add(j, 1) } {
                    let parity_pos := shl(j, 1)
                    let parity_val := 0
                    for { let i := 1 } lt(i, 64) { i := add(i, 1) } {
                        if and(i, parity_pos) {
                            if and(shr(i, ecc_word), 1) {
                                parity_val := xor(parity_val, 1)
                            }
                        }
                    }
                    if parity_val {
                        ecc_word := or(ecc_word, shl(parity_pos, 1))
                    }
                }
                
                // Calculate overall parity (bit 0)
                let overall_parity := 0
                for { let i := 1 } lt(i, 64) { i := add(i, 1) } {
                    if and(shr(i, ecc_word), 1) {
                        overall_parity := xor(overall_parity, 1)
                    }
                }
                if overall_parity {
                    ecc_word := or(ecc_word, 1)
                }
            }
            
            function decode(word) -> corrected, err {
                // Calculate overall parity over bits 0..63
                let overall_parity := 0
                for { let i := 0 } lt(i, 64) { i := add(i, 1) } {
                    if and(shr(i, word), 1) {
                        overall_parity := xor(overall_parity, 1)
                    }
                }
                
                // Calculate syndrome
                let syndrome := 0
                for { let j := 0 } lt(j, 6) { j := add(j, 1) } {
                    let parity_pos := shl(j, 1)
                    let parity_val := 0
                    for { let i := 1 } lt(i, 64) { i := add(i, 1) } {
                        if and(i, parity_pos) {
                            if and(shr(i, word), 1) {
                                parity_val := xor(parity_val, 1)
                            }
                        }
                    }
                    if parity_val {
                        syndrome := or(syndrome, parity_pos)
                    }
                }
                
                if iszero(syndrome) {
                    if iszero(overall_parity) {
                        // No error
                        corrected := extract_data(word)
                        err := 0
                    }
                    if overall_parity {
                        // Error in overall parity bit (bit 0)
                        corrected := extract_data(word)
                        err := 1 // Corrected single error
                    }
                }
                if syndrome {
                    if overall_parity {
                        // Single bit error in Hamming bits at position `syndrome`
                        if lt(syndrome, 64) {
                            word := xor(word, shl(syndrome, 1))
                        }
                        corrected := extract_data(word)
                        err := 1 // Corrected single error
                    }
                    if iszero(overall_parity) {
                        // Double-bit error detected (uncorrectable)
                        corrected := 0
                        err := 2
                    }
                }
            }
            
            function extract_data(word) -> data {
                data := 0
                for { let i := 0 } lt(i, 56) { i := add(i, 1) } {
                    let bit := and(shr(get_data_pos(i), word), 1)
                    if bit {
                        data := or(data, shl(i, 1))
                    }
                }
            }
        }
    }
}
