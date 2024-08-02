// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "05_zheng.sol";


contract ZHOU is SH {
    address constant MathLib = 0x430d082e46091173B8A4f9f48752e16e3A3a4c62;

    Bao internal Upsilon;
    ZHENG internal Theta;

    constructor() ERC20(unicode"VM Zhou", unicode"ZHOU") SH(MathLib, 313) Ownable(msg.sender) {
        Theta = new ZHENG(MathLib);
    }
}