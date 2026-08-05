object "crows_rank" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))
            
            // optIn(address player) -> returns (uint256)
            // Selector: 0x1c1e0a29
            if eq(selector, 0x1c1e0a29) {
                let player := calldataload(4)
                sstore(player, 1)
                mstore(0, 1)
                return(0, 32)
            }
            
            // updateAndGetRankings(address player, address playerYue, address crowsQing)
            // Selector: 0xe003a27b
            if eq(selector, 0xe003a27b) {
                let player := calldataload(4)
                let playerYue := calldataload(36)
                let crowsQing := calldataload(68)
                
                // Verify player is opted in
                if iszero(sload(player)) { revert(0, 0) }
                
                // Call Bar(crowsQing) on playerYue
                mstore(0, 0x0ebc857700000000000000000000000000000000000000000000000000000000)
                mstore(4, crowsQing)
                let success := call(gas(), playerYue, 0, 0, 36, 0, 64)
                if iszero(success) { revert(0, 0) }
                
                let hypo := mload(0)
                let epi := mload(32)
                let score := add(hypo, epi)
                
                // Zero memory for return layout
                // We return address[10] then uint256[10] -> total 20 * 32 = 640 bytes
                // Set addresses[0] = player
                mstore(0, player)
                // Set other addresses to 0
                for { let i := 1 } lt(i, 10) { i := add(i, 1) } {
                    mstore(mul(i, 32), 0)
                }
                // Set scores[0] = score (at offset 320)
                mstore(320, score)
                // Set other scores to 0
                for { let i := 11 } lt(i, 20) { i := add(i, 1) } {
                    mstore(mul(i, 32), 0)
                }
                
                return(0, 640)
            }
            revert(0, 0)
        }
    }
}
