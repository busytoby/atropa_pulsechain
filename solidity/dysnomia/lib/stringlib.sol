// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../01_dysnomia_v2.sol";

contract STRINGLIB is DYSNOMIA {
    string public constant Type = "LibStrings";

    constructor(address mathlib) DYSNOMIA(unicode"DYSNOMIA LibStrings", unicode"LibStrings", mathlib, address(0x0)) {
        _mint(address(this), 111111 * 10 ** decimals());
    }

    function Reverse(string memory S) public pure returns(string memory Reversed) {
        return string(Reverse(bytes(S)));
    }

    function Reverse(bytes memory S) public pure returns(bytes memory Reversed) {
        Reversed = new bytes(S.length);
        uint256 j = S.length - 1;
        uint256 i = 0;

        for (; i < S.length - 1;)      
            Reversed[j--] = S[i++];
        Reversed[0] = S[i]; // uint does not like decrement to 0
    }

    error MinimumLength2();
    function RandomAcronym(uint8 MaxLength) public returns(bytes memory Acronym) {
        if(MaxLength < 3) revert MinimumLength2();
        bytes memory LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        uint64 length = 2 + (Xiao.Random() % (MaxLength - 2));
        Acronym = new bytes(length);
        for(uint i = 0; i < length; i++) {
            uint64 nxtchar = Xiao.Random() % 26;
            Acronym[i] = LETTERS[nxtchar];
        }
    }

    function CaseInsensitiveCompare(bytes1 A, bytes1 B) public pure returns (bool) {
        return(A == B || uint8(A) == (uint8(B) + 32));
    }

    function CheckAcronym(string memory _A, string memory _B) public pure returns(bool) {
        return CheckAcronym(bytes(_A), _B);
    }

    error CheckAcronymError(uint which, uint8 A, uint8 B);
    function CheckAcronym(bytes memory _acronym, string memory _Beta) public pure returns(bool) {
        bytes memory Beta = bytes(_Beta);
        if(!CaseInsensitiveCompare(Beta[0], _acronym[0])) revert CheckAcronymError(1, uint8(Beta[0]), uint8(_acronym[0]));
        uint8 _pos = 1;
        for(uint256 i = 1; i < Beta.length; i++) {
            if(uint8(Beta[i]) == 32)
                if(!CaseInsensitiveCompare(Beta[i+1], _acronym[_pos])) revert CheckAcronymError(2, uint8(Beta[i+1]), uint8(_acronym[_pos]));
                else {
                    _pos = _pos + 1;
                    i = i + 1;
                }
            if(_pos > _acronym.length) revert CheckAcronymError(3, _pos, uint8(_acronym.length));
        }
        if(_pos != _acronym.length) revert CheckAcronymError(4, _pos, uint8(_acronym.length));
        return true;
    }

    function log10(uint256 value) public pure returns (uint256) {
        uint256 result = 0;
        unchecked {
            for(uint8 _e = 64; _e >= 1; _e /= 2) {
                if (value >= 10 ** _e) {
                    value /= 10 ** _e;
                    result += _e;
                }
            }
        }
        return result;
    }

    function String(uint256 value) public pure returns (string memory buffer) {
        unchecked {
            uint256 length = log10(value) + 1;
            buffer = new string(length);
            uint256 ptr;
            assembly { ptr := add(buffer, add(32, length)) }
            while (true) {
                ptr--;
                assembly { mstore8(ptr, byte(mod(value, 10), "0123456789abcdef")) }
                value /= 10;
                if (value == 0) break;
            }
        }
    }

    function Hex(address account) public pure returns(string memory) {
        return Hex(abi.encodePacked(account));
    }

    function Hex(uint256 value) public pure returns(string memory) {
        return Hex(abi.encodePacked(value));
    }

    function Hex(bytes32 value) public pure returns(string memory) {
        return Hex(abi.encodePacked(value));
    }

    function Hex(bytes memory data) public pure returns(string memory) {
        bytes memory hexa = "0123456789abcdef";

        bytes memory str = new bytes(2 + data.length * 2);
        str[0] = "0";
        str[1] = "x";
        for (uint i = 0; i < data.length; i++) {
            str[2+i*2] = hexa[uint(uint8(data[i] >> 4))];
            str[3+i*2] = hexa[uint(uint8(data[i] & 0x0f))];
        }
        return string(str);
    }
}