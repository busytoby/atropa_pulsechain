// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../../interfaces/17b_xieinterface.sol";
import "../../interfaces/yueinterface.sol";

interface CHAN {
    function maxSupply() external view returns(uint256);
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
    function Xie() external view returns (XIE);
    function Yan(address Origin) external view returns (address Yue);
    function AddYue(address Origin, address Yue) external;
    function TransferYue(address Yue, address NewOrigin) external;
    function ReactYue(YUEINTERFACE Yue, address Qing) external returns(uint256 Charge);
    function YueWithdraw(YUEINTERFACE Yue, address Asset, uint256 Amount) external;
    function YueMintToOrigin(YUEINTERFACE Yue) external;
    function YueForceTransfer(YUEINTERFACE Yue, address From, address To, uint256 Amount) external;
}