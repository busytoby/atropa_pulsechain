object "ThermalHalfLife" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            function SCALE() -> val { val := 1000000000000000000 } // 1e18

            // processSamples(uint256 count)
            // selector: 0x11223344
            if eq(shr(224, calldataload(0)), 0x11223344) {
                let count := calldataload(4)
                let scale := SCALE()
                let memOffset := 0x80

                // Load thermal and attenuation state
                let t_junction := sload(100)
                if iszero(t_junction) {
                    t_junction := 293000000000000000000 // 293K ambient
                }
                let attenuation := sload(101)
                if iszero(attenuation) {
                    attenuation := scale
                }

                for { let i := 0 } lt(i, count) { i := add(i, 1) } {
                    let input := calldataload(add(36, mul(i, 64)))
                    let statePack := calldataload(add(68, mul(i, 64)))

                    let isOverloading := shr(128, statePack)

                    // Virtual Power: P = input^2
                    let power := sdiv(mul(input, input), scale)

                    // dT = 10 * power - 0.1 * (t_junction - 293)
                    let heatTerm := mul(power, 10)
                    let coolingTerm := sdiv(sub(t_junction, 293000000000000000000), 10)
                    t_junction := add(t_junction, sub(heatTerm, coolingTerm))

                    if isOverloading {
                        // Quick attenuation drop on overload
                        attenuation := sdiv(mul(attenuation, 85), 100)
                    }
                    if iszero(isOverloading) {
                        // Exponential recovery using Yttrium 90 half-life step (10% recovery towards 1.0)
                        let diff := sub(scale, attenuation)
                        attenuation := add(attenuation, sdiv(mul(diff, 100000000000000000), scale))
                    }

                    let output := sdiv(mul(input, attenuation), scale)

                    // Thermal drift correction: -1% gain per degree K above 320K
                    let overheat := sub(t_junction, 320000000000000000000)
                    if sgt(overheat, 0) {
                        // corr = 1.0 - overheat / 100
                        let corr := sub(scale, sdiv(overheat, 100))
                        if slt(corr, 500000000000000000) {
                            corr := 500000000000000000 // floor at 50% gain
                        }
                        output := sdiv(mul(output, corr), scale)
                    }

                    mstore(add(memOffset, mul(i, 32)), output)
                }

                sstore(100, t_junction)
                sstore(101, attenuation)

                return(memOffset, mul(count, 32))
            }

            // processSample(int256 inputSignal, int256 stateAndThermal) -> int256 outputSignal
            // selector: 0x07a96d8c
            if eq(shr(224, calldataload(0)), 0x07a96d8c) {
                let input := calldataload(4)
                let statePack := calldataload(36)
                let scale := SCALE()

                let isOverloading := shr(128, statePack)

                let t_junction := sload(100)
                if iszero(t_junction) {
                    t_junction := 293000000000000000000
                }
                let attenuation := sload(101)
                if iszero(attenuation) {
                    attenuation := scale
                }

                let power := sdiv(mul(input, input), scale)
                let heatTerm := mul(power, 10)
                let coolingTerm := sdiv(sub(t_junction, 293000000000000000000), 10)
                t_junction := add(t_junction, sub(heatTerm, coolingTerm))

                if isOverloading {
                    attenuation := sdiv(mul(attenuation, 85), 100)
                }
                if iszero(isOverloading) {
                    let diff := sub(scale, attenuation)
                    attenuation := add(attenuation, sdiv(mul(diff, 100000000000000000), scale))
                }

                let output := sdiv(mul(input, attenuation), scale)

                let overheat := sub(t_junction, 320000000000000000000)
                if sgt(overheat, 0) {
                    let corr := sub(scale, sdiv(overheat, 100))
                    if slt(corr, 500000000000000000) {
                        corr := 500000000000000000
                    }
                    output := sdiv(mul(output, corr), scale)
                }

                sstore(100, t_junction)
                sstore(101, attenuation)

                mstore(0, output)
                return(0, 32)
            }
            revert(0, 0)
        }
    }
}
