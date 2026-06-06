// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;

contract MockAtropaMath {
    function modExp(uint256 _b, uint256 _e, uint256 _m) external pure returns (uint256) {
        return uint256(keccak256(abi.encodePacked(_b, _e, _m)));
    }
}

contract MockLau {
    address public xiaoAddress;
    
    constructor(address _xiao) {
        xiaoAddress = _xiao;
    }
    
    function Xiao() external view returns (address) {
        return xiaoAddress;
    }
}
