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
let vmreqaddress = ethers.utils.getAddress("0x5FbDB2315678afecb367f032d93F642f64180aa3")
let shafactoryaddress = ethers.utils.getAddress("0xe7f1725E7734CE288F8367e1Bb143E90bb3F0512")
let shiofactoryaddress = ethers.utils.getAddress("0x9fE46736679d2D9a65F0992F2272dE9f3c7fa6e0")
let yiaddress = ethers.utils.getAddress("0xCf7Ed3AccA5a467e9e704C703E8D87F634fB0Fc9")
let zhengaddress = ethers.utils.getAddress("0xDc64a140Aa3E981100a9becA4E685f962f0cF6C9")
let zhouaddress = ethers.utils.getAddress("0x5FC8d32690cc91D4c39d9d3abcBD16989F875707")
let yauaddress = ethers.utils.getAddress("0x0165878A594ca255338adfa4d48449f69242Eb8F")
let yangaddress = ethers.utils.getAddress("0xa513E6E4b8f2a923D98304ec87F64353C4D5C853")
let siuaddress = ethers.utils.getAddress("0x2279B7A0a67DB372996a5FaB50D91eAA73d2eBe6")
let voidaddress = ethers.utils.getAddress("0x8A791620dd6260079BF849Dc5567aDC3F2FdC318")
let libattributeaddress = ethers.utils.getAddress("0x610178dA211FEF7D417bC0e6FeD39F05609AD788")
let laufactoryaddress = ethers.utils.getAddress("0xB7f8BC63BbcaD18155201308C8f3540b07f84F5e")
let lauaddress1 = ethers.utils.getAddress("0x8dAF17A20c9DBA35f005b6324F493785D239719d")
let lauaddress2 = ethers.utils.getAddress("0x3Ca8f9C04c7e3E1624Ac2008F92f6F366A869444")
let lauaddress3
let lauaddress4
let libstringsaddress = ethers.utils.getAddress("0xA51c1fc2f0D1a1b8494Ed1FE312d7C3a78Ed91C0")
let choaddress = ethers.utils.getAddress("0x3E2F9abADcF76dDc68B5cB347C48A245001469b4")
let libyaiaddress = ethers.utils.getAddress("0x6b3634FcFeF25B69D91EE959B9a1B20B495aBb9a")
let qingfactoryaddress = ethers.utils.getAddress("0x88B1Ea6a5D4b870070537379f4885382F375E472")
let voidqingaddress
let yaiqingaddress
let libcorereactionsaddress = ethers.utils.getAddress("0x959922bE3CAee4b8Cd9a407cc3ac1C251C2007B1")
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

let user1soul
let user2soul
let user3soul
let user4soul

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
        console.log(`LAU Deployed: ${lauaddress2} from non-origin 2`)
        laucontract = await getContract('LAU', lauaddress2, 2)
        result = await laucontract["Username(string)"]("TestUser")
        console.log("successful Set Username from non-origin")
        result = await laucontract["Chat(string)"]("Chat Test 2")
        console.log("successful Chat from non-origin")

        laufactorycontract = await getContract2('LAUFactory', laufactoryaddress, 3)
        result = await laufactorycontract.New("User Test 3", "USERTOKEN3")
        r2wtf = await result.wait()
        lauaddress3 = r2wtf.events[0].address
        console.log(`LAU Deployed: ${lauaddress3} from non-origin 3`)
        laucontract = await getContract('LAU', lauaddress3, 3)
        result = await laucontract["Username(string)"]("TestUser")

        laufactorycontract = await getContract2('LAUFactory', laufactoryaddress, 4)
        result = await laufactorycontract.New("User Test 4", "USERTOKEN3")
        r2wtf = await result.wait()
        lauaddress4 = r2wtf.events[0].address
        console.log(`LAU Deployed: ${lauaddress4} from non-origin 4`)
        laucontract = await getContract('LAU', lauaddress4, 4)
        result = await laucontract["Username(string)"]("TestUser")        

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

        let chocontract = await getContract2('CHO', choaddress)
        result = await chocontract.Enter(lauaddress1)
        user1soul = await chocontract.GetUserSoul()
        console.log(`Test GetUserSoul(User1): ${user1soul}`)

        chocontract = await getContract2('CHO', choaddress, 2)
        result = await chocontract.Enter(lauaddress2)
        user2soul = await chocontract.GetUserSoul()
        console.log(`Test GetUserSoul(User2): ${user2soul}`)

        chocontract = await getContract2('CHO', choaddress, 3)
        result = await chocontract.Enter(lauaddress3)
        user3soul = await chocontract.GetUserSoul()
        console.log(`Test GetUserSoul(User3): ${user3soul}`)

        chocontract = await getContract2('CHO', choaddress, 4)
        result = await chocontract.Enter(lauaddress4)
        console.log(`Cho Entered From Lau 4: ${lauaddress4}`)
        user4soul = await chocontract.GetUserSoul()
        console.log(`Test GetUserSoul(User4): ${user4soul}`)
        result = await chocontract.ReactUser(user4soul, 5555)

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

        let voidqingcontract = await getContract2('QING', voidqingaddress)
        result = await voidqingcontract.Join(lauaddress1)
        console.log(`Joined Void Qing From LAU 1`)

        qingfactorycontract = await getContract2('QINGFactory', qingfactoryaddress, 2)
        result = await qingfactorycontract.New(libyaiaddress)
        r2wtf = await result.wait()
        yaiqingaddress = r2wtf.events[4].address
        console.log(`YAI QING Deployed: ${yaiqingaddress} from non-origin`)

        voidqingcontract = await getContract2('QING', voidqingaddress, 2)
        result = await voidqingcontract.Join(lauaddress2)
        console.log(`Joined Void Qing From LAU 2`)

      case 19:
        result = await deploy('QI', [voidqingaddress]) 
        console.log(`QI address: ${result.address}`)
        qiaddress = result.address

      case 20:
        result = await deploy('MAI', [qiaddress]) 
        console.log(`MAI address: ${result.address}`)
        maiaddress = result.address

        let maicontract = await getContract2('MAI', maiaddress)
        result = await maicontract.MovePlayer(user1soul, voidqingaddress)
        console.log(`Mai ownerPlayer Move Test 1 Successful`)
        result = await maicontract.MovePlayer(user1soul, yaiqingaddress)
        console.log(`Mai ownerPlayer Move Test 2 Successful`)
        result = await maicontract.MovePlayer(user1soul, voidqingaddress)
        console.log(`Mai ownerPlayer Move Test 3 Successful`)

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
