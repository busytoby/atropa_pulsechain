// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";

contract VITUS is DYSNOMIA {
    string public constant Type = "VITUS";

    constructor(address CreationAddress) DYSNOMIA("Dysnomia Creators", "VITUS", address(DYSNOMIA(CreationAddress).Xiao())) {
        maxSupply = 0;

        DYSNOMIA wmtoken = DYSNOMIA(WMContract);
        _mint(address(this), wmtoken.totalSupply());
        _addMarketRate(WMContract, 1 * 10 ** decimals());

        _mintToCap();
    }

    function Mint(address To, uint256 Amount) public onlyOwners {
        _mint(To, Amount);
    }
}