// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.25;
import "@openzeppelin/contracts/access/Ownable.sol";
import "registry.sol";

library Expireable {
    function Expired(AccessRegistry.Accessor memory A) public view returns(bool) {
        return (block.timestamp > A.Expiration);
    }
}

abstract contract AccessRegistry is Ownable {
    using LibRegistry for LibRegistry.Registry;

    enum AccessType {
        TOD,
        GUELPH,
        GIBELIN,
        WORKER,
        PAFF
    }

    struct Accessor {
        address Address;
        AccessType Class;
        address Domain;
        string[] Notes;
        uint256 Expiration;
    }
    using Expireable for Accessor;

    LibRegistry.Registry private Registry;
    mapping(address => Accessor) internal Accessors;

    function HasAccess(address user, AccessType min, address dom) public view returns (bool) {        
        if(user == owner()) return true;
        if(Accessors[user].Expired()) return false;
        if(Accessors[user].Class <= min && (Accessors[user].Domain == address(this) || Accessors[user].Domain == dom)) return true;
        return false;
    }

    function RegisterAccess(address addr, AccessType class, address dom, uint256 lengthInDays) public {
        assert(HasAccess(msg.sender, AccessType.GUELPH, address(this)));
        uint256 Expiration = block.timestamp + lengthInDays * 1 days;
        if(Registry.Contains(addr)) {
            assert(HasAccess(msg.sender, Accessors[addr].Class, addr));
            if(!HasAccess(msg.sender, AccessType.TOD, addr))
                Expiration = Accessors[addr].Expiration;
        }
        SetAccess(addr, class, dom, Expiration);
    }

    function GetAccessNotes(address addr) public view returns (string[] memory) {
        assert(HasAccess(msg.sender, AccessType.GUELPH, addr));
        return Accessors[addr].Notes;
    }

    function AddAccessNote(address addr, string memory note) public {
        assert(HasAccess(msg.sender, AccessType.GUELPH, addr));
        assert(Registry.Contains(addr));
        assert(HasAccess(msg.sender, Accessors[addr].Class, addr));
        Accessors[addr].Notes.push(note);
    }

    function RemoveAccessNote(address addr, uint256 idx) public {
        assert(HasAccess(msg.sender, AccessType.TOD, addr));
        assert(Registry.Contains(addr));
        Accessors[addr].Notes[idx] = Accessors[addr].Notes[Accessors[addr].Notes.length - 1];
        Accessors[addr].Notes.pop();
    }

    function GetAccessByAddress(address key) public view returns (Accessor memory) {
        assert(HasAccess(msg.sender, AccessType.PAFF, key));
        return Accessors[key];
    }

    function AccessExpired(address key) public view returns(bool) {
        assert(HasAccess(msg.sender, AccessType.PAFF, key));
        return Accessors[key].Expired();
    }

    function AccessIsClass(address key, AccessType class) public view returns(bool) {
        assert(HasAccess(msg.sender, AccessType.PAFF, key));
        return Accessors[key].Class == class;
    }

    function AccessRegistryCount() public view returns(uint256) {
        assert(HasAccess(msg.sender, AccessType.WORKER, address(this)));
        return Registry.Count();
    }

    function GetAccessByIndex(uint256 i) public view returns(Accessor memory) {
        address addr = Registry.GetAddressByIndex(i);
        assert(HasAccess(msg.sender, AccessType.WORKER, addr));
        return Accessors[addr];
    }

    function SetAccess(address key, AccessType Class, address Domain, uint256 Length) internal {
        Registry.Register(key);
        Accessors[key].Address = key;
        Accessors[key].Class = Class;
        Accessors[key].Domain = Domain;
        Accessors[key].Expiration = Length;
    }

    function RemoveAccess(address key) public {
        Accessor memory A = GetAccessByAddress(key);
        assert(HasAccess(msg.sender, AccessType.TOD, A.Domain));
        Registry.Remove(key);
        delete Accessors[key];
    }
}