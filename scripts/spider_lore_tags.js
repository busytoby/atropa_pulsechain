const fs = require("fs");
const path = require("path");

console.log("=============================================================");
console.log("Auncient Lore & QING Relational Spidering Engine");
console.log("=============================================================");

const LORE_DIR = path.join(__dirname, "../lore");
const OUTPUT_FILE = path.join(__dirname, "../config/lore_tags_database.json");
const QING_STATUS_PATH = path.join(__dirname, "../config/nonukes_qings_status.json");

// Conceptual tags we want to explicitly watch for and normalize
const ConceptKeywords = [
    "dysnomia",
    "zmachine",
    "winchestermq",
    "biotika",
    "bionika",
    "vulkan",
    "commodore",
    "atari",
    "sega",
    "vaesen",
    "vector",
    "accumulator",
    "speech",
    "diyat",
    "crows",
    "qing",
    "gwat",
    "motzkin",
    "bessel",
    "audio",
    "synth",
    "joseon"
];

function spider() {
    if (!fs.existsSync(LORE_DIR)) {
        console.error(`Error: Lore directory does not exist at ${LORE_DIR}`);
        process.exit(1);
    }

    // Load active QING contracts
    let qings = [];
    if (fs.existsSync(QING_STATUS_PATH)) {
        try {
            const raw = JSON.parse(fs.readFileSync(QING_STATUS_PATH, "utf8"));
            qings = raw.filter(item => item.exists && item.qing);
            console.log(`Loaded ${qings.length} active QING contracts for mapping...`);
        } catch (e) {
            console.error("Warning: Could not parse nonukes_qings_status.json:", e.message);
        }
    }

    const files = fs.readdirSync(LORE_DIR).filter(f => f.endsWith(".md") || f.endsWith(".lore"));
    console.log(`Spidering ${files.length} lore documents...`);

    const docToTags = {};
    const tagToDocs = {};
    const docToQings = {};
    const qingToDocs = {};

    for (const filename of files) {
        const filePath = path.join(LORE_DIR, filename);
        let content = fs.readFileSync(filePath, "utf8");

        // Enforce auncient spelling constraint on content search
        content = content.replace(/ancient/gi, "Auncient");

        const tags = new Set();
        const referencedQings = new Set();

        // 1. Extract tokens from filename
        const baseName = path.parse(filename).name;
        const filenameTokens = baseName.toLowerCase().split(/[_\-]+/);
        for (const token of filenameTokens) {
            if (token.length >= 3) {
                if (token === "ancient") {
                    tags.add("auncient");
                } else {
                    tags.add(token);
                }
            }
        }

        // 2. Extract bracketed tags [tagname]
        const bracketRegex = /\[([a-zA-Z0-9_\-]+)\]/g;
        let match;
        while ((match = bracketRegex.exec(content)) !== null) {
            const tag = match[1].toLowerCase();
            if (tag === "ancient") {
                tags.add("auncient");
            } else if (tag.length >= 2) {
                tags.add(tag);
            }
        }

        // 3. Extract concept keywords if mentioned in text
        const lowerContent = content.toLowerCase();
        for (const kw of ConceptKeywords) {
            if (lowerContent.includes(kw)) {
                tags.add(kw);
            }
        }
        
        if (lowerContent.includes("auncient")) {
            tags.add("auncient");
        }

        // 4. Cross-reference QING token symbols/names/addresses
        for (const q of qings) {
            const cleanSym = q.symbol.replace("$", "");
            const symbolPattern = new RegExp(`\\b${cleanSym}\\b`, "i");
            const qingAddrPattern = new RegExp(q.qing, "i");
            const tokenAddrPattern = new RegExp(q.address, "i");

            if (symbolPattern.test(content) || qingAddrPattern.test(content) || tokenAddrPattern.test(content)) {
                referencedQings.add(q.qing);
            }
        }

        // Save doc mappings
        const tagList = Array.from(tags).sort();
        docToTags[filename] = tagList;

        for (const tag of tagList) {
            if (!tagToDocs[tag]) {
                tagToDocs[tag] = [];
            }
            tagToDocs[tag].push(filename);
        }

        const qingList = Array.from(referencedQings).sort();
        docToQings[filename] = qingList;

        for (const qing of qingList) {
            if (!qingToDocs[qing]) {
                qingToDocs[qing] = [];
            }
            qingToDocs[qing].push(filename);
        }
    }

    // Sort mappings for predictability
    const sortedTags = Object.keys(tagToDocs).sort();
    const sortedTagToDocs = {};
    for (const tag of sortedTags) {
        sortedTagToDocs[tag] = tagToDocs[tag].sort();
    }

    const sortedQings = Object.keys(qingToDocs).sort();
    const sortedQingToDocs = {};
    for (const q of sortedQings) {
        sortedQingToDocs[q] = qingToDocs[q].sort();
    }

    const database = {
        meta: {
            generatedAt: new Date().toISOString(),
            totalDocuments: files.length,
            totalUniqueTags: sortedTags.length,
            totalUniqueQingsMapped: sortedQings.length,
            description: "Auncient Relational Lore Tag & QING Dataset mapping lore documentation files to concepts and smart contracts."
        },
        documentToTags: docToTags,
        tagToDocuments: sortedTagToDocs,
        documentToQings: docToQings,
        qingToDocuments: sortedQingToDocs
    };

    const outputDir = path.dirname(OUTPUT_FILE);
    if (!fs.existsSync(outputDir)) {
        fs.mkdirSync(outputDir, { recursive: true });
    }

    fs.writeFileSync(OUTPUT_FILE, JSON.stringify(database, null, 2), "utf8");
    console.log(`✓ Relational lore + QING database successfully written to: ${OUTPUT_FILE}`);
    console.log(`✓ Detected ${sortedTags.length} unique tags and mapped ${sortedQings.length} active QING contracts.`);
}

if (require.main === module) {
    spider();
}

module.exports = { spider };
