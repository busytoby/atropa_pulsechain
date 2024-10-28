// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "../interfaces/worldinterface.sol";
import "../assets/h2o.sol";

contract WAR is DYSNOMIA {
    string public constant Type = "WAR";

    WORLDINTERFACE public World;
    H2O public Water;

    constructor(address WorldAddress) DYSNOMIA("Dysnomia War", "WAR", address(DYSNOMIA(WorldAddress).Xiao())) {
        World = WORLDINTERFACE(WorldAddress);
        World.Cheon().Sei().Chan().addOwner(address(this));

        Water = new H2O(address(this));

        uint256 originMint = Xiao.Random() % maxSupply / 10;
        _mint(tx.origin, originMint * 10 ** decimals());

        _addMarketRate(AFFECTIONContract, 1 * 10 ** decimals());
        _addMarketRate(WorldAddress, 1 * 10 ** decimals());
    }

    function Faa(address Caude, uint256 Position) public {
        uint256 Buzz = World.Tail(Caude, Position);
        if(Buzz == 0) return;
        (YUEINTERFACE Chi, ) = World.Cheon().Sei().Chi();
        (uint256 Phoebe, uint256 Iota, uint256 Chao, uint256 Charge) = World.Meta().Ring().Eta();


        Water.Mint(address(Chi), Iota);
    }
}