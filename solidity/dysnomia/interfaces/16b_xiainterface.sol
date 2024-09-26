// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./15b_maiinterface.sol";

interface XIA {
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
    function Mai() external view returns (MAI);
    function e() external view returns (uint256);
    function Balance() external view returns (uint256);
    function Charge(uint256 QingWaat, string memory adjective) external returns (uint256);
    function Decay(uint256 Power) external view returns (uint256);
    function Lambda() external view returns (uint256);
    function GetUserDepositCount(uint64 UserSoul) external view returns (uint256);
    function GetUserDepositByIndex(uint64 UserSoul, uint256 Index) external view returns (TimeDeposit memory Stake);
    function GetDepositCount() external view returns (uint256);
    function GetDeposit(uint256 Id) external view returns (TimeDeposit memory);
    function GetQingAdjectiveDeposit(uint256 QingWaat, string memory Adjective) external view returns (TimeDeposit memory);
    function Deposit(address Qing, string memory Adjective, uint256 amount) external;
    function Withdraw(uint256 Id, uint256 Amount) external;
}