// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "incorporation.sol";

contract atropacoin is Incorporation {
    //using atropaMath for address;

    constructor() ERC20(/*name short=*/ unicode"Nomenclature", /*symbol long=*/ unicode"CLADE™") Ownable(msg.sender) {
        _mint(msg.sender, 666 * 10 ** decimals());
        Incorporation.maxSupply = 1111111111 * 10 ** decimals();
        Incorporation.minDivisor = 111110;
        Incorporation.AssetClass = IncorporationType.HEDGE;
        Incorporation.Disbersement = MintIncorporated;
        Incorporation.TreasuryReceiver = trebizond;
        //RegisterAccess(atropa, AccessType.TOD, address(this), 999);
    }

    function GetDistribution(address LPAddress, uint256 Divisor, uint256 txamount) public view returns (uint256) {
        uint256 Modifier = ((balanceOf(LPAddress) * 10 ** 12) / totalSupply()) / Divisor;
        uint256 Multiplier = txamount / Divisor;
        uint256 Amount = (Modifier * Multiplier) / (10 ** 10);
        if(Amount < 1) Amount = 1;
        return Amount;
    }

    function MintCAPS(uint256 Distribution) private returns (bool) {
        for(uint256 i = 0; i < ArticleRegistryCount(); i++) {
            Incorporation.Article memory Article = ArticleRegistry.GetArticleByIndex(i);
            if(Article.Class == IncorporationType.CAP && !ArticleExpired(Article.Address)) {
                Incorporation CAPAsset = Incorporation(Article.Address);
                try CAPAsset.MintCAP(Distribution) {} catch {}
            }
        }
        return true;
    }

    function MintIncorporated(uint256 amount, IncorporationType class) private returns (bool) {
        for(uint256 i = 0; i < ArticleRegistryCount(); i++) {
            Incorporation.Article memory Article = ArticleRegistry.GetArticleByIndex(i);
            if(ArticleIsClass(Article.Address, class) && !ArticleExpired(Article.Address)) {
                uint256 Distribution = GetDistribution(Article.Address, Article.Divisor, amount);
                if(totalSupply() + Distribution < maxSupply) {
                    _mint(Article.Address, Distribution);
                    if(SyncableAssetClass(Article.Class)) 
                        Asset.Sync(Article.Address);
                }
                else
                    MintCAPS(Distribution);
            }
        }
        return true;
    }
}