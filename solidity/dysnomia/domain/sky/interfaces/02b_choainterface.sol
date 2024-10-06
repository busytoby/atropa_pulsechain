// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01b_chaninterface.sol";
import "../../tang/interfaces/01b_seiinterface.sol";

interface CHOA {
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
    function Sei() external view returns (SEI);
    function Play(address UserTokenAddress) external returns (YUEINTERFACE Chi, LAU UserToken, QIN memory Player);
    function Chat(string memory MSG) external returns (uint256 Charge, uint256 UserQi, uint64 Omicron, uint64 Omega);
    function GetExits() external view returns (string[] memory Exits);
    function Move(string memory To) external;
}