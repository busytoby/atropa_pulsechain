// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.25;

import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "@openzeppelin/contracts/access/Ownable.sol";

contract DiscoveryCoin is ERC20, ERC20Burnable, Ownable {
    constructor() ERC20("Hammer Time", "DISCOVERY") {
        _mint(msg.sender, 666 * 10 ** decimals());
    }
}
