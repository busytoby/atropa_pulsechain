// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./04b_yiinterface.sol";

interface ZHENG {
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
    function Eta() external returns(YI);
    function GetRodByIdx(uint64 _theta) external returns(Bao memory);
    function InstallRod(uint64 Theta, Bao memory Beta, uint64 DIRAC) external returns(Bao memory);
    function InstallCone(uint64 Theta, Bao memory Beta, uint64 DIRAC) external returns(Bao memory);
    function Mau(string memory name, string memory symbol, uint64 Xi, uint64 Rho, uint64 Upsilon) external returns (Bao memory On);
    function Iodize(SHIO Shio) external;
}