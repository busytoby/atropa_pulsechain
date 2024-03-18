// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.25;
import "registry.sol";

abstract contract ArticleRegistry {
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

    LibRegistry.Registry internal Registry;
    mapping(address => Article) internal Articles;

    function Register(address pool, uint256 divisor, address registree, uint256 length, IncorporationType class) public virtual;
    function(address) internal AssertAccess;

    function GetArticleByAddress(address key) public view returns (Article memory) {
        return Articles[key];
    }

    function Expired(address key) public view returns(bool) {
        return (block.timestamp > Articles[key].Expiration);
    }

    function IsClass(address key, IncorporationType class) public view returns(bool) {
        return Articles[key].Class == class;
    }

    function ArticleRegistryCount() public view returns(uint256) {
        return Registry.Count();
    }

    function GetArticleByIndex(uint256 i) public view returns(Article memory) {
        address addr = Registry.GetAddressByIndex(i);
        return Articles[addr];
    }

    function SetArticle(address key, uint256 Divisor, address ResponsibleParty, uint256 Length, IncorporationType Class) internal {
        Registry.Register(key);
        assert(Class == IncorporationType.COMMODITY || Class == IncorporationType.OPTION || Class == IncorporationType.EXCHANGE);
        Articles[key].Address = key;
        Articles[key].Divisor = Divisor;
        Articles[key].ResponsibleParty = ResponsibleParty;
        Articles[key].Expiration = block.timestamp + Length;
        Articles[key].Class = Class;
    }

    function Deregister(address key) public {
        Article memory A = GetArticleByAddress(key);
        if(A.ResponsibleParty != msg.sender) 
            AssertAccess(msg.sender);
        Registry.Remove(key);
        delete Articles[key];
    }
}