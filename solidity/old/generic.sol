// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "@openzeppelin/contracts/access/Ownable.sol";
import "../addresses.sol";

contract ASGARD is ERC20, ERC20Burnable {
    constructor() ERC20(unicode"Make The Grade", unicode"ÁSGARD") {
        _mint(address(0xBF182955401aF3f2f7e244cb31184E93E74a2501), 5554 * 10 ** decimals());
    }
}