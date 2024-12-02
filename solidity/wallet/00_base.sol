// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../dysnomia/01_dysnomia_v2.sol";
import "../addresses.sol";

contract BASE is DYSNOMIA {
    mapping(string key => bytes[] iter) private _a;
    
    constructor(string memory name, string memory symbol, address VMRNG) DYSNOMIA(name, symbol, VMRNG) {
        uint256 originMint = Xiao.Random() % maxSupply / 10;
        _mint(tx.origin, originMint * 10 ** decimals());
    }

    function AddMarketRate(address asset, uint256 rate) public onlyOwners {
        _addMarketRate(asset, rate);
        _mintToCap();
    }

    function Ryu(string calldata key) public returns (uint64 R) {
        bytes memory L = _a[key][_a[key].length - 1];
        uint8 M;
        uint8[5] memory V = [ 0, 1, 2, 3, 4 ];
        uint8[8] memory K = [ 1, 2, 4, 8, 16, 32, 64, 128 ];

        R = 0;
        for(uint i = 0; i < L.length;) {
            M = 1;
            for(uint j=0; j<5; j++) {
                if(V[j] >= 8) {
                    V[j] -= 8;
                    if(V[j] == 0)
                        if(++i >= L.length) break;
                }
                if(V[j] < 5)
                    M += uint8(L[i] & bytes1(K[V[j]]));
                else 
                    M += uint8(L[i] & bytes1(K[V[j]])) >> 5;
                V[j] += 5;
            }
            if(R == 0) R = M;
            else if(M > 1) R = Xiao.modExp64(R, M, MotzkinPrime);
        }
    }

    function Set(string calldata key, bytes calldata value) public onlyOwners {
        _a[key].push(value);
        _mintToCap();
    }

    function Last(string calldata key) public view returns (bytes memory) {
        return Get(key, _a[key].length - 1);
    }

    function Length(string calldata key) public view returns (uint256) {
        return _a[key].length;
    }

    function Get(string calldata key, uint256 position) public view returns (bytes memory value) {
        return _a[key][position];
    }
}