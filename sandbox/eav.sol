// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.25;

library EAV {
    enum EAVDataType {
        BOOL,
        INT,
        ADDRESS,
        BYTES
    }

    struct Data {
        string Entity;
        EAVDataType Attribute;
        bool valueBool;
        uint256 valueInt;
        address valueAddress;
        bytes valueBytes;
    }

    function Set(Data memory E, bool V) internal pure {
        assert(E.Attribute == EAVDataType.BOOL);
        E.valueBool = V;
    }

    function Set(Data memory E, uint256 V) internal pure {
        assert(E.Attribute == EAVDataType.INT);
        E.valueInt = V;
    }

    function Set(Data memory E, address V) internal pure {
        assert(E.Attribute == EAVDataType.ADDRESS);
        E.valueAddress = V;
    }

    function Set(Data memory E, bytes memory V) internal pure {
        assert(E.Attribute == EAVDataType.BYTES);
        E.valueBytes = V;
    }

    function New(bool V) internal pure returns (Data memory) {
        Data memory E;
        E.Attribute = EAVDataType.BOOL;
        E.valueBool = V;
        return E;
    }

    function New(uint256 V) internal pure returns (Data memory) {
        Data memory E;
        E.Attribute = EAVDataType.INT;
        E.valueInt = V;
        return E;
    }

    function New(address V) internal pure returns (Data memory) {
        Data memory E;
        E.Attribute = EAVDataType.ADDRESS;
        E.valueAddress = V;
        return E;
    }

    function New(bytes memory V) internal pure returns (Data memory) {
        Data memory E;
        E.Attribute = EAVDataType.BYTES;
        E.valueBytes = V;
        return E;
    }
}