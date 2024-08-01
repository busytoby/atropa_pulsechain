// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "03_shio.sol";

contract BELT is SH {
    address constant MathLib = 0x430d082e46091173B8A4f9f48752e16e3A3a4c62;

    SHIO internal Psi;
    uint64 internal Xi;
    uint64 internal Ring;

    constructor() ERC20(unicode"VM Belt", unicode"BELT") SH(MathLib, 999) Ownable(msg.sender) {
        Psi = new SHIO(MathLib);
        Xi = Xiao.Random();
        Ring = Psi.OpenManifolds(Xi, Xiao.Random(), Xiao.Random());
    }
}