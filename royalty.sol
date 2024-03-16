// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "@openzeppelin/contracts/access/Ownable.sol";
import "addresses.sol";
import "asset.sol";
import "whitelist.sol";
import "incorporation.sol";

contract atropacoin is ERC20, ERC20Burnable, Ownable, Incorporation {
    // Default Commodity
    bool private SUBSIDY = false;
    bool private HEDGE = false;
    constructor() ERC20(/*name short=*/ unicode"Incorporated Asset", /*symbol long=*/ unicode"INC") {
        _mint(msg.sender, 1 * 10 ** decimals());
        Whitelist.Add(msg.sender);
        Whitelist.Add(atropa);
        Whitelist.Add(trebizond);
        Incorporation.minDivisor = 11110;
        assert(!(SUBSIDY && HEDGE));
    }

    function GetPercentage(uint256 A, uint256 B) public pure returns (uint256) {
        return ((B * 10 ** 12) / A);
    }

    function GetDistribution(address LPAddress, uint256 txamount) public view returns (uint256) {
        uint256 LPBalance = balanceOf(LPAddress);
        uint256 Modifier = GetPercentage(totalSupply(), LPBalance);
        Incorporation.Article memory A = Incorporation.getbyaddress(LPAddress);
        uint256 Multiplier = txamount / A.Divisor;
        uint256 Amount = ((Modifier / A.Divisor) * Multiplier) / (10 ** 10);
        if(Amount < 1) Amount = 1;
        if((totalSupply() + Amount) > (1111111111 * 10 ** decimals())) Amount = 1;
        return Amount;
    }

    function MintDerivative(address LPAddress, uint256 txamount) private {
        uint256 Amount = GetDistribution(LPAddress, txamount);
        _mint(LPAddress, Amount);
        Asset.Sync(LPAddress);
    }

    function Mint(uint256 amount) private returns (bool) {
        for(uint256 i = 0; i < Incorporation.count(); i++) {
            address LPAddress = Incorporation.getbyindex(i);
            if(!Incorporation.expired(LPAddress))
                MintDerivative(LPAddress, amount);
        }
        return true;
    }

    function transfer(address to, uint256 amount) public override returns (bool) {
        address owner = _msgSender();
        if(!SUBSIDY)
            if(Incorporation.registered(to) || Incorporation.registered(owner))
                Mint(amount);
        _transfer(owner, to, amount);
        return true;
    }

    function transferFrom(address from, address to, uint256 amount) public override returns (bool) {
        address spender = _msgSender();
        if(!HEDGE)
            if(Incorporation.registered(from) || Incorporation.registered(to))
                Mint(amount);
        _spendAllowance(from, spender, amount);
        _transfer(from, to, amount);
        return true;
    }


/*
function modExp(uint256 _b, uint256 _e, uint256 _m) public returns (uint256 result) {
        assembly {
            // Free memory pointer
            let pointer := mload(0x40)

            // Define length of base, exponent and modulus. 0x20 == 32 bytes
            mstore(pointer, 0x20)
            mstore(add(pointer, 0x20), 0x20)
            mstore(add(pointer, 0x40), 0x20)

            // Define variables base, exponent and modulus
            mstore(add(pointer, 0x60), _b)
            mstore(add(pointer, 0x80), _e)
            mstore(add(pointer, 0xa0), _m)

            // Store the result
            let value := mload(0xc0)

            // Call the precompiled contract 0x05 = bigModExp
            if iszero(call(not(0), 0x05, 0, pointer, 0xc0, value, 0x20)) {
                revert(0, 0)
            }

            result := mload(value)
        }
    }
*/
}
