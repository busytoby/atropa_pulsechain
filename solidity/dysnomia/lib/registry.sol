// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./multiownable.sol";

contract Registry is MultiOwnable {
    mapping(bytes => bytes) private _values;
    mapping(bytes => bool) private _inserted;
    mapping(bytes => uint256) private _indexOf;
    bytes[] private _keys;

    constructor() MultiOwnable(msg.sender) {
    }

    function Get(bytes memory key) public view returns (bytes memory) {
        return _values[key];
    }

    function Get(uint256 idx) public view returns (bytes memory) {
        return _values[_keys[idx]];
    }

    function Inserted(bytes memory what) public view returns (bool) {
        return _inserted[what];
    }

    function Inserted(string memory what) public view returns (bool) {
        return _inserted[bytes(what)];
    }

    function Uint64ToBytes(uint64 what) public pure returns (bytes memory _w) {
        _w = new bytes(8);
        for(uint i = 0; i < 8; i++)
            _w[i] = bytes1(uint8((what >> (8*i)) & 255));
    }

    function Inserted(uint64 what) public view returns (bool) {
        return _inserted[Uint64ToBytes(what)];
    }

    function Uint256ToBytes(uint256 what) public pure returns (bytes memory _w) {
        _w = new bytes(32);
        for(uint i = 0; i < 32; i++)
            _w[i] = bytes1(uint8((what >> (8*i)) & 255));
    }

    function Inserted(uint256 what) public view returns (bool) {
        return _inserted[Uint256ToBytes(what)];
    }

    function AddressToBytes(address what) public pure returns (bytes memory _w) {
        _w = new bytes(20);
        bytes20 _w2 = bytes20(what);
        for(uint i = 0; i < 20; i++)
            _w[i] = _w2[i];
    }

    function Inserted(address what) public view returns (bool) {
        return _inserted[AddressToBytes(what)];
    }

    function Count() public view returns(uint256) {
        return uint256(_keys.length);
    }

    function GetHashByIndex(uint256 index) public view returns(bytes memory) {
        return _keys[index];
    }

    function Register(bytes memory key, bytes memory value) public onlyOwners {
        if(!_inserted[key])
        {
            _inserted[key] = true;
            _indexOf[key] = _keys.length;
            _values[key] = value;
            _keys.push(key);
        }
    }

    function Register(string memory _key, bytes memory value) public onlyOwners {
        Register(bytes(_key), value);
    }

    function Register(string memory _key, string memory value) public onlyOwners {
        Register(bytes(_key), bytes(value));
    }

    function Register(string memory _key, uint64 value) public onlyOwners {
        Register(bytes(_key), Uint64ToBytes(value));
    }

    function Register(string memory _key, uint256 value) public onlyOwners {
        Register(bytes(_key), Uint256ToBytes(value));
    }

    function Register(string memory _key, address value) public onlyOwners {
        Register(bytes(_key), AddressToBytes(value));
    }

    function Register(uint64 _key, bytes memory value) public onlyOwners {
        Register(Uint64ToBytes(_key), value);
    }

    function Register(uint64 _key, string memory value) public onlyOwners {
        Register(Uint64ToBytes(_key), bytes(value));
    }

    function Register(uint64 _key, uint64 value) public onlyOwners {
        Register(Uint64ToBytes(_key), Uint64ToBytes(value));
    }

    function Register(uint64 _key, uint256 value) public onlyOwners {
        Register(Uint64ToBytes(_key), Uint256ToBytes(value));
    }

    function Register(uint64 _key, address value) public onlyOwners {
        Register(Uint64ToBytes(_key), AddressToBytes(value));
    }

    function Register(uint256 _key, bytes memory value) public onlyOwners {
        Register(Uint256ToBytes(_key), value);
    }

    function Register(uint256 _key, string memory value) public onlyOwners {
        Register(Uint256ToBytes(_key), bytes(value));
    }

    function Register(uint256 _key, uint64 value) public onlyOwners {
        Register(Uint256ToBytes(_key), Uint64ToBytes(value));
    }

    function Register(uint256 _key, uint256 value) public onlyOwners {
        Register(Uint256ToBytes(_key), Uint256ToBytes(value));
    }

    function Register(uint256 _key, address value) public onlyOwners {
        Register(Uint256ToBytes(_key), AddressToBytes(value));
    }

    function Register(address _key, bytes memory value) public onlyOwners {
        Register(AddressToBytes(_key), value);
    }

    function Register(address _key, string memory value) public onlyOwners {
        Register(AddressToBytes(_key), bytes(value));
    }

    function Register(address _key, uint64 value) public onlyOwners {
        Register(AddressToBytes(_key), Uint64ToBytes(value));
    }

    function Register(address _key, uint256 value) public onlyOwners {
        Register(AddressToBytes(_key), Uint256ToBytes(value));
    }

    function Register(address _key, address value) public onlyOwners {
        Register(AddressToBytes(_key), AddressToBytes(value));
    }

    function Remove(bytes memory key) public onlyOwners {
        if(!_inserted[key]) return;
        delete _inserted[key];
        uint256 index = _indexOf[key];
        bytes memory lastKey = _keys[_keys.length - 1];
        _indexOf[lastKey] = index;
        delete _indexOf[key];
        delete _values[key];
        _keys[index] = lastKey;
        _keys.pop();
    }

    function Remove(string memory _key) public onlyOwners {
        Remove(bytes(_key));
    }

    function Remove(uint64 _key) public onlyOwners {
        Remove(Uint64ToBytes(_key));
    }

    function Remove(uint256 _key) public onlyOwners {
        Remove(Uint256ToBytes(_key));
    }

    function Remove(address _key) public onlyOwners {
        Remove(AddressToBytes(_key));
    }
}