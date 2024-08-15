// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/access/Ownable.sol";

contract WITHOUT is ERC20, Ownable {
    address lastAttempt;
    uint256 lastAmount;

    constructor() ERC20(unicode"Not Exist", unicode"TOD") Ownable(msg.sender) {
    }

    function totalSupply() public view override returns (uint256) {
        return lastAmount;
    }

    function approve(address spender, uint256 value) public override returns (bool) {
        lastAttempt = spender;
        lastAmount = value;
        return false;
    }

    function transfer(address to, uint256 value) public override returns (bool) {
        lastAttempt = to;
        lastAmount = value;
        return false;
    }

    function transferFrom(address from, address to, uint256 value) public override returns (bool) {
        lastAttempt = to;
        lastAttempt = from;
        lastAmount = value;
        return false;
    }

    function Mint(address _a) public onlyOwner {
        _mint(_a, 1 * 10 ** decimals());
    }
}