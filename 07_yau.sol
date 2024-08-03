// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "06_zhou.sol";

contract YAU is SH {
    string public constant Type = "YAU";
    address public constant MathLib = 0x48087234ed7bc38e04347176b08B860E643806e2;

    ZHOU private Theta;
    mapping(uint64 => uint64) private Connections;
    mapping(uint64 => address) private ConnectionAddresses;

    constructor() ERC20(unicode"VM Yau", unicode"YAU") SH(MathLib, 111) MultiOwnable(msg.sender) {
        Theta = new ZHOU(MathLib);
        address[] memory ZhouAddresses = Theta.KnownAddresses();
        for(uint256 i = 0; i < ZhouAddresses.length; i++) {
            RegisterAddress(ZhouAddresses[i]);
        }
        Theta.RegisterAddress(address(this));
        RegisterAddress(address(Theta));
    }
}