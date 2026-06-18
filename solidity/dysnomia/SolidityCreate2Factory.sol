// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;

contract SolidityCreate2Factory {
    event ContractDeployed(address indexed contractAddress, bytes32 indexed salt, bytes32 indexed bytecodeHash);

    constructor() {}

    /**
     * @dev Deploys a contract using CREATE2.
     * @param bytecode The creation bytecode of the contract to deploy.
     * @param salt The salt used for deterministic address generation.
     */
    function deploy(bytes memory bytecode, bytes32 salt) public payable returns (address deployedAddress) {
        require(bytecode.length > 0, "Empty bytecode");
        
        assembly {
            // create2(v, p, n, s)
            // v = value (msg.value)
            // p = pointer to bytecode in memory (add(bytecode, 0x20) skips length field)
            // n = size of bytecode in memory (mload(bytecode))
            // s = salt
            deployedAddress := create2(callvalue(), add(bytecode, 0x20), mload(bytecode), salt)
            
            // Revert if deployment failed (address is 0) or codesize is 0
            if iszero(extcodesize(deployedAddress)) {
                revert(0, 0)
            }
        }

        emit ContractDeployed(deployedAddress, salt, keccak256(bytecode));
    }

    /**
     * @dev Computes the deterministic address for a deployment.
     */
    function computeAddress(bytes memory bytecode, bytes32 salt) public view returns (address) {
        bytes32 hash = keccak256(
            abi.encodePacked(
                bytes1(0xff),
                address(this),
                salt,
                keccak256(bytecode)
            )
        );
        return address(uint160(uint256(hash)));
    }
}
