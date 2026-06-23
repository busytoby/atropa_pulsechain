// SPDX-License-Identifier: MIT
pragma solidity ^0.8.21;

contract Batcher {
    function batchCall(address[] calldata targets, bytes[] calldata datas) external {
        require(targets.length == datas.length, "Length mismatch");
        for (uint256 i = 0; i < targets.length; i++) {
            (bool success, ) = targets[i].call(datas[i]);
            require(success, "Call failed");
        }
    }
}
