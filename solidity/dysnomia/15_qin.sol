// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./14_qi.sol";

contract QIN is DYSNOMIA {
    uint64[3] public Saat;
    Bao public On;
    uint256 public qp;
    address[3][] private _inventory;

    constructor(string memory name, string memory symbol, address VoidQingAddress) DYSNOMIA(name, symbol, address(DYSNOMIA(VoidQingAddress).Xiao())) {
        addOwner(tx.origin);
    }

    function AddMarketRate(address _a, uint256 _r) public onlyOwners {
        _addMarketRate(_a, _r);
    }

    function GetInventoryCount(uint256 class) public view returns (uint256) {
        return _inventory[class].length;
    }

    function GetDefense(uint256 n) public view returns (address) {
        return _inventory[0][n];
    }

    function GetDecoration(uint256 n) public view returns (address) {
        return _inventory[1][n];
    }

    function GetOffense(uint256 n) public view returns (address) {
        return _inventory[2][n];
    }

    function Throw() public view returns (uint64) {
        // TODO
    }

    function Save(uint64 pQi) public view returns (uint64) {
        // TODO
    }

    function Equip(address qi) public onlyOwners {
        // TODO
    }

    function Remove(address qi) public onlyOwners {
        // TODO
    }
}