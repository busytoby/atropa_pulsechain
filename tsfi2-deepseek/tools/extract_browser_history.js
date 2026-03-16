// Google Labs Project Flow - Deep History Extractor (v2)
// Run this in Chrome Console on https://labs.google/fx/tools/flow

async function extractDeepHistory() {
    console.log("🚀 Starting Deep History Extraction...");
    const allData = { projects: [], prompts: [] };

    // 1. Try to find the Project List in the internal store
    // (Next.js often exposes this in __NEXT_DATA__ or a global store)
    try {
        const nextData = JSON.parse(document.getElementById('__NEXT_DATA__').textContent);
        console.log("📂 Found Next.js Data:", nextData);
        
        // Save raw dump just in case
        allData.raw_next_data = nextData;
        
        // Try to locate project list in props
        // Note: Structure varies, we grab what we can
    } catch (e) {
        console.log("⚠️ Could not read __NEXT_DATA__ directly.");
    }

    // 2. Try to hit the "List Projects" API directly
    // We guess standard RPC/REST endpoints used by Google Labs
    const listEndpoints = [
        '/api/fx/flow/listProjects', 
        '/api/fx/flow/projects',
        '/api/fx/user/library',
        '/_/api/fx/flow/batchGetProjects'
    ];

    let projectIds = [];

    for (const url of listEndpoints) {
        try {
            const resp = await fetch(url);
            if (resp.ok) {
                const data = await resp.json();
                console.log(`✅ API Success: ${url}`, data);
                allData.api_list = data;
                
                // Heuristic: Try to extract IDs from the response
                // (Looking for "id", "projectId", or similar strings)
                const jsonStr = JSON.stringify(data);
                const ids = jsonStr.match(/"[a-zA-Z0-9_-]{10,}"/g); // Rough regex for IDs
                if (ids) projectIds.push(...ids.map(s => s.replace(/"/g, '')));
            }
        } catch (e) { /* ignore */ }
    }

    // Deduplicate IDs
    projectIds = [...new Set(projectIds)];
    console.log(`🔍 Found ${projectIds.length} potential Project IDs.`);

    // 3. (Optional) Fetch details for each ID if we found them
    // This part is speculative without knowing the exact "Get Project" URL
    // But we record the IDs so we can construct the URLs: https://labs.google/fx/tools/flow/project/ID
    allData.project_ids = projectIds;

    console.log("************************************************");
    console.log("📋 COPY THE JSON BELOW THIS LINE");
    console.log("************************************************");
    console.log(JSON.stringify(allData, null, 2));
}

extractDeepHistory();
