require("@nomicfoundation/hardhat-toolbox");

require('dotenv').config()

/** @type import('hardhat/config').HardhatUserConfig */
module.exports = {
  solidity: {
    version: "0.8.26",
    settings: {
      optimizer: {
        enabled: true,
        runs: 1000,
      },
    },
  },
  allowUnlimitedContractSize: true,
  networks: {
    pulsechain: {
      url:  `http://127.0.0.1:8545/`,
      accounts: [process.env.PK],
    },
    
  },
  
};
