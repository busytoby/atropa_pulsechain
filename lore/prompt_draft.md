# Teamwork Project Prompt — Draft

> Status: Launched
> Goal: Craft prompt → get user approval → delegate to teamwork_preview

An interactive hypervisor dashboard comparing native ZMM VM holdings and Anvil EVM holdings for deployed ERC20 tokens and Yul systems, showing full balances and listing deployed contracts without any mock placeholders.

Working directory: /home/mariarahel/src/tsfi2/atropa_pulsechain
Integrity mode: development

## Requirements

### R1. Dual VM Holdings Explorer
The dashboard must allow querying the local processes (Anvil on port 8545, ZMM VM on port 3000 via `/api/zmm-exec`) to fetch the holdings of the default wallet address (`0x00000000000000000000000000000000000004cc` for ZMM and `0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266` for Anvil).

### R2. Deployed Contracts Inventory
It must show both ERC20 token balances and list deployed Yul objects or system contracts (such as `zmachine` and `zmachineParser`) that may not have standard token balances.

### R3. Automated Comparison Validation
Implement a programmatic validation script that queries both endpoints (ZMM VM and Anvil EVM) directly, confirms they return valid comparable state data, and validates token distribution consistency.

## Acceptance Criteria

### System Integration
- [ ] A validation script successfully executes, querying both port 3000 and port 8545.
- [ ] The dashboard displays actual holdings fetched from the native ZMM VM process and Anvil EVM without placeholders.
- [ ] All deployed tokens and balance-less Yul contracts are listed correctly in the dashboard interface.

---
*Launched: delegation conversation ID `00c511e5-36cb-4d4a-a8d1-801c02b4b2e6`*
