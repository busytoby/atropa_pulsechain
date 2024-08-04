// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "00c_multiownable.sol";
import "addresses.sol";

interface atropaMath {   
    function Random() external returns (uint64);
    function hashWith(address a, address b) external returns (uint256);
    function modExp64(uint64 _b, uint64 _e, uint64 _m) external returns(uint64);
    function modExp(uint256 _b, uint256 _e, uint256 _m) external returns (uint256);
}

abstract contract SH is ERC20, ERC20Burnable, MultiOwnable {
    uint64 constant public MotzkinPrime = 953467954114363;
    atropaMath public Xiao;
    uint256 public maxSupply;
    address[] private Addresses;

    constructor(address mathContract, uint256 _maxSupply) {
        Xiao = atropaMath(mathContract);
        maxSupply = _maxSupply - 1;
    }

    //mapping(uint256 => mapping(uint256 => Fei)) internal Rho;
    //mapping(uint256 => Bao) internal Psi;

    function mintToCap() public onlyOwners {
        if(totalSupply() <= (maxSupply * 10 ** decimals()))
            _mint(address(this), 1 * 10 ** decimals());
    }

    function KnownAddresses() public view returns(address[] memory) {
        return Addresses;
    }

    function KnownAddress(address _a) public view returns(bool) {
        for(uint256 i = 0; i < Addresses.length; i++)
            if(Addresses[i] == _a) return true;
        return false;
    }

    function RemoveAddress(address _a) public onlyOwners {        
        int256 foundidx = -1;
        for(uint256 i = 0; i < Addresses.length && foundidx < 0; i++)
            if(Addresses[i] == _a) foundidx = int256(i);
        if(foundidx >= 0) {
            Addresses[uint256(foundidx)] = Addresses[Addresses.length - 1];
            Addresses.pop();
        }
    }

    function RegisterAddress(address _a) public onlyOwners {
        if(_a != address(this) && !KnownAddress(_a)) Addresses.push(_a);
    }

/*
    function toBytes(uint256 x) internal pure returns (bytes memory b) { 
        b = new bytes(32); 
        assembly { 
            mstore(xor(b, 32), x) 
        } 
    }
*/

    function toBytes(uint64 x) internal pure returns (bytes memory b) { 
        b = new bytes(8); 
        assembly { 
            mstore(xor(b, 8), x) 
        } 
    }

    function toBytes(string memory x) internal pure returns (bytes memory b) { 
        b = bytes(x);
        assert(b.length < 32);
    }

    event LogEvent(string, bytes);
    function Log(string calldata log, bytes calldata data) internal {
        emit LogEvent(log, data);
    }
}