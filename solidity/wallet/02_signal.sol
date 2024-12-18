// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../dysnomia/01_dysnomia_v2.sol";
import "../addresses.sol";

interface BASE {
    function Last(string calldata key) external view returns (bytes memory);
}

contract SIGNAL is DYSNOMIA {
    BASE public Base;
   
    constructor(address BaseAddress) DYSNOMIA(unicode"Dysnomia Signal", unicode"SIGNAL", address(DYSNOMIA(BaseAddress).Xiao())) {
        Base = BASE(BaseAddress);
        uint256 originMint = Xiao.Random() % maxSupply / 10;
        _mint(tx.origin, originMint * 10 ** decimals());
    }

    function AddMarketRate(address asset, uint256 rate) public onlyOwners {
        _addMarketRate(asset, rate);
        _mintToCap();
    }

    error Weak(uint64 _b, uint64 _e);
    function Signal(string calldata S, address SecretAddress) public returns (uint64 R) {
        uint64 _b = Ryu(Base.Last(S));
        uint64 _e = Ryu(BASE(SecretAddress).Last(S));
        if(_b < 1551 || _e < 1551) revert Weak(_b, _e);
        return Xiao.modExp64(_b, _e, MotzkinPrime);
    }

    function Ryu(bytes memory L) internal returns (uint64 R) {
        uint8 M;
        uint8[5] memory V = [ 0, 1, 2, 3, 4 ];
        uint8[8] memory K = [ 1, 2, 4, 8, 16, 32, 64, 128 ];

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

}