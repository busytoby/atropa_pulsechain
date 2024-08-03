// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "05_zheng.sol";


contract ZHOU is SH {
    string public constant Type = "ZHOU";

    uint64[] private Upsilon;
    ZHENG private Theta;

    constructor(address MathLib) ERC20(unicode"VM Zhou", unicode"ZHOU") SH(MathLib, 313) MultiOwnable(msg.sender) {
        Theta = new ZHENG(MathLib);
        address[] memory ZhengAddresses = Theta.KnownAddresses();
        for(uint256 i = 0; i < ZhengAddresses.length; i++) {
            RegisterAddress(ZhengAddresses[i]);
        }
        Theta.RegisterAddress(address(this));
        RegisterAddress(address(Theta));
    }
}