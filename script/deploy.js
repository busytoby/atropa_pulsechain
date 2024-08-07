const { AbiCoder } = require("ethers");
const { ethers} = require("hardhat");


async function deployRouter() {

     const class21 = await ethers.getContractFactory(`VMREQ`);

     const tx1 = await class21.deploy( {
       // nonce: nonce++,
        maxFeePerGas: 631000373911098n
    });

 

 
    const lib2 = await ethers.getContractFactory(`contracts/02_sha.sol:SHA`
    );

    const lib3 = await ethers.getContractFactory(`contracts/03_shio.sol:SHIO`);
    const lib33 = await ethers.getContractFactory(`contracts/04_yi.sol:YI`);
    const lib313 = await ethers.getContractFactory(`contracts/05_zheng.sol:ZHENG`);
    const lib323 = await ethers.getContractFactory(`contracts/06_zhou.sol:ZHOU`);
    const lib23 = await ethers.getContractFactory(`YAU`);
    const lib4 = await ethers.getContractFactory(`YANG`);



    const lib2C = await lib2.deploy("sha1", "sha1", await tx1.getAddress(),{

    });
    const lib22C = await lib2.deploy("sha2", "sha2", await tx1.getAddress(),{
   
    });
    const lib3C = await lib3.deploy(await lib2C.getAddress(), await lib22C.getAddress(), await tx1.getAddress(),{
  
    });
    const lib22C2 = await lib33.deploy(await tx1.getAddress(),{


     });

     const lib22C12 = await lib313.deploy(await lib22C2.getAddress(),{


     });


     const lib22C212 = await lib323.deploy(await lib22C2.getAddress(), await lib22C12.getAddress(),{

        gasLimit: 2000000n
     });

     const lib22C2112 = await lib23.deploy(await lib22C212.getAddress(),{

        gasLimit: 2000000n
     });

     const lib22C21112 = await lib4.deploy(await lib22C2112.getAddress(),{

        gasLimit: 2000000n
     });


console.log("sha1", await lib2C.getAddress())
console.log("sha2", await lib22C.getAddress())

console.log("shio", await lib3C.getAddress())
console.log("yi", await lib22C2.getAddress())
console.log("zheng", await lib22C12.getAddress())
console.log("zhou", await lib22C212.getAddress())
console.log("yau", await lib22C2112.getAddress())
console.log("yang", await lib22C21112.getAddress())




};




// eslint-disable-next-line @typescript-eslint/explicit-function-return-type
async function main() {
    // run the scripts one by one
    await deployRouter();  

}

main()
    .then(() => {
        console.log(`Successfully deployed all contracts!`);
        // eslint-disable-next-line no-process-exit
        process.exit(0);
    })
    .catch(error => {
        console.error(error);
        process.exitCode = 1;
    });
