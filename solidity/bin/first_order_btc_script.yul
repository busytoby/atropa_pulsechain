object "FirstOrderBtcScript" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))
            
            // verify_witness(bytes32 expected_hash, bytes witness_data) -> returns (uint256 success)
            // Selector: 0x8dfb0f9c
            if iszero(sub(selector, 0x8dfb0f9c)) {
                let expected := calldataload(4)
                let offset := add(4, calldataload(36))
                let len := calldataload(offset)
                let start := add(offset, 32)
                
                // Copy witness data to memory at 0x1000
                calldatacopy(0x1000, start, len)
                
                // Execute standard first-order OP_SHA256 validation via precompile 0x02
                let success_call := staticcall(gas(), 2, 0x1000, len, 0x1100, 32)
                if iszero(success_call) { revert(0, 0) }
                let computed := mload(0x1100)
                
                // OP_EQUALVERIFY assertion check
                let success := eq(computed, expected)
                if iszero(success) { revert(0, 0) }
                
                mstore(0, 1)
                return(0, 32)
            }
            
            revert(0, 0)
        }
    }
}
