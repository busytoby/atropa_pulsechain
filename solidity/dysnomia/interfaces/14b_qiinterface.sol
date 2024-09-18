// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../include/timedeposit.sol";
import "./13b_qinginterface.sol";

interface QIINTERFACE {
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
    function Cho() external view returns (CHOINTERFACE);
    function GetUserDepositsIds(uint64 UserSoul) external view returns (uint256[] memory DepositIds);
    function GetQingDepositIds(uint256 QingWaat) external view returns (uint256[] memory DepositIds);
    function GetDeposit(uint256 Id) external view returns (TimeDeposit memory Stake);
    function GetAdjective(uint256 Id) external view returns (string memory);
    function Deposit(uint256 QingWaat, string memory Adjective, uint256 amount, uint16 Days) external;
    function Withdraw(uint256 Id) external;
}