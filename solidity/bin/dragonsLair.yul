object "DragonsLair" {
    code {
        if callvalue() { revert(0, 0) }
        datacopy(0x00, dataoffset("runtime"), datasize("runtime"))
        return(0x00, datasize("runtime"))
    }
    
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))

            // Storage slots
            // 0x100: Score
            // 0x101: Lives
            // 0x102: CurrentStage
            // 0x103: PromptActive (0: inactive, 1: active)
            // 0x104: ExpectedAction (1: LEFT, 2: RIGHT, 3: SWORD, 4: UP, 5: DOWN)
            // 0x105: Status (0: playing, 1: victory, 2: gameover)
            // 0x106: PRNG Seed

            function getSlot(offset) -> slot {
                slot := offset
            }

            // Simple pseudo-random helper
            function prng() -> val {
                let seed := sload(getSlot(0x106))
                if iszero(seed) { seed := 0x12345678 }
                // LCG multiplier & increment
                seed := add(mul(seed, 1664525), 1013904223)
                sstore(getSlot(0x106), seed)
                val := mod(seed, 5) // Return 0 to 4
            }

            // ----------------------------------------------------------------
            // METHOD 1: initializeGame() -> (score, lives, stage, prompt, expected, status)
            // Selector: 0xd6b046a2
            // ----------------------------------------------------------------
            if eq(selector, 0xd6b046a2) {
                sstore(getSlot(0x100), 0)    // Score = 0
                sstore(getSlot(0x101), 3)    // Lives = 3
                sstore(getSlot(0x102), 1)    // Stage = 1
                sstore(getSlot(0x103), 0)    // PromptActive = 0
                sstore(getSlot(0x104), 0)    // ExpectedAction = 0
                sstore(getSlot(0x105), 0)    // Status = 0 (playing)
                
                mstore(0x00, 0)
                mstore(0x20, 3)
                mstore(0x40, 1)
                mstore(0x60, 0)
                mstore(0x80, 0)
                mstore(0xa0, 0)
                return(0x00, 192)
            }

            // ----------------------------------------------------------------
            // METHOD 2: triggerPrompt() -> (expectedAction)
            // Selector: 0xbc848da0
            // Generates the next action prompt (1 to 5) depending on stage
            // ----------------------------------------------------------------
            if eq(selector, 0xbc848da0) {
                let status := sload(getSlot(0x105))
                if eq(status, 0) {
                    let stage := sload(getSlot(0x102))
                    let expected := 1 // Default: LEFT
                    
                    // Stage-specific or randomized cues
                    if eq(stage, 1) {
                        expected := 2 // RIGHT (Stage 1 crumbling hall)
                    }
                    if eq(stage, 2) {
                        expected := 3 // SWORD (Stage 2 dragon cavern)
                    }
                    if eq(stage, 3) {
                        expected := 1 // LEFT (Stage 3 falling bridge)
                    }
                    if gt(stage, 3) {
                        // Generate random prompt (1 to 5) for higher stages
                        expected := add(prng(), 1)
                    }

                    sstore(getSlot(0x103), 1) // PromptActive = 1
                    sstore(getSlot(0x104), expected)
                }

                mstore(0x00, sload(getSlot(0x104)))
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 3: updateGame(action) -> (score, lives, stage, prompt, expected, status)
            // Selector: 0x789b708d
            // action: 1: LEFT, 2: RIGHT, 3: SWORD, 4: UP, 5: DOWN
            // ----------------------------------------------------------------
            if eq(selector, 0x789b708d) {
                let action := calldataload(4)
                let score := sload(getSlot(0x100))
                let lives := sload(getSlot(0x101))
                let stage := sload(getSlot(0x102))
                let promptActive := sload(getSlot(0x103))
                let expected := sload(getSlot(0x104))
                let status := sload(getSlot(0x105))

                if eq(status, 0) {
                    // Check if player executed action without a prompt (wrong timing)
                    if iszero(promptActive) {
                        lives := sub(lives, 1)
                        if iszero(lives) {
                            status := 2 // Game Over
                        }
                    }
                    // Prompt is active, check input
                    if promptActive {
                        sstore(getSlot(0x103), 0) // Clear Prompt
                        promptActive := 0

                        if eq(action, expected) {
                            // Correct Action
                            score := add(score, 1500)
                            stage := add(stage, 1)
                            
                            // Win condition (Finished 5 stages)
                            if eq(stage, 6) {
                                status := 1 // Victory
                            }
                        }
                        if iszero(eq(action, expected)) {
                            // Incorrect Action
                            lives := sub(lives, 1)
                            if iszero(lives) {
                                status := 2 // Game Over
                            }
                        }
                    }
                }

                sstore(getSlot(0x100), score)
                sstore(getSlot(0x101), lives)
                sstore(getSlot(0x102), stage)
                sstore(getSlot(0x103), promptActive)
                sstore(getSlot(0x105), status)

                mstore(0x00, score)
                mstore(0x20, lives)
                mstore(0x40, stage)
                mstore(0x60, promptActive)
                mstore(0x80, expected)
                mstore(0xa0, status)
                return(0x00, 192)
            }

            revert(0, 0)
        }
    }
}
