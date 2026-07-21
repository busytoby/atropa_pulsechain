object "TapeLabelDDL" {
    code {
        // Constructor: Initialize WinchesterMQ Tape Label Register Layout
        sstore(0x00, 0x564F4C31) // "VOL1" magic header at slot 0
        sstore(0x01, 80)         // VOL1 length = 80 bytes
        sstore(0x02, 0x48445231) // "HDR1" magic header at slot 2
        sstore(0x03, 80)         // HDR1 length = 80 bytes
    }
    object "TapeLabelDDL_deployed" {
        code {
            // Function selector routing
            let selector := shr(224, calldataload(0))
            switch selector
            case 0x70a2b0a2 { // get_vol1_offset(field_id)
                let field_id := calldataload(4)
                switch field_id
                case 0 { mstore(0, 0) }   // label_id offset = 0
                case 1 { mstore(0, 4) }   // volume_id offset = 4
                case 2 { mstore(0, 10) }  // accessibility offset = 10
                case 3 { mstore(0, 37) }  // owner_id offset = 37
                default { mstore(0, 0) }
                return(0, 32)
            }
            case 0x3a4b12c1 { // get_hdr1_offset(field_id)
                let field_id := calldataload(4)
                switch field_id
                case 0 { mstore(0, 80) }  // HDR1 start = offset 80
                case 1 { mstore(0, 84) }  // file_id offset = 84 (must end with .dat.bin)
                case 2 { mstore(0, 101) } // set_id offset = 101
                default { mstore(0, 80) }
                return(0, 32)
            }
            default {
                revert(0, 0)
            }
        }
    }
}
