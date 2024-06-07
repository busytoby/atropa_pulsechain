// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "@openzeppelin/contracts/access/Ownable.sol";

interface atropaMath {   
    function Random() external returns (uint64);
    function hashWith(address a, address b) external returns (uint256);
    function modExp64(uint64 _b, uint64 _e, uint64 _m) external returns(uint64);
    function modExp(uint256 _b, uint256 _e, uint256 _m) external returns (uint256);
}

abstract contract Feh is ERC20, ERC20Burnable, Ownable {
    uint64 constant public MotzkinPrime = 953467954114363;
    //atropaMath internal Xiao = atropaMath(libAtropaMathContract);
    atropaMath internal Xiao = atropaMath(0xEAEfC777D0b087e5feF003bC32835d92fADF63C0);

    function _mintToCap() internal {
        if(totalSupply() <= (1111111111 * 10 ** decimals()))
            _mint(address(this), 1 * 10 ** decimals());
    }
}