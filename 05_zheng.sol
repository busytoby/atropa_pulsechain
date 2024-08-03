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
    string public constant Type = "ZHENG";

    mapping(address => mapping(address => uint64[])) private Eta;
    Fan private Sigma;
    YI private Upsilon;

    constructor(address MathLib) ERC20(unicode"VM Zheng", unicode"ZHENG") SH(MathLib, 100) MultiOwnable(msg.sender) {
        Upsilon = new YI(MathLib);
        address[] memory YiAddresses = Upsilon.KnownAddresses();
        for(uint256 i = 0; i < YiAddresses.length; i++) {
            RegisterAddress(YiAddresses[i]);
        }
        Upsilon.RegisterAddress(address(this));
        RegisterAddress(address(Upsilon));
    }
}