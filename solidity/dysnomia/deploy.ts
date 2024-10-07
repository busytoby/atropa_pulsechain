const fs = require('fs')
import { ethers } from 'ethers'

// execute remix.exeCurrent() from the > command prompt at the bottom of the remixz

const getContract = async (contractName: string, contractAddress, accountIndex?: number): Promise<ethers.Contract> => {
  if(accountIndex == undefined) accountIndex = 0;
  console.log(`getting contract ${contractName} as account ${accountIndex}`)
  const domainArtifactsPath = `browser/solidity/dysnomia/domain/artifacts/${contractName}.json`
  const skyArtifactsPath = `browser/solidity/dysnomia/domain/sky/artifacts/${contractName}.json`
  const tangArtifactsPath = `browser/solidity/dysnomia/domain/tang/artifacts/${contractName}.json`
  const songArtifactsPath = `browser/solidity/dysnomia/domain/song/artifacts/${contractName}.json`
  const libartifactsPath = `browser/solidity/dysnomia/lib/artifacts/${contractName}.json`
  const artifactsPath = `browser/solidity/dysnomia/artifacts/${contractName}.json`
  let metadata
  try { metadata = JSON.parse(await remix.call('fileManager', 'getFile', artifactsPath)) } catch {
    try { metadata = JSON.parse(await remix.call('fileManager', 'getFile', libartifactsPath)) } catch {
      try { metadata = JSON.parse(await remix.call('fileManager', 'getFile', domainArtifactsPath)) } catch {
        try { metadata = JSON.parse(await remix.call('fileManager', 'getFile', skyArtifactsPath)) } catch {
          try { metadata = JSON.parse(await remix.call('fileManager', 'getFile', tangArtifactsPath)) } catch {
            try { metadata = JSON.parse(await remix.call('fileManager', 'getFile', songArtifactsPath)) } catch {
      } } } } } }

  const signer = (new ethers.providers.Web3Provider(web3Provider)).getSigner(accountIndex)
  const factory = new ethers.ContractFactory(metadata.abi, metadata.data.bytecode.object, signer)
  const contract = new ethers.Contract(contractAddress, factory.interface, signer);
  return contract
}

const getContract2 = async (contractName: string, contractAddress, accountIndex?: number): Promise<ethers.Contract> => {
  if(accountIndex == undefined) accountIndex = 0;
  console.log(`getting contract ${contractName} as account ${accountIndex}`)
  const domainArtifactsPath = `browser/solidity/dysnomia/domain/artifacts/${contractName}.json`
  const skyArtifactsPath = `browser/solidity/dysnomia/domain/sky/artifacts/${contractName}.json`
  const tangArtifactsPath = `browser/solidity/dysnomia/domain/tang/artifacts/${contractName}.json`
  const songArtifactsPath = `browser/solidity/dysnomia/domain/song/artifacts/${contractName}.json`
  const libartifactsPath = `browser/solidity/dysnomia/lib/artifacts/${contractName}.json`
  const artifactsPath = `browser/solidity/dysnomia/artifacts/${contractName}.json`
  let metadata
  try { metadata = JSON.parse(await remix.call('fileManager', 'getFile', artifactsPath)) } catch {
    try { metadata = JSON.parse(await remix.call('fileManager', 'getFile', libartifactsPath)) } catch {
      try { metadata = JSON.parse(await remix.call('fileManager', 'getFile', domainArtifactsPath)) } catch {
        try { metadata = JSON.parse(await remix.call('fileManager', 'getFile', skyArtifactsPath)) } catch {
          try { metadata = JSON.parse(await remix.call('fileManager', 'getFile', tangArtifactsPath)) } catch {
            try { metadata = JSON.parse(await remix.call('fileManager', 'getFile', songArtifactsPath)) } catch {
      } } } } } }
  const signer = (new ethers.providers.Web3Provider(web3Provider)).getSigner(accountIndex)
  //const factory = new ethers.ContractFactory(metadata.abi, metadata.data.bytecode.object, signer)
  //const contract = new ethers.Contract(contractAddress, factory.interface, signer);
  const contract = new ethers.Contract(contractAddress, metadata.abi, signer);
  return contract
}

