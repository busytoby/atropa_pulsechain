// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../interfaces/17b_xieinterface.sol";

interface YUEINTERFACE {
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
    function Origin() external view returns (address);
    function Maat() external view returns (uint256);
    function Hypobar(string memory Adjective) external view returns (uint256 Gram);
    function Epibar(string memory Adjective) external view returns (uint256 Gram);
    function ChangeOrigin(address NewOrigin) external;
    function Withdraw(address what, uint256 amount) external;
    function MintToOrigin() external;
    function ForceTransfer(address From, address To, uint256 Amount) external;
    function Bar(uint256 n) external view returns (uint256 Hypogram, uint256 Epigram, uint256 BarLength);
    function React(string calldata Adjective) external returns (uint64 Jong);
}