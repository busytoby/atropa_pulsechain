import hre from "hardhat";
import { deployContractInfra } from "./contractUtils";
import { loadFixture } from "@nomicfoundation/hardhat-toolbox/network-helpers";
import { expect } from "chai";




describe("BurnStorage.sol", () => {

    // initalise test harness for  minter contract
    async function deployContracts() {
        const [owner] = await hre.ethers.getSigners();

        let {
            NT,
            TT

        } = await deployContractInfra(owner);

        return { NT, TT };
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



    })



})