const deploy = async (contractName: string, args: Array<any>, accountIndex?: number): Promise<ethers.Contract> => {
  if(accountIndex == undefined) accountIndex = 0;
  console.log(`deploying ${contractName} from account ${accountIndex}`)
  const domainArtifactsPath = `browser/solidity/dysnomia/domain/artifacts/${contractName}.json`
  const skyArtifactsPath = `browser/solidity/dysnomia/domain/sky/artifacts/${contractName}.json`
  const tangArtifactsPath = `browser/solidity/dysnomia/domain/tang/artifacts/${contractName}.json`
  const songArtifactsPath = `browser/solidity/dysnomia/domain/song/artifacts/${contractName}.json`
  const libartifactsPath = `browser/solidity/dysnomia/lib/artifacts/${contractName}.json`
  const artifactsPath = `browser/solidity/dysnomia/artifacts/${contractName}.json`
  let metadata
  try { metadata = JSON.parse(await remix.call('fileManager', 'getFile', artifactsPath)) } catch {
    try { metadata = JSON.parse(await remix.call('fileManager', 'getFile', libartifactsPath)) } catch {
      try { metadata = JSON.parse(await remix.call('fileManager', 'getFile', domainArtifactsPath)) } catch {
        try { metadata = JSON.parse(await remix.call('fileManager', 'getFile', skyArtifactsPath)) } catch {
          try { metadata = JSON.parse(await remix.call('fileManager', 'getFile', tangArtifactsPath)) } catch {
            try { metadata = JSON.parse(await remix.call('fileManager', 'getFile', songArtifactsPath)) } catch {
      } } } } } }
  const signer = (new ethers.providers.Web3Provider(web3Provider)).getSigner(accountIndex)
  const factory = new ethers.ContractFactory(metadata.abi, metadata.data.bytecode.object, signer)
  const contract = await factory.deploy(...args)
  await contract.deployed()
  return contract
}

let result
let r2wtf
let vmreqaddress
let shafactoryaddress
let shiofactoryaddress
let yiaddress
let zhengaddress
let zhouaddress
let yauaddress
let yangaddress
let siuaddress //= ethers.utils.getAddress("0x3be998c75ae8CD79E808B0038DAFF593D60DC4f4")
let voidaddress = ethers.utils.getAddress("0x511048267d5606797C126e54Ca20842D9771906F")
let libattributeaddress //= ethers.utils.getAddress("0x53D09dc8896bf463A7561199da0d56a5Ca25223b")
let laufactoryaddress = ethers.utils.getAddress("0x2E69344b68a8a16f754e81e0A2408ed491329e6E")
let lauaddress1 = ethers.utils.getAddress("0xbec5a7e99A1C007049bf0C225658aAb03a07a137")
let lauaddress2
let libstringsaddress
let choaddress = ethers.utils.getAddress("0x3E2F9abADcF76dDc68B5cB347C48A245001469b4")
let libyaiaddress = ethers.utils.getAddress("0x6b3634FcFeF25B69D91EE959B9a1B20B495aBb9a")
let qingfactoryaddress = ethers.utils.getAddress("0x88B1Ea6a5D4b870070537379f4885382F375E472")
let voidqingaddress
let yaiqingaddress
let libcorereactionsaddress
let qiaddress
let maiaddress
let xiaaddress
let xieaddress
let chanaddress
let choaaddress
let seiaddress
let yueaddress
let cheonaddress
let dingaddress
let seicontract

