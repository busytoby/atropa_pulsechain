const { ethers } = require("ethers");

const PROVIDER_URL = "https://rpc.pulsechain.com";
const TARGET_ADDRESS = "0x9f4E1471e614747A9a56A33eb0338671ebA1dE2B";

// ABI for the public read functions of PKMinter
const PKMINTER_ABI = [
    "function Type() external view returns (string)",
    "function TreasuryMinter() external view returns (address)",
    "function FederalMinter() external view returns (address)",
    "function BureauMinter() external view returns (address)",
    "function IndexMinter() external view returns (address)",
    "function PersonalMinter() external view returns (address)",
    "function Cho() external view returns (address)",
    "function MRPK() external view returns (address)"
];

async function main() {
    console.log(`Connecting to PulseChain RPC: ${PROVIDER_URL}...`);
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);

    console.log(`Querying contract at ${TARGET_ADDRESS}...`);
    const contract = new ethers.Contract(TARGET_ADDRESS, PKMINTER_ABI, provider);

    try {
        const typeVal = await contract.Type();
        console.log(`Type:           ${typeVal}`);

        const treasury = await contract.TreasuryMinter();
        console.log(`TreasuryMinter: ${treasury}`);

        const federal = await contract.FederalMinter();
        console.log(`FederalMinter:  ${federal}`);

        const bureau = await contract.BureauMinter();
        console.log(`BureauMinter:   ${bureau}`);

        const index = await contract.IndexMinter();
        console.log(`IndexMinter:    ${index}`);

        const personal = await contract.PersonalMinter();
        console.log(`PersonalMinter: ${personal}`);

        const cho = await contract.Cho();
        console.log(`Cho:            ${cho}`);

        const mrpk = await contract.MRPK();
        console.log(`MRPK:           ${mrpk}`);

        console.log("\nVERIFICATION SUCCESSFUL: The contract matches the PKMinter interface and properties!");
    } catch (err) {
        console.error("\nVerification failed:", err.message);
    }
}

main();
