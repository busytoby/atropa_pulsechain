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
    const multiWords = {}; // To store n-grams of size 2, 3, and 4

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

            // Extract multi-word sequences of lengths 2, 3, and 4
            for (let n = 2; n <= 4; n++) {
                if (i <= words.length - n) {
                    const sequence = words.slice(i, i + n);
                    const firstWord = sequence[0];
                    const lastWord = sequence[sequence.length - 1];

                    // Multi-word phrase discovery heuristic:
                    // Must not start or end with a stop word to ensure semantic integrity
                    if (!StopWords.has(firstWord) && !StopWords.has(lastWord)) {
                        const phrase = sequence.join(" ");
                        multiWords[phrase] = (multiWords[phrase] || 0) + 1;
                    }
                }
            }
        }
    }

    // Sort and limit output
    const rawTopUnigrams = Object.entries(unigrams)
        .sort((a, b) => b[1] - a[1])
        .slice(0, 150);

    const rawTopMultiWords = Object.entries(multiWords)
        .sort((a, b) => b[1] - a[1])
        .slice(0, 150);

    // Build rich snippets helper
    function getTermSnippets(term) {
        const snippets = [];
        const regex = new RegExp(`([^.!?\\n\\r]*?\\b${term.replace(/[-\/\\^$*+?.()|[\]{}]/g, '\\$&')}\\b[^.!?\\n\\r]*)`, "i");
        
        for (const filename of files) {
            const filePath = path.join(LORE_DIR, filename);
            const content = fs.readFileSync(filePath, "utf8").replace(/ancient/gi, "Auncient");
            
            // Split into lines/sentences
            const lines = content.split(/\r?\n/);
            for (const line of lines) {
                if (regex.test(line)) {
                    const cleanLine = line.replace(/[#\*_\-\>\|]/g, " ").trim();
                    if (cleanLine.length > term.length + 5 && !snippets.includes(cleanLine)) {
                        snippets.push(`${cleanLine} [${filename}]`);
                        if (snippets.length >= 3) break;
                    }
                }
            }
            if (snippets.length >= 3) break;
        }
        return snippets;
    }

    console.log("Extracting context snippets for top terms...");
    const topUnigrams = rawTopUnigrams.map(([word, count]) => ({
        word,
        count,
        snippets: getTermSnippets(word)
    }));

    const topMultiWords = rawTopMultiWords.map(([sequence, count]) => ({
        sequence,
        count,
        snippets: getTermSnippets(sequence)
    }));

    const glossary = {
        meta: {
            generatedAt: new Date().toISOString(),
            totalDocumentsAnalyzed: files.length,
            description: "Auncient Lore Glossary index tracking the most frequent unigrams and automatically discovered multi-word terms (length 2-4) with context snippets."
        },
        topWords: topUnigrams,
        topMultiWordTerms: topMultiWords
    };

    const outputDir = path.dirname(GLOSSARY_FILE);
    if (!fs.existsSync(outputDir)) {
        fs.mkdirSync(outputDir, { recursive: true });
    }

    fs.writeFileSync(GLOSSARY_FILE, JSON.stringify(glossary, null, 2), "utf8");
    console.log(`✓ Glossary successfully generated and written to: ${GLOSSARY_FILE}`);
    console.log(`✓ Extracted top ${topUnigrams.length} words and top ${topMultiWords.length} word sequences.`);
}

buildGlossary();
module.exports = { buildGlossary };
