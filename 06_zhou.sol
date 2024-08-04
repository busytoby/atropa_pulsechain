// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "05_zheng.sol";

contract ZHOU is SH {
    string public constant Type = "ZHOU";
    address public constant MathLib = 0x48087234ed7bc38e04347176b08B860E643806e2;

    uint64[] private Upsilon;
    ZHENG private Theta;

    constructor() ERC20(unicode"VM Zhou", unicode"ZHOU") SH(MathLib, 313) MultiOwnable(msg.sender) {
        Theta = new ZHENG(MathLib);
        address[] memory ZhengAddresses = Theta.KnownAddresses();
        for(uint256 i = 0; i < ZhengAddresses.length; i++) {
            RegisterAddress(ZhengAddresses[i]);
        }
        Theta.RegisterAddress(address(this));
        RegisterAddress(address(Theta));
    }
}