const fs = require("fs");
const path = require("path");

const poolsPath = "/home/mariarahel/src/tsfi2/atropa_pulsechain/nonukes_pools.json";
const reservesPath = "/home/mariarahel/.gemini/antigravity-cli/brain/5a8d4144-99a3-4e64-93ac-47c55dad5b24/scratch/nonukes_pulsex_reserves.json";

const poolsMap = JSON.parse(fs.readFileSync(poolsPath, "utf8"));
const reservesMap = JSON.parse(fs.readFileSync(reservesPath, "utf8"));

const partnerToPool = {};
for (const [poolAddr, poolInfo] of Object.entries(poolsMap)) {
    if (poolInfo.other_addr) {
        partnerToPool[poolInfo.other_addr.toLowerCase()] = poolAddr.toLowerCase();
    }
}

const bzoAddr = "0x008fc4bbb1998bfee060f780be7688f0cec66bff";
const poolAddr = partnerToPool[bzoAddr];

console.log("Pool Addr for BZO:", poolAddr);
if (poolAddr && reservesMap[poolAddr]) {
    const res = reservesMap[poolAddr];
    console.log("Reserves for BZO pool:", JSON.stringify(res, null, 2));
    const reserve0 = parseFloat(res.reserve0 || 0);
    const reserve1 = parseFloat(res.reserve1 || 0);
    const t0 = (res.token0 || "").toLowerCase();
    const nonukesAddr = "0x174a0ad99c60c20d9b3d94c3095bc1fb9defd62";

    let reserveNoNukes = reserve0;
    let reservePartner = reserve1;

    if (t0 !== nonukesAddr) {
        reserveNoNukes = reserve1;
        reservePartner = reserve0;
    }
    
    console.log("t0:", t0);
    console.log("nonukesAddr:", nonukesAddr);
    console.log("t0 !== nonukesAddr:", t0 !== nonukesAddr);
    console.log("reserveNoNukes:", reserveNoNukes);
    console.log("reservePartner:", reservePartner);

    if (reservePartner > 0) {
        const priceInNoNukes = reserveNoNukes / reservePartner;
        const priceUsd = (priceInNoNukes * 1.74).toFixed(6);
        console.log("Calculated priceInNoNukes:", priceInNoNukes);
        console.log("Calculated priceUsd:", priceUsd);
    }
}
