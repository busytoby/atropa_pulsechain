const fs = require('fs')
import { ethers } from 'ethers'

// execute remix.exeCurrent() from the > command prompt at the bottom of the remixz

const getContract = async (contractName: string, contractAddress, accountIndex?: number): Promise<ethers.Contract> => {
  if(accountIndex == undefined) accountIndex = 0;
  console.log(`getting contract ${contractName} as account ${accountIndex}`)
  const libartifactsPath = `browser/solidity/dysnomia/lib/artifacts/${contractName}.json`
  const artifactsPath = `browser/solidity/dysnomia/artifacts/${contractName}.json`
  let metadata
  try { metadata = JSON.parse(await remix.call('fileManager', 'getFile', artifactsPath)) } catch {
    metadata = JSON.parse(await remix.call('fileManager', 'getFile', libartifactsPath))
  }
  const signer = (new ethers.providers.Web3Provider(web3Provider)).getSigner(accountIndex)
  const factory = new ethers.ContractFactory(metadata.abi, metadata.data.bytecode.object, signer)
  const contract = new ethers.Contract(contractAddress, factory.interface, signer);
  return contract
}

const deploy = async (contractName: string, args: Array<any>, accountIndex?: number): Promise<ethers.Contract> => {
  if(accountIndex == undefined) accountIndex = 0;
  console.log(`deploying ${contractName} from account ${accountIndex}`)
  const libartifactsPath = `browser/solidity/dysnomia/lib/artifacts/${contractName}.json`
  const artifactsPath = `browser/solidity/dysnomia/artifacts/${contractName}.json`
  let metadata
  try { metadata = JSON.parse(await remix.call('fileManager', 'getFile', artifactsPath)) } catch {
    metadata = JSON.parse(await remix.call('fileManager', 'getFile', libartifactsPath))
  }
  const signer = (new ethers.providers.Web3Provider(web3Provider)).getSigner(accountIndex)
  const factory = new ethers.ContractFactory(metadata.abi, metadata.data.bytecode.object, signer)
  const contract = await factory.deploy(...args)
  await contract.deployed()
  return contract
}

let result
let vmreqaddress
let shafactoryaddress
let shiofactoryaddress
let yiaddress
let zhengaddress
let zhouaddress
let yauaddress
let yangaddress
let siuaddress //= ethers.utils.getAddress("0x3be998c75ae8CD79E808B0038DAFF593D60DC4f4")
let voidaddress //= ethers.utils.getAddress("0xaBb8aB7447263083455214d630e6d44aEF3BB7c8")
let libattributeaddress //= ethers.utils.getAddress("0x53D09dc8896bf463A7561199da0d56a5Ca25223b")
let START = 0;
// set pre-requisite address to deploy only subset ie: = ethers.utils.getAddress("0xD2F5793e91D3043002f478aa06A023D4FAE12777")

(async () => {
  try {
    switch (START) {
      case 0:
        result = await deploy('VMREQ', [])
        console.log(`VMREQ address: ${result.address}`)
        vmreqaddress = result.address

      case 1:
        result = await deploy('SHAFactory', [])
        console.log(`SHAFactory address: ${result.address}`)
        shafactoryaddress = result.address

      case 2:
        result = await deploy('SHIOFactory', [])
        console.log(`SHIOFactory address: ${result.address}`)
        shiofactoryaddress = result.address

      case 3:
        result = await deploy('YI', [shafactoryaddress, shiofactoryaddress, vmreqaddress])
        console.log(`YI address: ${result.address}`)
        yiaddress = result.address

      case 4:
        result = await deploy('ZHENG', [yiaddress])
        console.log(`ZHENG address: ${result.address}`)
        zhengaddress = result.address

      case 5:
        result = await deploy('ZHOU', [zhengaddress])
        console.log(`ZHOU address: ${result.address}`)
        zhouaddress = result.address

      case 6:
        result = await deploy('YAU', [zhouaddress])
        console.log(`YAU address: ${result.address}`)
        yauaddress = result.address

      case 7:
        result = await deploy('YANG', [yauaddress])
        console.log(`YANG address: ${result.address}`)
        yangaddress = result.address

      case 8:
        result = await deploy('SIU', [yangaddress])
        console.log(`SIU address: ${result.address}`)
        siuaddress = result.address

      case 9:
        result = await deploy('VOID', [siuaddress])
        console.log(`VOID address: ${result.address}`)
        voidaddress = result.address

      case 10:
        result = await deploy('ATTRIBUTE', [voidaddress]) 
        console.log(`ATTRIBUTE address: ${result.address}`)
        libattributeaddress = result.address

      case 99:
        let voidcontract = await getContract('VOID', voidaddress)
        try { result = await voidcontract["Enter(string,string)"]("Test", "TEST") } catch {}
        result = await voidcontract["Enter()"]()
        console.log("successful Enter from origin")
        result = await voidcontract["SetAttribute(string,string)"]("Username", "mariarahel")
        console.log("successful SetAttribute from origin")
        result = await voidcontract["Chat(string)"]("Chat Test")
        console.log("successful Chat from origin")
        //console.log(result)
        console.log("successful from origin")

        voidcontract = await getContract('VOID', voidaddress, 2)
        try { result = await voidcontract["Enter(string,string)"]("Test2", "TEST2") } catch {}
        result = await voidcontract["Enter()"]()
        console.log("successful Enter from non-origin")
        result = await voidcontract["SetAttribute(string,string)"]("Username", "TestUser")
        console.log("successful SetAttribute from non-origin")
        result = await voidcontract["Chat(string)"]("Chat Test 2")
        console.log("successful Chat from non-origin")
        console.log("successful from non-origin")
    }
  } catch (e) {
    console.log(e.message)
  }
})()
