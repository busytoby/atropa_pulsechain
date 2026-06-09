// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

contract Target {
    function echo(address /* to */, uint256 val) external pure returns (uint256) {
        return val * 2;
    }
}
