// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../include/bao.sol";
import "./02d_shafactoryinterface.sol";
import "./03b_shiointerface.sol";
import "./03d_shiofactoryinterface.sol";

interface YI {
    function addOwner(address newOwner) external;
    function renounceOwnership(address toRemove) external;
    function mintToCap() external;
    function Type() external returns(string memory);
    function Psi() external returns(SHIO);
    function Xi() external returns(uint64);        
    function Ring() external returns(uint64);
    function Alpha() external returns(uint64);
    function Beta(string calldata Name, string calldata Symbol) external returns(SHA);
    function Bing(Bao memory _b) external;
    function Bang(address _a) external view returns(Bao memory);
    function Charge(Bao memory Gamma, uint64 Rho) external returns(uint64);
    function Induce(Bao memory Gamma, uint64 Epsilon) external returns(uint64);
    function Torque(Bao memory Gamma, uint64 Delta) external returns(uint64);
    function Amplify(Bao memory Gamma, uint64 Upsilon) external returns(uint64);
    function Sustain(Bao memory Gamma, uint64 Ohm) external returns(uint64);
    function React(Bao memory Gamma, uint64 Pi) external returns(Bao memory);
    function SHAFactoryInterface() external returns(SHAFactory);
    function SHIOFactoryInterface() external returns(SHIOFactory);
 }