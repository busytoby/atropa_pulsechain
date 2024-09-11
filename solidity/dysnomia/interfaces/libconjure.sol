// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./13b_qinginterface.sol";

interface LIBCONJURE {
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
    function VoidQing() external view returns(QING);
    function Levels(uint256) external view returns(uint64);
    function Level(uint64 Saat) external view returns (uint8);
    function Enchant(address QiAddress, uint64 Rho, uint64 Upsilon, uint64 Ohm, string memory IntendedNoun) external returns (uint64);
    function Imbue(address QiAddress, uint64 Rho, uint64 Upsilon, uint64 Ohm, string memory IntendedAdjective) external returns (uint64);
    function RenameQi(string memory Adverb, string memory Noun, string memory Adjective) external;
    function qName() external view returns (string memory);
    function qSymbol() external view returns (string memory);
}