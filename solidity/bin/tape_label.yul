object "TapeLabelDDL" {
    code {
        // Constructor: Initialize Full 8-Block Tape Label Magic Selectors
        sstore(0x00, 0x564F4C31) // "VOL1"
        sstore(0x01, 0x48445231) // "HDR1"
        sstore(0x02, 0x48445232) // "HDR2"
        sstore(0x03, 0x48445233) // "HDR3" (Crypto Sig)
        sstore(0x04, 0x48445234) // "HDR4" (Spatial Bounds)
        sstore(0x05, 0x48445235) // "HDR5" (Volume Spanning)
        sstore(0x06, 0x48445236) // "HDR6" (Compression & Vulkan Tags)
        sstore(0x07, 0x48445237) // "HDR7" (Lissajous Phase Invariants)
        sstore(0x08, 0x48445238) // "HDR8" (SCSI WinchesterMQ Register Map)
    }
    object "TapeLabelDDL_deployed" {
        code {
            let selector := shr(224, calldataload(0))
            switch selector
            case 0x70a2b0a2 { // get_label_offset(label_type, field_id)
                let label_type := calldataload(4)
                let field_id := calldataload(8)
                switch label_type
                case 1 { mstore(0, 0) }     // VOL1 (Offset 0)
                case 2 { mstore(0, 80) }    // HDR1 (Offset 80)
                case 3 { mstore(0, 160) }   // HDR2 (Offset 160)
                case 4 { mstore(0, 240) }   // HDR3 (Offset 240) - Sig
                case 5 { mstore(0, 320) }   // HDR4 (Offset 320) - Bounds
                case 6 { mstore(0, 400) }   // HDR5 (Offset 400) - Spanning
                case 7 { // HDR6 (Offset 480) - Compression & Vulkan
                    switch field_id
                    case 0 { mstore(0, 480) } // label_id ("HDR6")
                    case 1 { mstore(0, 484) } // compression_type ("RLE")
                    case 2 { mstore(0, 492) } // vulkan_accel ("VK_ENABLE")
                    default { mstore(0, 480) }
                }
                case 8 { // HDR7 (Offset 560) - Lissajous Phase Space
                    switch field_id
                    case 0 { mstore(0, 560) } // label_id ("HDR7")
                    case 1 { mstore(0, 564) } // phase_phi
                    case 2 { mstore(0, 572) } // freq_fx
                    case 3 { mstore(0, 580) } // freq_fy
                    case 4 { mstore(0, 588) } // freq_fz
                    default { mstore(0, 560) }
                }
                case 9 { // HDR8 (Offset 640) - WinchesterMQ SCSI State Map
                    switch field_id
                    case 0 { mstore(0, 640) } // label_id ("HDR8")
                    case 1 { mstore(0, 644) } // scsi_reg_20 (keycode 32)
                    case 2 { mstore(0, 652) } // scsi_reg_1E (keycode 30)
                    default { mstore(0, 640) }
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
