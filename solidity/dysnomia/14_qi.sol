// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./include/user.sol";
import "./01_dysnomia_v2.sol";
import "./interfaces/13b_qinginterface.sol";
import "./interfaces/13d_qingfactoryinterface.sol";

contract QI is DYSNOMIA {
    User public Creator;
    uint64 public Quality;
    mapping(uint64 => int64) private _flux;

    constructor(User memory _creator, bytes memory Geng, address VoidQingAddress) DYSNOMIA("Geng Qi", "QI", address(DYSNOMIA(VoidQingAddress).Xiao())) {
        Creator = _creator;
        addOwner(tx.origin);


    }

    function AddMarketRate(address _a, uint256 _r) public onlyOwners {
        _addMarketRate(_a, _r);
    }

    function Consider(uint64 User) public onlyOwners returns (int64) {
        // TODO
        //return _flux[User];
    }

    function BlessForUser(uint64 User) public onlyOwners {
        // TODO
    }

    function CurseForUser(uint64 User) public onlyOwners {
        // TODO
    }

    function Bless() public onlyOwners {
        // TODO
    }

    function Curse() public onlyOwners {
        // TODO
    }
}