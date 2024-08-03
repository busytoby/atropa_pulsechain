// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "06_zhou.sol";

contract YAU is SH {
    string public constant Type = "YAU";
    address public constant MathLib = 0x1a6e3ABCcFFaA12b8F24192A44B42A86341410B0;

    ZHOU private Theta;
    mapping(uint64 => uint64) private Connections;
    mapping(uint64 => address) private ConnectionAddresses;

    constructor() ERC20(unicode"VM Yau", unicode"YAU") SH(MathLib, 111) MultiOwnable(msg.sender) {
        Theta = new ZHOU(MathLib);
        address[] memory ZhouAddresses = Theta.KnownAddresses();
        for(uint256 i = 0; i < ZhouAddresses.length; i++)
            Addresses.push(ZhouAddresses[i]);
        Addresses.push(address(this));
    }
}