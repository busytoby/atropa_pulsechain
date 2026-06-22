# Dysnomia Ring: `Eta` Function Disassembly Analysis

We analyzed the `RING` contract on PulseChain at address [`0x1574c84Ec7fA78fC6C749e1d242dbde163675e72`](https://otter.pulsechain.com/address/0x1574c84Ec7fA78fC6C749e1d242dbde163675e72) and compared it to the local Solidity implementation.

## Key Discovery
The mainnet deployment of the `RING` contract on PulseChain **completely lacks the division and multiplication math logic** present in our local Solidity contract. Specifically, the division operations that cause divide-by-zero reverts in `META` do not exist in the deployed `RING` bytecode.

---

## Code Comparison

Here is the direct comparison of the `Eta` function logic:

```carousel
### Local Solidity (`03_ring.sol`)
```solidity
function Eta() public returns (uint256 Phoebe, uint256 Iota, uint256 Chao, uint256 Charge) {
    uint256 Omicron;
    uint256 Omega;
    (YUEINTERFACE Yue, LAU Usertoken) = Pang.Zi().Choa().Sei().Chi();
    Chao = Yue.React(address(Phobos));
    (Iota, Omicron, Phoebe, Omega, Charge)  = Pang.Push(Phobos.Waat());
    
    // CRITICAL MATH IN LOCAL SOLIDER
    Chao = Chao / Omicron;        // <--- Reverts locally if Omicron is 0
    Charge = Charge / Omega;      // <--- Reverts locally if Omega is 0
    Moments[Usertoken.Saat(1)] = Iota;
    Iota = Iota * Iota;           // <--- Squaring math
}
```
<!-- slide -->
### PulseChain Bytecode Behavior (Equivalent Code)
```solidity
function Eta() public returns (uint256 Phoebe, uint256 Iota, uint256 Chao, uint256 Charge) {
    uint256 Omicron;
    uint256 Omega;
    (YUEINTERFACE Yue, LAU Usertoken) = Pang.Zi().Choa().Sei().Chi();
    Chao = Yue.React(address(Phobos));
    (Iota, Omicron, Phoebe, Omega, Charge)  = Pang.Push(Phobos.Waat());

    // ALL MATH OMITTED ON PULSECHAIN
    Moments[Usertoken.Saat(1)] = Iota; 
    
    // No divisions or squaring operations are performed!
}
```
```

---

## Disassembly Analysis Details

In the bytecode disassembly of the mainnet `RING` contract, the entry point of the `Eta` function (`0x4209b07c`) resolves to `0x08d7`:

1. **Setup**: It initializes 6 slots on the stack (for the 4 return values and 2 local variables):
   ```assembly
   08d7: JUMPDEST
   08d8: PUSH0
   08d9: DUP1
   08da: PUSH0
   08db: DUP1
   08dc: PUSH0
   08dd: DUP1
   ```
2. **Sub-contract Calls**: It cascades the staticcalls to query `Pang.Zi().Choa().Sei().Chi()`, calling:
   - `0x7c396cc0` (`Zi()`)
   - `0xa5754463` (`Choa()`)
   - `0x6456147b` (`Sei()`)
   - `0x1cb77ea7` (`Chi()`)
3. **Execution**:
   - Calls `Yue.React(address(Phobos))` via selector `0x7a9bd221`.
   - Queries `Phobos.Waat()` via selector `0xa138da0b`.
   - Calls `Pang.Push()` via selector `0xd5220cc9` returning 5 decoded parameters.
4. **Where the Math is Omitted**:
   - In a compiler output that translates `Chao = Chao / Omicron`, we would expect a call to the compiler-generated Safe Division helper (which exists in this bytecode at PC offset `1571` but is only referenced by the `Purchase` and `Redeem` functions) or a raw `DIV` instruction.
   - **No such references or division instructions (`DIV`) exist** anywhere inside the `Eta` function body block.
   - The value of `Iota` is directly written to the storage mapping `Moments[Usertoken.Saat(1)]` at PC offset `0x0de3` without being squared (`Iota * Iota` is omitted).

## Impact
Because the division operations `Chao = Chao / Omicron` and `Charge = Charge / Omega` were omitted from the deployed PulseChain code:
1. The returned value of `Chao` remains the raw value from `Yue.React(address(Phobos))`.
2. Under natural/initial state where `Yue.React` returns `0`, `Chao` is returned as `0`.
3. Because the `RING.sol` call itself does not attempt division by `Omicron` (which is also `0`), **`RING.Eta()` succeeds without reverting**.
4. However, the downstream contract `META.sol` attempts `Yeo = Yeo / Chao`, which panics with `DIVIDE_BY_ZERO` because `Chao` is `0`.
