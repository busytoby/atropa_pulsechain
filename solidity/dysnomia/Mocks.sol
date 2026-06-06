// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;

contract MockAtropaMath {
    function modExp(uint256 _b, uint256 _e, uint256 _m) external pure returns (uint256) {
        return uint256(keccak256(abi.encodePacked(_b, _e, _m)));
    }

    function Random() external pure returns (uint64) {
        return 42069;
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

contract MockCho {
    address public userTokenAddress;

    constructor(address _userTokenAddress) {
        userTokenAddress = _userTokenAddress;
    }

    function GetUser() external pure returns (address) {
        // Struct User has format in user.sol, but we can return dummy or not implement since PKMinter doesn't call it in constructor
    }

    function VerifyUserTokenPermissions(address UserToken) external pure {
        // No-op
    }

    function GetUserTokenAddress(address wallet) external view returns (address) {
        return userTokenAddress;
    }
}

