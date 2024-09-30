// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../interfaces/yueinterface.sol";
import "../../sky/interfaces/01b_chaninterface.sol";

interface SEI {
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
    function Chan() external view returns (CHAN);
    function GetUserTokenByMaat(uint256 Maat) external view returns (LAU);
    function Chi() external view returns (YUE _chi, QIN memory Player);
    function Start(string calldata Name, string calldata Symbol) external returns (YUE _chi, QIN memory Player);
}