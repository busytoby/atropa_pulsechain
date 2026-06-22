object "QuaternionFlight" {
    code {
        datacopy(0x00, dataoffset("runtime"), datasize("runtime"))
        return(0x00, datasize("runtime"))
    }

    object "runtime" {
        code {
            if callvalue() { revert(0, 0) }
            if lt(calldatasize(), 4) { revert(0, 0) }

            let selector := shr(224, calldataload(0))
            let one := 1000000000000000000 // 1.0 in 1e18 scale

            function fixedMul(a, b) -> c {
                c := sdiv(mul(a, b), 1000000000000000000)
            }

            function fixedSqrt(val) -> res {
                if slt(val, 0) { res := 0 leave }
                let x := val
                let y := 1000000000000000000
                if gt(val, 0) {
                    for { let i := 0 } lt(i, 12) { i := add(i, 1) } {
                        x := sdiv(add(x, y), 2)
                        y := sdiv(mul(val, 1000000000000000000), x)
                    }
                }
                res := x
            }

            // 1. initializeAttitude() -> Reset to Identity Stance
            // Selector: 0x12b50de2
            if eq(selector, 0x12b50de2) {
                sstore(0, one) // qw = 1.0
                sstore(1, 0)   // qx = 0.0
                sstore(2, 0)   // qy = 0.0
                sstore(3, 0)   // qz = 0.0
                mstore(0, 1)
                return(0, 32)
            }

            // 2. getAttitude() -> Returns (qw, qx, qy, qz)
            // Selector: 0x7a6bc90e
            if eq(selector, 0x7a6bc90e) {
                mstore(0, sload(0))
                mstore(32, sload(1))
                mstore(64, sload(2))
                mstore(96, sload(3))
                return(0, 128)
            }

            // 3. updateAttitude(int256 velPitch, int256 velYaw, int256 velRoll) -> Updates and Returns (qw, qx, qy, qz)
            // Selector: 0x0ebc8577
            if eq(selector, 0x0ebc8577) {
                let vp := calldataload(4)
                let vy := calldataload(36)
                let vr := calldataload(68)

                // Retrieve current attitude
                let qw := sload(0)
                let qx := sload(1)
                let qy := sload(2)
                let qz := sload(3)

                // Calculate small-angle approximations for delta quaternions:
                // sin(theta/2) ~ theta/2, cos(theta/2) ~ 1 - (theta/2)^2 / 2
                let sp := sdiv(vp, 2)
                let cp := sub(one, sdiv(fixedMul(sp, sp), 2))

                let sy := sdiv(vy, 2)
                let cy := sub(one, sdiv(fixedMul(sy, sy), 2))

                let sr := sdiv(vr, 2)
                let cr := sub(one, sdiv(fixedMul(sr, sr), 2))

                // Delta quaternions:
                // qp = (cp, sp, 0, 0)
                // qy = (cy, 0, sy, 0)
                // qr = (cr, 0, 0, sr)

                // 1. Multiply Current * Pitch (q_temp1 = q_current * qp)
                // w1 = w*cp - x*sp
                // x1 = x*cp + w*sp
                // y1 = y*cp + z*sp
                // z1 = z*cp - y*sp
                let w1 := sub(fixedMul(qw, cp), fixedMul(qx, sp))
                let x1 := add(fixedMul(qx, cp), fixedMul(qw, sp))
                let y1 := add(fixedMul(qy, cp), fixedMul(qz, sp))
                let z1 := sub(fixedMul(qz, cp), fixedMul(qy, sp))

                // 2. Multiply Result1 * Yaw (q_temp2 = q_temp1 * qy)
                // w2 = w1*cy - y1*sy
                // x2 = x1*cy - z1*sy
                // y2 = y1*cy + w1*sy
                // z2 = z1*cy + x1*sy
                let w2 := sub(fixedMul(w1, cy), fixedMul(y1, sy))
                let x2 := sub(fixedMul(x1, cy), fixedMul(z1, sy))
                let y2 := add(fixedMul(y1, cy), fixedMul(w1, sy))
                let z2 := add(fixedMul(z1, cy), fixedMul(x1, sy))

                // 3. Multiply Result2 * Roll (q_final = q_temp2 * qr)
                // w3 = w2*cr - z2*sr
                // x3 = x2*cr + y2*sr
                // y3 = y2*cr - x2*sr
                // z3 = z2*cr + w2*sr
                let w3 := sub(fixedMul(w2, cr), fixedMul(z2, sr))
                let x3 := add(fixedMul(x2, cr), fixedMul(y2, sr))
                let y3 := sub(fixedMul(y2, cr), fixedMul(x2, sr))
                let z3 := add(fixedMul(z2, cr), fixedMul(w2, sr))

                // Re-normalize quaternion: q_norm = q_final / sqrt(w3^2 + x3^2 + y3^2 + z3^2)
                let mag2 := add(add(add(fixedMul(w3, w3), fixedMul(x3, x3)), fixedMul(y3, y3)), fixedMul(z3, z3))
                let mag := fixedSqrt(mag2)

                if gt(mag, 0) {
                    w3 := sdiv(mul(w3, one), mag)
                    x3 := sdiv(mul(x3, one), mag)
                    y3 := sdiv(mul(y3, one), mag)
                    z3 := sdiv(mul(z3, one), mag)
                }

                // Save to storage
                sstore(0, w3)
                sstore(1, x3)
                sstore(2, y3)
                sstore(3, z3)

                // Return final coordinate arrays
                mstore(0, w3)
                mstore(32, x3)
                mstore(64, y3)
                mstore(96, z3)
                return(0, 128)
            }
        }
    }
}
