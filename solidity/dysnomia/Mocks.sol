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
contract MockHecke {
    function Meridians(uint256 idx) external pure returns (uint256) {
        return 1000 + idx;
    }
    function Compliment(uint256 Waat) external pure returns (int256 Longitude, int256 Latitude) {
        return (int256(Waat), int256(Waat * 2));
    }
    function GetMeridian(uint256 Waat) external pure returns (uint256) {
        return Waat % 100;
    }
}
