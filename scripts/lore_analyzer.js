const fs = require("fs");
const path = require("path");

const loreAnalysisCache = {};

function performLoreAnalysis() {
    const loreDir = path.join(__dirname, "../lore");
    if (!fs.existsSync(loreDir)) return loreAnalysisCache;
    const files = fs.readdirSync(loreDir).filter(f => f.endsWith(".md") || f.endsWith(".lore"));
    
    // Normalize defined concepts to a map and set
    const conceptMap = {};
    files.forEach(f => {
        const base = path.basename(f, path.extname(f)).toLowerCase().replace(/_/g, ' ');
        conceptMap[base] = f;
    });
    const definedConcepts = new Set(Object.keys(conceptMap));
    
    const rawData = {};
    
    // PHASE 1 & PHASE 2: Structural and Lexical analysis pass
    files.forEach(file => {
        const fullPath = path.join(loreDir, file);
        try {
            const text = fs.readFileSync(fullPath, "utf8");
            
            const annotationsRegex = /<!--\s*AUNCIENT_ANNOTATIONS_START\s*([\s\S]*?)\s*AUNCIENT_ANNOTATIONS_END\s*-->/;
            const cleanText = text.replace(annotationsRegex, "");
            
            // Phase 1: Structural analysis
            const words = cleanText.split(/\s+/).filter(Boolean);
            const wordCount = words.length;
            
            const sentences = cleanText.split(/[.!?]+/).filter(s => s.trim().length > 0);
            const sentenceCount = sentences.length;
            
            let questionCount = 0;
            const match = text.match(annotationsRegex);
            if (match) {
                try {
                    const annotations = JSON.parse(match[1].trim());
                    annotations.forEach(ann => {
                        if (ann.comment && ann.comment.includes('?')) {
                            questionCount++;
                        }
                    });
                } catch (e) {}
            }
            
            const contentQuestions = (cleanText.match(/\?/g) || []).length;
            const codeBlockCount = (cleanText.match(/```/g) || []).length / 2;
            const headerCount = (cleanText.match(/^#{1,6}\s+/gm) || []).length;
            
            // Phase 2: Lexical Analysis & Jargon Extraction
            const jargonMatches = cleanText.match(/\b[A-Z][a-zA-Z0-9_]{2,}\b/g) || [];
            const uniqueJargon = [...new Set(jargonMatches)];
            const undefinedJargon = [];
            
            uniqueJargon.forEach(term => {
                const normalized = term.toLowerCase();
                if (!definedConcepts.has(normalized) && 
                    !['the', 'and', 'for', 'this', 'that', 'with', 'evm', 'pki', 'gdk', 'gtk', 'xml', 'api', 'rpc', 'sdk', 'url', 'uri', 'web', 'dom', 'cdp', 'cli', 'pid', 'sigint', 'sigterm', 'json', 'html', 'css', 'git', 'vcs', 'mjpeg', 'vulkan', 'wayland'].includes(normalized)) {
                    undefinedJargon.push(term);
                }
            });
            
            const todoMatches = cleanText.match(/\b(TODO|FIXME|TBD|PLACEHOLDER|UNRESOLVED|INVESTIGATE|DRAFT)\b/gi) || [];
            const todoCount = todoMatches.length;

            rawData[file] = {
                file,
                text: cleanText.toLowerCase(),
                wordCount,
                sentenceCount,
                questionCount: questionCount + contentQuestions,
                codeBlockCount,
                headerCount,
                undefinedJargon,
                todoCount,
                referencedConcepts: []
            };
        } catch (e) {}
    });

    // PHASE 3: Relationship Mapping and Concept Reference Graph (Aho-Corasick Optimized)
    class AhoCorasick {
        constructor() {
            this.trie = [ { next: {}, fail: 0, output: [] } ];
        }
        insert(pattern, file) {
            let state = 0;
            for (let i = 0; i < pattern.length; i++) {
                const char = pattern[i];
                if (!this.trie[state].next[char]) {
                    this.trie[state].next[char] = this.trie.length;
                    this.trie.push({ next: {}, fail: 0, output: [] });
                }
                state = this.trie[state].next[char];
            }
            this.trie[state].output.push(file);
        }
        build() {
            const queue = [];
            for (const char in this.trie[0].next) {
                const nextState = this.trie[0].next[char];
                this.trie[nextState].fail = 0;
                queue.push(nextState);
            }
            while (queue.length > 0) {
                const current = queue.shift();
                for (const char in this.trie[current].next) {
                    const nextState = this.trie[current].next[char];
                    let failState = this.trie[current].fail;
                    while (failState > 0 && !this.trie[failState].next[char]) {
                        failState = this.trie[failState].fail;
                    }
                    const resolvedFail = this.trie[failState].next[char] || 0;
                    this.trie[nextState].fail = resolvedFail;
                    this.trie[nextState].output = this.trie[nextState].output.concat(this.trie[resolvedFail].output);
                    queue.push(nextState);
                }
            }
        }
        search(text) {
            let state = 0;
            const matches = new Set();
            for (let i = 0; i < text.length; i++) {
                const char = text[i];
                while (state > 0 && !this.trie[state].next[char]) {
                    state = this.trie[state].fail;
                }
                state = this.trie[state].next[char] || 0;
                if (this.trie[state].output.length > 0) {
                    this.trie[state].output.forEach(file => matches.add(file));
                }
            }
            return Array.from(matches);
        }
    }

    const ac = new AhoCorasick();
    definedConcepts.forEach(concept => {
        ac.insert(concept, conceptMap[concept]);
    });
    ac.build();

    Object.keys(rawData).forEach(file => {
        const data = rawData[file];
        const matches = ac.search(data.text);
        matches.forEach(conceptFile => {
            if (conceptFile !== file) {
                data.referencedConcepts.push(conceptFile);
            }
        });
    });

    // PHASE 4: Score Aggregation and Weight Synthesis
    const baseAnalysis = {};
    Object.keys(rawData).forEach(file => {
        const data = rawData[file];
        let score = 0;
        const reasons = [];

        if (data.wordCount < 150) {
            score += 100;
            reasons.push(`Sparse content (${data.wordCount} words)`);
        } else if (data.wordCount < 400) {
            score += 30;
            reasons.push(`Short document (${data.wordCount} words)`);
        }
        
        if (data.headerCount === 0) {
            score += 25;
            reasons.push(`Lacks proper Markdown headers`);
        }

        if (data.questionCount > 0) {
            score += data.questionCount * 40;
            reasons.push(`Contains ${data.questionCount} unresolved question(s)`);
        }

        if (data.undefinedJargon.length > 0) {
            const added = Math.min(5, data.undefinedJargon.length) * 15;
            score += added;
            reasons.push(`Undefined terms: ${data.undefinedJargon.slice(0, 3).join(', ')}`);
        }

        if (data.referencedConcepts.length === 0) {
            score += 40;
            reasons.push(`Isolated node (references no other Auncient concepts)`);
        } else {
            const linkDensity = data.referencedConcepts.length / (data.wordCount / 100);
            if (linkDensity < 0.5) {
                score += 20;
                reasons.push(`Low linkage density to other concepts`);
            }
        }

        if (data.todoCount > 0) {
            score += Math.min(4, data.todoCount) * 25;
            reasons.push(`Contains ${data.todoCount} draft/TODO marker(s)`);
        }

        baseAnalysis[file] = {
            score,
            reasons,
            wordCount: data.wordCount,
            undefinedJargon: data.undefinedJargon,
            referencedConcepts: data.referencedConcepts
        };
    });

    // Pass 2: Propagate weights through concept references to penalize dependency gaps
    const finalAnalysis = {};
    Object.keys(baseAnalysis).forEach(file => {
        const item = baseAnalysis[file];
        let score = item.score;
        const reasons = [...item.reasons];
        
        const weakReferences = [];
        item.referencedConcepts.forEach(refFile => {
            const refItem = baseAnalysis[refFile];
            if (refItem && refItem.score >= 100) {
                weakReferences.push(refFile);
            }
        });

        if (weakReferences.length > 0) {
            const added = Math.min(4, weakReferences.length) * 15;
            score += added;
            const names = weakReferences.slice(0, 2).map(f => path.basename(f, '.md'));
            reasons.push(`Depends on poorly understood concepts: ${names.join(', ')}`);
        }

        finalAnalysis[file] = {
            score,
            reasons,
            wordCount: item.wordCount,
            undefinedJargon: item.undefinedJargon,
            referencedConcepts: item.referencedConcepts
        };
    });
    
    // Clear old keys and assign new keys to keep the reference intact
    for (const key in loreAnalysisCache) {
        delete loreAnalysisCache[key];
    }
    Object.assign(loreAnalysisCache, finalAnalysis);
    return loreAnalysisCache;
}

function processAnnotationReplies(content, file) {
    const regex = /<!--\s*AUNCIENT_ANNOTATIONS_START\s*([\s\S]*?)\s*AUNCIENT_ANNOTATIONS_END\s*-->/;
    const match = content.match(regex);
    if (!match) return content;
    
    try {
        const annotations = JSON.parse(match[1].trim());
        console.log(`[ANN DEB] file: ${file}, annotations length: ${annotations.length}`);
        let updated = false;
        
        const loreDir = path.join(__dirname, "../lore");
        const files = fs.readdirSync(loreDir).filter(f => f.endsWith(".md") || f.endsWith(".lore"));
        const concepts = files.map(f => ({
            file: f,
            name: path.basename(f, path.extname(f)).toLowerCase().replace(/_/g, ' ')
        }));

        annotations.forEach((ann, idx) => {
            const isQuestion = ann.comment && ann.comment.includes('?');
            const hasNoReplies = !ann.replies || ann.replies.length === 0;
            console.log(`[ANN DEB][${idx}] author: ${ann.author}, comment: "${ann.comment}", isQuestion: ${isQuestion}, hasNoReplies: ${hasNoReplies}`);
            
            if (ann.author === 'User' && isQuestion && hasNoReplies) {
                if (!ann.replies) ann.replies = [];
                
                let replyText = "";
                const textToSearch = (ann.comment + " " + (ann.range ? content.substring(ann.range[0], ann.range[1]) : "")).toLowerCase();
                const cleanSearch = textToSearch.replace(/[\s_-]/g, '');
                console.log(`[ANN DEB][${idx}] cleanSearch: "${cleanSearch}"`);
                
                const matchingConcept = concepts.find(c => {
                    const cleanConcept = c.name
                        .replace(/\b(lore|spec|analysis|integration|roadmap|proposal|deep\s+dive|guide|blueprint|report)\b/g, '')
                        .trim()
                        .replace(/[\s_-]/g, '');
                    if (!cleanConcept) return false;
                    return cleanSearch.includes(cleanConcept);
                });
                console.log(`[ANN DEB][${idx}] matchingConcept:`, matchingConcept ? matchingConcept.file : "none");

                if (matchingConcept) {
                    try {
                        const targetPath = path.join(loreDir, matchingConcept.file);
                        const targetText = fs.readFileSync(targetPath, "utf8");
                        const cleanTarget = targetText.replace(/<!--[\s\S]*?-->/g, "").trim();
                        const paragraph = cleanTarget.split("\n\n")[0] || cleanTarget;
                        const sentences = paragraph.split(/[.!?]+/).slice(0, 2).join(". ");
                        replyText = `Based on Auncient lore in ${matchingConcept.file}: "${sentences.trim()}."`;
                    } catch (e) {
                        replyText = `Found reference to ${matchingConcept.file}, but could not load definition.`;
                    }
                } else {
                    replyText = `I detected a question about this reference, but I do not have a defined Auncient concept for it. This remains a knowledge gap.`;
                }
                
                ann.replies.push({
                    author: "AI",
                    comment: replyText,
                    timestamp: Date.now()
                });
                updated = true;
            }
        });
        
        if (updated) {
            const newBlock = `<!-- AUNCIENT_ANNOTATIONS_START\n${JSON.stringify(annotations, null, 2)}\nAUNCIENT_ANNOTATIONS_END -->`;
            return content.replace(regex, newBlock);
        }
    } catch (err) {
        console.error("[SERVER] Failed to process annotation replies:", err);
    }
    return content;
}

module.exports = {
    loreAnalysisCache,
    performLoreAnalysis,
    processAnnotationReplies
};
