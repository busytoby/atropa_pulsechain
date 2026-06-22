const { ethers } = require("ethers");

const signatures = [
    "setCustomPatternStep(uint256,uint256,uint256)",
    "setPatternLength(uint256,uint256)",
    "getPatternStepTriggers(uint256,uint256)",
    "getCurrentStep()"
];

signatures.forEach(sig => {
    const selector = ethers.id(sig).substring(0, 10);
    console.log(`${sig} => ${selector}`);
});
