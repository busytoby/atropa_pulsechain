object "StarCastle" {
    code {
        if callvalue() { revert(0, 0) }
        datacopy(0x00, dataoffset("runtime"), datasize("runtime"))
        return(0x00, datasize("runtime"))
    }
    
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))

            // Storage slots mapping
            // 0x100: Score
            // 0x101: Lives
            // 0x102: Lead Harvested
            // 0x103: Mercury Harvested
            // 0x104: Sulfur Harvested
            // 0x105: Status (0: not started/playing, 1: active, 2: gameover)

            function getSlot(offset) -> slot {
                slot := offset
            }

            // Function to perform cross-contract call to Zarrella Ledger
            // postTransaction(uint256 debitAccount, uint256 creditAccount, uint256 amount) -> 0xda8480a4
            function postToLedger(ledgerAddr, debitAcc, creditAcc, amount) -> success {
                mstore(0x00, shl(224, 0xda8480a4))
                mstore(0x04, debitAcc)
                mstore(0x24, creditAcc)
                mstore(0x44, amount)
                success := call(gas(), ledgerAddr, 0, 0x00, 100, 0x80, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 1: initializeGame() -> (score, lives, lead, mercury, sulfur, status)
            // Selector: 0xa0a1f49b
            // ----------------------------------------------------------------
            if eq(selector, 0xa0a1f49b) {
                sstore(getSlot(0x100), 0)    // Score = 0
                sstore(getSlot(0x101), 3)    // Lives = 3
                sstore(getSlot(0x102), 0)    // Lead = 0
                sstore(getSlot(0x103), 0)    // Mercury = 0
                sstore(getSlot(0x104), 0)    // Sulfur = 0
                sstore(getSlot(0x105), 1)    // Status = 1 (active)
                
                mstore(0x00, 0)
                mstore(0x20, 3)
                mstore(0x40, 0)
                mstore(0x60, 0)
                mstore(0x80, 0)
                mstore(0xa0, 1)
                return(0x00, 192)
            }

            // ----------------------------------------------------------------
            // METHOD 2: recordHarvest(score, lives, lead, mercury, sulfur, status)
            // Selector: 0xeb694570
            // ----------------------------------------------------------------
            if eq(selector, 0xeb694570) {
                sstore(getSlot(0x100), calldataload(4))
                sstore(getSlot(0x101), calldataload(36))
                sstore(getSlot(0x102), calldataload(68))
                sstore(getSlot(0x103), calldataload(100))
                sstore(getSlot(0x104), calldataload(132))
                sstore(getSlot(0x105), calldataload(164))

                mstore(0x00, 1) // Return true
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 3: getHarvestState() -> (score, lives, lead, mercury, sulfur, status)
            // Selector: 0xa93b11ec
            // ----------------------------------------------------------------
            if eq(selector, 0xa93b11ec) {
                mstore(0x00, sload(getSlot(0x100)))
                mstore(0x20, sload(getSlot(0x101)))
                mstore(0x40, sload(getSlot(0x102)))
                mstore(0x60, sload(getSlot(0x103)))
                mstore(0x80, sload(getSlot(0x104)))
                mstore(0xa0, sload(getSlot(0x105)))
                return(0x00, 192)
            }

            // ----------------------------------------------------------------
            // METHOD 4: mintHarvestOnChain(address ledgerAddr) -> (bool)
            // Selector: 0x7b261050
            // ----------------------------------------------------------------
            if eq(selector, 0x7b261050) {
                let ledgerAddr := calldataload(4)
                let lead := sload(getSlot(0x102))
                let mercury := sload(getSlot(0x103))
                let sulfur := sload(getSlot(0x104))
                let success := 1

                if lead {
                    if iszero(postToLedger(ledgerAddr, 1101, 1200, lead)) {
                        success := 0
                    }
                }
                if mercury {
                    if iszero(postToLedger(ledgerAddr, 1102, 1200, mercury)) {
                        success := 0
                    }
                }
                if sulfur {
                    if iszero(postToLedger(ledgerAddr, 1103, 1200, sulfur)) {
                        success := 0
                    }
                }

                if success {
                    sstore(getSlot(0x102), 0)
                    sstore(getSlot(0x103), 0)
                    sstore(getSlot(0x104), 0)
                }

                mstore(0x00, success)
                return(0x00, 32)
            }

            revert(0, 0)
        }
    }
}
