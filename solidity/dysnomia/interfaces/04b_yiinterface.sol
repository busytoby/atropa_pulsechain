// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../include/bao.sol";
import "./02d_shafactoryinterface.sol";
import "./03b_shiointerface.sol";
import "./03d_shiofactoryinterface.sol";

interface YI {
    function addOwner(address newOwner) external;
    function renounceOwnership(address toRemove) external;
    function owner(address cOwner) external view returns (bool);
    function mintToCap() external;
    function Type() external returns(string memory);
    function Psi() external returns(SHIO);
    function Xi() external returns(uint64);        
    function Ring() external returns(uint64);
    function Beta(string calldata Name, string calldata Symbol) external returns(SHA);
    function Kappa(SHA Rod, SHA Cone) external returns(SHIO);
    function Bing(Bao memory _b) external;
    function Bang(address _a) external view returns(Bao memory);
    function React(Bao memory Gamma, uint64 Pi) external returns(Bao memory);
    function SHAFactoryInterface() external returns(SHAFactory);
    function SHIOFactoryInterface() external returns(SHIOFactory);
 }