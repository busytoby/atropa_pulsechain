// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./10b_voidinterface.sol";

interface LIBYAI {
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
    function has(address _contract, string memory what) external view returns (bool does);
    function Void() external view returns(VOID);
    function Forbid(address what) external;
    function Unforbid(address what) external;
    function IsForbidden(address Token) external view returns (bool);
    function GetAliasCount(uint256 Waat) external view returns (uint256);
    function GetAlias(uint256 Waat, uint256 idx) external view returns (string memory);
    function Remove(uint256 Waat, string memory name) external;
    function Alias(uint256 Waat, string memory name, address token) external;
    function Alias(uint256 Waat, string memory name) external view returns (address _a);
}