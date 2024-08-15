// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./03_shio.sol";

contract SHIOFactory {
    constructor() {} 

    function New(address Rod, address Cone, address MathLib) public returns(SHIO) {
        return new SHIO(Rod, Cone, MathLib);
    }
}