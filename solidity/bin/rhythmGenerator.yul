object "RhythmGenerator" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))

            // Namespaced storage slot mapper for multi-user isolation
            function getNamespacedSlot(index) -> slot {
                mstore(0x280, caller())
                mstore(0x2A0, index)
                slot := keccak256(0x280, 64)
            }

            // ----------------------------------------------------------------
            // METHOD 1: processSample(int256 triggerClock, int256 patternSelect) -> int256 drumTriggers
            // Selector: 0x07a96d8c
            // ----------------------------------------------------------------
            if eq(selector, 0x07a96d8c) {
                let clockSignal := calldataload(4)
                let patternSelect := calldataload(36)

                // Load internal clock state:
                // slot 100: last clock signal (namespaced)
                // slot 101: step index (namespaced)
                let lastClock := sload(getNamespacedSlot(100))
                let step := sload(getNamespacedSlot(101))

                let trigger := 0

                // Detect clock rising edge (clock transition from <= 0 to > 0)
                if and(sgt(clockSignal, 0), iszero(sgt(lastClock, 0))) {
                    // Get pattern length (default 16, custom stored at 0x1000 + patternSelect)
                    let patLength := sload(getNamespacedSlot(add(0x1000, patternSelect)))
                    if iszero(patLength) {
                        patLength := 16
                        // Default Waltz override (Pattern 1)
                        if eq(patternSelect, 1) {
                            patLength := 12
                        }
                    }

                    // Increment step index
                    step := add(step, 1)
                    if iszero(lt(step, patLength)) {
                        step := 0
                    }
                    sstore(getNamespacedSlot(101), step)

                    // Retrieve triggers for the current step under selected pattern
                    trigger := getPatternTrigger(patternSelect, step)
                }

                // Save clock state
                sstore(getNamespacedSlot(100), clockSignal)

                mstore(0, trigger)
                return(0, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 2: setCustomPatternStep(uint256 patternId, uint256 stepIdx, uint256 triggers) -> void
            // Selector: 0x18e5e261
            // ----------------------------------------------------------------
            if eq(selector, 0x18e5e261) {
                let patternId := calldataload(4)
                let stepIdx := calldataload(36)
                let triggers := calldataload(68)

                // Store custom triggers at namespaced slot
                sstore(getNamespacedSlot(add(0x2000, add(mul(patternId, 256), stepIdx))), triggers)
                return(0, 0)
            }

            // ----------------------------------------------------------------
            // METHOD 3: setPatternLength(uint256 patternId, uint256 length) -> void
            // Selector: 0x64ef500d
            // ----------------------------------------------------------------
            if eq(selector, 0x64ef500d) {
                let patternId := calldataload(4)
                let length := calldataload(36)

                // Store custom length at namespaced slot
                sstore(getNamespacedSlot(add(0x1000, patternId)), length)
                return(0, 0)
            }

            // ----------------------------------------------------------------
            // METHOD 4: getPatternStepTriggers(uint256 patternId, uint256 stepIdx) -> uint256
            // Selector: 0xf3dac6f4
            // ----------------------------------------------------------------
            if eq(selector, 0xf3dac6f4) {
                let patternId := calldataload(4)
                let stepIdx := calldataload(36)

                let triggers := getPatternTrigger(patternId, stepIdx)
                mstore(0, triggers)
                return(0, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 5: getCurrentStep() -> uint256
            // Selector: 0x1d3824ea
            // ----------------------------------------------------------------
            if eq(selector, 0x1d3824ea) {
                mstore(0, sload(getNamespacedSlot(101)))
                return(0, 32)
            }

            revert(0, 0)

            // SGS M252 Pattern Selector Logic:
            // returns bit-packed triggers:
            // Bit 0 = Kick (1)
            // Bit 1 = Snare (2)
            // Bit 2 = Closed Hihat (4)
            // Bit 3 = Open Hihat (8)
            // Bit 4 = Cowbell (16)
            // Bit 5 = Clap (32)
            // Bit 7 = Accent (128)
            function getPatternTrigger(pattern, stepIdx) -> triggers {
                // First, check if there's a custom trigger stored for this pattern and step
                // Storage slot: 0x2000 + pattern * 256 + stepIdx (namespaced)
                triggers := sload(getNamespacedSlot(add(0x2000, add(mul(pattern, 256), stepIdx))))
                if triggers {
                    leave
                }

                // Fallback to pre-coded patterns if no custom trigger is stored
                switch pattern
                case 0 {
                    // Pattern 0: 4/4 Rock Beat
                    switch stepIdx
                    case 0 { triggers := 5 }
                    case 1 { triggers := 4 }
                    case 2 { triggers := 4 }
                    case 3 { triggers := 4 }
                    case 4 { triggers := 6 }
                    case 5 { triggers := 4 }
                    case 6 { triggers := 4 }
                    case 7 { triggers := 4 }
                    case 8 { triggers := 5 }
                    case 9 { triggers := 4 }
                    case 10 { triggers := 5 }
                    case 11 { triggers := 4 }
                    case 12 { triggers := 6 }
                    case 13 { triggers := 4 }
                    case 14 { triggers := 4 }
                    case 15 { triggers := 4 }
                }
                case 1 {
                    // Pattern 1: Waltz 3/4 Beat (repeats every 12 steps)
                    switch stepIdx
                    case 0 { triggers := 1 }
                    case 1 { triggers := 4 }
                    case 2 { triggers := 4 }
                    case 3 { triggers := 2 }
                    case 4 { triggers := 4 }
                    case 5 { triggers := 4 }
                    case 6 { triggers := 1 }
                    case 7 { triggers := 4 }
                    case 8 { triggers := 4 }
                    case 9 { triggers := 2 }
                    case 10 { triggers := 4 }
                    case 11 { triggers := 4 }
                }
                case 2 {
                    // Pattern 2: Auncient Clave Polyrhythm (16 steps)
                    switch stepIdx
                    case 0 { triggers := 5 }
                    case 2 { triggers := 4 }
                    case 3 { triggers := 9 }
                    case 4 { triggers := 6 }
                    case 6 { triggers := 5 }
                    case 7 { triggers := 8 }
                    case 8 { triggers := 4 }
                    case 10 { triggers := 5 }
                    case 11 { triggers := 8 }
                    case 12 { triggers := 7 }
                    case 14 { triggers := 4 }
                    case 15 { triggers := 8 }
                }
                case 3 {
                    // Pattern 3: Detroit Techno / Electro Breakbeat
                    switch stepIdx
                    case 0 { triggers := 1 }
                    case 2 { triggers := 4 }
                    case 3 { triggers := 16 }
                    case 4 { triggers := 2 }
                    case 6 { triggers := 4 }
                    case 7 { triggers := 16 }
                    case 8 { triggers := 8 }
                    case 10 { triggers := 5 }
                    case 11 { triggers := 16 }
                    case 12 { triggers := 2 }
                    case 13 { triggers := 16 }
                    case 14 { triggers := 4 }
                }
                case 4 {
                    // Pattern 4: Auncient Hip-Hop Boom Bap Beat
                    switch stepIdx
                    case 0 { triggers := 133 }
                    case 1 { triggers := 4 }
                    case 2 { triggers := 4 }
                    case 3 { triggers := 4 }
                    case 4 { triggers := 134 }
                    case 5 { triggers := 4 }
                    case 6 { triggers := 4 }
                    case 7 { triggers := 4 }
                    case 8 { triggers := 133 }
                    case 9 { triggers := 4 }
                    case 10 { triggers := 4 }
                    case 11 { triggers := 5 }
                    case 12 { triggers := 134 }
                    case 13 { triggers := 4 }
                    case 14 { triggers := 4 }
                    case 15 { triggers := 4 }
                }
            }
        }
    }
}
