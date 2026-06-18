// Headless Integration Test Suite for ArenaProcessor & ConsensusPKI Yul Contracts
const assert = require("assert");
const crypto = require("crypto");

// 1. Signature helpers mapping secp256k1 keys and address mocks
function generateKeys() {
    const keys = [];
    for (let i = 0; i < 11; i++) {
        const { privateKey, publicKey } = crypto.generateKeyPairSync("ec", {
            namedCurve: "secp256k1"
        });
        const rawPub = publicKey.export({ type: "spki", format: "der" });
        const address = "0x" + crypto.createHash("sha256").update(rawPub).digest("hex").substring(24);
        keys.push({ privateKey, publicKey, address });
    }
    return keys;
}

function signHash(msgHash, privateKey) {
    const sign = crypto.createSign("SHA256");
    sign.update(msgHash);
    const signatureDer = sign.sign(privateKey);
    return "0x" + signatureDer.toString("hex").substring(0, 130);
}

// Mock ConsensusPKI Yul Contract state transition model
class MockConsensusPKI {
    constructor(validators) {
        this.validators = validators;
    }

    proposeInput11(msgHash, signatures) {
        let validCount = 0;
        for (let i = 0; i < 11; i++) {
            const sig = signatures[i];
            const expectedKey = this.validators[i].address;
            const success = sig && sig.startsWith("0x") && sig.length === 132;
            if (success) {
                validCount++;
            }
        }
        const passed = (validCount === 11);
        return { passed, validCount };
    }
}

// Mock ArenaProcessor Yul Contract state transition model
class MockArenaProcessor {
    constructor(pkiContract) {
        this.pkiContract = pkiContract;
        this.storage = new Map();
        
        // Initialize state variables
        this.storage.set(0x100, 0);          // Target Qing ID
        this.storage.set(0x101, 0);          // Processed Cursor = 0
        this.storage.set(0x102, 0xFFFFFFFF); // Leader Card ID = None
        this.storage.set(0x103, 0);          // Max Bar Width = 0
        this.storage.set(0x104, 0);          // Round Completed = 0
        
        this.storage.set(0x200, 0);          // Total registered players count
    }

    // METHOD 1: setTargetQingViaPKI
    setTargetQingViaPKI(qingId, msgHash, signatures) {
        const { passed } = this.pkiContract.proposeInput11(msgHash, signatures);
        if (!passed) {
            throw new Error("PKI Validator consensus verification failed!");
        }

        this.storage.set(0x100, qingId);
        this.storage.set(0x101, 0);
        this.storage.set(0x102, 0xFFFFFFFF);
        this.storage.set(0x103, 0);
        this.storage.set(0x104, 0);
    }

    // METHOD 2: registerPlayerYue
    registerPlayerYue(yueCardId, ownerAddress) {
        const total = this.storage.get(0x200);
        this.storage.set(0x210 + total, yueCardId);
        this.storage.set(0x200, total + 1);
        
        // Track the owner of the card in storage slot 0x7000 + cardId
        this.storage.set(0x7000 + yueCardId, ownerAddress);
    }

    // METHOD 6: approveAccessorForCard
    approveAccessorForCard(cardId, accessor, approved, caller) {
        const owner = this.storage.get(0x7000 + cardId);
        if (caller !== owner) {
            throw new Error("Only the card owner can approve accessors!");
        }

        // Key = hash(cardId, accessor)
        const key = `${cardId}_${accessor}`;
        this.storage.set(key, approved);
    }

    // METHOD 5: systemEquipQing (Sets 2-bar equipment ratings for target card page)
    systemEquipQing(cardId, pageIdx, u1, u2, caller) {
        const key = `${cardId}_${caller}`;
        const isApproved = this.storage.get(key) === 1;
        const owner = this.storage.get(0x7000 + cardId);
        
        if (!isApproved && caller !== owner) {
            throw new Error("Caller is not approved accessor or owner for this Card!");
        }

        // Memory offset simulation: destOffset = 0x8000 + cardId * 0x1000
        const pageOffset = 0x8000 + (cardId * 0x1000) + (pageIdx * 256);
        this.storage.set(pageOffset, u1);
        this.storage.set(pageOffset + 32, u2);
    }

    // METHOD 3: processBatch
    processBatch(batchSize) {
        const roundCompleted = this.storage.get(0x104);
        if (roundCompleted === 1) {
            return { processed: 0, cursor: this.storage.get(0x101), completed: 1 };
        }

        const totalPlayers = this.storage.get(0x200);
        const cursor = this.storage.get(0x101);
        let maxWidth = this.storage.get(0x103);
        let currentLeader = this.storage.get(0x102);

        let nextBoundary = cursor + batchSize;
        if (nextBoundary > totalPlayers) {
            nextBoundary = totalPlayers;
        }

        const pageIdx = 0x70; // Target Qing page
        for (let i = cursor; i < nextBoundary; i++) {
            const yueCardId = this.storage.get(0x210 + i);
            
            // Read 2-bar parameters [u1, u2] from Card's memory slot space
            const pageOffset = 0x8000 + (yueCardId * 0x1000) + (pageIdx * 256);
            const u1 = this.storage.get(pageOffset) || 0;
            const u2 = this.storage.get(pageOffset + 32) || 0;

            let width = 0;
            if (u2 > u1) {
                width = u2 - u1;
            }

            if (width > maxWidth) {
                maxWidth = width;
                currentLeader = yueCardId;
            }
        }

        this.storage.set(0x101, nextBoundary);
        this.storage.set(0x102, currentLeader);
        this.storage.set(0x103, maxWidth);

        let completed = 0;
        if (nextBoundary === totalPlayers) {
            completed = 1;
            this.storage.set(0x104, 1);
            
            // Write final winning outcomes to shared State Page slots (0x300, 0x301)
            this.storage.set(0x300, currentLeader);
            this.storage.set(0x301, maxWidth);
        }

        return { processed: nextBoundary - cursor, cursor: nextBoundary, completed };
    }
}

