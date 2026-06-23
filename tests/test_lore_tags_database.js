const assert = require("assert");
const fs = require("fs");
const path = require("path");

console.log("=============================================================");
console.log("Auncient Lore Tag Database Unit Tests");
console.log("=============================================================");

const DB_PATH = path.join(__dirname, "../config/lore_tags_database.json");

function runTests() {
    try {
        console.log("1. Verifying lore tags database exists...");
        assert.ok(fs.existsSync(DB_PATH), "Lore tags database file should exist");
        console.log("   ✓ Database file exists.");

        console.log("2. Loading and validating database schema...");
        const db = JSON.parse(fs.readFileSync(DB_PATH, "utf8"));
        
        assert.ok(db.meta, "Database must contain 'meta' block");
        assert.ok(db.documentToTags, "Database must contain 'documentToTags' mapping");
        assert.ok(db.tagToDocuments, "Database must contain 'tagToDocuments' mapping");
        
        assert.ok(db.meta.totalDocuments > 0, "Total documents count must be greater than zero");
        assert.ok(db.meta.totalUniqueTags > 0, "Total unique tags count must be greater than zero");
        console.log(`   ✓ Loaded successfully. Metadata: ${db.meta.totalDocuments} docs, ${db.meta.totalUniqueTags} tags.`);

        console.log("3. Validating specific document-to-tags relationship...");
        const targetDoc = "DYSNOMIA_VM_LORE.md";
        const tags = db.documentToTags[targetDoc];
        assert.ok(tags, `Document '${targetDoc}' must be present in database`);
        assert.ok(tags.includes("dysnomia"), "Tags list must include 'dysnomia'");
        assert.ok(tags.includes("lore"), "Tags list must include 'lore'");
        console.log(`   ✓ Found tags for ${targetDoc}:`, tags);

        console.log("4. Validating specific tag-to-documents relationship...");
        const targetTag = "zmachine";
        const docs = db.tagToDocuments[targetTag];
        assert.ok(docs, `Tag '${targetTag}' must be present in database`);
        assert.ok(docs.length > 0, `Tag '${targetTag}' must reference at least one document`);
        
        // Check that at least one zmachine spec file is linked
        const hasSpec = docs.some(d => d.toLowerCase().includes("zmachine"));
        assert.ok(hasSpec, "Linked documents list should contain a zmachine-related doc");
        console.log(`   ✓ Found docs for tag '${targetTag}' (sample):`, docs.slice(0, 5));

        console.log("5. Testing case normalization and constraint bounds...");
        // Ensure no "ancient" keys are present (must be normalized to "auncient")
        assert.strictEqual(db.tagToDocuments["ancient"], undefined, "The tag 'ancient' must not exist (normalized to auncient)");
        console.log("   ✓ Verified strict 'Auncient' terminology compliance.");

        console.log("\n=============================================================");
        console.log("ALL LORE RELATIONAL TAG TESTS PASSED");
        console.log("=============================================================");
    } catch (err) {
        console.error("\nTEST RUN FAILURE:", err.message);
        process.exit(1);
    }
}

runTests();
