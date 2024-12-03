// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../dysnomia/01_dysnomia_v2.sol";
import "../addresses.sol";

contract SIGNAL is DYSNOMIA {
   
    constructor(string memory name, string memory symbol, address VMRNG) DYSNOMIA(name, symbol, VMRNG) {
        uint256 originMint = Xiao.Random() % maxSupply / 10;
        _mint(tx.origin, originMint * 10 ** decimals());
    }

    function AddMarketRate(address asset, uint256 rate) public onlyOwners {
        _addMarketRate(asset, rate);
        _mintToCap();
    }

    function Ryu(bytes calldata L) internal returns (uint64 R) {
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