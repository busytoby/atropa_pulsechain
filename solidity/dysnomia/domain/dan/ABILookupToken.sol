// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;

/**
 * @title ABILookupToken
 * @dev Solidity interface matching the CL9 ABI lookup contract on PulseChain.
 *      Allows registering and querying ABI hashes, function selectors, and method interfaces.
 */
interface IABILookupToken {
    function registerABI(string calldata signature) external returns (bytes4 selector);
    function getSignature(bytes4 selector) external view returns (string memory signature);
    function isRegistered(bytes4 selector) external view returns (bool);
    
    event ABIRegistered(bytes4 indexed selector, string signature, address indexed registrar);
}

contract ABILookupToken is IABILookupToken {
    string public constant name = "CL9 ABI Lookup Token";
    string public constant symbol = "CL9";
    uint8 public constant decimals = 18;

    mapping(bytes4 => string) private _signatures;
    mapping(bytes4 => bool) private _registered;

    constructor() {
        // Pre-register standard ERC20/Void functions
        _register("transfer(address,uint256)");
        _register("approve(address,uint256)");
        _register("transferFrom(address,address,uint256)");
        _register("balanceOf(address)");
        _register("totalSupply()");
        _register("Chat(string)");
        _register("Log(string)");
        _register("execute_dc(bytes)");
        _register("run_command(address,bytes)");
    }

    function registerABI(string calldata signature) external override returns (bytes4 selector) {
        selector = _register(signature);
        emit ABIRegistered(selector, signature, msg.sender);
    }

    function getSignature(bytes4 selector) external view override returns (string memory signature) {
        require(_registered[selector], "Selector not registered");
        return _signatures[selector];
    }

    function isRegistered(bytes4 selector) external view override returns (bool) {
        return _registered[selector];
    }

    function _register(string memory signature) internal returns (bytes4 selector) {
        selector = bytes4(keccak256(bytes(signature)));
        _signatures[selector] = signature;
        _registered[selector] = true;
    }
}
