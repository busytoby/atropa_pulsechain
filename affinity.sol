// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "@openzeppelin/contracts/access/Ownable.sol";
import "dynamic.sol";

struct Registry {
    uint256[] keys;
    mapping(uint256 => uint256) indexOf;
    mapping(uint256 => bool) inserted;
}

abstract contract Affinity is Dynamic {
    Registry internal Sigma;
    Fa private Upsilon;

    function _dynamicCall(uint64 _Context, uint64 Operation, uint64 Entity) internal {
        if(_Context == 0) {
            Nu.Context = uint256(uint160(msg.sender));
            Nu.Operation = Operation;
            Nu.Entity = Entity;
            Nu.Theta = new bytes(0);
        } else if(_Context == Mu.Rod.Signal) {
            if(Operation == 0 && Entity == 0) {
                // then we know this is request to access level 0 setup-ish data
            }
            // then we knoe this is during setup-ish
        } else assert(false);
    }
}