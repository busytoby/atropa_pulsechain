// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "@openzeppelin/contracts/access/Ownable.sol";
import "addresses.sol";
import "articleregistry.sol";
import "asset.sol";

abstract contract Incorporation is ERC20, ERC20Burnable, Ownable, Asset, ArticleRegistry {
    using LibRegistry for LibRegistry.Registry;

    IncorporationType immutable internal AssetClass;
    uint256 immutable internal maxSupply;
    uint256 immutable internal minDivisor = 1110;
    address immutable internal TreasuryReceiver;

    function(uint256, IncorporationType) internal returns (bool) Disbersement;

    function SyncableAssetClass(IncorporationType class) public pure returns (bool) {
        return (class != IncorporationType.EXCHANGE && class != IncorporationType.FUTURE && class != IncorporationType.CAP && class != IncorporationType.CLIMA);
    }

    function RegisterArticle(address pool, uint256 divisor, address registree, uint256 length, IncorporationType class) public override {
        assert(length < 367);
        assert(HasAccess(msg.sender, AccessType.GUELPH, address(this)));
        if(class != IncorporationType.FUTURE && class != IncorporationType.CAP)
            assert(divisor > minDivisor);
        if(SyncableAssetClass(class)) assert(Asset.Sync(pool) == true);
        uint256 Expiration = block.timestamp + (length * 1 days);
        if(ArticleRegistryContains(pool)) {
            assert(HasAccess(msg.sender, AccessType.TOD, pool));
            Expiration = GetArticleByAddress(pool).Expiration;
        }
        SetArticle(pool, divisor, registree, Expiration, class);
        RegisterAccess(msg.sender, AccessType.TOD, pool, Expiration);
    }

    function MintCAP(uint256 amount) public {
        Article memory Clima = GetArticleByAddress(msg.sender);
        assert(Clima.Class == IncorporationType.CLIMA);
        uint256 DisbersementAmount = amount / Clima.Divisor;
        if(totalSupply() + DisbersementAmount < maxSupply)
            _mint(TreasuryReceiver, DisbersementAmount);
        else
            Disbersement(DisbersementAmount, IncorporationType.OFFSET);
    }

    function transfer(address to, uint256 amount) public override returns (bool) {
        address owner = _msgSender();
        if(!(AssetClass == IncorporationType.SUBSIDY))
            if(ArticleRegistryContains(to) || ArticleRegistryContains(owner))
                Disbersement(amount, IncorporationType.COMMODITY);
        if(ArticleRegistryContains(to) && ArticleRegistryContains(owner) && (ArticleIsClass(owner, IncorporationType.EXCHANGE)))
                Disbersement(amount, IncorporationType.OPTION);
        if(ArticleRegistryContains(to) && ArticleRegistryContains(owner) && (ArticleIsClass(to, IncorporationType.EXCHANGE)))
                Disbersement(amount, IncorporationType.FUTURE);
        _transfer(owner, to, amount);
        return true;
    }

    function transferFrom(address from, address to, uint256 amount) public override returns (bool) {
        address spender = _msgSender();
        if(!(AssetClass == IncorporationType.HEDGE))
            if(ArticleRegistryContains(from) || ArticleRegistryContains(to))
                Disbersement(amount, IncorporationType.COMMODITY);
        if(ArticleRegistryContains(from) && ArticleRegistryContains(to) && (ArticleIsClass(from, IncorporationType.EXCHANGE)))
            Disbersement(amount, IncorporationType.OPTION);
        if(ArticleRegistryContains(from) && ArticleRegistryContains(to) && (ArticleIsClass(to, IncorporationType.EXCHANGE)))
            Disbersement(amount, IncorporationType.FUTURE);
        _spendAllowance(from, spender, amount);
        _transfer(from, to, amount);
        return true;
    }
}