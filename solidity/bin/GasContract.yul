object "GasContract" {
    code {
        // Set deployer as the owner in storage slot 0
        sstore(0, caller())
        
        // Copy runtime code to memory and return it
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    
    object "runtime" {
        code {
            // Prevent ether transfers
            if callvalue() { revert(0, 0) }
            
            // Extract the 4-byte selector from calldata
            let selector := shr(224, calldataload(0))
            
            switch selector
            
            // balanceOf(address) -> selector: 0x70a08231
            case 0x70a08231 {
                let account := calldataload(4)
                let userBal := getBalance(account)
                mstore(0, userBal)
                return(0, 32)
            }
            
            // transfer(address,uint256) -> selector: 0xa9059cbb
            case 0xa9059cbb {
                let recipient := calldataload(4)
                let amount := calldataload(36)
                let sender := caller()
                
                let senderBalance := getBalance(sender)
                if lt(senderBalance, amount) { revert(0, 0) }
                
                let recipientBalance := getBalance(recipient)
                
                setBalance(sender, sub(senderBalance, amount))
                setBalance(recipient, add(recipientBalance, amount))
                
                // Return true for ERC20 compliance
                mstore(0, 1)
                return(0, 32)
            }
            
            // burn(uint256) -> selector: 0x42966c68
            case 0x42966c68 {
                let amount := calldataload(4)
                let sender := caller()
                
                let senderBalance := getBalance(sender)
                if lt(senderBalance, amount) { revert(0, 0) }
                
                setBalance(sender, sub(senderBalance, amount))
                
                // Track total burned amount in storage slot 1
                let totalBurned := sload(1)
                sstore(1, add(totalBurned, amount))
                
                return(0, 0)
            }
            
            // mint(address,uint256) -> selector: 0x40c10f19
            case 0x40c10f19 {
                // Only owner can mint (storage slot 0)
                let owner := sload(0)
                if iszero(eq(caller(), owner)) { revert(0, 0) }
                
                let account := calldataload(4)
                let amount := calldataload(36)
                
                let currentBalance := getBalance(account)
                setBalance(account, add(currentBalance, amount))
                
                return(0, 0)
            }
            
            default {
                revert(0, 0)
            }
            
            // Helper function to resolve balance storage slots
            function getBalance(account) -> userBal {
                mstore(0, account)
                mstore(32, 2) // Balances mapped to index slot 2
                let slot := keccak256(0, 64)
                userBal := sload(slot)
            }
            
            // Helper function to update balance storage slots
            function setBalance(account, newBalance) {
                mstore(0, account)
                mstore(32, 2)
                let slot := keccak256(0, 64)
                sstore(slot, newBalance)
            }
        }
    }
}
