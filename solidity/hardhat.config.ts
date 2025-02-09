import {HardhatUserConfig} from "hardhat/config";
import "@nomicfoundation/hardhat-toolbox";
import "hardhat-gas-reporter";
import "solidity-coverage";
import 'dotenv/config'


const config: HardhatUserConfig = {

    solidity: {
        compilers: [
            {
                version: "0.8.11",
                settings: {
                    optimizer: {
                        enabled: true,
                        runs: 200,
                    },
                },
            }, {
                version: '0.8.18',
                settings: {
                    optimizer: {
                        enabled: true,
                        runs: 199,
                    },
                },
            },
            {
                version: "0.6.6",
            },
            {
                version: "0.8.24",
                settings: {
                    viaIR: true,
                    optimizer: {
                        enabled: true,
                        runs: 200,
                    },
                },
            },
            {
                version: "0.8.23",
                settings: {
                    optimizer: {
                        enabled: true,
                        runs: 200,
                    },
                },
            },


            {
                version: "0.8.26",
                settings: {
                    viaIR: true,
                    optimizer: {


                        enabled: true,
                        runs: 200,

                    },
                },

            },
        ],
    },

    networks: {

    
        
        //  default test network for  faster tests during deveopment
        hardhat: {
            loggingEnabled: false,
            forking: {
            url: 'https://rpc.pulsechain.com',
            },
            // just and well known address for just reading.  not valuable
     
            accounts: [
              
                { privateKey: '0x92db14e403b83dfe3df233f83dfa3a0d7096f21ca9b0d6d6b8d88b2b4ec1564e', balance: '1000000000000000000000000000000000000' },
              

            ],
            mining: {
                mempool: {
                    order: "fifo"
                }
            },
            initialBaseFeePerGas: 1000000000, // 1 gwei base fee

        },


    },
    mocha: {
        timeout: 8000000, // Optional: Increase test timeout for slow forks
    },
    gasReporter: {
        enabled: true, // Enable gas reporting
        currency: "USD", // Display gas costs in USD
        coinmarketcap: "67b4952d-f2c7-43ca-8e1c-5e4b9a62f966", // (Optional) To fetch real-time gas prices
        gasPrice: 20, // (Optional) Use a custom gas price (in gwei)
        outputFile: "gas-report.txt", // (Optional) Output gas report to a file
        noColors: false, // Disable colors in the console output
    },


}


export default config;