let START = 0;
// set pre-requisite address to deploy only subset

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

      case 11:
        result = await deploy('LAUFactory', [voidaddress]) 
        console.log(`LAUFactory address: ${result.address}`)
        laufactoryaddress = result.address

      case 12:
        result = await deploy('STRINGLIB', [voidaddress]) 
        console.log(`STRINGLIB address: ${result.address}`)
        libstringsaddress = result.address

      case 13:
        let laufactorycontract = await getContract2('LAUFactory', laufactoryaddress)
        result = await laufactorycontract.New("User Test", "USERTOKEN")
        r2wtf = await result.wait()
        lauaddress1 = r2wtf.events[0].address
        console.log(`LAU Deployed: ${lauaddress1} from origin`)
        let laucontract = await getContract('LAU', lauaddress1)
        result = await laucontract["Username(string)"]("mariarahel")
        console.log("successful Set Username from origin")
        result = await laucontract["Chat(string)"]("Chat Test")
        console.log("successful Chat from origin")

        laufactorycontract = await getContract2('LAUFactory', laufactoryaddress, 2)
        result = await laufactorycontract.New("User Test 2", "USERTOKEN2")
        r2wtf = await result.wait()
        lauaddress2 = r2wtf.events[0].address
        console.log(`LAU Deployed: ${lauaddress2} from non-origin`)
        laucontract = await getContract('LAU', lauaddress2, 2)
        result = await laucontract["Username(string)"]("TestUser")
        console.log("successful Set Username from non-origin")
        result = await laucontract["Chat(string)"]("Chat Test 2")
        console.log("successful Chat from non-origin")

      case 14:
        result = await deploy('COREREACTIONSLIB', [voidaddress]) 
        console.log(`COREREACTIONSLIB address: ${result.address}`)
        libcorereactionsaddress = result.address

      case 15:
        result = await deploy('CHO', [voidaddress]) 
        console.log(`CHO address: ${result.address}`)
        choaddress = result.address
        let corereactionscontract = await getContract2('COREREACTIONSLIB', libcorereactionsaddress)
        result = await corereactionscontract.RegisterChoForTalk(choaddress)
        r2wtf = await result.wait()
        console.log(`Cho Registered For Talk`)

      case 16:
        result = await deploy('YAI', [voidaddress]) 
        console.log(`YAI address: ${result.address}`)
        libyaiaddress = result.address

      case 17:
        result = await deploy('QINGFactory', [choaddress]) 
        console.log(`QINGFactory address: ${result.address}`)
        qingfactoryaddress = result.address

      case 18:
        let qingfactorycontract = await getContract2('QINGFactory', qingfactoryaddress)
        result = await qingfactorycontract.New(voidaddress)
        console.log(`QINGFactory Contract retrieved: ${qingfactoryaddress}`)
        r2wtf = await result.wait()
        voidqingaddress = r2wtf.events[4].address
        console.log(`VOID QING Deployed: ${voidqingaddress} from origin`)

        qingfactorycontract = await getContract2('QINGFactory', qingfactoryaddress, 2)
        result = await qingfactorycontract.New(libyaiaddress)
        r2wtf = await result.wait()
        yaiqingaddress = r2wtf.events[4].address
        console.log(`YAI QING Deployed: ${yaiqingaddress} from non-origin`)

      case 19:
        result = await deploy('QI', [voidqingaddress]) 
        console.log(`QI address: ${result.address}`)
        qiaddress = result.address

      case 20:
        result = await deploy('MAI', [qiaddress]) 
        console.log(`MAI address: ${result.address}`)
        maiaddress = result.address

      case 21:
        result = await deploy('XIA', [maiaddress]) 
        console.log(`XIA address: ${result.address}`)
        xiaaddress = result.address

      case 22:
        result = await deploy('XIE', [xiaaddress]) 
        console.log(`XIE address: ${result.address}`)
        xieaddress = result.address

      case 23:
        result = await deploy('CHAN', [xieaddress]) 
        console.log(`CHAN address: ${result.address}`)
        chanaddress = result.address

      case 24:
        result = await deploy('SEI', [chanaddress]) 
        console.log(`SEI address: ${result.address}`)
        seiaddress = result.address
      
      case 25:
        result = await deploy('CHOA', [seiaddress]) 
        console.log(`CHOA address: ${result.address}`)
        choaaddress = result.address

      case 26:
        result = await deploy('CHEON', [seiaddress]) 
        console.log(`CHEON address: ${result.address}`)
        cheonaddress = result.address

      case 26:
        result = await deploy('DING', [cheonaddress]) 
        console.log(`DING address: ${result.address}`)
        dingaddress = result.address

      case 27:
        seicontract = await getContract2('SEI', seiaddress)
        result = await seicontract.Start(lauaddress1, "Yue 1", "YUE1")
        r2wtf = await result.wait()

        seicontract = await getContract2('SEI', seiaddress, 2)
        result = await seicontract.Start(lauaddress2, "Yue 2", "YUE2")
        r2wtf = await result.wait()
    }
  } catch (e) {
    console.log(e.message)
  }
})()
