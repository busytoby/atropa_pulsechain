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

abstract contract Affinity is ERC20Burnable, Dynamic {
    Registry internal Sigma;

    function _dynamicCall(uint64 Context, uint64 Operation, uint64 Entity) internal {
        assert(false);
    }
}