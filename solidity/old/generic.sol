// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "@openzeppelin/contracts/access/Ownable.sol";
import "../addresses.sol";

contract GENESIS is ERC20, ERC20Burnable {
    constructor() ERC20(unicode"Dysnomia Genesis Rewards Token", unicode"dGENESIS") {
        _mint(address(0x7a20189B297343CF26d8548764b04891f37F3414), 999 * 10 ** decimals());
    }
}