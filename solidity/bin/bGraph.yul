object "BGraph" {
    // ========================================================================
    // DEPLOYMENT INITIALIZATION BLOCK (INITCODE)
    // ========================================================================
    code {
        if callvalue() { revert(0, 0) }

        // Copy runtime code to memory and return it
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

            // ----------------------------------------------------------------
            // METHOD 1: linearRegression(int256[] x, int256[] y) -> (int256 slope, int256 intercept, int256 rSquared)
            // Selector: 0xfefbb62c
            // ----------------------------------------------------------------
            if eq(selector, 0xfefbb62c) {
                let xOffset := add(4, calldataload(4))
                let yOffset := add(4, calldataload(36))

                let xLen := calldataload(xOffset)
                let yLen := calldataload(yOffset)

                // Ensure arrays match and are not empty
                if or(iszero(xLen), iszero(eq(xLen, yLen))) { revert(0, 0) }

                let sumX := 0
                let sumY := 0
                let sumXY := 0
                let sumXX := 0
                let sumYY := 0

                for { let i := 0 } lt(i, xLen) { i := add(i, 1) } {
                    let vx := calldataload(add(add(xOffset, 32), mul(i, 32)))
                    let vy := calldataload(add(add(yOffset, 32), mul(i, 32)))

                    sumX := add(sumX, vx)
                    sumY := add(sumY, vy)
                    sumXY := add(sumXY, mul(vx, vy))
                    sumXX := add(sumXX, mul(vx, vx))
                    sumYY := add(sumYY, mul(vy, vy))
                }

                let n := xLen

                // Numerator = N * sumXY - sumX * sumY
                let num := sub(mul(n, sumXY), mul(sumX, sumY))
                // Denominator = N * sumXX - sumX * sumX
                let den := sub(mul(n, sumXX), mul(sumX, sumX))

                let slope := 0
                let intercept := 0
                let rSquared := 0

                if iszero(eq(den, 0)) {
                    // Scale calculations by 1000 for fixed-point precision
                    slope := sdiv(mul(num, 1000), den)
                    // Intercept = (sumY - slope * sumX) / N
                    intercept := sdiv(sub(mul(sumY, 1000), mul(slope, sumX)), mul(n, 1000))
                    
                    // R-Squared approximation
                    let denY := sub(mul(n, sumYY), mul(sumY, sumY))
                    if iszero(eq(denY, 0)) {
                        let rNum := mul(num, num)
                        let rDen := mul(den, denY)
                        rSquared := sdiv(mul(rNum, 1000), rDen)
                    }
                }

                mstore(0x00, slope)
                mstore(0x20, intercept)
                mstore(0x40, rSquared)
                return(0x00, 96)
            }

            // ----------------------------------------------------------------
            // METHOD 2: plotBarChart(uint256[] values) -> (uint256[] coordinates)
            // Selector: 0x5511281b
            // Computes 320x200 screen coordinate rectangles for plotting a bar chart.
            // coordinates format: flat array of [x1, y1, x2, y2, x1, y1, x2, y2, ...]
            // ----------------------------------------------------------------
            if eq(selector, 0x5511281b) {
                let valOffset := add(4, calldataload(4))
                let len := calldataload(valOffset)
                
                if iszero(len) { revert(0, 0) }
                
                // Find maximum value for scaling
                let maxVal := 0
                for { let i := 0 } lt(i, len) { i := add(i, 1) } {
                    let v := calldataload(add(add(valOffset, 32), mul(i, 32)))
                    if gt(v, maxVal) { maxVal := v }
                }
                
                let barWidth := div(320, len)
                if iszero(barWidth) { barWidth := 1 }
                
                // Return flat array: offset = 32, size = len * 4
                let outSize := mul(len, 4)
                mstore(0x00, 32)
                mstore(0x20, outSize)
                
                for { let i := 0 } lt(i, len) { i := add(i, 1) } {
                    let v := calldataload(add(add(valOffset, 32), mul(i, 32)))
                    let scaledHeight := 0
                    if gt(maxVal, 0) {
                        scaledHeight := div(mul(v, 160), maxVal)
                    }
                    
                    let x1 := mul(i, barWidth)
                    let y1 := sub(180, scaledHeight)
                    let x2 := sub(add(x1, barWidth), 2)
                    let y2 := 180
                    
                    let baseIdx := add(0x40, mul(i, 128))
                    mstore(baseIdx, x1)
                    mstore(add(baseIdx, 32), y1)
                    mstore(add(baseIdx, 64), x2)
                    mstore(add(baseIdx, 96), y2)
                }
                
                let totalBytes := add(0x40, mul(outSize, 32))
                return(0x00, totalBytes)
            }

            // ----------------------------------------------------------------
            // METHOD 3: evaluateJobDecisionTree(uint256[] answers) -> (uint256 jobIndex)
            // Selector: 0x88d99f86
            // Emulates B.W. Behling's occupation classification tree from Ahoy! Issue 3.
            // answers format: binary 1 (Yes) or 0 (No) for decision tree nodes.
            // ----------------------------------------------------------------
            if eq(selector, 0x88d99f86) {
                let ansOffset := add(4, calldataload(4))
                let len := calldataload(ansOffset)
                
                // Read questions answers (default to 0 if out of bounds)
                let a0 := 0
                let a1 := 0
                let a2 := 0
                
                if gt(len, 0) { a0 := calldataload(add(add(ansOffset, 32), 0)) }
                if gt(len, 1) { a1 := calldataload(add(add(ansOffset, 32), 32)) }
                if gt(len, 2) { a2 := calldataload(add(add(ansOffset, 32), 64)) }
                
                let jobIndex := 0 // Default: Unemployed / General
                
                // Q0: Do you work with computers?
                if a0 {
                    // Q1: Do you write code?
                    if a1 {
                        jobIndex := 1 // Software Engineer / Developer
                    }
                    if iszero(a1) {
                        jobIndex := 2 // IT Specialist / Support
                    }
                }
                if iszero(a0) {
                    // Q1: Do you work in medical?
                    if a1 {
                        // Q2: Do you perform surgeries?
                        if a2 {
                            jobIndex := 3 // Surgeon
                        }
                        if iszero(a2) {
                            jobIndex := 4 // Nurse / Practitioner
                        }
                    }
                    if iszero(a1) {
                        // Q2: Do you teach students?
                        if a2 {
                            jobIndex := 5 // Teacher / Educator
                        }
                        if iszero(a2) {
                            jobIndex := 6 // Artist / Creator
                        }
                    }
                }
                
                mstore(0x00, jobIndex)
                return(0x00, 32)
            }

            revert(0, 0)
        }
    }
}
