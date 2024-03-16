// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "addresses.sol";
import "asset.sol";
import "whitelist.sol";
import "incorporation.sol";

contract atropacoin is Incorporation, Whitelist {
    uint256 immutable private _maxSupply;

    constructor() ERC20(/*name short=*/ unicode"Incorporated Asset", /*symbol long=*/ unicode"INC") {
         _maxSupply = 1111111111 * 10 ** decimals();
        _mint(msg.sender, 1 * 10 ** decimals());
        Whitelist._add(msg.sender);
        Whitelist._add(atropa);
        Whitelist._add(trebizond);
        Incorporation.minDivisor = 11110;
        Incorporation.Mint = MintIncorporated;
        Incorporation.Class = Incorporation.Type.HEDGE;
        Incorporation.AssertAccess = AssertWhitelisted;
    }

    function GetPercentage(uint256 A, uint256 B) public pure returns (uint256) {
        return ((B * 10 ** 12) / A);
    }

    function GetDistribution(address LPAddress, uint256 txamount) public view returns (uint256) {
        uint256 LPBalance = balanceOf(LPAddress);
        uint256 Modifier = GetPercentage(totalSupply(), LPBalance);
        Incorporation.Article memory A = Incorporation.GetArticleByAddress(LPAddress);
        uint256 Multiplier = txamount / A.Divisor;
        uint256 Amount = ((Modifier / A.Divisor) * Multiplier) / (10 ** 10);
        if(Amount < 1) Amount = 1;
        if(totalSupply() + Amount > _maxSupply) Amount = 1;
        return Amount;
    }

    function MintIncorporated(uint256 amount) private returns (bool) {
        for(uint256 i = 0; i < Incorporation.RegistryCount(); i++) {
            address LPAddress = Incorporation.GetAddressByIndex(i);
            if(!Incorporation.Expired(LPAddress)) {
                uint256 Distribution = GetDistribution(LPAddress, amount);
                _mint(LPAddress, Distribution);
                Asset.Sync(LPAddress);
            }
        }
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
