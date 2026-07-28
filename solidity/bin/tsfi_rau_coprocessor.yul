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
            //           0x033c0d45 -> add_virtual(uint256 v_a, uint256 v_b, uint256 v_dest)
            //           0x044d1e56 -> mul_virtual(uint256 v_a, uint256 v_b, uint256 v_dest)
            //           0x055e2f67 -> push_context()
            //           0x066f3078 -> pop_context()
            //           0x077a4189 -> dump_registers(uint256 mem_ptr)
            //           0x088b529a -> load_registers(uint256 mem_ptr)
            //           0x099c63ab -> sync_register_with_core(uint256 target_core_id, uint256 v_reg)
            // [4:36]  - Parameter 1 (v_reg, v_a, mem_ptr, or target_core_id)
            // [36:68] - Parameter 2 (val, v_b, or v_reg)
            // [68:100]- Parameter 3 (v_dest)

            let selector := shr(224, calldataload(0))
            
            // Memory Layout mapping:
            // 0x00: Packed mapping word (32 bytes holding 32 8-bit mappings)
            // 0x20 - 0x9f: Physical Registers R0, R1, R2, R3 (32 bytes each)
            // 0xa0 - 0x1ff: Spill Cache Memory (aligned 32-byte slots for V0-V31)
            // 0x200: LRU Access Order Array (4 bytes tracking R0-R3 access age)
            // 0x210: Dirty Bit Mask (4 bits representing R0-R3 dirty state)
            // 0x220: Context Stack Pointer (SP, initialized to 0x1000)
            // 0x4800: WinchesterMQ SCSI Output Port (Destination Core ID)
            // 0x4820: WinchesterMQ SCSI Register Index Port
            // 0x4840: WinchesterMQ SCSI Data Value Port
            // 0x1000+: Context Stack Memory (160 bytes per context frame)

            // Initialize stack pointer if it is zero
            if iszero(mload(0x220)) {
                mstore(0x220, 0x1000)
            }

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
            case 0x033c0d45 {
                let v_a := calldataload(4)
                let v_b := calldataload(36)
                let v_dest := calldataload(68)
                let val_a := read_reg(v_a)
                let val_b := read_reg(v_b)
                write_reg(v_dest, add(val_a, val_b))
                return(0, 0)
            }
            case 0x044d1e56 {
                let v_a := calldataload(4)
                let v_b := calldataload(36)
                let v_dest := calldataload(68)
                let val_a := read_reg(v_a)
                let val_b := read_reg(v_b)
                write_reg(v_dest, mul(val_a, val_b))
                return(0, 0)
            }
            case 0x055e2f67 {
                push_context()
                return(0, 0)
            }
            case 0x066f3078 {
                pop_context()
                return(0, 0)
            }
            case 0x077a4189 {
                let mem_ptr := calldataload(4)
                dump_registers(mem_ptr)
                return(0, 0)
            }
            case 0x088b529a {
                let mem_ptr := calldataload(4)
                load_registers(mem_ptr)
                return(0, 0)
            }
            case 0x099c63ab {
                let target_core_id := calldataload(4)
                let v_reg := calldataload(36)
                sync_register_with_core(target_core_id, v_reg)
                return(0, 0)
            }
            default {
                revert(0, 0)
            }

            // Reads the 32-byte value of virtual register v_reg (V0 hardwired to constant 0)
            function read_reg(v_reg) -> val {
                if iszero(v_reg) {
                    val := 0
                    leave
                }
                let p_reg := get_mapping(v_reg)
                // If not mapped (0xff), fetch from spill cache
                if eq(p_reg, 0xff) {
                    val := mload(add(0xa0, mul(v_reg, 32)))
                    p_reg := allocate_physical_reg(v_reg)
                    mstore(add(0x20, mul(p_reg, 32)), val)
                    clear_dirty(p_reg)
                    leave
                }
                update_lru(p_reg)
                val := mload(add(0x20, mul(p_reg, 32)))
            }

            // Writes 32-byte value to virtual register v_reg (Writes to V0 are ignored)
            function write_reg(v_reg, val) {
                if iszero(v_reg) {
                    leave
                }
                let p_reg := get_mapping(v_reg)
                if eq(p_reg, 0xff) {
                    p_reg := allocate_physical_reg(v_reg)
                }
                update_lru(p_reg)
                set_dirty(p_reg)
                mstore(add(0x20, mul(p_reg, 32)), val)
            }

            // Retrieve physical mapping from packed mapping word
            function get_mapping(v_reg) -> p_reg {
                let mapping_word := mload(0)
                p_reg := and(shr(mul(v_reg, 8), mapping_word), 0xff)
            }

            // Set physical mapping in packed mapping word
            function set_mapping(v_reg, p_reg) {
                let mapping_word := mload(0)
                let shift := mul(v_reg, 8)
                let mask := not(shl(shift, 0xff))
                mapping_word := or(and(mapping_word, mask), shl(shift, p_reg))
                mstore(0, mapping_word)
            }

            // Update LRU queue age tracking
            function update_lru(p_reg) {
                let order := mload(0x200)
                let new_order := 0
                let shift_count := 0
                for { let i := 0 } lt(i, 4) { i := add(i, 1) } {
                    let entry := and(shr(mul(i, 8), order), 0xff)
                    if and(iszero(eq(entry, p_reg)), iszero(eq(entry, 0))) {
                        new_order := or(new_order, shl(mul(shift_count, 8), entry))
                        shift_count := add(shift_count, 1)
                    }
                }
                new_order := or(new_order, shl(mul(shift_count, 8), p_reg))
                mstore(0x200, new_order)
            }

            // Set register dirty state
            function set_dirty(p_reg) {
                let mask := mload(0x210)
                mstore(0x210, or(mask, shl(mul(p_reg, 8), 1)))
            }

            // Clear register dirty state
            function clear_dirty(p_reg) {
                let mask := mload(0x210)
                mstore(0x210, and(mask, not(shl(mul(p_reg, 8), 1))))
            }

            // Check if register is dirty
            function is_dirty(p_reg) -> dirty {
                let mask := mload(0x210)
                dirty := and(mask, shl(mul(p_reg, 8), 1))
            }

            // Allocates a physical register, spilling LRU to cache if full
            function allocate_physical_reg(v_reg) -> p_reg {
                p_reg := find_free_physical()
                if eq(p_reg, 0xff) {
                    let lru_word := mload(0x200)
                    let victim_p := and(lru_word, 0xff)
                    let victim_v := find_virtual_for_physical(victim_p)
                    spill_to_cache(victim_v, victim_p)
                    p_reg := victim_p
                }
                set_mapping(v_reg, p_reg)
            }

            // Finds free physical register slot
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

            // Scans mappings to find owner virtual register
            function find_virtual_for_physical(p_reg) -> v_reg {
                v_reg := 0xff
                for { let i := 0 } lt(i, 32) { i := add(i, 1) } {
                    if eq(get_mapping(i), p_reg) {
                        v_reg := i
                        break;
                    }
                }
            }

            // Spills physical register state with dirty bit checking
            function spill_to_cache(v_reg, p_reg) {
                if is_dirty(p_reg) {
                    let val := mload(add(0x20, mul(p_reg, 32)))
                    mstore(add(0xa0, mul(v_reg, 32)), val)
                    clear_dirty(p_reg)
                }
                set_mapping(v_reg, 0xff)
            }

            // Push entire context to stack
            function push_context() {
                let sp := mload(0x220)
                mstore(sp, mload(0))
                mstore(add(sp, 32), mload(0x20))
                mstore(add(sp, 64), mload(0x40))
                mstore(add(sp, 96), mload(0x60))
                mstore(add(sp, 128), mload(0x80))
                mstore(add(sp, 160), mload(0x200))
                mstore(add(sp, 192), mload(0x210))
                mstore(0x220, add(sp, 224))

                // Reset active context
                mstore(0, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
                mstore(0x20, 0)
                mstore(0x40, 0)
                mstore(0x60, 0)
                mstore(0x80, 0)
                mstore(0x200, 0)
                mstore(0x210, 0)
            }

            // Pop context from stack
            function pop_context() {
                let sp := mload(0x220)
                if eq(sp, 0x1000) { revert(0, 0) }
                let target_sp := sub(sp, 224)
                mstore(0, mload(target_sp))
                mstore(0x20, mload(add(target_sp, 32)))
                mstore(0x40, mload(add(target_sp, 64)))
                mstore(0x60, mload(add(target_sp, 96)))
                mstore(0x80, mload(add(target_sp, 128)))
                mstore(0x200, mload(add(target_sp, 160)))
                mstore(0x210, mload(add(target_sp, 192)))
                mstore(0x220, target_sp)
            }

            // Bulk dump all 32 virtual registers
            function dump_registers(mem_ptr) {
                for { let i := 0 } lt(i, 32) { i := add(i, 1) } {
                    let val := read_reg(i)
                    mstore(add(mem_ptr, mul(i, 32)), val)
                }
            }

            // Bulk load all 32 virtual registers
            function load_registers(mem_ptr) {
                for { let i := 0 } lt(i, 32) { i := add(i, 1) } {
                    let val := mload(add(mem_ptr, mul(i, 32)))
                    write_reg(i, val)
                }
            }

            // Inter-ALU register synchronization via WinchesterMQ SCSI ports
            function sync_register_with_core(target_core_id, v_reg) {
                let val := read_reg(v_reg)
                // Write packet directly to memory-mapped WMQ SCSI ports
                mstore(0x4800, target_core_id)
                mstore(0x4820, v_reg)
                mstore(0x4840, val)
            }
        }
    }
}
