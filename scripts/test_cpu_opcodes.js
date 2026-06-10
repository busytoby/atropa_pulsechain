const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = "/home/mariarahel/src/tsfi2/atropa_pulsechain/config/user_config.json";

const cpuABI = [
    "function getCPUState() public view returns (uint256, uint256, uint256, uint256, uint256, uint256)",
    "function runSteps(uint256 maxSteps) public returns (uint256)",
    "function poke(uint256 addr, uint256 val) public returns (uint256)",
    "function peek(uint256 addr) public view returns (uint256)",
    "function pokeBytes(uint256 startAddr, bytes calldata data) public returns (uint256)",
    "function batchPoke(uint256[] calldata addrs, uint256[] calldata vals) public returns (uint256)"
];

async function main() {
    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const cpu6502Address = config.networks.localhost.cpu6502Address;
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const deployer = signers[0];
    const cpu = new ethers.Contract(cpu6502Address, cpuABI, deployer);

    console.log("=== Running 6502 Opcode Isolation Tests ===");

    let nonce = await provider.getTransactionCount(deployer.address, "pending");

    async function sendTx(promise) {
        const tx = await promise;
        nonce++;
        return tx.wait();
    }

    // Test helper to load program, run, and check state
    async function testProgram(name, bytes, setupFunc, assertFunc, maxSteps) {
        console.log(`\nRunning Test: ${name}`);
        
        // Reset CPU State atomically
        await sendTx(cpu.batchPoke(
            [0x80, 0x81, 0x82, 0x83, 0x84, 0x85],
            [0, 0, 0, 0xFF, 0x20, 8192], // 0x83 is SP (0xFF), 0x84 is SR (0x20), 0x85 is PC (8192)
            { nonce }
        ));

        // Poke bytes atomically
        await sendTx(cpu.pokeBytes(8192, ethers.getBytes(new Uint8Array(bytes)), { nonce }));

        // Custom setup
        if (setupFunc) {
            await setupFunc(cpu);
        }

        // Run steps
        const steps = maxSteps || bytes.length;
        await sendTx(cpu.runSteps(steps, { nonce }));

        // Get and verify state
        const state = await cpu.getCPUState();
        const A = Number(state[0]);
        const X = Number(state[1]);
        const Y = Number(state[2]);
        const SR = Number(state[3]);
        const SP = Number(state[4]);
        const PC = Number(state[5]);

        try {
            await assertFunc({ A, X, Y, SR, SP, PC }, cpu);
            console.log(`  PASS: ${name}`);
        } catch (err) {
            console.error(`  FAIL: ${name}`);
            console.error(`  State: A=${A}, X=${X}, Y=${Y}, SR=0x${SR.toString(16)} (bin:${SR.toString(2)}), SP=0x${SP.toString(16)}, PC=0x${PC.toString(16)}`);
            console.error(err);
        }
    }

    // 1. Test Zero-Page Load/Store (LDA/STA zp)
    await testProgram(
        "LDA zp / STA zp",
        [
            0xA5, 0x10, // LDA $10
            0x85, 0x20, // STA $20
            0x00        // BRK
        ],
        async (cpu) => {
            await sendTx(cpu.batchPoke([0x10, 0x20], [42, 0], { nonce }));
        },
        async (state, cpu) => {
            if (state.A !== 42) throw new Error(`Expected A to be 42, got ${state.A}`);
            const val = Number(await cpu.peek(0x20));
            if (val !== 42) throw new Error(`Expected Zero Page $20 to be 42, got ${val}`);
        }
    );

    // 2. Test Compare X/Y Immediate (CPX #, CPY #)
    await testProgram(
        "CPX # / CPY #",
        [
            0xA2, 0x05, // LDX #5
            0xA0, 0x08, // LDY #8
            0xE0, 0x05, // CPX #5 (X == 5, sets Z, C)
            0xC0, 0x0A, // CPY #10 (Y < 10, clears Z, C, sets N)
            0x00        // BRK
        ],
        null,
        async (state) => {
            if (state.X !== 5) throw new Error(`Expected X to be 5, got ${state.X}`);
            if (state.Y !== 8) throw new Error(`Expected Y to be 8, got ${state.Y}`);
            const negativeSet = (state.SR & 0x80) !== 0;
            const zeroSet = (state.SR & 0x02) !== 0;
            const carrySet = (state.SR & 0x01) !== 0;
            if (!negativeSet) throw new Error("Expected Negative flag to be set");
            if (zeroSet) throw new Error("Expected Zero flag to be clear");
            if (carrySet) throw new Error("Expected Carry flag to be clear");
        }
    );

    // 3. Test Increment / Decrement Absolute (INC abs, DEC abs)
    await testProgram(
        "INC absolute / DEC absolute",
        [
            0xEE, 0x00, 0x30, // INC $3000
            0xCE, 0x00, 0x30, // DEC $3000
            0x00              // BRK
        ],
        async (cpu) => {
            await sendTx(cpu.poke(0x3000, 100, { nonce }));
        },
        async (state, cpu) => {
            const val = Number(await cpu.peek(0x3000));
            if (val !== 100) throw new Error(`Expected $3000 to be 100, got ${val}`);
        }
    );

    // 4. Test Subroutines & Stack (PHA, PLA, JSR, RTS)
    await testProgram(
        "Subroutines & Stack (PHA, PLA, JSR, RTS)",
        [
            0xA9, 0x42,       // LDA #$42
            0x48,             // PHA
            0xA9, 0x00,       // LDA #$00
            0x68,             // PLA
            0x20, 0x0B, 0x20, // JSR $200B (8192 + 11 = 8203)
            0x00,             // BRK (at 0x2009)
            0x00,             // Padding/BRK (at 0x200A)
            0xE8,             // INX (at 0x200B)
            0x60              // RTS (at 0x200C)
        ],
        null,
        async (state) => {
            if (state.A !== 0x42) throw new Error(`Expected A to be 0x42, got 0x${state.A.toString(16)}`);
            if (state.X !== 1) throw new Error(`Expected X to be 1 (incremented in subroutine), got ${state.X}`);
            if (state.SP !== 0xFF) throw new Error(`Expected Stack Pointer to be 0xFF after return, got 0x${state.SP.toString(16)}`);
        },
        10 // maxSteps
    );

    // 5. Test Branching, Loops & Indexed Stores (LDX, DEX, BNE, STA abs,X)
    await testProgram(
        "Branching & Loops (DEX/BNE Countdown)",
        [
            0xA2, 0x05,       // LDX #$05
            0x8A,             // TXA (loop start at 0x2002)
            0x9D, 0x00, 0x30, // STA $3000, X
            0xCA,             // DEX
            0xD0, 0xF9,       // BNE -7 (offset from PC 0x2009 to 0x2002)
            0x00              // BRK
        ],
        null,
        async (state, cpu) => {
            if (state.X !== 0) throw new Error(`Expected X to be 0 at end of loop, got ${state.X}`);
            for (let i = 1; i <= 5; i++) {
                const val = Number(await cpu.peek(0x3000 + i));
                if (val !== i) {
                    throw new Error(`Expected $3000 + ${i} to contain ${i}, got ${val}`);
                }
            }
        },
        30 // maxSteps
    );

    // 6. Test Arithmetic Flags (CLC, ADC, SEC, SBC)
    await testProgram(
        "Arithmetic Flags (ADC / SBC Carry, Zero, Negative)",
        [
            0x18,             // CLC
            0xA9, 0xFF,       // LDA #$FF
            0x69, 0x01,       // ADC #$01 -> A=0x00, Carry (C) and Zero (Z) set, Negative (N) clear
            0x38,             // SEC
            0xE9, 0x01,       // SBC #$01 -> A=0xFF, Carry (C) and Zero (Z) clear, Negative (N) set
            0x00              // BRK
        ],
        null,
        async (state) => {
            if (state.A !== 0xFF) throw new Error(`Expected A to be 0xFF, got 0x${state.A.toString(16)}`);
            const carrySet = (state.SR & 0x01) !== 0;
            const zeroSet = (state.SR & 0x02) !== 0;
            const negativeSet = (state.SR & 0x80) !== 0;
            if (carrySet) throw new Error("Expected Carry flag (C) to be clear after SBC borrow");
            if (zeroSet) throw new Error("Expected Zero flag (Z) to be clear after SBC");
            if (!negativeSet) throw new Error("Expected Negative flag (N) to be set after SBC (result 0xFF)");
        },
        10 // maxSteps
    );
}

main().catch(console.error);

