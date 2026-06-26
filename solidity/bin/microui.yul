object "MicroUI" {
    // ========================================================================
    // DEPLOYMENT INITIALIZATION BLOCK (INITCODE)
    // ========================================================================
    code {
        if callvalue() { revert(0, 0) }
        datacopy(0x00, dataoffset("runtime"), datasize("runtime"))
        return(0x00, datasize("runtime"))
    }

    // ========================================================================
    // MAIN RUNTIME ENVIRONMENT
    // ========================================================================
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))

            // Standard layout config stored in Transient Storage (0x0 - 0x100)
            // 0x01: Mouse X
            // 0x02: Mouse Y
            // 0x03: Mouse Button Clicked (Left = 1)
            // 0x10: Current draw command offset/count

            // ----------------------------------------------------------------
            // METHOD 1: updateInputState(uint16 x, uint16 y, uint8 buttonState) -> void
            // Selector: 0x8b5c90d2
            // ----------------------------------------------------------------
            if eq(selector, 0x8b5c90d2) {
                let x := and(calldataload(4), 0xFFFF)
                let y := and(calldataload(36), 0xFFFF)
                let btn := and(calldataload(68), 0xFF)
                storeTransient(0x01, x)
                storeTransient(0x02, y)
                storeTransient(0x03, btn)
                return(0, 0)
            }

            // ----------------------------------------------------------------
            // METHOD 2: drawRect(uint16 x, uint16 y, uint16 w, uint16 h, uint32 color) -> void
            // Selector: 0x9e7a02c8
            // ----------------------------------------------------------------
            if eq(selector, 0x9e7a02c8) {
                let x := and(calldataload(4), 0xFFFF)
                let y := and(calldataload(36), 0xFFFF)
                let w := and(calldataload(68), 0xFFFF)
                let h := and(calldataload(100), 0xFFFF)
                let color := calldataload(132)

                // Log immediate-mode draw call for the Vulkan compositor to ingest
                // Event hash: 0x9e7a02c89e7a02c89e7a02c89e7a02c89e7a02c89e7a02c89e7a02c89e7a02c8
                mstore(0x00, x)
                mstore(0x20, y)
                mstore(0x40, w)
                mstore(0x60, h)
                mstore(0x80, color)
                log1(0x00, 160, 0x9e7a02c89e7a02c89e7a02c89e7a02c89e7a02c89e7a02c89e7a02c89e7a02c8)
                return(0, 0)
            }

            // ----------------------------------------------------------------
            // METHOD 3: button(uint16 x, uint16 y, uint16 w, uint16 h) -> uint8 (1: Clicked, 0: Idle)
            // Selector: 0xb5ba0c68
            // ----------------------------------------------------------------
            if eq(selector, 0xb5ba0c68) {
                let x := and(calldataload(4), 0xFFFF)
                let y := and(calldataload(36), 0xFFFF)
                let w := and(calldataload(68), 0xFFFF)
                let h := and(calldataload(100), 0xFFFF)

                let mx := loadTransient(0x01)
                let my := loadTransient(0x02)
                let click := loadTransient(0x03)

                let hover := and(
                    and(ge(mx, x), lt(mx, add(x, w))),
                    and(ge(my, y), lt(my, add(y, h)))
                )

                // Draw button border/body depending on hover state
                let color := 0x444444 // default grey
                if hover {
                    color := 0x666666 // lighter grey
                    if click {
                        color := 0xa855f7 // brand purple when clicked
                    }
                }

                // Call internal drawRect helper
                mstore(0x00, x)
                mstore(0x20, y)
                mstore(0x40, w)
                mstore(0x60, h)
                mstore(0x80, color)
                log1(0x00, 160, 0x9e7a02c89e7a02c89e7a02c89e7a02c89e7a02c89e7a02c89e7a02c89e7a02c8)

                let isClicked := and(hover, click)
                mstore(0x00, isClicked)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 4: slider(uint16 x, uint16 y, uint16 w, uint16 h, uint16 val, uint16 minVal, uint16 maxVal) -> uint16
            // Selector: 0x0b940b51
            // ----------------------------------------------------------------
            if eq(selector, 0x0b940b51) {
                let x := and(calldataload(4), 0xFFFF)
                let y := and(calldataload(36), 0xFFFF)
                let w := and(calldataload(68), 0xFFFF)
                let h := and(calldataload(100), 0xFFFF)
                let val := and(calldataload(132), 0xFFFF)
                let minVal := and(calldataload(164), 0xFFFF)
                let maxVal := and(calldataload(196), 0xFFFF)

                let mx := loadTransient(0x01)
                let my := loadTransient(0x02)
                let click := loadTransient(0x03)

                let hover := and(
                    and(ge(mx, x), lt(mx, add(x, w))),
                    and(ge(my, y), lt(my, add(y, h)))
                )

                let result := val

                if and(hover, click) {
                    let rx := sub(mx, x)
                    let range := sub(maxVal, minVal)
                    result := add(minVal, div(mul(rx, range), w))
                    if lt(result, minVal) { result := minVal }
                    if gt(result, maxVal) { result := maxVal }
                }

                // Draw background bar (dark grey)
                mstore(0x00, x)
                mstore(0x20, y)
                mstore(0x40, w)
                mstore(0x60, h)
                mstore(0x80, 0x222222)
                log1(0x00, 160, 0x9e7a02c89e7a02c89e7a02c89e7a02c89e7a02c89e7a02c89e7a02c89e7a02c8)

                // Draw handle (purple indicator, 8px wide)
                let range_val := sub(maxVal, minVal)
                let handle_x := x
                if gt(range_val, 0) {
                    handle_x := add(x, div(mul(sub(result, minVal), sub(w, 8)), range_val))
                }

                mstore(0x00, handle_x)
                mstore(0x20, y)
                mstore(0x40, 8)
                mstore(0x60, h)
                mstore(0x80, 0xa855f7)
                log1(0x00, 160, 0x9e7a02c89e7a02c89e7a02c89e7a02c89e7a02c89e7a02c89e7a02c89e7a02c8)

                mstore(0x00, result)
                return(0x00, 32)
            }

            // ----------------------------------------------------------------
            // METHOD 5: toggle(uint16 x, uint16 y, uint16 w, uint16 h, uint8 state) -> uint8
            // Selector: 0x407470f5
            // ----------------------------------------------------------------
            if eq(selector, 0x407470f5) {
                let x := and(calldataload(4), 0xFFFF)
                let y := and(calldataload(36), 0xFFFF)
                let w := and(calldataload(68), 0xFFFF)
                let h := and(calldataload(100), 0xFFFF)
                let state := and(calldataload(132), 0xFF)

                let mx := loadTransient(0x01)
                let my := loadTransient(0x02)
                let click := loadTransient(0x03)

                let hover := and(
                    and(ge(mx, x), lt(mx, add(x, w))),
                    and(ge(my, y), lt(my, add(y, h)))
                )

                let result := state

                if and(hover, click) {
                    result := iszero(state)
                }

                // Draw toggle background (grey if off, green if on)
                let bg_color := 0x444444
                if result { bg_color := 0x22c55e }

                mstore(0x00, x)
                mstore(0x20, y)
                mstore(0x40, w)
                mstore(0x60, h)
                mstore(0x80, bg_color)
                log1(0x00, 160, 0x9e7a02c89e7a02c89e7a02c89e7a02c89e7a02c89e7a02c89e7a02c89e7a02c8)

                mstore(0x00, result)
                return(0x00, 32)
            }

            revert(0, 0)

            // ================================================================
            // MATH COMPATIBILITY HELPERS
            // ================================================================
            function ge(a, b) -> r {
                r := or(gt(a, b), eq(a, b))
            }

            // Standard transient storage loaders/storage helpers
            function storeTransient(slot, val) {
                sstore(add(0xF000000000000000000000000000000000000000000000000000000000000000, slot), val)
            }

            function loadTransient(slot) -> val {
                val := sload(add(0xF000000000000000000000000000000000000000000000000000000000000000, slot))
            }
        }
    }
}
