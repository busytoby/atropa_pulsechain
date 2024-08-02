// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "05_zheng.sol";


contract ZHOU is SH {
    string public constant Type = "ZHOU";
    address public constant MathLib = 0x1a6e3ABCcFFaA12b8F24192A44B42A86341410B0;

    Fei private Upsilon;
    ZHENG private Theta;

    constructor() ERC20(unicode"VM Zhou", unicode"ZHOU") SH(MathLib, 313) MultiOwnable(msg.sender) {
        Theta = new ZHENG(MathLib);
        address[] memory ZhengAddresses = Theta.KnownAddresses();
        for(uint256 i = 0; i < ZhengAddresses.length; i++)
            Addresses.push(ZhengAddresses[i]);
        Addresses.push(address(this));
    }
}