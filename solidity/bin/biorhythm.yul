object "Biorhythm" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }

    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))

            // Helper to check if year is leap
            function isLeap(y) -> leap {
                leap := 0
                if iszero(mod(y, 4)) {
                    leap := 1
                    if iszero(mod(y, 100)) {
                        leap := 0
                        if iszero(mod(y, 400)) {
                            leap := 1
                        }
                    }
                }
            }

            // Days in months for year (leap or non-leap)
            function daysInMonth(m, leap) -> days {
                days := 31
                if eq(m, 2) {
                    days := add(28, leap)
                }
                if or(or(eq(m, 4), eq(m, 6)), or(eq(m, 9), eq(m, 11))) {
                    days := 30
                }
            }

            // Calculate absolute days since year 1-01-01
            function dateToDays(y, m, d) -> totalDays {
                let years := sub(y, 1)
                let leapYears := add(add(div(years, 4), div(years, 400)), sub(0, div(years, 100)))
                totalDays := add(mul(years, 365), leapYears)
                
                let leap := isLeap(y)
                for { let curM := 1 } lt(curM, m) { curM := add(curM, 1) } {
                    totalDays := add(totalDays, daysInMonth(curM, leap))
                }
                totalDays := add(totalDays, d)
            }

            // Bhaskara I fixed-point sine approximation
            // x: index (0 to period - 1)
            // period: total cycle days
            // Returns: sine value in fixed-point percentage (-100 to 100)
            function getSineVal(x, period) -> val {
                let xScaled := mul(x, 2)
                let h := period // scaled half-period (period * 2 / 2 = period)

                if lt(xScaled, h) {
                    let term := mul(xScaled, sub(h, xScaled))
                    let numerator := mul(1600, term)
                    let denominator := sub(mul(5, mul(h, h)), mul(4, term))
                    val := div(numerator, denominator)
                }
                if iszero(lt(xScaled, h)) {
                    let x2 := sub(xScaled, h)
                    let term := mul(x2, sub(h, x2))
                    let numerator := mul(1600, term)
                    let denominator := sub(mul(5, mul(h, h)), mul(4, term))
                    // Return negative value (stored as two's complement sign-extended)
                    val := sub(0, div(numerator, denominator))
                }
            }

            // 1. daysBetween(uint256 y1, uint256 m1, uint256 d1, uint256 y2, uint256 m2, uint256 d2) -> uint256 days
            // Selector: 0x95227a8b
            if eq(selector, 0x95227a8b) {
                let y1 := calldataload(4)
                let m1 := calldataload(36)
                let d1 := calldataload(68)
                let y2 := calldataload(100)
                let m2 := calldataload(132)
                let d2 := calldataload(164)

                let days1 := dateToDays(y1, m1, d1)
                let days2 := dateToDays(y2, m2, d2)
                let diff := 0
                if gt(days2, days1) {
                    diff := sub(days2, days1)
                }
                mstore(0x00, diff)
                return(0x00, 32)
            }

            // 2. calculateBiorhythm(uint256 yBirth, uint256 mBirth, uint256 dBirth, uint256 yTarget, uint256 mTarget, uint256 dTarget)
            //     -> (int256 physical, int256 emotional, int256 intellectual, int256 diffDays)
            // Selector: 0x709d9090
            if eq(selector, 0x709d9090) {
                let yBirth := calldataload(4)
                let mBirth := calldataload(36)
                let dBirth := calldataload(68)
                let yTarget := calldataload(100)
                let mTarget := calldataload(132)
                let dTarget := calldataload(164)

                let daysBirth := dateToDays(yBirth, mBirth, dBirth)
                let daysTarget := dateToDays(yTarget, mTarget, dTarget)
                
                if lt(daysTarget, daysBirth) { revert(0, 0) }
                let diffDays := sub(daysTarget, daysBirth)

                let physIdx := mod(diffDays, 23)
                let emotIdx := mod(diffDays, 28)
                let intelIdx := mod(diffDays, 33)

                let physVal := getSineVal(physIdx, 23)
                let emotVal := getSineVal(emotIdx, 28)
                let intelVal := getSineVal(intelIdx, 33)

                mstore(0x00, physVal)
                mstore(0x20, emotVal)
                mstore(0x40, intelVal)
                mstore(0x60, diffDays)
                return(0x00, 128)
            }

            revert(0, 0)
        }
    }
}
