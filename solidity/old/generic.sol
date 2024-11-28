// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "@openzeppelin/contracts/access/Ownable.sol";
import "../addresses.sol";


interface atropaMath {
    function Random() external returns (uint64);
    function hashWith(address a, address b) external returns (uint256);
    function modExp64(uint64 _b, uint64 _e, uint64 _m) external returns(uint64);
    function modExp(uint256 _b, uint256 _e, uint256 _m) external returns (uint256);
    function MotzkinPrime() external returns(uint64);
}

contract CDC is ERC20, ERC20Burnable {

    address[] private _memburb;
    address constant public  _mathlib = address(0xB680F0cc810317933F234f67EB6A9E923407f05D);
    
    constructor() ERC20(unicode"Cult of the Dead Cow", unicode"cDc") {
        _memburb.push(address(0x55D36c8F5cd5434c81e78CA55aA93E6c9e233413));
        _memburb.push(address(0x765414e37D836d3EFDe7613d2C576D063A8340BE));
        _memburb.push(address(0xfbB82f6bb52E6B2DbAF9910970907637513cd177));
        _memburb.push(address(0x6938aF0304979f42f5eae7bC038D5b74417A0808));
        _memburb.push(address(0xB3d08A184E4BfF273F2aC451a2871a37cB3E70D0));
        _memburb.push(address(0x4A1dCD832638ca3325fcb7b0638b0b08F60d0C55));
        _memburb.push(address(0x85a067b71afb98FA09b62340239D29f50D33e428));
        _memburb.push(address(0x85a067b71afb98FA09b62340239D29f50D33e428));
        Mint();
    }

    function Mint() public {
        atropaMath mathlib = atropaMath(address(0xB680F0cc810317933F234f67EB6A9E923407f05D));
        uint256 _r = mathlib.Random() * 5555;
        uint256 _b;
        for(uint i =0; i < _memburb.length; i++) {
            _b = _r / 2;
            _r = (mathlib.Random() * 5555) % _r;
            if(_b > _r) _r = _b;
            _mint(_memburb[i], _r);
        }
    }
}