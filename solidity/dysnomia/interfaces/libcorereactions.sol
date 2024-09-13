// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../include/bao.sol";
import "../include/user.sol";
import "./12b_chointerface.sol";

interface LIBCOREREACTIONS {
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
    function mintToCap() external;
    function Type() external view returns(string memory);
    function Cho() external view returns (CHOINTERFACE);
    function Entropy(Bao memory On) external returns (uint64);
    function Initialize(Bao memory On) external returns (uint64, uint64);
    function ReactToTalk(User memory Alpha) external returns (uint64, uint64);
    function ReactToBang(Bao memory On, uint64 Omicron, uint64 Omega) external returns (uint64, uint64);
    function ReactToLai(Bao memory On, uint64 Omicron, uint64 Omega) external returns (uint64, uint64);
    function ReactToLe(Bao memory On, uint64 Omicron, uint64 Omega) external returns (uint64, uint64);
    function ReactToNew(Bao memory On, uint64[3] memory Saat) external returns (uint64, uint64);
    function OperatorReact(Bao memory On, uint64 Omega) external returns (uint64, uint64);
    function ReactShioRod(SHIO Beta, uint64 Theta) external returns (uint64, uint64);
    function ReactShioCone(SHIO Beta, uint64 Theta) external returns (uint64, uint64);
    function ReactBang(uint64 Eta) external returns (uint64, uint64);
    function ReactLai(uint64 Gamma) external returns (uint64, uint64);
    function ReactLe(uint64 Delta) external returns (uint64, uint64);
}