// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

/**
 * @title BesselSolver
 * @notice On-chain numerical solver for half-integer Bessel functions (spherical Bessel J_{n+1/2})
 *         optimized for fixed-point math using inline Yul assembly.
 */
library BesselSolver {
    // 1e18 scaling factor for fixed-point arithmetic
    int256 constant SCALE = 1e18;

    /**
     * @notice Computes J_{order + 1/2}(x) using a 4-term Taylor series expansion.
     *         J_{n+1/2}(x) = sqrt(2 / (pi * x)) * j_n(x)
     *         where j_n(x) is the spherical Bessel function of the first kind.
     *         For small x, j_n(x) ≈ x^n / (1 * 3 * 5 * ... * (2n+1)) * (1 - x^2/(2*(2n+3)) + ...)
     * @param order The index n of the half-integer order (n = 0, 1, 2, ...)
     * @param x The scaled input value (scaled by 1e18)
     * @return result The computed value (scaled by 1e18)
     */
    function computeHalfBessel(uint256 order, int256 x) public pure returns (int256 result) {
        // Handle boundary condition for x = 0
        if (x == 0) {
            return order == 0 ? SCALE : int256(0);
        }

        assembly {
            // SCALE is 1e18
            let scale := 1000000000000000000

            // Term 1: x^n / (2n+1)!!
            let term1 := scale
            let double_fact := 1
            for { let i := 0 } lt(i, order) { i := add(i, 1) } {
                // term1 = (term1 * x) / scale
                term1 := sdiv(mul(term1, x), scale)
                double_fact := mul(double_fact, add(mul(i, 2), 3))
            }
            term1 := sdiv(mul(term1, scale), double_fact)

            // Term 2: -x^2 / (2 * (2n + 3))
            let x2 := sdiv(mul(x, x), scale)
            let denom2 := mul(2, add(mul(order, 2), 3))
            let factor2 := sub(scale, sdiv(mul(x2, scale), denom2))

            // Term 3: +x^4 / (8 * (2n + 3) * (2n + 5))
            let x4 := sdiv(mul(x2, x2), scale)
            let denom3 := mul(8, mul(add(mul(order, 2), 3), add(mul(order, 2), 5)))
            let factor3 := add(factor2, sdiv(mul(x4, scale), denom3))

            // Term 4: -x^6 / (48 * (2n + 3) * (2n + 5) * (2n + 7))
            let x6 := sdiv(mul(x4, x2), scale)
            let denom4 := mul(48, mul(add(mul(order, 2), 3), mul(add(mul(order, 2), 5), add(mul(order, 2), 7))))
            let factor4 := sub(factor3, sdiv(mul(x6, scale), denom4))

            // result = (term1 * factor4) / scale
            result := sdiv(mul(term1, factor4), scale)
        }
    }
}
