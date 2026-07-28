object "RAUCoprocessor" {
    code {
        // Constructor: Deploy code to EVM state
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            // Execution Entry Point
            // Calldata Layout:
            // [0:4]   - Function Selector:
            //           0x011a8b23 -> read_reg(uint256 v_reg)
            //           0x022b9c34 -> write_reg(uint256 v_reg, uint256 val)
            // [4:36]  - Parameter 1 (v_reg)
            // [36:68] - Parameter 2 (val)

            let selector := shr(224, calldataload(0))
            
            // Memory Layout mapping:
            // 0x00: Packed mapping word (32 bytes holding 32 8-bit mappings)
            // 0x20 - 0x9f: Physical Registers R0, R1, R2, R3 (32 bytes each)
            // 0xa0+: Spill Cache Memory (aligned 32-byte slots for V0-V31)

            switch selector
            case 0x011a8b23 {
                let v_reg := calldataload(4)
                let val := read_reg(v_reg)
                mstore(0, val)
                return(0, 32)
            }
            case 0x022b9c34 {
                let v_reg := calldataload(4)
                let val := calldataload(36)
                write_reg(v_reg, val)
                return(0, 0)
            }
            default {
                revert(0, 0)
            }

            // Reads the 32-byte value of virtual register v_reg
            function read_reg(v_reg) -> val {
                let p_reg := get_mapping(v_reg)
                // If not mapped (0xff), fetch from spill cache
                if eq(p_reg, 0xff) {
                    val := mload(add(0xa0, mul(v_reg, 32)))
                    leave
                }
                // Fetch from physical register slot
                val := mload(add(0x20, mul(p_reg, 32)))
            }

            // Writes 32-byte value to virtual register v_reg
            function write_reg(v_reg, val) {
                let p_reg := get_mapping(v_reg)
                // If not mapped, allocate physical register (spilling if necessary)
                if eq(p_reg, 0xff) {
                    p_reg := allocate_physical_reg(v_reg)
                }
                // Store in physical slot
                mstore(add(0x20, mul(p_reg, 32)), val)
            }

            // Retrieve physical mapping from packed mapping word
            function get_mapping(v_reg) -> p_reg {
                let mapping_word := mload(0)
                // Each mapping is 8 bits (1 byte)
                p_reg := and(shr(mul(v_reg, 8), mapping_word), 0xff)
            }

            // Set physical mapping in packed mapping word
            function set_mapping(v_reg, p_reg) {
                let mapping_word := mload(0)
                let shift := mul(v_reg, 8)
                // Clear old byte and set new byte mapping
                let mask := not(shl(shift, 0xff))
                mapping_word := or(and(mapping_word, mask), shl(shift, p_reg))
                mstore(0, mapping_word)
            }

            // Allocates a physical register (R0-R3) for a virtual register, spilling if full
            function allocate_physical_reg(v_reg) -> p_reg {
                // Find unused physical register
                p_reg := find_free_physical()
                if eq(p_reg, 0xff) {
                    // All physical registers in use; spill R0 (index 0) to cache
                    let victim_v := find_virtual_for_physical(0)
                    spill_to_cache(victim_v, 0)
                    p_reg := 0
                }
                set_mapping(v_reg, p_reg)
            }

            // Finds a physical register not mapped to any virtual register
            function find_free_physical() -> p_reg {
                p_reg := 0xff
                for { let i := 0 } lt(i, 4) { i := add(i, 1) } {
                    let owner := find_virtual_for_physical(i)
                    if eq(owner, 0xff) {
                        p_reg := i
                        break;
                    }
                }
            }

            // Scans mappings to find which virtual register maps to physical register p_reg
            function find_virtual_for_physical(p_reg) -> v_reg {
                v_reg := 0xff
                for { let i := 0 } lt(i, 32) { i := add(i, 1) } {
                    if eq(get_mapping(i), p_reg) {
                        v_reg := i
                        break;
                    }
                }
            }

            // Spills physical register state to aligned 32-byte cache slot
            function spill_to_cache(v_reg, p_reg) {
                let val := mload(add(0x20, mul(p_reg, 32)))
                mstore(add(0xa0, mul(v_reg, 32)), val)
                set_mapping(v_reg, 0xff) // Mark as spilled
            }
        }
    }
}
