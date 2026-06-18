object "PmgSystem" {
    // ========================================================================
    // DEPLOYMENT INITIALIZATION BLOCK (INITCODE)
    // ========================================================================
    code {
        if callvalue() { revert(0, 0) }
        datacopy(0x00, dataoffset("runtime"), datasize("runtime"))
        return(0x00, datasize("runtime"))
    }

    // ========================================================================
    // RUNTIME ENVIRONMENT
    // ========================================================================
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))

            // Atari GTIA PMG Register Address Offsets
            // HPOSP0-HPOSP3: 53248 - 53251 ($D000 - $D003)
            // HPOSM0-HPOSM3: 53252 - 53255 ($D004 - $D007)
            // SIZEP0-SIZEP3: 53256 - 53259 ($D008 - $D00B)
            // SIZEM: 53260 ($D00C)
            // COLPM0-COLPM3: 53266 - 53269 ($D012 - $D015)
            // M0PF-M3PF (Missile-to-Playfield Collisions): 53248 (reuse offsets for reads)
            // P0PF-P3PF (Player-to-Playfield Collisions): 53252

            // Helper to get slot key
            function getRegKey(addr) -> slot {
                mstore(0x00, caller())
                mstore(0x20, addr)
                slot := keccak256(0x00, 64)
            }

            // ----------------------------------------------------------------
            // METHOD 1: updatePmg(uint8 isMissile, uint8 index, uint8 hpos, uint8 size, uint8 color)
            // Selector: 0xaeed2db8
            // ----------------------------------------------------------------
            if eq(selector, 0xaeed2db8) {
                let isMissile := and(calldataload(4), 0xFF)
                let index := and(calldataload(36), 0xFF)
                let hpos := and(calldataload(68), 0xFF)
                let size := and(calldataload(100), 0xFF)
                let color := and(calldataload(132), 0xFF)

                if gt(index, 3) { revert(0, 0) }

                let baseHposAddr := 53248 // HPOSP0
                let baseSizeAddr := 53256 // SIZEP0
                let baseColorAddr := 53266 // COLPM0

                if isMissile {
                    baseHposAddr := 53252 // HPOSM0
                    baseSizeAddr := 53260 // SIZEM (shared offset but indexed here)
                }

                sstore(getRegKey(add(baseHposAddr, index)), hpos)
                sstore(getRegKey(add(baseSizeAddr, index)), size)
                sstore(getRegKey(add(baseColorAddr, index)), color)

                mstore(0x00, 1)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 2: getPmgState(uint8 isMissile, uint8 index) -> (uint8 hpos, uint8 size, uint8 color)
            // Selector: 0xb36e80b4
            // ----------------------------------------------------------------
            if eq(selector, 0xb36e80b4) {
                let isMissile := and(calldataload(4), 0xFF)
                let index := and(calldataload(36), 0xFF)

                if gt(index, 3) { revert(0, 0) }

                let baseHposAddr := 53248
                let baseSizeAddr := 53256
                let baseColorAddr := 53266

                if isMissile {
                    baseHposAddr := 53252
                    baseSizeAddr := 53260
                }

                let hpos := sload(getRegKey(add(baseHposAddr, index)))
                let size := sload(getRegKey(add(baseSizeAddr, index)))
                let color := sload(getRegKey(add(baseColorAddr, index)))

                mstore(0x00, hpos)
                mstore(0x20, size)
                mstore(0x40, color)
                return(0x00, 96)
            }

            // ----------------------------------------------------------------
            // METHOD 3: checkPmgCollisions() -> uint8 (Atari GTIA Collision Mask)
            // Simulates overlap collisions between Player 0 and playfields/other players
            // Selector: 0xb966a078
            // ----------------------------------------------------------------
            if eq(selector, 0xb966a078) {
                let collisionMask := 0
                
                // Fetch horizontal positions of Player 0 & Player 1
                let p0_hpos := sload(getRegKey(53248))
                let p1_hpos := sload(getRegKey(53249))

                // Standard Player width = 8 pixels, scale by size modifier (0=1x, 1=2x, 3=4x width)
                let p0_size := sload(getRegKey(53256))
                let p0_width := 8
                if eq(p0_size, 1) { p0_width := 16 }
                if eq(p0_size, 3) { p0_width := 32 }

                let p1_size := sload(getRegKey(53257))
                let p1_width := 8
                if eq(p1_size, 1) { p1_width := 16 }
                if eq(p1_size, 3) { p1_width := 32 }

                // Check overlap on horizontal scanline space
                let overlap := 0
                let diff := 0
                if gt(p0_hpos, p1_hpos) {
                    diff := sub(p0_hpos, p1_hpos)
                    if lt(diff, p1_width) { overlap := 1 }
                }
                if iszero(gt(p0_hpos, p1_hpos)) {
                    diff := sub(p1_hpos, p0_hpos)
                    if lt(diff, p0_width) { overlap := 1 }
                }

                if overlap {
                    // M0PF/P0PF Collision flag: Set Bit 0 (Player 0 and Player 1 collision)
                    collisionMask := or(collisionMask, 1)
                }

                // Update collision flag register (Atari GTIA P0PL register mapping)
                sstore(getRegKey(53252), collisionMask)

                mstore(0x00, collisionMask)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 4: loadDnaGenome(uint256 dnaRaw) -> void
            // Selector: 0xd6a111a9 (Parses 12-byte genome and configures P0 size/color registers)
            // ----------------------------------------------------------------
            if eq(selector, 0xd6a111a9) {
                let dna := calldataload(4)
                
                let fur_r := byte(0, dna)
                let fur_g := byte(1, dna)
                let fur_b := byte(2, dna)
                let sickness := byte(6, dna)
                let scale := byte(7, dna)
                
                let luma := div(add(add(fur_r, fur_g), fur_b), 3)
                let hue := mod(add(fur_r, mul(fur_g, 2)), 16)
                let atari_color := or(shl(4, hue), shr(4, luma))
                
                sstore(getRegKey(53266), atari_color)
                
                let pmg_size := 0
                if gt(scale, 120) { pmg_size := 1 }
                if gt(scale, 180) { pmg_size := 3 }
                sstore(getRegKey(53256), pmg_size)
                
                sstore(getRegKey(53280), sickness)
                
                return(0, 0)
            }

            revert(0, 0)
        }
    }
}
