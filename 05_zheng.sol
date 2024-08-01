// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "04_yi.sol";


struct Fan {
    Bao[] Rods;
    Bao[] Cones;
}

contract ZHENG is SH {
    address constant MathLib = 0x430d082e46091173B8A4f9f48752e16e3A3a4c62;

    mapping(uint256 => mapping(uint256 => Fei)) internal Eta;
    Fan internal Sigma;
    YI internal Upsilon;

    constructor() ERC20(unicode"VM Zheng", unicode"ZHENG") SH(MathLib, 5) Ownable(msg.sender) {
        Upsilon = new YI(MathLib);
    }
}