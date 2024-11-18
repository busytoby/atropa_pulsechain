// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../dysnomia/01_dysnomia_v2.sol";
import "../addresses.sol";

contract BASE is DYSNOMIA {
    constructor(string memory name, string memory symbol, address VMRNG) DYSNOMIA(name, symbol, VMRNG) {
        uint256 originMint = Xiao.Random() % maxSupply / 10;
        _mint(tx.origin, originMint * 10 ** decimals());
    }

    function AddMarketRate(address _a, uint256 _r) public onlyOwners {
        _addMarketRate(_a, _r);
    }
}