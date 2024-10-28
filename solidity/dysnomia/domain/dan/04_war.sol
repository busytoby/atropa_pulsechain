// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";

contract WAR is DYSNOMIA {
    string public constant Type = "WAR";

    constructor(address WorldAddress) DYSNOMIA("Dysnomia War", "WAR", address(DYSNOMIA(WorldAddress).Xiao())) {
        uint256 originMint = Xiao.Random() % maxSupply / 10;
        _mint(tx.origin, originMint * 10 ** decimals());

        _addMarketRate(AFFECTIONContract, 1 * 10 ** decimals());
    }
}