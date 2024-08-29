// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./10b_voidinterface.sol";

interface LAU {
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
    function Eta() external returns(VOID);
    function Saat(uint256) external returns(uint64);
    function On() external returns(Bao memory);
    function Withdraw(address what, uint256 amount) external;
    function Username() external view returns (string memory);
    function Username(string memory newUsername) external;
    function Chat(string memory chatline) external;
    function Alias(address name, string memory value) external;
    function Alias(address name) external view returns (string memory);
    function Alias(Bao memory Theta, string memory value) external;
    function Alias(Bao memory Theta) external view returns (string memory);
}