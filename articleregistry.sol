// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "accessregistry.sol";

abstract contract ArticleRegistry is AccessRegistry {
    using LibRegistry for LibRegistry.Registry;
    using atropaMath for address;

    enum IncorporationType {
        COMMODITY,
        HEDGE,
        SUBSIDY,
        OPTION,
        EXCHANGE,
        FUTURE,
        CAP,
        CLIMA,
        OFFSET
    }

    struct Article {
        address Address;
        uint256 Divisor;
        address ResponsibleParty;
        uint256 Expiration;
        IncorporationType Class;
    }

    LibRegistry.Registry private Registry;
    mapping(uint256 => Article) internal Articles;

    function RegisterArticle(address pool, uint256 divisor, address registree, uint256 length, IncorporationType class) public virtual;

    function GetArticleByAddress(address key) public view returns (Article memory) {
        uint256 hash = address(this).hashWith(key);
        return Articles[hash];
    }

    function ArticleExpired(address key) public view returns(bool) {
        uint256 hash = address(this).hashWith(key);
        return (block.timestamp > Articles[hash].Expiration);
    }

    function ArticleIsClass(address key, IncorporationType class) public view returns(bool) {
        uint256 hash = address(this).hashWith(key);
        return Articles[hash].Class == class;
    }

    function ArticleRegistryContains(address key) public view returns (bool) {
        uint256 hash = address(this).hashWith(key);
        return Registry.Contains(hash);
    }

    function ArticleRegistryCount() public view returns(uint256) {
        return Registry.Count();
    }

    function GetArticleByIndex(uint256 i) public view returns(Article memory) {
        uint256 addr = Registry.GetHashByIndex(i);
        return Articles[addr];
    }

    function SetArticle(address key, uint256 Divisor, address ResponsibleParty, uint256 Length, IncorporationType Class) internal {
        assert(Class == IncorporationType.COMMODITY || Class == IncorporationType.OPTION || Class == IncorporationType.EXCHANGE);
        uint256 hash = address(this).hashWith(key);
        Registry.Register(hash);
        Articles[hash].Address = key;
        Articles[hash].Divisor = Divisor;
        Articles[hash].ResponsibleParty = ResponsibleParty;
        Articles[hash].Expiration = block.timestamp + Length;
        Articles[hash].Class = Class;
    }

    function RemoveArticle(address key) public {
        Article memory A = GetArticleByAddress(key);
        if(A.ResponsibleParty != msg.sender) 
            assert(HasAccess(msg.sender, AccessType.TOD, key));
        uint256 hash = address(this).hashWith(key);
        Registry.Remove(hash);
        delete Articles[hash];
    }
}