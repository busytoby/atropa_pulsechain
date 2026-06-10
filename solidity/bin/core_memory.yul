object "CoreMemoryUnit" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }

    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))

            // Helper to get raw core magnetic state (0 or 1)
            function getCore(x, y) -> state {
                state := sload(add(90000, add(mul(x, 16), y)))
            }
            // Helper to set raw core magnetic state
            function setCore(x, y, state) {
                sstore(add(90000, add(mul(x, 16), y)), state)
            }

            // 1. writeCore(uint256 x, uint256 y, uint8 bit)
            // Selector: 0xfc53e58a
            if eq(selector, 0xfc53e58a) {
                let x := calldataload(4)
                let y := calldataload(36)
                let bit := and(calldataload(68), 1)

                // Simulates coincident write:
                // Cores are assumed to be in 0 state before writing (following a read cycle)
                // If bit == 1:
                //   X line current (+I/2) and Y line current (+I/2) combine to exceed hysteresis threshold (+I)
                //   and flip the core at (x,y) to 1.
                // If bit == 0:
                //   Inhibit line current (-I/2) opposes the Y line current.
                //   Net current is +I/2, which is below the threshold, so the core remains at 0.
                if eq(bit, 1) {
                    setCore(x, y, 1)
                }
                if eq(bit, 0) {
                    // Core remains 0 due to inhibit current
                    setCore(x, y, 0)
                }

                mstore(0x00, 1)
                return(0x00, 32)
            }

            // 2. readCoreDestructive(uint256 x, uint256 y) -> (uint8 bit, uint256 sensePulse)
            // Selector: 0x84421eaa
            if eq(selector, 0x84421eaa) {
                let x := calldataload(4)
                let y := calldataload(36)

                let previousState := getCore(x, y)
                let sensePulse := 0

                // Simulates destructive read:
                // We apply current -I/2 on X and -I/2 on Y (net -I write-0 current).
                // If previousState was 1, the core flips to 0, inducing a voltage pulse on the shared Sense Wire.
                // If previousState was 0, no change occurs, and no pulse is induced.
                if eq(previousState, 1) {
                    sensePulse := 1
                    setCore(x, y, 0) // Destructive: state flips to 0
                }

                mstore(0x00, previousState)
                mstore(0x20, sensePulse)
                return(0x00, 64)
            }

            // 3. readRestoreCore(uint256 x, uint256 y) -> uint8 bit
            // Selector: 0x3dd90b2d
            if eq(selector, 0x3dd90b2d) {
                let x := calldataload(4)
                let y := calldataload(36)

                // Destructive read step
                let previousState := getCore(x, y)
                
                // Read-Restore Cycle:
                // If previousState was 1, we must immediately rewrite 1 to restore the data!
                if eq(previousState, 1) {
                    setCore(x, y, 1)
                }

                mstore(0x00, previousState)
                return(0x00, 32)
            }

            revert(0, 0)
        }
    }
}
