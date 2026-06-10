// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

contract Target {
    function echo(address /* to */, uint256 val) external pure returns (uint256) {
        return val * 2;
    }

    function Type() external pure returns (string memory) {
        return "ROOM";
    }

    function name() external pure returns (string memory) {
        return "On-Chain Echo Chamber";
    }

    function symbol() external pure returns (string memory) {
        return "ECHO";
    }

    function execute(string calldata cmd) external pure returns (string memory) {
        return string(abi.encodePacked("[ECHO] The room reverberates with your command: '", cmd, "'"));
    }
}

