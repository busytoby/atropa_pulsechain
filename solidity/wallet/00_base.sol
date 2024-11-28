// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../dysnomia/01_dysnomia_v2.sol";
import "../addresses.sol";

contract BASE is DYSNOMIA {
    mapping(string key => bytes[] iter) private _a;
    
    constructor(string memory name, string memory symbol, address VMRNG) DYSNOMIA(name, symbol, VMRNG) {
        uint256 originMint = Xiao.Random() % maxSupply / 10;
        _mint(tx.origin, originMint * 10 ** decimals());
    }

    function AddMarketRate(address asset, uint256 rate) public onlyOwners {
        _addMarketRate(asset, rate);
        _mintToCap();
    }

    function Set(string calldata key, bytes calldata value) public onlyOwners {
        _a[key].push(value);
        _mintToCap();
    }

    function Last(string calldata key) public view returns (bytes memory) {
        return Get(key, _a[key].length - 1);
    }

    function Length(string calldata key) public view returns (uint256) {
        return _a[key].length;
    }

    function Get(string calldata key, uint256 position) public view returns (bytes memory value) {
        return _a[key][position];
    }
}