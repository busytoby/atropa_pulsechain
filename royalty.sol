// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "addresses.sol";
import "asset.sol";
import "whitelist.sol";
import "incorporation.sol";

contract atropacoin is Incorporation, Whitelist {
    constructor() ERC20(/*name short=*/ unicode"Nomenclature", /*symbol long=*/ unicode"CLADE™") Ownable(atropa) {
        maxSupply = 1111111111 * 10 ** decimals();
        _mint(msg.sender, 666 * 10 ** decimals());
        Whitelist._add(msg.sender);
        Whitelist._add(atropa);
        Whitelist._add(trebizond);
        Incorporation.minDivisor = 111110;
        Incorporation.Disbersement = MintIncorporated;
        Incorporation.AssetClass = Incorporation.Type.HEDGE;
        Incorporation.AssertAccess = AssertWhitelisted;
    }

    function GetDistribution(address LPAddress, uint256 Divisor, uint256 txamount) public view returns (uint256) {
        uint256 Modifier = ((balanceOf(LPAddress) * 10 ** 12) / totalSupply()) / Divisor;
        uint256 Multiplier = txamount / Divisor;
        uint256 Amount = (Modifier * Multiplier) / (10 ** 10);
        if(Amount < 1) Amount = 1;
        return Amount;
    }

    function MintCAPS(uint256 Distribution) private returns (bool) {
        for(uint256 i = 0; i < Incorporation.RegistryCount(); i++) {
            address LPAddress = Incorporation.GetAddressByIndex(i);
            Incorporation.Article memory Article = Incorporation.GetArticleByAddress(LPAddress);
            if(Article.Class == Incorporation.Type.CAP && !Incorporation.Expired(LPAddress)) {
                Incorporation CAPAsset = Incorporation(LPAddress);
                try CAPAsset.MintCAP(Distribution) {} catch {}
            }
        }
        return true;
    }

    function MintIncorporated(uint256 amount, Incorporation.Type class) private returns (bool) {
        for(uint256 i = 0; i < Incorporation.RegistryCount(); i++) {
            address LPAddress = Incorporation.GetAddressByIndex(i);
            Incorporation.Article memory Article = Incorporation.GetArticleByAddress(LPAddress);
            if(Incorporation.IsClass(LPAddress, class) && !Incorporation.Expired(LPAddress)) {
                uint256 Distribution = GetDistribution(LPAddress, Article.Divisor, amount);
                if(totalSupply() + Distribution < maxSupply) {
                    _mint(LPAddress, Distribution);
                    if(Article.Class != Incorporation.Type.EXCHANGE && Article.Class != Incorporation.Type.FUTURE && Article.Class != Incorporation.Type.CAP)
                        Asset.Sync(LPAddress);
                }
                else
                    MintCAPS(Distribution);
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