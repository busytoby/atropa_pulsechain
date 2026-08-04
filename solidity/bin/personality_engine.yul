object "PersonalityEngine" {
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
            // METHOD: simulate_snubber_clamped_flyback (peak_voltage, inductance, snubber_resistance, time_step)
            // Selector: 0xe399f0f0 (integer scaled by 1000)
            // ----------------------------------------------------------------
            if eq(selector, 0xe399f0f0) {
                let peak_voltage := calldataload(4)
                let inductance := calldataload(36)
                let snubber_resistance := calldataload(68)
                let time_step := calldataload(100)
                
                let clamped_voltage := 0
                if gt(inductance, 0) {
                    let X := div(mul(mul(time_step, snubber_resistance), 1000), inductance)
                    let factor := 1000
                    if lt(X, 1000) {
                        factor := sub(add(1000, div(mul(X, X), 2000)), X)
                    }
                    if iszero(lt(X, 1000)) {
                        factor := 0
                    }
                    clamped_voltage := div(mul(peak_voltage, factor), 1000)
                }
                
                mstore(0x00, clamped_voltage)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD: simulate_rcd_snubber_decay (peak_voltage, resistance, capacitance, inductance, time_step)
            // Selector: 0xe399f0f1 (integer scaled by 1000)
            // ----------------------------------------------------------------
            if eq(selector, 0xe399f0f1) {
                let peak_voltage := calldataload(4)
                let resistance := calldataload(36)
                let capacitance := calldataload(68)
                let inductance := calldataload(100)
                let time_step := calldataload(132)
                
                let decay_voltage := 0
                let denom := mul(resistance, capacitance)
                if gt(denom, 0) {
                    let X := div(mul(time_step, 1000000), denom)
                    let factor := 1000
                    if lt(X, 1000) {
                        factor := sub(add(1000, div(mul(X, X), 2000)), X)
                    }
                    if iszero(lt(X, 1000)) {
                        factor := 0
                    }
                    
                    let res_factor := 1000
                    let lc := mul(inductance, capacitance)
                    if gt(lc, 0) {
                        let y_sq := div(mul(mul(time_step, time_step), 1000000), lc)
                        if lt(y_sq, 2000) {
                            res_factor := sub(1000, div(y_sq, 2))
                        }
                        if iszero(lt(y_sq, 2000)) {
                            res_factor := 0
                        }
                    }
                    decay_voltage := div(mul(mul(peak_voltage, factor), res_factor), 1000000)
                }
                
                mstore(0x00, decay_voltage)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD: simulate_hbridge_flyback_transient (supply_voltage, load_inductance, switching_time_sec)
            // Selector: 0xe399f0f2 (integer scaled by 1000)
            // ----------------------------------------------------------------
            if eq(selector, 0xe399f0f2) {
                let supply_voltage := calldataload(4)
                let load_inductance := calldataload(36)
                let switching_time_sec := calldataload(68)
                
                let flyback_voltage := supply_voltage
                if gt(switching_time_sec, 0) {
                    let dI := 2000
                    let term := div(mul(load_inductance, dI), switching_time_sec)
                    flyback_voltage := add(supply_voltage, term)
                }
                
                mstore(0x00, flyback_voltage)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD: simulate_snubber_flyback_transient (supply_voltage, load_inductance, switching_time_sec, snubber_resistance)
            // Selector: 0xe399f0f3 (integer scaled by 1000)
            // ----------------------------------------------------------------
            if eq(selector, 0xe399f0f3) {
                let supply_voltage := calldataload(4)
                let load_inductance := calldataload(36)
                let switching_time_sec := calldataload(68)
                let snubber_resistance := calldataload(100)
                
                let flyback_voltage := supply_voltage
                if gt(switching_time_sec, 0) {
                    let dI := 2000
                    let term := div(mul(load_inductance, dI), switching_time_sec)
                    let peak := add(supply_voltage, term)
                    let denom := add(1000, mul(snubber_resistance, 20))
                    flyback_voltage := div(mul(peak, 1000), denom)
                }
                
                mstore(0x00, flyback_voltage)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD: evaluate_gumbel_flyback_transient_tax (flyback_voltage, threshold_limit)
            // Selector: 0xe399f0f4 (integer scaled by 1000)
            // ----------------------------------------------------------------
            if eq(selector, 0xe399f0f4) {
                let flyback_voltage := calldataload(4)
                let threshold_limit := calldataload(36)
                
                let tax := 0
                if gt(threshold_limit, 0) {
                    let x := div(mul(sub(flyback_voltage, threshold_limit), 1000), threshold_limit)
                    
                    let exp_x := 1000
                    let sign := 1
                    if slt(x, 0) {
                        sign := sub(0, 1)
                    }
                    
                    let abs_x := x
                    if slt(x, 0) {
                        abs_x := sub(0, x)
                    }
                    
                    if lt(abs_x, 1500) {
                        let term := add(1000, abs_x)
                        let term2 := div(mul(abs_x, abs_x), 2000)
                        if eq(sign, 1) {
                            exp_x := add(term, term2)
                        }
                        if eq(sign, sub(0, 1)) {
                            exp_x := add(sub(1000, abs_x), term2)
                        }
                    }
                    if iszero(lt(abs_x, 1500)) {
                        if eq(sign, 1) {
                            exp_x := 4500
                        }
                        if eq(sign, sub(0, 1)) {
                            exp_x := 220
                        }
                    }
                    
                    let exp_y := 1000
                    if lt(exp_x, 1500) {
                        exp_y := add(sub(1000, exp_x), div(mul(exp_x, exp_x), 2000))
                    }
                    if iszero(lt(exp_x, 1500)) {
                        exp_y := 0
                    }
                    
                    if lt(exp_y, 1000) {
                        tax := sub(1000, exp_y)
                    }
                }
                
                mstore(0x00, tax)
                return(0x00, 32)
            }

            revert(0, 0)
        }
    }
}
