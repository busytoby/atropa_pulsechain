// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "06_zhou.sol";

contract YAU is SH {
    string public constant Type = "YAU";

    ZHOU private Theta;
    mapping(uint64 => uint64) private Connections;
    mapping(uint64 => address) private ConnectionAddresses;

    constructor(address MathLib, address ZhouAddress) ERC20(unicode"VM Yau", unicode"YAU") SH(MathLib, 111) MultiOwnable(msg.sender) {
        Theta = ZHOU(ZhouAddress);
        address[] memory ZhouAddresses = Theta.KnownAddresses();
        for(uint256 i = 0; i < ZhouAddresses.length; i++) {
            RegisterAddress(ZhouAddresses[i]);
        }
        Theta.RegisterAddress(address(this));
        RegisterAddress(address(Theta));
    }
}