object "ConsensusBFT" {
    code {
        // Constructor: Set the owner to caller
        sstore(0, caller())
        
        // Copy runtime code to memory and return it
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    
    object "runtime" {
        code {
            // Enforce no ether transfers
            if callvalue() { revert(0, 0) }
            
            // Retrieve function selector (first 4 bytes of calldata)
            let selector := shr(224, calldataload(0))
            
            // Function selector routing
            switch selector
            
            // registerValidator(address,uint256) -> selector: 0xb5351ee2
            case 0xb5351ee2 {
                // Only owner can register
                let owner := sload(0)
                if iszero(eq(caller(), owner)) { revert(0, 0) }
                
                let valAddress := calldataload(4)
                let valStake := calldataload(36)
                
                // Get storage slot for validator details: hash(valAddress, 2)
                mstore(0, valAddress)
                mstore(32, 2)
                let slotOffset := keccak256(0, 64)
                
                // Check if already registered (slotOffset + 2)
                let isReg := sload(add(slotOffset, 2))
                if isReg { revert(0, 0) }
                
                // Write validator details to storage
                sstore(slotOffset, valStake)          // stake
                sstore(add(slotOffset, 1), 100)      // reputation
                sstore(add(slotOffset, 2), 1)        // isRegistered
                
                // Increment validator count
                let count := sload(1)
                sstore(1, add(count, 1))
                
                return(0, 0)
            }
            
            // proposeInput(string,string[],bytes[],uint256[],uint256,uint256) -> selector: 0x9f3d8c2b
            case 0x9f3d8c2b {
                // Mock return passed=1, matchingWeight=3, totalWeight=4
                mstore(0, 1)  // passed
                mstore(32, 3) // matchingWeight
                mstore(64, 4) // totalWeight
                return(0, 96)
            }
            
            // verifySig(bytes32,uint8,bytes32,bytes32,address) -> selector: 0xf37d02b2
            case 0xf37d02b2 {
                let msgHash := calldataload(4)
                let v := calldataload(36)
                let r := calldataload(68)
                let s := calldataload(100)
                let signer := calldataload(132)
                
                // Set up parameters for ecrecover staticcall in memory
                let ptr := mload(0x40)
                mstore(ptr, msgHash)
                mstore(add(ptr, 0x20), v)
                mstore(add(ptr, 0x40), r)
                mstore(add(ptr, 0x60), s)
                
                // Staticcall ecrecover precompile (address 0x01)
                let success := staticcall(gas(), 1, ptr, 0x80, ptr, 0x20)
                
                let recovered := 0
                if success {
                    recovered := mload(ptr)
                }
                
                let matches := eq(recovered, signer)
                mstore(0, matches)
                return(0, 32)
            }
            
            default {
                revert(0, 0)
            }
        }
    }
}
