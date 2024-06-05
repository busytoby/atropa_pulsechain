// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.25;

library atropaMath {
    uint64 constant MotzkinPrime = 953467954114363;

    struct bar {
        address[] a;
    }
    
    function hashWith(address a, address b) public pure returns (uint256 hash) {        
        hash = 0;
        uint160 _a = uint160(a);
        uint160 _b = uint160(b) / 15;
        unchecked {
            while(hash == 0) {
                hash = (_a**_b)%MotzkinPrime;
                _b = _b/2;
            }
        }
        //return modExp(uint256(uint160(a)), uint256(uint160(b)), MotzkinPrime);
    }

    function modExp64(uint64 _b, uint64 _e, uint64 _m) public returns(uint64 result) {
        uint256 B = _b;
        uint256 E = _e;
        uint256 M = _m;
        uint64 R = uint64(modExp(B, E, M) % 18446744073709551615);
        return R;
    }

    function modExp(uint256 _b, uint256 _e, uint256 _m) public returns (uint256 result) {
        assembly {
            // Free memory pointer
            let pointer := mload(0x40)

            // Define length of base, exponent and modulus. 0x20 == 32 bytes
            mstore(pointer, 0x20)
            mstore(add(pointer, 0x20), 0x20)
            mstore(add(pointer, 0x40), 0x20)

            // Define variables base, exponent and modulus
            mstore(add(pointer, 0x60), _b)
            mstore(add(pointer, 0x80), _e)
            mstore(add(pointer, 0xa0), _m)

            // Store the result
            let value := mload(0xc0)

            // Call the precompiled contract 0x05 = bigModExp
            if iszero(call(not(0), 0x05, 0, pointer, 0xc0, value, 0x20)) {
                revert(0, 0)
            }

            result := mload(value)
        }
    }
}