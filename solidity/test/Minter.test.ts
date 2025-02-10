import hre from "hardhat";
import { deployContractInfra } from "./contractUtils";
import { loadFixture } from "@nomicfoundation/hardhat-toolbox/network-helpers";
import { expect } from "chai";


const INPUT_LENGTH_MISMATCH = 'INPUT_LENGTH_MISMATCH';
const INSUFFICIENT_FLASH_LOAN_BALANCE = 'INSUFFICIENT_FLASH_LOAN_BALANCE'

describe("Minter.sol", () => {

    // initalise test harness for  minter contract
    async function deployContracts() {
        const [owner] = await hre.ethers.getSigners();

        let {
            NT,
            TT,
            flashLoan,
            flashLoanRevert,
            TT2

        } = await deployContractInfra(owner);

        return { NT, TT, flashLoan, flashLoanRevert, TT2 };
    }

    describe('deploy and config', () => {
        //Test deployment of minter
        it('shall deploy and create FD㉾C', async () => {
            const [owner, vibePass] = await hre.ethers.getSigners();
            let NTFactory = await hre.ethers.getContractFactory('NT');
            await expect(await NTFactory.connect(owner).deploy('0xC7bDAc3e6Bb5eC37041A11328723e9927cCf430B', '0x1D177CB9EfEEa49A8B97ab1C72785a3A37ABc9Ff', '0xB680F0cc810317933F234f67EB6A9E923407f05D')).to.not.be.reverted



        })
        it('shall deploy and create TT', async () => {
            const [owner, vibePass] = await hre.ethers.getSigners();
            const { TT } = await loadFixture(deployContracts);
            //when created owner will be sent minted amount
            expect(await TT.balanceOf(owner.address)).equals(1n)



        })
        it('shall mint TT', async () => {
            const [owner, vibePass] = await hre.ethers.getSigners();
            const { TT } = await loadFixture(deployContracts);
            let user = "0xBF182955401aF3f2f7e244cb31184E93E74a2501"; // address that holds tokens

            // Impersonate the account
            await hre.network.provider.request({
                method: "hardhat_impersonateAccount",
                params: [user],
            });

            // Get signer
            let userS = await hre.ethers.getSigner(user);

            const erc20 = await hre.ethers.getContractAt('ERC20', '0x1d177cb9efeea49a8b97ab1c72785a3a37abc9ff', userS);
            expect(await erc20.connect(userS).approve(TT, 999999999999999999999999999999999n)).not.to.be.reverted
            //test mint
            expect(await TT.connect(userS).mint(1n)).not.to.be.reverted
            //check if minted
            expect(await TT.balanceOf(userS.address)).equals(1n)



        })


        it('shall ha ho', async () => {
            const [owner, owner2] = await hre.ethers.getSigners();
            const { TT } = await loadFixture(deployContracts);
            let user = "0xBF182955401aF3f2f7e244cb31184E93E74a2501"; // address that holds tokens

            // Impersonate the account
            await hre.network.provider.request({
                method: "hardhat_impersonateAccount",
                params: [user],
            });

            // Get signer
            let userS = await hre.ethers.getSigner(user);


            expect(await TT.connect(userS).ha()).not.to.be.reverted
            expect(await TT.connect(userS).ho(userS, 200)).not.to.be.reverted





        })

        it('shall make NEW', async () => {
            const [owner, vibePass] = await hre.ethers.getSigners();
            const { NT } = await loadFixture(deployContracts);
            let user = "0xBF182955401aF3f2f7e244cb31184E93E74a2501"; // address that holds tokens

            // Impersonate the account
            await hre.network.provider.request({
                method: "hardhat_impersonateAccount",
                params: [user],
            });

            // Get signer
            let userS = await hre.ethers.getSigner(user);
            
            const erc20 = await hre.ethers.getContractAt('ERC20', '0xE1030d35B912dd2a209998788dBCD564869a522C', userS);
            expect(await erc20.connect(userS).approve(NT, 999999999999999999999999999999999n)).not.to.be.reverted
            const erc202 = await hre.ethers.getContractAt('ERC20', '0xa1bee1dae9af77dac73aa0459ed63b4d93fc6d29', userS);
            expect(await erc202.connect(userS).approve(NT, 999999999999999999999999999999999n)).not.to.be.reverted
            //test mint
            expect(await NT.connect(userS).New('r', 'r', 1, '0xE1030d35B912dd2a209998788dBCD564869a522C')).not.to.be.reverted
         



        })



    })

    describe('deploy and check', () => {
        it('shall calculate flashLoan fee correctly', async () => {
            const [owner, vibePass] = await hre.ethers.getSigners();
            const { flashLoan, TT } = await loadFixture(deployContracts);

            let user = "0xBF182955401aF3f2f7e244cb31184E93E74a2501"; // address that holds tokens

            // Impersonate the account
            await hre.network.provider.request({
                method: "hardhat_impersonateAccount",
                params: [user],
            });

            // Get signer
            let userS = await hre.ethers.getSigner(user);

            const erc202 = await hre.ethers.getContractAt('ERC20', '0x1d177cb9efeea49a8b97ab1c72785a3a37abc9ff', userS);
            const atropa = await hre.ethers.getContractAt('ERC20', '0xcc78a0acdf847a2c1714d2a925bb4477df5d48a6', userS);
            await atropa.transfer(TT, 1000000000000000000000000n)
            await atropa.transfer(flashLoan, 1000000000000000000000000n)
            expect(await erc202.balanceOf(TT)).to.be.equal(0n)
            expect(await erc202.balanceOf(flashLoan)).to.be.equal(0n)
            expect(await erc202.transfer(flashLoan, 250000000000000000n)).to.not.be.reverted
            expect(await erc202.balanceOf(flashLoan)).to.be.equal(250000000000000000n)
            expect(await erc202.connect(userS).approve(TT, 999999999999999999999999999999999n)).not.to.be.reverted
            expect(await erc202.balanceOf(TT)).to.be.equal(0n)
            expect(await TT.connect(userS).mint(100000000000000000000n)).not.to.be.reverted
            expect(await erc202.balanceOf(TT)).to.be.equal(100000000000000000000n)
            console.log(await TT.balanceOf(flashLoan))
           // await expect(flashLoan.initiateFlashLoan(['0xcc78a0acdf847a2c1714d2a925bb4477df5d48a6', '0x1d177cb9efeea49a8b97ab1c72785a3a37abc9ff'], [ 100000000000n , 10000000000000000000n])).to.not.be.reverted
           // expect(await erc202.balanceOf(TT)).to.be.equal(100250000000000000000n)
            //expect(await erc202.balanceOf(flashLoan)).to.be.equal(0n)
            console.log(await TT.balanceOf(flashLoan))

        })

        it('shall revert with flashloan', async () => {
            const [owner, vibePass] = await hre.ethers.getSigners();
            const { flashLoanRevert, TT, flashLoan } = await loadFixture(deployContracts);

            let user = "0xBF182955401aF3f2f7e244cb31184E93E74a2501"; // address that holds tokens

            // Impersonate the account
            await hre.network.provider.request({
                method: "hardhat_impersonateAccount",
                params: [user],
            });

            // Get signer
            let userS = await hre.ethers.getSigner(user);
            
            const erc202 = await hre.ethers.getContractAt('ERC20', '0x1d177cb9efeea49a8b97ab1c72785a3a37abc9ff', userS);
            const atropa = await hre.ethers.getContractAt('ERC20', '0xcc78a0acdf847a2c1714d2a925bb4477df5d48a6', userS);
            await atropa.transfer(TT, 1000000000000000000000000n)
            await atropa.transfer(flashLoan, 1000000000000000000000000n)
            expect(await erc202.balanceOf(TT)).to.be.equal(0n)
            expect(await erc202.balanceOf(flashLoanRevert)).to.be.equal(0n)
            expect(await erc202.transfer(flashLoanRevert, 250000000000000000n)).to.not.be.reverted
            expect(await erc202.balanceOf(flashLoanRevert)).to.be.equal(250000000000000000n)
            expect(await erc202.connect(userS).approve(TT, 999999999999999999999999999999999n)).not.to.be.reverted
            expect(await erc202.balanceOf(TT)).to.be.equal(0n)
            expect(await TT.connect(userS).mint(100000000000000000000n)).not.to.be.reverted
            expect(await erc202.balanceOf(TT)).to.be.equal(100000000000000000000n)
            //await expect(flashLoan.initiateFlashLoan(['0x1d177cb9efeea49a8b97ab1c72785a3a37abc9ff', '0xcc78a0acdf847a2c1714d2a925bb4477df5d48a6'], [100000000000000000000000000000000n, 1000000000000000000000000n])).to.be.revertedWith(INSUFFICIENT_FLASH_LOAN_BALANCE)
          //  await expect(flashLoanRevert.initiateFlashLoan('0x1d177cb9efeea49a8b97ab1c72785a3a37abc9ff', 100000000000000000000n)).to.be.revertedWith(INPUT_LENGTH_MISMATCH)
     

        })

        it('shall buy with flash loan', async () => {
            const [owner, vibePass] = await hre.ethers.getSigners();
            const { flashLoanRevert, TT, flashLoan, TT2 } = await loadFixture(deployContracts);

            let user = "0xBF182955401aF3f2f7e244cb31184E93E74a2501"; // address that holds tokens

            // Impersonate the account
            await hre.network.provider.request({
                method: "hardhat_impersonateAccount",
                params: [user],
            });
            const routerabi = [
                {
                    "inputs": [
                        {
                            "internalType": "address",
                            "name": "_factory",
                            "type": "address"
                        },
                        {
                            "internalType": "address",
                            "name": "_WPLS",
                            "type": "address"
                        }
                    ],
                    "type": "constructor"
                },
                {
                    "inputs": [],
                    "name": "WPLS",
                    "outputs": [
                        {
                            "internalType": "address",
                            "name": "",
                            "type": "address"
                        }
                    ],
                    "stateMutability": "view",
                    "type": "function"
                },
                {
                    "inputs": [
                        {
                            "internalType": "address",
                            "name": "tokenA",
                            "type": "address"
                        },
                        {
                            "internalType": "address",
                            "name": "tokenB",
                            "type": "address"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountADesired",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountBDesired",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountAMin",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountBMin",
                            "type": "uint256"
                        },
                        {
                            "internalType": "address",
                            "name": "to",
                            "type": "address"
                        },
                        {
                            "internalType": "uint256",
                            "name": "deadline",
                            "type": "uint256"
                        }
                    ],
                    "name": "addLiquidity",
                    "outputs": [
                        {
                            "internalType": "uint256",
                            "name": "amountA",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountB",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "liquidity",
                            "type": "uint256"
                        }
                    ],
                    "stateMutability": "nonpayable",
                    "type": "function"
                },
                {
                    "inputs": [
                        {
                            "internalType": "address",
                            "name": "token",
                            "type": "address"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountTokenDesired",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountTokenMin",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountETHMin",
                            "type": "uint256"
                        },
                        {
                            "internalType": "address",
                            "name": "to",
                            "type": "address"
                        },
                        {
                            "internalType": "uint256",
                            "name": "deadline",
                            "type": "uint256"
                        }
                    ],
                    "name": "addLiquidityETH",
                    "outputs": [
                        {
                            "internalType": "uint256",
                            "name": "amountToken",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountETH",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "liquidity",
                            "type": "uint256"
                        }
                    ],
                    "stateMutability": "payable",
                    "type": "function"
                },
                {
                    "inputs": [],
                    "name": "factory",
                    "outputs": [
                        {
                            "internalType": "address",
                            "name": "",
                            "type": "address"
                        }
                    ],
                    "stateMutability": "view",
                    "type": "function"
                },
                {
                    "inputs": [
                        {
                            "internalType": "uint256",
                            "name": "amountOut",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "reserveIn",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "reserveOut",
                            "type": "uint256"
                        }
                    ],
                    "name": "getAmountIn",
                    "outputs": [
                        {
                            "internalType": "uint256",
                            "name": "amountIn",
                            "type": "uint256"
                        }
                    ],
                    "stateMutability": "pure",
                    "type": "function"
                },
                {
                    "inputs": [
                        {
                            "internalType": "uint256",
                            "name": "amountIn",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "reserveIn",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "reserveOut",
                            "type": "uint256"
                        }
                    ],
                    "name": "getAmountOut",
                    "outputs": [
                        {
                            "internalType": "uint256",
                            "name": "amountOut",
                            "type": "uint256"
                        }
                    ],
                    "stateMutability": "pure",
                    "type": "function"
                },
                {
                    "inputs": [
                        {
                            "internalType": "uint256",
                            "name": "amountOut",
                            "type": "uint256"
                        },
                        {
                            "internalType": "address[]",
                            "name": "path",
                            "type": "address[]"
                        }
                    ],
                    "name": "getAmountsIn",
                    "outputs": [
                        {
                            "internalType": "uint256[]",
                            "name": "amounts",
                            "type": "uint256[]"
                        }
                    ],
                    "stateMutability": "view",
                    "type": "function"
                },
                {
                    "inputs": [
                        {
                            "internalType": "uint256",
                            "name": "amountIn",
                            "type": "uint256"
                        },
                        {
                            "internalType": "address[]",
                            "name": "path",
                            "type": "address[]"
                        }
                    ],
                    "name": "getAmountsOut",
                    "outputs": [
                        {
                            "internalType": "uint256[]",
                            "name": "amounts",
                            "type": "uint256[]"
                        }
                    ],
                    "stateMutability": "view",
                    "type": "function"
                },
                {
                    "inputs": [
                        {
                            "internalType": "uint256",
                            "name": "amountA",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "reserveA",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "reserveB",
                            "type": "uint256"
                        }
                    ],
                    "name": "quote",
                    "outputs": [
                        {
                            "internalType": "uint256",
                            "name": "amountB",
                            "type": "uint256"
                        }
                    ],
                    "stateMutability": "pure",
                    "type": "function"
                },
                {
                    "inputs": [
                        {
                            "internalType": "address",
                            "name": "tokenA",
                            "type": "address"
                        },
                        {
                            "internalType": "address",
                            "name": "tokenB",
                            "type": "address"
                        },
                        {
                            "internalType": "uint256",
                            "name": "liquidity",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountAMin",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountBMin",
                            "type": "uint256"
                        },
                        {
                            "internalType": "address",
                            "name": "to",
                            "type": "address"
                        },
                        {
                            "internalType": "uint256",
                            "name": "deadline",
                            "type": "uint256"
                        }
                    ],
                    "name": "removeLiquidity",
                    "outputs": [
                        {
                            "internalType": "uint256",
                            "name": "amountA",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountB",
                            "type": "uint256"
                        }
                    ],
                    "stateMutability": "nonpayable",
                    "type": "function"
                },
                {
                    "inputs": [
                        {
                            "internalType": "address",
                            "name": "token",
                            "type": "address"
                        },
                        {
                            "internalType": "uint256",
                            "name": "liquidity",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountTokenMin",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountETHMin",
                            "type": "uint256"
                        },
                        {
                            "internalType": "address",
                            "name": "to",
                            "type": "address"
                        },
                        {
                            "internalType": "uint256",
                            "name": "deadline",
                            "type": "uint256"
                        }
                    ],
                    "name": "removeLiquidityETH",
                    "outputs": [
                        {
                            "internalType": "uint256",
                            "name": "amountToken",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountETH",
                            "type": "uint256"
                        }
                    ],
                    "stateMutability": "nonpayable",
                    "type": "function"
                },
                {
                    "inputs": [
                        {
                            "internalType": "address",
                            "name": "token",
                            "type": "address"
                        },
                        {
                            "internalType": "uint256",
                            "name": "liquidity",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountTokenMin",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountETHMin",
                            "type": "uint256"
                        },
                        {
                            "internalType": "address",
                            "name": "to",
                            "type": "address"
                        },
                        {
                            "internalType": "uint256",
                            "name": "deadline",
                            "type": "uint256"
                        }
                    ],
                    "name": "removeLiquidityETHSupportingFeeOnTransferTokens",
                    "outputs": [
                        {
                            "internalType": "uint256",
                            "name": "amountETH",
                            "type": "uint256"
                        }
                    ],
                    "stateMutability": "nonpayable",
                    "type": "function"
                },
                {
                    "inputs": [
                        {
                            "internalType": "address",
                            "name": "token",
                            "type": "address"
                        },
                        {
                            "internalType": "uint256",
                            "name": "liquidity",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountTokenMin",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountETHMin",
                            "type": "uint256"
                        },
                        {
                            "internalType": "address",
                            "name": "to",
                            "type": "address"
                        },
                        {
                            "internalType": "uint256",
                            "name": "deadline",
                            "type": "uint256"
                        },
                        {
                            "internalType": "bool",
                            "name": "approveMax",
                            "type": "bool"
                        },
                        {
                            "internalType": "uint8",
                            "name": "v",
                            "type": "uint8"
                        },
                        {
                            "internalType": "bytes32",
                            "name": "r",
                            "type": "bytes32"
                        },
                        {
                            "internalType": "bytes32",
                            "name": "s",
                            "type": "bytes32"
                        }
                    ],
                    "name": "removeLiquidityETHWithPermit",
                    "outputs": [
                        {
                            "internalType": "uint256",
                            "name": "amountToken",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountETH",
                            "type": "uint256"
                        }
                    ],
                    "stateMutability": "nonpayable",
                    "type": "function"
                },
                {
                    "inputs": [
                        {
                            "internalType": "address",
                            "name": "token",
                            "type": "address"
                        },
                        {
                            "internalType": "uint256",
                            "name": "liquidity",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountTokenMin",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountETHMin",
                            "type": "uint256"
                        },
                        {
                            "internalType": "address",
                            "name": "to",
                            "type": "address"
                        },
                        {
                            "internalType": "uint256",
                            "name": "deadline",
                            "type": "uint256"
                        },
                        {
                            "internalType": "bool",
                            "name": "approveMax",
                            "type": "bool"
                        },
                        {
                            "internalType": "uint8",
                            "name": "v",
                            "type": "uint8"
                        },
                        {
                            "internalType": "bytes32",
                            "name": "r",
                            "type": "bytes32"
                        },
                        {
                            "internalType": "bytes32",
                            "name": "s",
                            "type": "bytes32"
                        }
                    ],
                    "name": "removeLiquidityETHWithPermitSupportingFeeOnTransferTokens",
                    "outputs": [
                        {
                            "internalType": "uint256",
                            "name": "amountETH",
                            "type": "uint256"
                        }
                    ],
                    "stateMutability": "nonpayable",
                    "type": "function"
                },
                {
                    "inputs": [
                        {
                            "internalType": "address",
                            "name": "tokenA",
                            "type": "address"
                        },
                        {
                            "internalType": "address",
                            "name": "tokenB",
                            "type": "address"
                        },
                        {
                            "internalType": "uint256",
                            "name": "liquidity",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountAMin",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountBMin",
                            "type": "uint256"
                        },
                        {
                            "internalType": "address",
                            "name": "to",
                            "type": "address"
                        },
                        {
                            "internalType": "uint256",
                            "name": "deadline",
                            "type": "uint256"
                        },
                        {
                            "internalType": "bool",
                            "name": "approveMax",
                            "type": "bool"
                        },
                        {
                            "internalType": "uint8",
                            "name": "v",
                            "type": "uint8"
                        },
                        {
                            "internalType": "bytes32",
                            "name": "r",
                            "type": "bytes32"
                        },
                        {
                            "internalType": "bytes32",
                            "name": "s",
                            "type": "bytes32"
                        }
                    ],
                    "name": "removeLiquidityWithPermit",
                    "outputs": [
                        {
                            "internalType": "uint256",
                            "name": "amountA",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountB",
                            "type": "uint256"
                        }
                    ],
                    "stateMutability": "nonpayable",
                    "type": "function"
                },
                {
                    "inputs": [
                        {
                            "internalType": "uint256",
                            "name": "amountOut",
                            "type": "uint256"
                        },
                        {
                            "internalType": "address[]",
                            "name": "path",
                            "type": "address[]"
                        },
                        {
                            "internalType": "address",
                            "name": "to",
                            "type": "address"
                        },
                        {
                            "internalType": "uint256",
                            "name": "deadline",
                            "type": "uint256"
                        }
                    ],
                    "name": "swapETHForExactTokens",
                    "outputs": [
                        {
                            "internalType": "uint256[]",
                            "name": "amounts",
                            "type": "uint256[]"
                        }
                    ],
                    "stateMutability": "payable",
                    "type": "function"
                },
                {
                    "inputs": [
                        {
                            "internalType": "uint256",
                            "name": "amountOutMin",
                            "type": "uint256"
                        },
                        {
                            "internalType": "address[]",
                            "name": "path",
                            "type": "address[]"
                        },
                        {
                            "internalType": "address",
                            "name": "to",
                            "type": "address"
                        },
                        {
                            "internalType": "uint256",
                            "name": "deadline",
                            "type": "uint256"
                        }
                    ],
                    "name": "swapExactETHForTokens",
                    "outputs": [
                        {
                            "internalType": "uint256[]",
                            "name": "amounts",
                            "type": "uint256[]"
                        }
                    ],
                    "stateMutability": "payable",
                    "type": "function"
                },
                {
                    "inputs": [
                        {
                            "internalType": "uint256",
                            "name": "amountOutMin",
                            "type": "uint256"
                        },
                        {
                            "internalType": "address[]",
                            "name": "path",
                            "type": "address[]"
                        },
                        {
                            "internalType": "address",
                            "name": "to",
                            "type": "address"
                        },
                        {
                            "internalType": "uint256",
                            "name": "deadline",
                            "type": "uint256"
                        }
                    ],
                    "name": "swapExactETHForTokensSupportingFeeOnTransferTokens",
                    "outputs": [],
                    "stateMutability": "payable",
                    "type": "function"
                },
                {
                    "inputs": [
                        {
                            "internalType": "uint256",
                            "name": "amountIn",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountOutMin",
                            "type": "uint256"
                        },
                        {
                            "internalType": "address[]",
                            "name": "path",
                            "type": "address[]"
                        },
                        {
                            "internalType": "address",
                            "name": "to",
                            "type": "address"
                        },
                        {
                            "internalType": "uint256",
                            "name": "deadline",
                            "type": "uint256"
                        }
                    ],
                    "name": "swapExactTokensForETH",
                    "outputs": [
                        {
                            "internalType": "uint256[]",
                            "name": "amounts",
                            "type": "uint256[]"
                        }
                    ],
                    "stateMutability": "nonpayable",
                    "type": "function"
                },
                {
                    "inputs": [
                        {
                            "internalType": "uint256",
                            "name": "amountIn",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountOutMin",
                            "type": "uint256"
                        },
                        {
                            "internalType": "address[]",
                            "name": "path",
                            "type": "address[]"
                        },
                        {
                            "internalType": "address",
                            "name": "to",
                            "type": "address"
                        },
                        {
                            "internalType": "uint256",
                            "name": "deadline",
                            "type": "uint256"
                        }
                    ],
                    "name": "swapExactTokensForETHSupportingFeeOnTransferTokens",
                    "outputs": [],
                    "stateMutability": "nonpayable",
                    "type": "function"
                },
                {
                    "inputs": [
                        {
                            "internalType": "uint256",
                            "name": "amountIn",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountOutMin",
                            "type": "uint256"
                        },
                        {
                            "internalType": "address[]",
                            "name": "path",
                            "type": "address[]"
                        },
                        {
                            "internalType": "address",
                            "name": "to",
                            "type": "address"
                        },
                        {
                            "internalType": "uint256",
                            "name": "deadline",
                            "type": "uint256"
                        }
                    ],
                    "name": "swapExactTokensForTokens",
                    "outputs": [
                        {
                            "internalType": "uint256[]",
                            "name": "amounts",
                            "type": "uint256[]"
                        }
                    ],
                    "stateMutability": "nonpayable",
                    "type": "function"
                },
                {
                    "inputs": [
                        {
                            "internalType": "uint256",
                            "name": "amountIn",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountOutMin",
                            "type": "uint256"
                        },
                        {
                            "internalType": "address[]",
                            "name": "path",
                            "type": "address[]"
                        },
                        {
                            "internalType": "address",
                            "name": "to",
                            "type": "address"
                        },
                        {
                            "internalType": "uint256",
                            "name": "deadline",
                            "type": "uint256"
                        }
                    ],
                    "name": "swapExactTokensForTokensSupportingFeeOnTransferTokens",
                    "outputs": [],
                    "stateMutability": "nonpayable",
                    "type": "function"
                },
                {
                    "inputs": [
                        {
                            "internalType": "uint256",
                            "name": "amountOut",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountInMax",
                            "type": "uint256"
                        },
                        {
                            "internalType": "address[]",
                            "name": "path",
                            "type": "address[]"
                        },
                        {
                            "internalType": "address",
                            "name": "to",
                            "type": "address"
                        },
                        {
                            "internalType": "uint256",
                            "name": "deadline",
                            "type": "uint256"
                        }
                    ],
                    "name": "swapTokensForExactETH",
                    "outputs": [
                        {
                            "internalType": "uint256[]",
                            "name": "amounts",
                            "type": "uint256[]"
                        }
                    ],
                    "stateMutability": "nonpayable",
                    "type": "function"
                },
                {
                    "inputs": [
                        {
                            "internalType": "uint256",
                            "name": "amountOut",
                            "type": "uint256"
                        },
                        {
                            "internalType": "uint256",
                            "name": "amountInMax",
                            "type": "uint256"
                        },
                        {
                            "internalType": "address[]",
                            "name": "path",
                            "type": "address[]"
                        },
                        {
                            "internalType": "address",
                            "name": "to",
                            "type": "address"
                        },
                        {
                            "internalType": "uint256",
                            "name": "deadline",
                            "type": "uint256"
                        }
                    ],
                    "name": "swapTokensForExactTokens",
                    "outputs": [
                        {
                            "internalType": "uint256[]",
                            "name": "amounts",
                            "type": "uint256[]"
                        }
                    ],
                    "stateMutability": "nonpayable",
                    "type": "function"
                },
                {
                    "type": "receive"
                }
            ]

            // Get signer
            let userS = await hre.ethers.getSigner(user);
            
            const erc202 = await hre.ethers.getContractAt('ERC20', '0x1d177cb9efeea49a8b97ab1c72785a3a37abc9ff', userS);
            const atropa = await hre.ethers.getContractAt('ERC20', '0xcc78a0acdf847a2c1714d2a925bb4477df5d48a6', userS);
            await atropa.transfer(TT, 1000000000000000000000000n)
            await atropa.transfer(flashLoan, 1000000000000000000000000n)
            expect(await erc202.balanceOf(TT)).to.be.equal(0n)
            expect(await erc202.balanceOf(flashLoanRevert)).to.be.equal(0n)
            expect(await erc202.transfer(flashLoanRevert, 250000000000000000n)).to.not.be.reverted
            expect(await erc202.balanceOf(flashLoanRevert)).to.be.equal(250000000000000000n)
            expect(await erc202.connect(userS).approve(TT, 999999999999999999999999999999999n)).not.to.be.reverted
            expect(await erc202.connect(userS).approve(TT2, 999999999999999999999999999999999n)).not.to.be.reverted
            expect(await erc202.balanceOf(TT)).to.be.equal(0n)
            expect(await TT.connect(userS).mint(1000000000000n)).not.to.be.reverted
            expect(await TT2.connect(userS).mint(1000000000000n)).not.to.be.reverted
           // expect(await erc202.balanceOf(TT)).to.be.equal(100000000000000000000n)
            //await expect(flashLoan.initiateFlashLoan(['0x1d177cb9efeea49a8b97ab1c72785a3a37abc9ff', '0xcc78a0acdf847a2c1714d2a925bb4477df5d48a6'], [100000000000000000000000000000000n, 1000000000000000000000000n])).to.be.revertedWith(INSUFFICIENT_FLASH_LOAN_BALANCE)
          //  await expect(flashLoanRevert.initiateFlashLoan('0x1d177cb9efeea49a8b97ab1c72785a3a37abc9ff', 100000000000000000000n)).to.be.revertedWith(INPUT_LENGTH_MISMATCH)
          const router2 = new hre.ethers.Contract(
                  "0x165C3410fC91EF562C50559f7d2289fEbed552d9", // Replace with actual Router address
                  routerabi,
                  userS
                );
                await TT.connect(userS).approve("0x165C3410fC91EF562C50559f7d2289fEbed552d9", 9999999999999999999999999999999999999n)
                await TT2.connect(userS).approve("0x165C3410fC91EF562C50559f7d2289fEbed552d9", 9999999999999999999999999999999999999n)
                await router2.addLiquidity(
                    TT,
                    TT2,
                    await TT.balanceOf(userS),
                    await TT2.balanceOf(userS),
                    1,
                    1,
                    userS,
                    999999999999999999999999999n
                )
               expect(await TT.connect(userS).mint(100000000000000000000n)).not.to.be.reverted
                await TT.connect(userS).approve(TT2, 9999999999999999999999999999999999999n)
                expect(await TT2.connect(userS).mint(100000000000000000000n)).not.to.be.reverted
                await TT2.connect(userS).approve(TT, 9999999999999999999999999999999999999n)
                expect(await TT.connect(userS).Buy(TT2, 100n )).not.to.be.reverted
                expect(await TT.connect(userS).mint(100000000000000000000n)).not.to.be.reverted
                await TT.connect(userS).approve(TT2, 9999999999999999999999999999999999999n)
                await TT2.connect(userS).approve(TT, 9999999999999999999999999999999999999n)
                console.log(await TT.balanceOf(userS))
                expect(await TT2.connect(userS).Buy(TT, 10000000 )).not.to.be.reverted
                //expect(await TT2.balanceOf(TT)).to.not.be.equal(0n)
        })

    })



})