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
        if(msg.sender == owner()) return true;
        Accessor memory A = GetAccessByAddress(user);
        if(A.Expired()) return false;
        if(A.Class <= min && (A.Domain == address(this) || A.Domain == dom)) return true;
        return false;
    }

    function RegisterAccess(address addr, AccessType class, address dom, uint256 lengthInDays) public {
        assert(HasAccess(msg.sender, AccessType.GUELPH, address(this)));
        SetAccess(addr, class, dom, lengthInDays * 1 days);
    }

    function GetAccessNotes(address addr) public view returns (string[] memory) {
        return Accessors[addr].Notes;
    }

    function AddAccessNote(address addr, string memory note) public {
        assert(Registry.Contains(addr));
        Accessors[addr].Notes.push(note);
    }

    function RemoveAccessNote(address addr, uint256 idx) public {
        assert(Registry.Contains(addr));
        Accessors[addr].Notes[idx] = Accessors[addr].Notes[Accessors[addr].Notes.length - 1];
        Accessors[addr].Notes.pop();
    }

    function GetAccessByAddress(address key) public view returns (Accessor memory) {
        return Accessors[key];
    }

    function AccessExpired(address key) public view returns(bool) {
        return Accessors[key].Expired();
    }

    function AccessIsClass(address key, AccessType class) public view returns(bool) {
        return Accessors[key].Class == class;
    }

    function AccessRegistryCount() public view returns(uint256) {
        return Registry.Count();
    }

    function GetAccessByIndex(uint256 i) public view returns(Accessor memory) {
        address addr = Registry.GetAddressByIndex(i);
        return Accessors[addr];
    }

    function SetAccess(address key, AccessType Class, address Domain, uint256 Length) internal {
        Registry.Register(key);
        Accessors[key].Address = key;
        Accessors[key].Class = Class;
        Accessors[key].Domain = Domain;
        Accessors[key].Expiration = block.timestamp + Length;
    }

    function RemoveAccess(address key) public {
        Accessor memory A = GetAccessByAddress(key);
        assert(HasAccess(msg.sender, AccessType.TOD, A.Domain));
        Registry.Remove(key);
        delete Accessors[key];
    }
}