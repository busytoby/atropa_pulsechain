object "MicroMinder" {
    code {
        // Initcode: Copy runtime code to memory and return it for deployment
        datacopy(0x00, dataoffset("runtime"), datasize("runtime"))
        return(0x00, datasize("runtime"))
    }

    object "runtime" {
        code {
            if callvalue() { revert(0, 0) }
            if lt(calldatasize(), 4) { revert(0, 0) }

            let selector := shr(224, calldataload(0))

            // Helpers for resolving context user
            function getContextUser() -> user {
                user := mload(0x1F0)
                if iszero(user) {
                    user := caller()
                }
            }

            // Storage key generators
            function getCountSlot(user, date) -> slot {
                mstore(0x00, user)
                mstore(0x20, date)
                mstore(0x40, 0) // Key type 0 = count
                slot := keccak256(0x00, 96)
            }

            function getItemSlot(user, date, index) -> slot {
                mstore(0x00, user)
                mstore(0x20, date)
                mstore(0x40, 1) // Key type 1 = item list
                let listBase := keccak256(0x00, 96)
                mstore(0x00, listBase)
                mstore(0x20, index)
                slot := keccak256(0x00, 64)
            }

            // ----------------------------------------------------------------
            // Method 1: addReminder(uint256 date, bytes32 part1, bytes32 part2) -> uint256 success
            // Selector: 0x5a557b7c -> keccak256("addReminder(uint256,bytes32,bytes32)") = 0x5a557b7c...
            // ----------------------------------------------------------------
            if eq(selector, 0x5a557b7c) {
                let date := calldataload(4)
                let part1 := calldataload(36)
                let part2 := calldataload(68)
                let user := getContextUser()

                let countSlot := getCountSlot(user, date)
                let count := sload(countSlot)

                let itemSlot1 := getItemSlot(user, date, count)
                sstore(itemSlot1, part1)
                sstore(add(itemSlot1, 1), part2)

                sstore(countSlot, add(count, 1))

                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method 2: getReminderCount(uint256 date) -> uint256 count
            // Selector: 0xb5f269a8 -> keccak256("getReminderCount(uint256)") = 0xb5f269a8...
            // ----------------------------------------------------------------
            if eq(selector, 0xb5f269a8) {
                let date := calldataload(4)
                let user := getContextUser()
                let countSlot := getCountSlot(user, date)
                mstore(0x00, sload(countSlot))
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // Method 3: getReminder(uint256 date, uint256 index) -> (bytes32 part1, bytes32 part2)
            // Selector: 0xd6533f81 -> keccak256("getReminder(uint256,uint256)") = 0xd6533f81...
            // ----------------------------------------------------------------
            if eq(selector, 0xd6533f81) {
                let date := calldataload(4)
                let index := calldataload(36)
                let user := getContextUser()

                let itemSlot1 := getItemSlot(user, date, index)
                mstore(0x00, sload(itemSlot1))
                mstore(0x20, sload(add(itemSlot1, 1)))
                return(0x00, 64)
            }

            // ----------------------------------------------------------------
            // Method 4: deleteReminder(uint256 date, uint256 index) -> uint256 success
            // Selector: 0x93bb2221 -> keccak256("deleteReminder(uint256,uint256)") = 0x93bb2221...
            // ----------------------------------------------------------------
            if eq(selector, 0x93bb2221) {
                let date := calldataload(4)
                let index := calldataload(36)
                let user := getContextUser()

                let countSlot := getCountSlot(user, date)
                let count := sload(countSlot)

                if lt(index, count) {
                    let lastIndex := sub(count, 1)
                    if iszero(eq(index, lastIndex)) { // not the last item, swap with last item
                        let lastSlot := getItemSlot(user, date, lastIndex)
                        let targetSlot := getItemSlot(user, date, index)
                        sstore(targetSlot, sload(lastSlot))
                        sstore(add(targetSlot, 1), sload(add(lastSlot, 1)))
                    }
                    // clear the last item slot
                    let lastSlot := getItemSlot(user, date, lastIndex)
                    sstore(lastSlot, 0)
                    sstore(add(lastSlot, 1), 0)

                    sstore(countSlot, lastIndex)
                    mstore(0x00, 1)
                    return(0x00, 32)
                }

                mstore(0x00, 0)
                return(0x00, 32)
            }

            revert(0, 0)
        }
    }
}
