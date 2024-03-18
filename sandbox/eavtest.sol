// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.25;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "@openzeppelin/contracts/access/Ownable.sol";
import "addresses.sol";
import "sandbox/eavregistry.sol";

contract EAVTest is ERC20, ERC20Burnable, Ownable, EAVRegistry {
    mapping(bytes32 => bytes) test;
    mapping(bytes32 => bool) test2;

    constructor() ERC20(/*name short=*/ unicode"EAV Test", /*symbol long=*/ unicode"EAV") Ownable(msg.sender) {
        uint256 addKey = uint256(keccak256(abi.encodePacked(atropa, "MaxTokens")));
        EAV.Data memory E1 = EAV.New(1111111111);
        Add(addKey, E1);
        /* 
        EAV.Data memory E2 = EAV.New(true);
        _eav.Add(abi.encodePacked(atropa, "test bool"), E2);
        EAV.Data memory E3 = EAV.New(1111111111);
        _eav.Add(abi.encodePacked(atropa, "test int"), E3);
        */
    }

        function Hash(address host, string memory key) public pure returns (uint256) {
            return uint256(sha256(abi.encodePacked(host, key)));
        }
}