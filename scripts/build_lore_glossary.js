const fs = require("fs");
const path = require("path");

console.log("=============================================================");
console.log("Auncient Lore Glossary & N-gram Extraction Engine");
console.log("=============================================================");

const LORE_DIR = path.join(__dirname, "../lore");
const GLOSSARY_FILE = path.join(__dirname, "../config/lore_glossary.json");

// Basic English stop words to filter out uninformative words
const StopWords = new Set([
    "the", "a", "and", "of", "to", "in", "is", "that", "it", "on", "for", "as", "with",
    "was", "for", "by", "an", "at", "are", "this", "be", "from", "or", "have", "with",
    "which", "but", "not", "he", "she", "they", "we", "you", "i", "his", "her", "their",
    "its", "has", "had", "been", "were", "been", "will", "would", "can", "could", "should"
]);

function buildGlossary() {
    if (!fs.existsSync(LORE_DIR)) {
        console.error(`Error: Lore directory does not exist at ${LORE_DIR}`);
        process.exit(1);
    }

    const files = fs.readdirSync(LORE_DIR).filter(f => f.endsWith(".md") || f.endsWith(".lore"));
    console.log(`Analyzing word frequencies across ${files.length} lore documents...`);

    const unigrams = {};
    const bigrams = {};

    for (const filename of files) {
        const filePath = path.join(LORE_DIR, filename);
        let content = fs.readFileSync(filePath, "utf8");

        // Enforce Auncient spelling constraint on content
        content = content.replace(/ancient/gi, "Auncient");

        // Clean text: strip code blocks, inline code, links, and markdown formatting
        let cleanText = content
            .replace(/```[\s\S]*?```/g, "")
            .replace(/`[^`]*?`/g, "")
            .replace(/\[([^\]]+)\]\(([^)]+)\)/g, "$1")
            .replace(/[#\*_\-\[\]\(\)\>\|]/g, " ")
            .replace(/[.,\/#!$%\^&\*;:{}=\-_`~()?"'\n\r]/g, " ")
            .toLowerCase();

        const words = cleanText.split(/\s+/).filter(w => w.trim().length > 1);

        for (let i = 0; i < words.length; i++) {
            const w1 = words[i];

            // Record unigram frequency if not a stop word
            if (!StopWords.has(w1)) {
                unigrams[w1] = (unigrams[w1] || 0) + 1;
            }

            // Record bigram frequency (word sequence of length 2)
            if (i < words.length - 1) {
                const w2 = words[i + 1];
                if (!StopWords.has(w1) && !StopWords.has(w2)) {
                    const bigram = `${w1} ${w2}`;
                    bigrams[bigram] = (bigrams[bigram] || 0) + 1;
                }
            }
        }
    }

    // Sort and limit output
    const topUnigrams = Object.entries(unigrams)
        .sort((a, b) => b[1] - a[1])
        .slice(0, 150)
        .map(([word, count]) => ({ word, count }));

    const topBigrams = Object.entries(bigrams)
        .sort((a, b) => b[1] - a[1])
        .slice(0, 150)
        .map(([sequence, count]) => ({ sequence, count }));

    const glossary = {
        meta: {
            generatedAt: new Date().toISOString(),
            totalDocumentsAnalyzed: files.length,
            description: "Auncient Lore Glossary index tracking the most frequent unigrams and bigram word sequences."
        },
        topWords: topUnigrams,
        topSequences: topBigrams
    };

    const outputDir = path.dirname(GLOSSARY_FILE);
    if (!fs.existsSync(outputDir)) {
        fs.mkdirSync(outputDir, { recursive: true });
    }

    fs.writeFileSync(GLOSSARY_FILE, JSON.stringify(glossary, null, 2), "utf8");
    console.log(`✓ Glossary successfully generated and written to: ${GLOSSARY_FILE}`);
    console.log(`✓ Extracted top ${topUnigrams.length} words and top ${topBigrams.length} word sequences.`);
}

buildGlossary();
module.exports = { buildGlossary };
