object "BtcEyeOfTheTigerPlayer" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))
            
            // playBionikaScore() -> returns (uint256 success)
            // Selector: 0x2be4a30e
            if iszero(sub(selector, 0x2be4a30e)) {
                // Eyecatch topic hash for BionikaScorePlayed(address,uint256)
                // Log the performance start
                let topic := 0x82f252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef
                
                // Construct score event details in memory: BPM=109, Tempo=136ms, NotesCount=32
                mstore(0, 109)
                mstore(32, 136)
                mstore(64, 32)
                log2(0, 96, topic, caller())
                
                mstore(0, 1)
                return(0, 32)
            }
            
            revert(0, 0)
        }
    }
}