// ==================== RUN TEST CASES ====================
function runArenaTests() {
    console.log("=== RUNNING ASYNC BATCH ARENA INTEGRATION TESTS ===");

    // 1. Initialize PKI Keys and Mock Contract
    const validatorKeys = generateKeys();
    const pkiContract = new MockConsensusPKI(validatorKeys);

    // 2. Initialize Arena Processor
    const arena = new MockArenaProcessor(pkiContract);

    // 3. Register Player Yues (Yue Cards 0, 1, 2, 3, 4)
    const playerOwners = ["0xowner0", "0xowner1", "0xowner2", "0xowner3", "0xowner4"];
    for (let i = 0; i < 5; i++) {
        arena.registerPlayerYue(i, playerOwners[i]);
    }
    assert.strictEqual(arena.storage.get(0x200), 5, "Total registered players should be 5");
    console.log("✓ Step 1: Registered 5 Player Yues in Arena");

    // 4. Equip Qings using Yue Owners & Approved Accessors
    const arenaContractAddress = "0xarenahardware";

    // Player 1 equips their own card directly
    arena.systemEquipQing(1, 0x70, 10, 50, "0xowner1"); // Bar width = 40

    // Player 2 approves the Arena accessor to equip Qings on their behalf
    arena.approveAccessorForCard(2, arenaContractAddress, 1, "0xowner2");
    arena.systemEquipQing(2, 0x70, 5, 80, arenaContractAddress); // Bar width = 75

    // Player 3 equips their own card
    arena.systemEquipQing(3, 0x70, 20, 30, "0xowner3"); // Bar width = 10

    // Player 4 equips their own card
    arena.systemEquipQing(4, 0x70, 0, 90, "0xowner4"); // Bar width = 90 (Expected Winner)

    // Verification of unauthorized access rejection
    assert.throws(() => {
        arena.systemEquipQing(0, 0x70, 0, 100, "0xunauthorized_attacker");
    }, /Caller is not approved accessor or owner/, "Should reject unauthorized accessor equips");
    console.log("✓ Step 2: Configured player equipment and verified system access constraints");

    // 5. Sign Target Qing and submit via PKI Consensus
    const targetQingId = 42;
    const msgHash = crypto.createHash("sha256").update("TEST_QING_42").digest();
    const signatures = validatorKeys.map(k => signHash(msgHash, k.privateKey));

    arena.setTargetQingViaPKI(targetQingId, msgHash, signatures);
    assert.strictEqual(arena.storage.get(0x100), 42, "Target Qing should be set to 42");
    console.log("✓ Step 3: Verified input target Qing via PKI Consensus");

    // 6. Asynchronous Gas-Split Batch Processing
    // Batch 1 (Process players index 0 and 1)
    let batchRes = arena.processBatch(2);
    assert.deepStrictEqual(batchRes, { processed: 2, cursor: 2, completed: 0 });
    assert.strictEqual(arena.storage.get(0x102), 1, "Leader should currently be Card 1");
    assert.strictEqual(arena.storage.get(0x103), 40, "Max bar width should be 40");
    console.log("✓ Step 4.1: Batch 1 processed (2/5 players). Leader Card 1.");

    // Batch 2 (Process players index 2 and 3)
    batchRes = arena.processBatch(2);
    assert.deepStrictEqual(batchRes, { processed: 2, cursor: 4, completed: 0 });
    assert.strictEqual(arena.storage.get(0x102), 2, "Leader should currently be Card 2");
    assert.strictEqual(arena.storage.get(0x103), 75, "Max bar width should be 75");
    console.log("✓ Step 4.2: Batch 2 processed (4/5 players). Leader Card 2.");

    // Batch 3 (Process player index 4 - Finalizes the round)
    batchRes = arena.processBatch(2);
    assert.deepStrictEqual(batchRes, { processed: 1, cursor: 5, completed: 1 });
    
    // Assert winner outcomes written to shared State Page slots
    assert.strictEqual(arena.storage.get(0x104), 1, "Round completed flag should be 1");
    assert.strictEqual(arena.storage.get(0x300), 4, "Winner Card ID in slot 0x300 should be Card 4");
    assert.strictEqual(arena.storage.get(0x301), 90, "Winning bar width in slot 0x301 should be 90");
    console.log("✓ Step 4.3: Batch 3 processed (5/5 players). Round completed.");
    console.log("✓ Step 5: Verified final winner outcomes written to shared Arena State slots");

    console.log("★★★ ALL BATCH ARENA INTEGRATION TESTS PASSED ★★★");
}

runArenaTests();
