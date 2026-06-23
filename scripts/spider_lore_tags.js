const fs = require("fs");
const path = require("path");

console.log("=============================================================");
console.log("Auncient Lore Tag Spidering Engine");
console.log("=============================================================");

const LORE_DIR = path.join(__dirname, "../lore");
const OUTPUT_FILE = path.join(__dirname, "../config/lore_tags_database.json");

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

    const files = fs.readdirSync(LORE_DIR).filter(f => f.endsWith(".md") || f.endsWith(".lore"));
    console.log(`Spidering ${files.length} lore documents...`);

    const docToTags = {};
    const tagToDocs = {};

    for (const filename of files) {
        const filePath = path.join(LORE_DIR, filename);
        let content = fs.readFileSync(filePath, "utf8");

        // Enforce auncient spelling constraint on content search
        content = content.replace(/ancient/gi, "Auncient");

        const tags = new Set();

        // 1. Extract tokens from filename
        const baseName = path.parse(filename).name;
        const filenameTokens = baseName.toLowerCase().split(/[_\-]+/);
        for (const token of filenameTokens) {
            if (token.length >= 3) {
                // If it's close to ancient, force spelling to preserve historical integrity
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
        
        // Also check explicitly for 'auncient' (since 'ancient' was normalized above)
        if (lowerContent.includes("auncient")) {
            tags.add("auncient");
        }

        const tagList = Array.from(tags).sort();
        docToTags[filename] = tagList;

        for (const tag of tagList) {
            if (!tagToDocs[tag]) {
                tagToDocs[tag] = [];
            }
            tagToDocs[tag].push(filename);
        }
    }

    // Sort tag lists to keep them predictable
    const sortedTags = Object.keys(tagToDocs).sort();
    const sortedTagToDocs = {};
    for (const tag of sortedTags) {
        sortedTagToDocs[tag] = tagToDocs[tag].sort();
    }

    const database = {
        meta: {
            generatedAt: new Date().toISOString(),
            totalDocuments: files.length,
            totalUniqueTags: sortedTags.length,
            description: "Auncient Relational Lore Tag Dataset mapping lore documentation files to concepts."
        },
        documentToTags: docToTags,
        tagToDocuments: sortedTagToDocs
    };

    // Ensure output parent directory exists
    const outputDir = path.dirname(OUTPUT_FILE);
    if (!fs.existsSync(outputDir)) {
        fs.mkdirSync(outputDir, { recursive: true });
    }

    fs.writeFileSync(OUTPUT_FILE, JSON.stringify(database, null, 2), "utf8");
    console.log(`✓ Relational lore tags database successfully written to: ${OUTPUT_FILE}`);
    console.log(`✓ Detected ${sortedTags.length} unique tags across ${files.length} documents.`);
}

spider();
