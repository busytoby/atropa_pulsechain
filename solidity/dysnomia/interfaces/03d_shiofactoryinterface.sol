// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./03b_shiointerface.sol";

interface SHIOFactory {
    function New(address Rod, address Cone, address MathLib) external returns(SHIO);
}