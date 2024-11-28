// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../dysnomia/01_dysnomia_v2.sol";
import "../addresses.sol";

contract SECRET is DYSNOMIA {
    mapping(string key => bytes[] iter) private _b;
    
    constructor(string memory name, string memory symbol, address VMRNG) DYSNOMIA(name, symbol, VMRNG) {
        uint256 originMint = Xiao.Random() % maxSupply / 10;
        _mint(tx.origin, originMint * 10 ** decimals());
    }

    function AddMarketRate(address asset, uint256 rate) public onlyOwners {
        _addMarketRate(asset, rate);
        _mintToCap();
    }

    function Set(string calldata key, bytes calldata value) public onlyOwners {
        _b[key].push(value);
        _mintToCap();
    }

    function Length(string calldata key) public view returns (uint256) {
        return _b[key].length;
    }

    function Get(string calldata key, uint256 position) public view onlyOwners returns (bytes memory value) {
        return _b[key][position];
    }
}
