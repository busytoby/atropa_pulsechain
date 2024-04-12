


pragma solidity ^0.8.10;


import "./ERC20.sol";
import "./Ownable.sol";
import "./ERC20Burnable.sol";

 contract DiscoveryCoin is ERC20, ERC20Burnable, Ownable {

constructor() ERC20(unicode"Hammer Time", unicode"DISCOVERY") Ownable(msg.sender) {
        _mint(msg.sender, 666 * 10 ** decimals());
   }
 }
 

