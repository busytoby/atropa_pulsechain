object "RhythmGenerator" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            // processSample(int256 triggerClock, int256 patternSelect) -> int256 drumTriggers
            // selector: 0x07a96d8c
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let clockSignal := calldataload(4)
                let patternSelect := calldataload(36)

                // Load internal clock state:
                // slot 100: last clock signal (to detect rising edge)
                // slot 101: step index (0 to 15)
                let lastClock := sload(100)
                let step := sload(101)

                let trigger := 0

                // Detect clock rising edge (clock transition from <= 0 to > 0)
                // Using standard Yul: sle(lastClock, 0) is equivalent to iszero(sgt(lastClock, 0))
                if and(sgt(clockSignal, 0), iszero(sgt(lastClock, 0))) {
                    // Increment step index
                    step := add(step, 1)
                    if gt(step, 15) {
                        step := 0
                    }
                    sstore(101, step)

                    // Retrieve triggers for the current step under selected pattern
                    trigger := getPatternTrigger(patternSelect, step)
                }

                // Save clock state
                sstore(100, clockSignal)

                mstore(0, trigger)
                return(0, 32)
            }
            revert(0, 0)

            // SGS M252 Pattern Selector Logic:
            // returns bit-packed triggers: Bit 0 = Kick (1), Bit 1 = Snare (2), Bit 2 = Hihat (4)
            function getPatternTrigger(pattern, stepIdx) -> triggers {
                triggers := 0
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
                    let waltzStep := mod(stepIdx, 12)
                    switch waltzStep
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
            }
        }
    }
}
