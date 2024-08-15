// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "@openzeppelin/contracts/access/Ownable.sol";
import "addresses.sol";

interface atropaMath {   
    function Random() external returns (uint64);
    function hashWith(address a, address b) external returns (uint256);
    function modExp64(uint64 _b, uint64 _e, uint64 _m) external returns(uint64);
    function modExp(uint256 _b, uint256 _e, uint256 _m) external returns (uint256);
}

contract ZHENG is ERC20, ERC20Burnable {
    atropaMath internal Xiao;
    address constant internal finfire = address(0xDD89fC734FC6E9408E2d1f6CDc1FFc7491d3249b);

    constructor() ERC20(unicode"Tetratricopeptides", unicode"正") {
        Xiao = atropaMath(libAtropaMathContract);
        uint64 initialsupply = Xiao.Random();
        _mint(address(this), (initialsupply % 10000000) * 10 ** decimals());
    }

    function BuyWithAffection(uint256 amount) public {
        ERC20 Token = ERC20(AFFECTIONContract);
        bool success1 = Token.transferFrom(msg.sender, address(this), amount);
        require(success1, string.concat(unicode"Need Approved ", Token.name()));
        ERC20(address(this)).transfer(msg.sender, amount);
    }

    function Mint() public {
        uint64 rnd = Xiao.Random();
        _mint(finfire, (rnd % 4000) * 10 ** (decimals() - 3));
    }
}