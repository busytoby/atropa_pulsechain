const assert = require("assert");
const puppeteer = require("puppeteer-core");
const path = require("path");

async function runTest() {
    console.log("=== Auncient TSFi Vision & Browser Controller E2E Test ===");
    
    console.log("[TEST] Launching headless browser to validate visual feel and inputs...");
    const browser = await puppeteer.launch({
        executablePath: "/usr/bin/google-chrome",
        headless: true,
        dumpio: true,
        args: [
            "--no-sandbox",
            "--disable-setuid-sandbox",
            "--disable-gpu",
            "--window-size=800,600",
            "--disable-dev-shm-usage"
        ]
    });

    try {
        const page = await browser.newPage();
        await page.setViewport({ width: 800, height: 600 });

        // 1. Validate DNA Splash Screen layout and elements
        const splashPath = `file://${path.join(__dirname, "../frontend/atropa_splash.html")}`;
        console.log(`[TEST] Navigating to DNA Splash Screen: ${splashPath}`);
        await page.goto(splashPath, { waitUntil: "load" });

        // Check essential elements
        const titleText = await page.evaluate(() => document.querySelector("h1")?.innerText);
        console.log(`  |- Splash Title: "${titleText}"`);
        assert.ok(titleText && titleText.includes("ATROPA"), "Splash screen title must contain 'ATROPA'");

        const sigilExists = await page.evaluate(() => !!document.getElementById("emblemContainer"));
        const bearExists = await page.evaluate(() => !!document.getElementById("bearCanvas"));
        const logsExists = await page.evaluate(() => !!document.getElementById("terminalLog"));
        
        console.log(`  |- Emblem Sigil exists: ${sigilExists}`);
        console.log(`  |- Phenotype Bear Canvas exists: ${bearExists}`);
        console.log(`  |- Terminal logs HUD exists: ${logsExists}`);
        
        assert.ok(sigilExists, "Emblem sigil container should exist");
        assert.ok(bearExists, "Phenotype Bear canvas should exist");
        assert.ok(logsExists, "Terminal logs HUD should exist");

        // 2. Navigate to target (YouTube)
        console.log("[TEST] Navigating to YouTube and waiting for load...");
        await page.goto("https://youtube.com", { waitUntil: "networkidle2" });

        // Handle Consent modal if any
        try {
            const consentButton = await page.$('button[aria-label*="Accept"], button[aria-label*="Agree"], ytd-button-renderer yt-button-shape button');
            if (consentButton) {
                console.log("[TEST] Consent dialog detected, clicking Accept...");
                await consentButton.click();
                await new Promise(resolve => setTimeout(resolve, 2000));
            }
        } catch (e) {
            console.log("[TEST] Consent check bypassed.");
        }

        // 3. Verify search box selection and input visual feel
        console.log("[TEST] Verifying Search Input Box layout...");
        const searchInputSelector = 'input[name="search_query"], input#search';
        await page.waitForSelector(searchInputSelector, { timeout: 10000 });
        
        const inputProps = await page.evaluate((selector) => {
            const el = document.querySelector(selector);
            if (!el) return null;
            const rect = el.getBoundingClientRect();
            return {
                x: rect.x,
                y: rect.y,
                width: rect.width,
                height: rect.height,
                visible: rect.width > 0 && rect.height > 0
            };
        }, searchInputSelector);

        console.log(`  |- Input element size: ${inputProps.width}x${inputProps.height} at (${inputProps.x}, ${inputProps.y})`);
        assert.ok(inputProps && inputProps.visible, "Search input should be visible on screen layout");

        console.log("[TEST] Simulating natural keyboard typing 'Atropa'...");
        await page.focus(searchInputSelector);
        await page.type(searchInputSelector, "Atropa", { delay: 100 });

        const inputValue = await page.evaluate((selector) => document.querySelector(selector)?.value, searchInputSelector);
        console.log(`  |- Input value: "${inputValue}"`);
        assert.strictEqual(inputValue, "Atropa", "Input value should reflect typed query");

        // Submit search
        console.log("[TEST] Submitting query...");
        await page.keyboard.press("Enter");
        await new Promise(resolve => setTimeout(resolve, 5000));

        // 4. Verify Filters dropdown and click action
        console.log("[TEST] Validating Filters Button discovery...");
        const filtersButtonSelector = await page.evaluate(() => {
            const elements = Array.from(document.querySelectorAll('ytd-toggle-button-renderer button, a[aria-label="Search filters"], button[aria-label="Search filters"]'));
            if (elements.length > 0) {
                elements[0].setAttribute('id', 'test-filter-btn');
                return '#test-filter-btn';
            }
            return null;
        });

        assert.ok(filtersButtonSelector, "Filters button should be found in search results");
        
        console.log("[TEST] Clicking Filters button...");
        await page.click(filtersButtonSelector);
        await new Promise(resolve => setTimeout(resolve, 2000));

        // Verify that the filter menu appears
        console.log("[TEST] Verifying 'This week' filter option layout...");
        const weekOptionSelector = await page.evaluate(() => {
            const elements = Array.from(document.querySelectorAll('ytd-search-filter-renderer a, ytd-search-filter-renderer, a.ytd-search-filter-renderer'));
            const found = elements.find(el => el.textContent && el.textContent.toLowerCase().includes('this week'));
            if (found) {
                const clickable = found.closest('a') || found.closest('button') || found;
                clickable.setAttribute('id', 'test-this-week-opt');
                return '#test-this-week-opt';
            }
            return null;
        });

        assert.ok(weekOptionSelector, "'This week' option must be visible after filter menu is clicked");

        console.log("[TEST] Selecting 'This week' filter...");
        await page.click(weekOptionSelector);
        await new Promise(resolve => setTimeout(resolve, 4000));

        // Verify parameter state in the URL
        const currentUrl = page.url();
        console.log(`  |- Redirected URL: ${currentUrl}`);
        assert.ok(currentUrl.includes("sp="), "URL should update with search parameter filter (sp)");

        // Save a screenshot for OpenCV visual checks
        const screenshotPath = path.join(__dirname, "../frontend/latest_frame.jpg");
        console.log(`[TEST] Saving screenshot for OpenCV verification to: ${screenshotPath}`);
        await page.screenshot({ path: screenshotPath, type: 'jpeg', quality: 90 });

        // Invoke python OpenCV validator
        console.log("[TEST] Running built-in OpenCV tsfi_vision suite...");
        const { execSync } = require("child_process");
        const pyOutput = execSync("python3 tests/verify_browser_opencv.py", { cwd: path.join(__dirname, "..") }).toString();
        console.log(pyOutput);

        console.log("[SUCCESS] Auncient Vision Verification: PASS");
    } catch (err) {
        console.error("[FAILURE] Auncient Vision Verification: FAIL");
        console.error(err);
        process.exit(1);
    } finally {
        await browser.close();
    }
}

runTest();
