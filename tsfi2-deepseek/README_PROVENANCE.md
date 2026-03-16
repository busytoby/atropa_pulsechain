# PulseChain C-Native Provenance Tests

The tests executed in this suite (`test_mariarahel_provenance.c` and `test_factory_provenance.c`) utilize the dependency-free `tsfi_pulsechain_rpc.c` client to probe the PulseChain mainnet. 

These tests directly fetch the state configuration of the Dysnomia token architecture by bypassing standard Application Binary Interface selectors and interrogating raw storage slots.

## Findings
The `mariarahel` token inherits a complex `CHATLOG` tree which culminates at `CHATLOG Zhou`. The `mariarahel` token was deployed by a factory smart contract which explicitly points to `CHATLOG Void` in its base memory slot (Slot 0). 

These tests were performed entirely in C.
