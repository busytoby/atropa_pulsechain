// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/access/Ownable.sol";
import "registry.sol";

library Expireable {
    function Expired(AccessRegistry.Accessor memory A) public view returns(bool) {
        return (block.timestamp > A.Expiration);
    }
}

abstract contract AccessRegistry is Ownable {
    using LibRegistry for LibRegistry.Registry;
    using atropaMath for address;

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
    mapping(uint256 => Accessor) internal Accessors;

    function _hasAccess(address user, AccessType min, address dom) private view returns (bool) {    
        if(user == owner()) return true;    
        uint256 hash = user.hashWith(dom);
        if(Registry.Contains(hash)) {
            if(Accessors[hash].Expired()) return false;
            if(Accessors[hash].Class <= min && (Accessors[hash].Domain == address(this) || Accessors[hash].Domain == dom)) return true;
        }
        return false;
    }

    function _hasAccess(uint256 hash, AccessType min, address dom) private view returns (bool) {    
        if(Accessors[hash].Expired()) return false;
        if(Accessors[hash].Class <= min && (Accessors[hash].Domain == address(this) || Accessors[hash].Domain == dom)) return true;
        return false;
    }

    function HasAccess(address user, AccessType min, address dom) public view returns (bool) {                
        if(user == owner()) return true;
        assert(_hasAccess(msg.sender, AccessType.PAFF, user));
        uint256 hash = user.hashWith(dom);
        if(Registry.Contains(hash)) {
            assert(_hasAccess(msg.sender, Accessors[hash].Class, user));
            return _hasAccess(hash, min, dom);
        }
        return false;
    }

    function RegisterAccess(address addr, AccessType class, address dom, uint256 lengthInDays) public {
        assert(HasAccess(msg.sender, AccessType.GUELPH, address(this)));
        uint256 Expiration = block.timestamp + lengthInDays * 1 days;
        uint256 hash = addr.hashWith(dom);
        if(Registry.Contains(hash)) {
            assert(HasAccess(msg.sender, Accessors[hash].Class, addr));
            assert(HasAccess(msg.sender, class, dom));
            if(!HasAccess(msg.sender, AccessType.TOD, addr))
                Expiration = Accessors[hash].Expiration;
        }
        SetAccess(addr, class, dom, Expiration);
    }

    function GetAccessNotes(address addr, address dom) public view returns (string[] memory) {
        assert(HasAccess(msg.sender, AccessType.GUELPH, addr));
        assert(HasAccess(msg.sender, AccessType.GUELPH, dom));
        uint256 hash = addr.hashWith(dom);
        return Accessors[hash].Notes;
    }

    function AddAccessNote(address addr, address dom, string memory note) public {
        assert(HasAccess(msg.sender, AccessType.GUELPH, addr));
        assert(HasAccess(msg.sender, AccessType.GUELPH, dom));
        uint256 hash = addr.hashWith(dom);
        assert(Registry.Contains(hash));
        assert(HasAccess(msg.sender, Accessors[hash].Class, addr));
        Accessors[hash].Notes.push(note);
    }

    function RemoveAccessNote(address addr, address dom, uint256 idx) public {
        assert(HasAccess(msg.sender, AccessType.TOD, addr));
        uint256 hash = addr.hashWith(dom);
        assert(Registry.Contains(hash));
        Accessors[hash].Notes[idx] = Accessors[hash].Notes[Accessors[hash].Notes.length - 1];
        Accessors[hash].Notes.pop();
    }

    function GetAccess(address user, address dom) public view returns (Accessor memory) {
        assert(HasAccess(msg.sender, AccessType.PAFF, user));
        uint256 hash = user.hashWith(dom);
        return Accessors[hash];
    }

    function AccessExpired(address user, address dom) public view returns(bool) {
        assert(HasAccess(msg.sender, AccessType.PAFF, user));
        uint256 hash = user.hashWith(dom);
        return Accessors[hash].Expired();
    }

    function AccessIsClass(address user, address dom, AccessType class) public view returns(bool) {
        assert(HasAccess(msg.sender, AccessType.PAFF, user));
        uint256 hash = user.hashWith(dom);
        return Accessors[hash].Class == class;
    }

    function AccessRegistryCount() public view returns(uint256) {
        assert(HasAccess(msg.sender, AccessType.WORKER, address(this)));
        return Registry.Count();
    }

    function GetAccessByIndex(uint256 i) public view returns(Accessor memory) {
        uint256 hash = Registry.GetHashByIndex(i);
        assert(HasAccess(msg.sender, AccessType.WORKER, address(this)));
        return Accessors[hash];
    }

    function SetAccess(address user, AccessType Class, address dom, uint256 Length) internal {
        uint256 hash = user.hashWith(dom);
        Registry.Register(hash);
        Accessors[hash].Address = user;
        Accessors[hash].Class = Class;
        Accessors[hash].Domain = dom;
        Accessors[hash].Expiration = Length;
    }

    function RemoveAccess(address user, address dom) public {
        Accessor memory A = GetAccess(user, dom);
        assert(HasAccess(msg.sender, AccessType.TOD, A.Domain));
        uint256 hash = user.hashWith(dom);
        Registry.Remove(hash);
        delete Accessors[hash];
    }
}