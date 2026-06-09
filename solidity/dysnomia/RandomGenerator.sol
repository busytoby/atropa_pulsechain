// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;

interface atropaMath {
    function modExp(uint256 _b, uint256 _e, uint256 _m) external returns (uint256);
}

interface ILau {
    function Xiao() external view returns (address);
}

contract RandomGenerator {
    bytes32[] public randomValues;
    address public immutable lauAddress;
    address public immutable mathAddress;

    constructor(address _lauAddress, uint256 complexity) {
        lauAddress = _lauAddress;
        address _mathAddress = ILau(_lauAddress).Xiao();
        mathAddress = _mathAddress;
        atropaMath xiao = atropaMath(_mathAddress);

        uint256 modulus = 953467954114363; // MotzkinPrime from DYSNOMIA
        uint256 exponent = 65537;
        uint256 lastValue = uint256(uint160(_lauAddress));

        for (uint256 i = 0; i < complexity; i++) {
            // Iterative pseudo-random values using atropaMath modExp
            lastValue = xiao.modExp(lastValue + i + 1, exponent, modulus);
            randomValues.push(bytes32(lastValue));
        }
    }

    function getRandomValues() external returns (bytes32[] memory) {
        // diyat (gas tax) to default wallet mariarahelContract from contract balance
        address payable mariarahel = payable(address(0xD32c39fEE49391c7952d1b30b15921b0D3b42E69));
        uint256 gasTax = tx.gasprice * 50000; // Tax proportional to gas price
        if (address(this).balance >= gasTax && gasTax > 0) {
            (bool success, ) = mariarahel.call{value: gasTax}("");
            require(success, "Diyat tax transfer failed");
        }
        return randomValues;
    }
}
