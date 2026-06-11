/*
    Philips Z550M Decade Gas-Discharge Glow Indicator.
    Simulates a cold-cathode Nixie-style numeric indicator.
    When trigger voltage exceeds striking potential (105V), the neon gas ignites
    and lights up the corresponding digit cathode (0-9), returning a bitmask of the active glow.
*/
object "Z550mIndicator" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            // processSample(int256 triggerVoltage, int256 digitInput) -> int256 activeGlowMask
            // selector: 0x07a96d8c (standard processSample signature)
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let trigger := calldataload(4)
                let digit := calldataload(36)

                let strikeVoltage := 105000000000000000000 // 105V striking potential (scaled 1e18)
                let glowMask := 0

                // The tube only fires/glows if trigger voltage exceeds striking threshold
                if sgt(trigger, strikeVoltage) {
                    // Safe digit clamping (0-9)
                    if slt(digit, 0) { digit := 0 }
                    if sgt(digit, 9) { digit := 9 }

                    // Set bit corresponding to the glowing cathode digit
                    glowMask := shl(digit, 1)
                }

                mstore(0, glowMask)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
