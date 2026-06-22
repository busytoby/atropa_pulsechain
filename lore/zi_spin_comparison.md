# Dysnomia Zi: `Spin` Function Disassembly Analysis

We analyzed the `ZI` contract on PulseChain at address [`0xCbAdd3C3957Bd9D6C036863CB053FEccf3D53338`](https://otter.pulsechain.com/address/0xCbAdd3C3957Bd9D6C036863CB053FEccf3D53338) and compared it to the local Solidity implementation.

## Key Discovery
Unlike the `RING` contract (where the divisions were deleted), the mainnet deployment of `ZI` on PulseChain **retains the exact math logic and divisions** found in our local Solidity implementation.

This means that calling `Spin` under natural (initial) states where `Alpha.Entropy` or `Qing.Entropy()` is `0` **will always revert with a `Panic(0x12)` (Divide-by-zero) error**.

---

## Code Comparison

Both the local and PulseChain implementations perform identical operations:

```solidity
function Spin(uint256 QingWaat) public returns (uint256 Iota, uint256 Omicron, uint256 Omega, uint256 Eta) {
    uint256 Charge;
    User memory Alpha = Choa.Sei().Chan().Xie().Xia().Mai().Qi().Zuo().Cho().GetUser();
    QINGINTERFACE Qing = Choa.Sei().Chan().Xie().Xia().Mai().Qi().Zuo().GetQing(QingWaat);
    _mintToCap();
    (Charge, Omicron, Omega) = Choa.Sei().Chan().Xie().Power(QingWaat);
    Iota = Xiao.modExp(Omicron, Charge, Choa.Yuan(address(Tethys)));
    Omicron = Xiao.modExp(Omega, Charge, Choa.Yuan(address(Tethys)));
    
    // BOTH DIVISIONS ARE FULLY PRESENT ON PULSECHAIN
    Omega = Tethys.balanceOf(Alpha.On.Phi) / Alpha.Entropy;  // <--- Divides by Alpha.Entropy (reverts if 0)
    Eta = Tethys.balanceOf(address(Qing)) / Qing.Entropy();  // <--- Divides by Qing.Entropy() (reverts if 0)
}
```

---

## Disassembly Analysis Details

In the bytecode disassembly of the mainnet `ZI` contract, the `Spin` function wrapper routes to PC offset `0x08ef`:

1. **Setup and Cascading Queries**:
   - Gets `Alpha` via `GetUser()` and `Qing` via `GetQing(QingWaat)`.
   - Calls the internal `_mintToCap()` function located at PC offset `131c`.
   - Calls `Power(QingWaat)` and `modExp` twice.
2. **First Division (`Omega`)**:
   - Calls `Tethys.balanceOf(Alpha.On.Phi)`.
   - Jumps to the Safe Division helper function at PC offset `1904`:
     ```assembly
     109e: PUSH2 0x1904
     10a1: JUMP
     ```
3. **Second Division (`Eta`)**:
   - Calls `Tethys.balanceOf(address(Qing))`.
   - Jumps to the Safe Division helper function at PC offset `1904`:
     ```assembly
     1184: PUSH2 0x1904
     1187: JUMP
     ```

### Safe Division Helper (`1904`)
The helper function at `1904` is the standard Solidity 0.8+ panic-guarded division:
- It checks if the divisor is `0` (`190a: JUMPI` to division or revert).
- If the divisor is `0`, it reverts with the `Panic(0x12)` division-by-zero error (offset `1915: MSTORE` of `0x12` and `191d: REVERT`).
- If it is non-zero, it performs `1920: DIV`.
