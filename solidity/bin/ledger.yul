object "ZarrellaLedger" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }

    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))

            // Helpers for ledger balances mapped to slot keccak256(accId, 85000)
            function getBalance(accId) -> bal {
                mstore(0x00, accId)
                mstore(0x20, 85000)
                bal := sload(keccak256(0x00, 64))
            }
            function setBalance(accId, bal) {
                mstore(0x00, accId)
                mstore(0x20, 85000)
                sstore(keccak256(0x00, 64), bal)
            }

            // Helpers for pointer redirection mapped to slot keccak256(ptrId, 86000)
            function getPointer(ptrId) -> accId {
                mstore(0x00, ptrId)
                mstore(0x20, 86000)
                accId := sload(keccak256(0x00, 64))
            }
            function setPointerVal(ptrId, accId) {
                mstore(0x00, ptrId)
                mstore(0x20, 86000)
                sstore(keccak256(0x00, 64), accId)
            }

            // Helpers for ERC20 allowances mapped to slot keccak256(owner, spender, 87000)
            function getAllowance(owner, spender) -> val {
                mstore(0x00, owner)
                mstore(0x20, spender)
                mstore(0x40, 87000)
                val := sload(keccak256(0x00, 96))
            }
            function setAllowance(owner, spender, val) {
                mstore(0x00, owner)
                mstore(0x20, spender)
                mstore(0x40, 87000)
                sstore(keccak256(0x00, 96), val)
            }

            // ----------------------------------------------------------------
            // 1. postTransaction(uint256 debitAccount, uint256 creditAccount, uint256 amount)
            // Selector: 0xda8480a4
            // ----------------------------------------------------------------
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

                // Emit Transfer event: creditAcc -> debitAcc
                mstore(0x00, amount)
                log3(0x00, 32, 0xddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef, creditAcc, debitAcc)

                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // 2. getBalanceDirect(uint256 accountId) -> int256
            // Selector: 0xc3d37d2e
            // ----------------------------------------------------------------
            if eq(selector, 0xc3d37d2e) {
                let accId := calldataload(4)
                mstore(0x00, getBalance(accId))
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // 3. setPointer(uint256 pointerId, uint256 targetAccountId)
            // Selector: 0x74b6ddf1
            // ----------------------------------------------------------------
            if eq(selector, 0x74b6ddf1) {
                let ptrId := calldataload(4)
                let target := calldataload(36)
                setPointerVal(ptrId, target)
                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // 4. getBalanceIndirect(uint256 pointerId) -> int256
            // Selector: 0xaa1b212f
            // ----------------------------------------------------------------
            if eq(selector, 0xaa1b212f) {
                let ptrId := calldataload(4)
                let target := getPointer(ptrId)
                mstore(0x00, getBalance(target))
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // 5. getBalanceIndexed(uint256 baseAccountId, uint256 index) -> int256
            // Selector: 0xd3711de0
            // ----------------------------------------------------------------
            if eq(selector, 0xd3711de0) {
                let baseId := calldataload(4)
                let idx := calldataload(36)
                let target := add(baseId, idx)
                mstore(0x00, getBalance(target))
                return(0x00, 32)
            }

            // ================================================================
            // ERC20 METADATA & COMPATIBILITY METHODS
            // ================================================================

            // 6. name() -> string
            // Selector: 0x06fdde03
            if eq(selector, 0x06fdde03) {
                mstore(0x00, 0x20)
                mstore(0x20, 26) // length
                mstore(0x40, 0x5a617272656c6c6120416c6368656d6963616c204c6564676572000000000000) // "Zarrella Alchemical Ledger"
                return(0x00, 96)
            }

            // 7. symbol() -> string
            // Selector: 0x95d89b41
            if eq(selector, 0x95d89b41) {
                mstore(0x00, 0x20)
                mstore(0x20, 3) // length
                mstore(0x40, 0x5a414c0000000000000000000000000000000000000000000000000000000000) // "ZAL"
                return(0x00, 96)
            }

            // 8. decimals() -> uint8
            // Selector: 0x313ce567
            if eq(selector, 0x313ce567) {
                mstore(0x00, 18)
                return(0x00, 32)
            }

            // 9. totalSupply() -> uint256
            // Selector: 0x18160ddd
            if eq(selector, 0x18160ddd) {
                mstore(0x00, 100000000000000000000000000) // 100 million ZAL
                return(0x00, 32)
            }

            // 10. balanceOf(address) -> uint256
            // Selector: 0x70a08231
            if eq(selector, 0x70a08231) {
                let owner := calldataload(4)
                mstore(0x00, getBalance(owner))
                return(0x00, 32)
            }

            // 11. transfer(address,uint256) -> bool
            // Selector: 0xa9059cbb
            if eq(selector, 0xa9059cbb) {
                let to := calldataload(4)
                let amount := calldataload(36)
                let sender := caller()

                let senderBal := getBalance(sender)
                if lt(senderBal, amount) { revert(0, 0) }

                setBalance(sender, sub(senderBal, amount))
                setBalance(to, add(getBalance(to), amount))

                mstore(0x00, amount)
                log3(0x00, 32, 0xddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef, sender, to)

                mstore(0x00, 1)
                return(0x00, 32)
            }

            // 12. approve(address,uint256) -> bool
            // Selector: 0x095ea7b3
            if eq(selector, 0x095ea7b3) {
                let spender := calldataload(4)
                let amount := calldataload(36)
                let owner := caller()

                setAllowance(owner, spender, amount)

                mstore(0x00, amount)
                log3(0x00, 32, 0x8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b925, owner, spender)

                mstore(0x00, 1)
                return(0x00, 32)
            }

            // 13. allowance(address,address) -> uint256
            // Selector: 0xdd62ed3e
            if eq(selector, 0xdd62ed3e) {
                let owner := calldataload(4)
                let spender := calldataload(36)
                mstore(0x00, getAllowance(owner, spender))
                return(0x00, 32)
            }

            // 14. transferFrom(address,address,uint256) -> bool
            // Selector: 0x23b872dd
            if eq(selector, 0x23b872dd) {
                let from := calldataload(4)
                let to := calldataload(36)
                let amount := calldataload(68)
                let spender := caller()

                let allowed := getAllowance(from, spender)
                if lt(allowed, amount) { revert(0, 0) }

                let fromBal := getBalance(from)
                if lt(fromBal, amount) { revert(0, 0) }

                setAllowance(from, spender, sub(allowed, amount))
                setBalance(from, sub(fromBal, amount))
                setBalance(to, add(getBalance(to), amount))

                mstore(0x00, amount)
                log3(0x00, 32, 0xddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef, from, to)

                mstore(0x00, 1)
                return(0x00, 32)
            }

            revert(0, 0)
        }
    }
}
