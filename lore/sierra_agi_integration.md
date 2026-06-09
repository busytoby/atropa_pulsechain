# Sierra AGI (Adventure Game Interpreter) On-Chain Integration

This document details the virtual hardware, registers, and Yul contract interface design for the **Sierra AGI Controller** emulation on the TSFi2 platform. It brings classic graphic-adventure logic (movement coordinates, room-transitions, room bounding-box object collisions, and inventory validation) on-chain under a unified **Diyat Game Tax**.

---

## 1. Emulated Sierra AGI Register Map ($D6C0–$D6CF)

The AGI Controller registers reside at `$D6C0`–`$D6C7` (54976–54983):

| Register Address (Hex) | Register Address (Dec) | Access Mode | Description |
| :--- | :--- | :--- | :--- |
| `$D6C0` | `54976` | Read / Write | **Ego X Coordinate**: Current character horizontal position (0–159). |
| `$D6C1` | `54977` | Read / Write | **Ego Y Coordinate**: Current character vertical position (0–167). |
| `$D6C2` | `54978` | Read / Write | **Room ID**: Current room/screen state identifier. |
| `$D6C3` | `54979` | Write-Only | **AGI Command Strobe**: Writing `1` triggers room transition checks; writing `2` triggers interactive item checks. |
| `$D6C4` | `54980` | Read / Write | **Selected Object ID**: Item or obstacle identifier Ego is attempting to interact with. |
| `$D6C5` | `54981` | Read-Only | **Inventory Flags**: Bitmask representing key inventory objects (Bit 0: Gold Key, Bit 1: Unlocked Castle Door). |
| `$D6C7` | `54983` | Read-Only | **AGI Status Flags**: Returns state check output (Bit 0: Collision, Bit 1: Action Succeeded, Bit 2: Room Changed). |

---

## 2. Dynamic Room Boundary & Interaction Logic (Yul)

Writing `1` or `2` to `$D6C3` launches the AGI script interpreter to process position boundaries and object checks:

```yul
case 54979 { // Sierra AGI Command Selector ($D6C3)
    sstore(getUserSlot(54979), val)
    if val {
        // 1. Room Transition Check
        if eq(val, 1) {
            let x := sload(getUserSlot(54976))
            let y := sload(getUserSlot(54977))
            let room := sload(getUserSlot(54978))
            
            let newRoom := room
            let changed := 0
            
            // Exit West
            if lt(x, 5) {
                newRoom := sub(room, 1)
                sstore(getUserSlot(54976), 150) // Warp to East edge
                changed := 1
            }
            // Exit East
            if gt(x, 155) {
                newRoom := add(room, 1)
                sstore(getUserSlot(54976), 10) // Warp to West edge
                changed := 1
            }
            
            if changed {
                // Excise 1 OTRT Diyat Game Tax on room transition
                let playTax := 1
                if exciseOnChainTax(playTax) {
                    log3(0, 0, 0x6e9f2cb42838841da92788e02012c2b71239e040f7b2291e5b200ac8c7c3b925, getContextUser(), playTax)
                }
                sstore(getUserSlot(54978), newRoom)
                let flags := sload(getUserSlot(54983))
                sstore(getUserSlot(54983), or(flags, 4))
            }
        }
        
        // 2. Parser / Interaction Check
        if eq(val, 2) {
            let itemID := sload(getUserSlot(54980))
            let room := sload(getUserSlot(54978))
            let success := 0
            
            // Room 1 (Forest Cleave) -> Gold Key (ID: 10) at position X: [70, 90], Y: [90, 110]
            if eq(room, 1) {
                if eq(itemID, 10) {
                    let x := sload(getUserSlot(54976))
                    let y := sload(getUserSlot(54977))
                    if and(and(gt(x, 69), lt(x, 91)), and(gt(y, 89), lt(y, 111))) {
                        let inv := sload(getUserSlot(54981))
                        sstore(getUserSlot(54981), or(inv, 1)) // Get Gold Key
                        success := 1
                    }
                }
            }
            
            // Room 2 (Castle Gate) -> Castle Door (ID: 20) at position X: [50, 70], Y: [120, 140]
            if eq(room, 2) {
                if eq(itemID, 20) {
                    let x := sload(getUserSlot(54976))
                    let y := sload(getUserSlot(54977))
                    if and(and(gt(x, 49), lt(x, 71)), and(gt(y, 119), lt(y, 141))) {
                        let inv := sload(getUserSlot(54981))
                        if and(inv, 1) { // Requires Gold Key (Bit 0)
                            sstore(getUserSlot(54981), or(inv, 2)) // Castle Unlocked
                            success := 1
                        }
                    }
                }
            }
            
            let flags := sload(getUserSlot(54983))
            if success {
                sstore(getUserSlot(54983), or(flags, 2))
                
                // Emit SierraAgiSettled event log
                mstore(0x300, room)
                mstore(0x320, itemID)
                log2(0x300, 64, 0xe11ca69da21867c2d5885c07342880a18413b63200ff9dfe8bc9c8be7ce68239, getContextUser())
            }
            if iszero(success) {
                sstore(getUserSlot(54983), and(flags, not(2)))
            }
        }
        sstore(getUserSlot(54979), 0)
    }
}
```
