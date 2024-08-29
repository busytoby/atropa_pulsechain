// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./09b_siuinterface.sol";

interface VOID {
    function Rename(string memory newName, string memory newSymbol) external;
    function GetMarketRate(address _a) external view returns(uint256);
    function Purchase(address _t, uint256 _a) external;
    function Redeem(address _t, uint256 _a) external;
    function name() external view returns (string memory);
    function symbol() external view returns (string memory);
    function decimals() external view returns (uint8);
    function totalSupply() external view returns (uint256);
    function balanceOf(address account) external view returns (uint256);
    function transfer(address to, uint256 value) external returns (bool);
    function allowance(address owner, address spender) external view returns (uint256);
    function approve(address spender, uint256 value) external returns (bool);
    function transferFrom(address from, address to, uint256 value) external returns (bool);
    function addOwner(address newOwner) external;
    function renounceOwnership(address toRemove) external;
    function owner(address cOwner) external view returns (bool);
    function mintToCap() external;
    function Type() external view returns(string memory);
    function Nu() external returns(SIU);
    function GetLibraryAddress(string memory name) external view returns (address);
    function AddLibrary(string memory name, address _a) external;
    function Clear(uint64 Iota) external returns(uint64);
    function Sign(uint64 DIRAC1) external returns(Bao memory);
    function Log(string memory LogLine) external;
    function Log(uint64 Sigma, string memory LogLine) external;
    function Log(address Sigma, string memory LogLine) external;
    function Chat(string memory chatline) external;
    function SetAttribute(string memory name, string memory value) external;
    function GetAttribute(string memory name) external view returns (string memory);
    function Alias(address name, string memory value) external;
    function Alias(address name) external view returns (string memory);
    function Alias(Bao memory Theta, string memory value) external;
    function Alias(Bao memory Theta) external view returns (string memory);
    function Enter() external returns(uint64[3] memory Saat, Bao memory On);
    function Enter(string memory name, string memory symbol) external returns(uint64[3] memory Saat, Bao memory On);
}