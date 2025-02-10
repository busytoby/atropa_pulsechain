//  create and return access control contract intance
import {HardhatEthersSigner} from "@nomicfoundation/hardhat-ethers/signers";
import hre, {ethers} from "hardhat";
import {BaseContract, ContractTransactionResponse} from "ethers";
import {
    NT,
    TT
  } from "../typechain-types";


async function deployContractInfra(owner: HardhatEthersSigner) {
    // instantiate contracts
    let NT = await createMinter(owner);  
    let TT = await createTT(owner, NT)
    let TT2 = await createTT2(owner, NT)
    let flashLoan = await createFlashoan(owner, TT)
    let flashLoanRevert = await createFlashoanRevert(owner, TT)

    return {
       NT,
       TT,
       flashLoan,
       flashLoanRevert,
       TT2
    };
}


async function createMinter(owner: HardhatEthersSigner) {

    let NTFactory = await hre.ethers.getContractFactory('NT');
    return await NTFactory.connect(owner).deploy('0xC7bDAc3e6Bb5eC37041A11328723e9927cCf430B', '0x1D177CB9EfEEa49A8B97ab1C72785a3A37ABc9Ff', '0xB680F0cc810317933F234f67EB6A9E923407f05D');
}

async function createTT(owner: HardhatEthersSigner, v2Mintor: NT) {

    let NTFactory = await hre.ethers.getContractFactory('TT');
    return await NTFactory.connect(owner).deploy('Name', 'Symbol', 1, v2Mintor, "0x1d177cb9efeea49a8b97ab1c72785a3a37abc9ff", '0xB680F0cc810317933F234f67EB6A9E923407f05D', '0x1D177CB9EfEEa49A8B97ab1C72785a3A37ABc9Ff');
}
async function createTT2(owner: HardhatEthersSigner, v2Mintor: NT) {

    let NTFactory = await hre.ethers.getContractFactory('TT');
    return await NTFactory.connect(owner).deploy('Name', 'Symbol', 1, v2Mintor, "0x1d177cb9efeea49a8b97ab1c72785a3a37abc9ff", '0xB680F0cc810317933F234f67EB6A9E923407f05D', '0x1D177CB9EfEEa49A8B97ab1C72785a3A37ABc9Ff');
}

async function createFlashoan(owner: HardhatEthersSigner, Tt: TT) {

    let NTFactory = await hre.ethers.getContractFactory('minterFlash');
    return await NTFactory.connect(owner).deploy(Tt);
}

async function createFlashoanRevert(owner: HardhatEthersSigner, Tt: TT) {

    let NTFactory = await hre.ethers.getContractFactory('minterFlashRevert');
    return await NTFactory.connect(owner).deploy(Tt);
}







export {
    createMinter,
    deployContractInfra
};