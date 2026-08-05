object "NCR304Rail" {
    code {
        codecopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            // Copy transaction calldata input directly to memory at 0x80
            calldatacopy(0x80, 0, calldatasize())

            // NCR 304 Red Rail Hardware Register offsets
            let reg_tape_seq := 0x20
            let reg_parity_chk := 0x21
            let reg_retry_count := 0x22
            let reg_write_lock := 0x23
            let reg_warn_ind := 0x24
            let reg_merge_validator := 0x25
            let reg_chk_error := 0x26
            let reg_parity_code := 0x27
            let reg_retry_limit := 0x28
            let reg_io_busy := 0x29

            // Complimentary registers for operating foundation
            let reg_merge_channels := 0x2a  // Multi-channel tape merge-sorter activation
            let reg_transcoder := 0x2b      // Card-to-tape transcoding status

            // Retrieve current values
            let tape_seq := sload(reg_tape_seq)
            let next_seq := add(tape_seq, 1)
            sstore(reg_tape_seq, next_seq)
            sstore(reg_merge_validator, next_seq)

            // Parity and Read-Retry Telemetry Loop
            let parity_key := sload(reg_parity_chk)
            if iszero(eq(mod(parity_key, 2), 0)) {
                sstore(reg_chk_error, 0xDEAD)
                sstore(reg_parity_code, 0x01)
                
                let retry_limit := sload(reg_retry_limit)
                let retries := sload(reg_retry_count)
                
                if lt(retries, retry_limit) {
                    let next_retries := add(retries, 1)
                    sstore(reg_retry_count, next_retries)
                    if eq(next_retries, retry_limit) {
                        sstore(reg_parity_chk, and(parity_key, not(1)))
                        sstore(reg_chk_error, 0x55AAAA55)
                        sstore(reg_parity_code, 0x00)
                    }
                }
            }

            // Write lock check
            let write_lock := sload(reg_write_lock)
            let current_pc := sload(0x01)
            let lock_boundary := sload(0x30)
            if and(lt(current_pc, lock_boundary), iszero(eq(write_lock, 0))) {
                sstore(reg_warn_ind, 1)
            }

            // Real Multi-channel Merge-Sorter (Insertion Sort on memory segment starting at 0x80)
            let merge_chan := sload(reg_merge_channels)
            let size := div(calldatasize(), 32)
            if merge_chan {
                if gt(size, 1) {
                    // Loop index i from 1 to size - 1
                    let i := 1
                    for {} lt(i, size) { i := add(i, 1) } {
                        let i_offset := add(0x80, mul(i, 32))
                        let key := mload(i_offset)
                        
                        let j := sub(i, 1)
                        let done := 0
                        
                        for {} and(iszero(done), 1) { } {
                            let j_offset := add(0x80, mul(j, 32))
                            let val_j := mload(j_offset)
                            
                            if gt(val_j, key) {
                                mstore(add(j_offset, 32), val_j)
                                if iszero(j) {
                                    mstore(0x80, key)
                                    done := 1
                                }
                                if iszero(done) {
                                    j := sub(j, 1)
                                }
                            }
                            if iszero(gt(val_j, key)) {
                                mstore(add(j_offset, 32), key)
                                done := 1
                            }
                        }
                    }
                }
                
                // Progress transcoding status
                let trans_status := sload(reg_transcoder)
                sstore(reg_transcoder, add(trans_status, 1))
            }

            // Occupancy busy line logic
            let err_state := sload(reg_chk_error)
            if eq(err_state, 0xDEAD) {
                sstore(reg_io_busy, 0x03)
            }
            if eq(err_state, 0x55AAAA55) {
                sstore(reg_io_busy, 0x00)
            }

            // Return sorted elements
            return(0x80, mul(size, 32))
        }
    }
}
