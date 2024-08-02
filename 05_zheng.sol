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
    mapping(address => mapping(address => Fei)) internal Eta;
    Fan internal Sigma;
    YI internal Upsilon;

    constructor(address MathLib) ERC20(unicode"VM Zheng", unicode"ZHENG") SH(MathLib, 111) MultiOwnable(msg.sender) {
        Upsilon = new YI(MathLib);
    }
}