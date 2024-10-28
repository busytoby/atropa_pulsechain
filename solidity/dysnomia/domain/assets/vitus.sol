// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "../interfaces/worldinterface.sol";

contract VITUS is DYSNOMIA {
    string public constant Type = "VITUS";

    WORLDINTERFACE public World;

    constructor(address WorldAddress) DYSNOMIA("Dysnomia Creators", "VITUS", address(DYSNOMIA(WorldAddress).Xiao())) {
        maxSupply = 0;

        World = WORLDINTERFACE(WorldAddress);

        DYSNOMIA wmtoken = DYSNOMIA(WMContract);
        _mint(address(this), wmtoken.totalSupply());
        _addMarketRate(WMContract, 1 * 10 ** decimals());

        _mintToCap();
    }

    function Withdraw(uint256 Amount) public {
        (YUEINTERFACE Yue, ) = World.Cheon().Sei().Chi();
        World.Cheon().Sei().Chan().YueWithdraw(Yue, address(this), msg.sender, Amount);
    }

    function Balance() public view returns (uint256) {
        (YUEINTERFACE Yue, ) = World.Cheon().Sei().Chi();
        return VITUS(address(this)).balanceOf(address(Yue));
    }

    function Mint(address To, uint256 Amount) public onlyOwners {
        _mint(To, Amount);
    }
}