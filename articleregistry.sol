// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.25;
import "accessregistry.sol";

abstract contract ArticleRegistry is AccessRegistry {
    using LibRegistry for LibRegistry.Registry;

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
    mapping(address => Article) internal Articles;

    function RegisterArticle(address pool, uint256 divisor, address registree, uint256 length, IncorporationType class) public virtual;

    function GetArticleByAddress(address key) public view returns (Article memory) {
        return Articles[key];
    }

    function ArticleExpired(address key) public view returns(bool) {
        return (block.timestamp > Articles[key].Expiration);
    }

    function ArticleIsClass(address key, IncorporationType class) public view returns(bool) {
        return Articles[key].Class == class;
    }

    function ArticleRegistryContains(address key) public view returns (bool) {
        return Registry.Contains(key);
    }

    function ArticleRegistryCount() public view returns(uint256) {
        return Registry.Count();
    }

    function GetArticleByIndex(uint256 i) public view returns(Article memory) {
        address addr = Registry.GetAddressByIndex(i);
        return Articles[addr];
    }

    function SetArticle(address key, uint256 Divisor, address ResponsibleParty, uint256 Length, IncorporationType Class) internal {
        assert(Class == IncorporationType.COMMODITY || Class == IncorporationType.OPTION || Class == IncorporationType.EXCHANGE);
        Registry.Register(key);
        Articles[key].Address = key;
        Articles[key].Divisor = Divisor;
        Articles[key].ResponsibleParty = ResponsibleParty;
        Articles[key].Expiration = block.timestamp + Length;
        Articles[key].Class = Class;
    }

    function RemoveArticle(address key) public {
        Article memory A = GetArticleByAddress(key);
        if(A.ResponsibleParty != msg.sender) 
            assert(HasAccess(msg.sender, AccessType.TOD, key));
        Registry.Remove(key);
        delete Articles[key];
    }
}