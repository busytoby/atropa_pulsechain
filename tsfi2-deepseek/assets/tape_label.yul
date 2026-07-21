object "TapeLabelDDL" {
    code {
        // Constructor: Initialize Tape Label Magic Selectors
        sstore(0x00, 0x564F4C31) // "VOL1"
        sstore(0x01, 0x48445231) // "HDR1"
        sstore(0x02, 0x48445232) // "HDR2"
        sstore(0x03, 0x454F4631) // "EOF1"
        sstore(0x04, 0x454F4632) // "EOF2"
        sstore(0x05, 0x454F5631) // "EOV1"
    }
    object "TapeLabelDDL_deployed" {
        code {
            let selector := shr(224, calldataload(0))
            switch selector
            case 0x70a2b0a2 { // get_label_offset(label_type, field_id)
                let label_type := calldataload(4)
                let field_id := calldataload(8)
                switch label_type
                case 1 { // VOL1 (Offset 0)
                    switch field_id
                    case 0 { mstore(0, 0) }   // label_id
                    case 1 { mstore(0, 4) }   // volume_id
                    case 2 { mstore(0, 37) }  // owner_id
                    default { mstore(0, 0) }
                }
                case 2 { // HDR1 (Offset 80)
                    switch field_id
                    case 0 { mstore(0, 80) }  // label_id
                    case 1 { mstore(0, 84) }  // file_id (.dat.bin)
                    case 2 { mstore(0, 127) } // security_code
                    default { mstore(0, 80) }
                }
                case 3 { // HDR2 (Offset 160)
                    switch field_id
                    case 0 { mstore(0, 160) } // label_id
                    case 1 { mstore(0, 164) } // record_format ("F")
                    default { mstore(0, 160) }
                }
                case 4 { // EOF1 (Trailer Offset)
                    mstore(0, 0xFFFFFFFF) // Dynamically calculated at end of file
                }
                default {
                    revert(0, 0)
                }
                return(0, 32)
            }
            default {
                revert(0, 0)
            }
        }
    }
}
