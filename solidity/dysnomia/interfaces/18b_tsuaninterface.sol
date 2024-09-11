// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./17b_zaiinterface.sol";

interface TSUAN {
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
    function Type() external view returns (string memory);
    function Zao() external view returns (ZAOINTERFACE);
    function VAI() external view returns (LIBCONJURE);
    function acceptLibraryUpdate(string memory lib) external;
    function KnownEnchantmentCount(address QiAddress) external view returns (uint256);
    function GetEnchantment(address QiAddress, string memory Noun) external view returns (uint64[3] memory);
    function Train(address QiAddress, string memory IntendedNoun) external;
}