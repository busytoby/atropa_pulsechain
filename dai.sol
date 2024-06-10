// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "affinity.sol";
import "bao.sol";

abstract contract Dai is Affinity {    
    Bao private Upsilon;
    bytes constant private NULL = bytes(unicode"\u0009\u0009\u0009");

    function Write(uint64 _v) public {
        assert(msg.sender == owner() || Nu.Context == uint256(uint160(msg.sender)));
        bytes memory _data = toBytes(_v);
        Pi(_v);
        Psi[Nu.Context][Nu.Operator].Theta = bytes.concat(Psi[Nu.Context][Nu.Operator].Theta, toBytes(_data.length), _data, NULL);
    }

    function Read() public returns(bytes memory) {
        Chi(Mu.Upsilon);
        return Psi[Nu.Context][Nu.Operator].Theta;
    }

    function Random() public returns(uint64) {
        return Phi();
    }
}
