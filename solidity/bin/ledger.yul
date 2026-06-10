object "ZarrellaLedger" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }

    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))

            // Helpers for ledger balances (85000 + accountId)
            function getBalance(accId) -> bal {
                bal := sload(add(85000, accId))
            }
            function setBalance(accId, bal) {
                sstore(add(85000, accId), bal)
            }

            // Helpers for pointer redirection (86000 + ptrId)
            function getPointer(ptrId) -> accId {
                accId := sload(add(86000, ptrId))
            }
            function setPointerVal(ptrId, accId) {
                sstore(add(86000, ptrId), accId)
            }

            // 1. postTransaction(uint256 debitAccount, uint256 creditAccount, uint256 amount)
            // Selector: 0xda8480a4
            // In double-entry bookkeeping: debit increases asset/expense (positive), credit increases liability/equity (negative).
            if eq(selector, 0xda8480a4) {
                let debitAcc := calldataload(4)
                let creditAcc := calldataload(36)
                let amount := calldataload(68)

                if iszero(amount) { revert(0, 0) }

                // Update Debit Account: Bal := Bal + Amount
                let debBal := getBalance(debitAcc)
                setBalance(debitAcc, add(debBal, amount))

                // Update Credit Account: Bal := Bal - Amount
                let credBal := getBalance(creditAcc)
                setBalance(creditAcc, sub(credBal, amount))

                mstore(0x00, 1)
                return(0x00, 32)
            }

            // 2. getBalanceDirect(uint256 accountId) -> int256
            // Selector: 0xc3d37d2e
            if eq(selector, 0xc3d37d2e) {
                let accId := calldataload(4)
                mstore(0x00, getBalance(accId))
                return(0x00, 32)
            }

            // 3. setPointer(uint256 pointerId, uint256 targetAccountId)
            // Selector: 0x74b6ddf1
            if eq(selector, 0x74b6ddf1) {
                let ptrId := calldataload(4)
                let target := calldataload(36)
                setPointerVal(ptrId, target)
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // 4. getBalanceIndirect(uint256 pointerId) -> int256
            // Selector: 0xaa1b212f
            if eq(selector, 0xaa1b212f) {
                let ptrId := calldataload(4)
                let target := getPointer(ptrId)
                mstore(0x00, getBalance(target))
                return(0x00, 32)
            }

            // 5. getBalanceIndexed(uint256 baseAccountId, uint256 index) -> int256
            // Selector: 0xd3711de0
            if eq(selector, 0xd3711de0) {
                let baseId := calldataload(4)
                let idx := calldataload(36)
                let target := add(baseId, idx)
                mstore(0x00, getBalance(target))
                return(0x00, 32)
            }

            revert(0, 0)
        }
    }
}
