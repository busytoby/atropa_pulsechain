// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;

library ToString {
    function String(address account) internal pure returns(string memory) {
        return String(abi.encodePacked(account));
    }

    function String(uint256 value) internal pure returns(string memory) {
        return String(abi.encodePacked(value));
    }

    function String(bytes32 value) internal pure returns(string memory) {
        return String(abi.encodePacked(value));
    }

    function String(bytes memory data) internal pure returns(string memory) {
        bytes memory hexa = "0123456789abcdef";

        bytes memory str = new bytes(2 + data.length * 2);
        str[0] = "0";
        str[1] = "x";
        for (uint i = 0; i < data.length; i++) {
            str[2+i*2] = hexa[uint(uint8(data[i] >> 4))];
            str[3+i*2] = hexa[uint(uint8(data[i] & 0x0f))];
        }
        return string(str);
    }
}