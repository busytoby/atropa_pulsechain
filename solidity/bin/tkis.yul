object "TKISDatabase" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))
            
            // Helper to compute inventory slot: keccak256("inventory", name)
            function getInventorySlot(name) -> slot {
                mstore(0x00, 0x696e76656e746f72790000000000000000000000000000000000000000000000) // "inventory"
                mstore(0x20, name)
                slot := keccak256(0x00, 64)
            }
            
            // Helper to compute recipe exist slot: keccak256("recipe_exists", recipeName)
            function getRecipeExistsSlot(recipeName) -> slot {
                mstore(0x00, 0x7265636970655f65786973747300000000000000000000000000000000000000) // "recipe_exists"
                mstore(0x20, recipeName)
                slot := keccak256(0x00, 64)
            }
            
            // Helper to compute recipe ingredients count slot: keccak256("recipe_count", recipeName)
            function getRecipeCountSlot(recipeName) -> slot {
                mstore(0x00, 0x7265636970655f636f756e740000000000000000000000000000000000000000) // "recipe_count"
                mstore(0x20, recipeName)
                slot := keccak256(0x00, 64)
            }
            
            // Helper to compute recipe ingredient name slot: keccak256("recipe_ing", recipeName, index)
            function getRecipeIngSlot(recipeName, idx) -> slot {
                mstore(0x00, 0x7265636970655f696e6700000000000000000000000000000000000000000000) // "recipe_ing"
                mstore(0x20, recipeName)
                mstore(0x40, idx)
                slot := keccak256(0x00, 96)
            }
            
            // Helper to compute recipe ingredient qty slot: keccak256("recipe_qty", recipeName, index)
            function getRecipeQtySlot(recipeName, idx) -> slot {
                mstore(0x00, 0x7265636970655f71747900000000000000000000000000000000000000000000) // "recipe_qty"
                mstore(0x20, recipeName)
                mstore(0x40, idx)
                slot := keccak256(0x00, 96)
            }
            
            // 1. addIngredient(bytes32 name, uint256 quantity)
            // selector: 0xfb9f4a8d
            if eq(selector, 0xfb9f4a8d) {
                let name := calldataload(4)
                let qty := calldataload(36)
                
                let slot := getInventorySlot(name)
                let current := sload(slot)
                sstore(slot, add(current, qty))
                
                mstore(0x00, 1)
                return(0x00, 32)
            }
            
            // 2. getIngredient(bytes32 name) -> uint256 quantity
            // selector: 0x17f14bb6
            if eq(selector, 0x17f14bb6) {
                let name := calldataload(4)
                let slot := getInventorySlot(name)
                mstore(0x00, sload(slot))
                return(0x00, 32)
            }
            
            // 3. addRecipe(bytes32 recipeName, bytes32[] ingredientNames, uint256[] quantities)
            // selector: 0x943ebc2c
            if eq(selector, 0x943ebc2c) {
                let recipeName := calldataload(4)
                let ingOffset := add(4, calldataload(36))
                let qtyOffset := add(4, calldataload(68))
                
                let ingCount := calldataload(ingOffset)
                let qtyCount := calldataload(qtyOffset)
                if iszero(eq(ingCount, qtyCount)) { revert(0, 0) }
                
                // Store recipe existence
                sstore(getRecipeExistsSlot(recipeName), 1)
                // Store recipe count
                sstore(getRecipeCountSlot(recipeName), ingCount)
                
                // Store each ingredient and quantity requirement
                for { let i := 0 } lt(i, ingCount) { i := add(i, 1) } {
                    let ingName := calldataload(add(ingOffset, add(32, mul(i, 32))))
                    let qtyVal := calldataload(add(qtyOffset, add(32, mul(i, 32))))
                    
                    sstore(getRecipeIngSlot(recipeName, i), ingName)
                    sstore(getRecipeQtySlot(recipeName, i), qtyVal)
                }
                
                mstore(0x00, 1)
                return(0x00, 32)
            }
            
            // 4. checkRecipe(bytes32 recipeName) -> bool (returns 1 if enough inventory, else 0)
            // selector: 0x9548ba91
            if eq(selector, 0x9548ba91) {
                let recipeName := calldataload(4)
                
                // If recipe does not exist, return 0
                if iszero(sload(getRecipeExistsSlot(recipeName))) {
                    mstore(0x00, 0)
                    return(0x00, 32)
                }
                
                let count := sload(getRecipeCountSlot(recipeName))
                let available := 1
                
                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    let ingName := sload(getRecipeIngSlot(recipeName, i))
                    let requiredQty := sload(getRecipeQtySlot(recipeName, i))
                    let currentQty := sload(getInventorySlot(ingName))
                    
                    if lt(currentQty, requiredQty) {
                        available := 0
                        break
                    }
                }
                
                mstore(0x00, available)
                return(0x00, 32)
            }
            
            // 5. cookRecipe(bytes32 recipeName)
            // selector: 0x427cb8d1
            if eq(selector, 0x427cb8d1) {
                let recipeName := calldataload(4)
                
                if iszero(sload(getRecipeExistsSlot(recipeName))) { revert(0, 0) }
                
                let count := sload(getRecipeCountSlot(recipeName))
                
                // First pass: verify availability
                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    let ingName := sload(getRecipeIngSlot(recipeName, i))
                    let requiredQty := sload(getRecipeQtySlot(recipeName, i))
                    let currentQty := sload(getInventorySlot(ingName))
                    
                    if lt(currentQty, requiredQty) { revert(0, 0) }
                }
                
                // Second pass: deduct inventory
                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    let ingName := sload(getRecipeIngSlot(recipeName, i))
                    let requiredQty := sload(getRecipeQtySlot(recipeName, i))
                    let slot := getInventorySlot(ingName)
                    let currentQty := sload(slot)
                    sstore(slot, sub(currentQty, requiredQty))
                }
                
                mstore(0x00, 1)
                return(0x00, 32)
            }
            
            revert(0, 0)
        }
    }
}
