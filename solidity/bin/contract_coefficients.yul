object "ContractCoefficientsDDL" {
    code {
        // Constructor: Store Magic "COEF" Header Identification
        sstore(0x00, 0x434F4546) // "COEF"
        sstore(0x01, 953467954114363) // MotzkinPrime Modulus
    }
    object "ContractCoefficientsDDL_deployed" {
        code {
            let selector := shr(224, calldataload(0))
            switch selector
            case 0xc0ef0001 { // resolve_coefficient_field_offset(field_id)
                let field_id := calldataload(4)
                switch field_id
                case 0 { mstore(0, 0) }   // magic ("COEF")
                case 1 { mstore(0, 4) }   // name_hash (32 bytes)
                case 2 { mstore(0, 36) }  // symbol_hash (16 bytes)
                case 3 { mstore(0, 52) }  // total_supply (8 bytes)
                case 4 { mstore(0, 60) }  // setting_count (4 bytes)
                case 5 { mstore(0, 64) }  // holder_count (4 bytes)
                case 6 { mstore(0, 68) }  // lp_count (4 bytes)
                case 7 { mstore(0, 72) }  // lissajous_f_x (double 8 bytes)
                case 8 { mstore(0, 80) }  // lissajous_f_y (double 8 bytes)
                case 9 { mstore(0, 88) }  // lissajous_f_z (double 8 bytes)
                case 10 { mstore(0, 96) } // edo22_pitch_class (double 8 bytes)
                case 11 { mstore(0, 104) }// motzkin_prime (uint64 8 bytes)
                default { revert(0, 0) }
                return(0, 32)
            }
            case 0xc0ef0002 { // resolve_motzkin_prime()
                mstore(0, 953467954114363)
                return(0, 32)
            }
            default {
                revert(0, 0)
            }
        }
    }
}
