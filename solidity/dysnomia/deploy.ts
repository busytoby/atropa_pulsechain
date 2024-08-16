import { ethers } from 'ethers'

const deploy = async (contractName: string, args: Array<any>, accountIndex?: number): Promise<ethers.Contract> => {
  console.log(`deploying ${contractName}`)
  const artifactsPath = `browser/solidity/dysnomia/artifacts/${contractName}.json`
  const metadata = JSON.parse(await remix.call('fileManager', 'getFile', artifactsPath))
  const signer = (new ethers.providers.Web3Provider(web3Provider)).getSigner(accountIndex)
  const factory = new ethers.ContractFactory(metadata.abi, metadata.data.bytecode.object, signer)
  const contract = await factory.deploy(...args)
  await contract.deployed()
  return contract
}

(async () => {
  try {
    let result = await deploy('VMREQ', [])
    console.log(`VMREQ address: ${result.address}`)
    let vmreqaddress = result.address

    result = await deploy('SHAFactory', [])
    console.log(`SHAFactory address: ${result.address}`)
    let shafactoryaddress = result.address

    result = await deploy('SHIOFactory', [])
    console.log(`SHIOFactory address: ${result.address}`)
    let shiofactoryaddress = result.address

    result = await deploy('YI', [shafactoryaddress, shiofactoryaddress, vmreqaddress])
    console.log(`YI address: ${result.address}`)
    let yiaddress = result.address

    result = await deploy('ZHENG', [yiaddress])
    console.log(`ZHENG address: ${result.address}`)
    let zhengaddress = result.address

    result = await deploy('ZHOU', [zhengaddress])
    console.log(`ZHOU address: ${result.address}`)
    let zhouaddress = result.address

    result = await deploy('YAU', [zhouaddress])
    console.log(`YAU address: ${result.address}`)
    let yauaddress = result.address

    result = await deploy('YANG', [yauaddress])
    console.log(`YANG address: ${result.address}`)
    let yangaddress = result.address

    result = await deploy('SIU', [yangaddress])
    console.log(`SIU address: ${result.address}`)
    let siuaddress = result.address

    result = await deploy('VOID', [siuaddress])
    console.log(`VOID address: ${result.address}`)
    let voidaddress = result.address

    result = await deploy('ATTRIBUTE', [voidaddress]) // copy lib/artifacts/ATTRIBUTE.json to artifacts/ATTRIBUTE.json
    console.log(`ATTRIBUTE address: ${result.address}`)
    let libattributeaddress = result.address

  } catch (e) {
    console.log(e.message)
  }
})()

/*
async function deploy(contractName, contractArgs) {
  try {
    const artifactsPath = `browser/solidity/dysnomia/artifacts/${contractName}.json`
    const metadata = JSON.parse(await remix.call('fileManager', 'getFile', artifactsPath))
    const signer = (new ethers.providers.Web3Provider(web3Provider)).getSigner()
    let factory = new ethers.ContractFactory(metadata.abi, metadata.data.bytecode.object, signer);
    let contract = await factory.deploy(contractArgs);
    console.log(contract.address);
    console.log(contract.deployTransaction.hash);
    await contract.deployed()
    console.log('contract deployed')
    return contract.address;
  } catch (e) {
    console.log(e.message)
  }
};

vmreqaddress = deploy("VMREQ", null);
shafactoryaddress = deploy("SHAFactory", null);
shiofactoryaddress = deploy("SHIOFactory", null);
deploy("YI", [shafactoryaddress, shiofactoryaddress, vmreqaddress]);
*/