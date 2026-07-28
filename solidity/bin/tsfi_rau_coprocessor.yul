object "RAUCoprocessor" {
    code {
        // Constructor: Deploy code to EVM state
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            // Execution Entry Point for Stateless Coaxial Crossbar RAU
            // Calldata Layout:
            // [0:4]   - Function Selector:
            //           0x011a8b23 -> read_reg(uint256 v_reg)
            //           0x022b9c34 -> write_reg(uint256 v_reg, uint256 val)
            //           0x099c63ab -> sync_register_with_core(uint256 target_core_id, uint256 v_reg)

            let selector := shr(224, calldataload(0))
            
            // Coaxial Memory Map mapping:
            // 0x4000 - 0x43ff: Coaxial Bus Channels V0-V31 (32 bytes per channel slot)
            // 0x4800: WinchesterMQ SCSI Output Port (Destination Core ID)
            // 0x4820: WinchesterMQ SCSI Register Index Port
            // 0x4840: WinchesterMQ SCSI Data Value Port

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
            case 0x099c63ab {
                let target_core_id := calldataload(4)
                let v_reg := calldataload(36)
                sync_register_with_core(target_core_id, v_reg)
                return(0, 0)
            }
            default {
                revert(0, 0)
            }

            // Peek: Read 32-byte value directly from Coaxial Bus channel (V0 is hardwired to constant 0)
            function read_reg(v_reg) -> val {
                if iszero(v_reg) {
                    val := 0
                    leave
                }
                // Directly read from the coaxial channel page
                val := mload(add(0x4000, mul(v_reg, 32)))
            }

            // Poke: Write 32-byte value directly to Coaxial Bus channel (V0 is ignored)
            function write_reg(v_reg, val) {
                if iszero(v_reg) {
                    leave
                }
                // Directly write to the coaxial channel page
                mstore(add(0x4000, mul(v_reg, 32)), val)
            }

            // Sync: Route coaxial register value directly through WinchesterMQ SCSI ports
            function sync_register_with_core(target_core_id, v_reg) {
                let val := read_reg(v_reg)
                mstore(0x4800, target_core_id)
                mstore(0x4820, v_reg)
                mstore(0x4840, val)
            }
        }
    }
}
