// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../tang/interfaces/02b_cheoninterface.sol";
import "../assets/interfaces/vitusinterface.sol";

interface WORLDINTERFACE {
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
    function Cheon() external view returns (CHEON);
    function Vitus() external view returns (VITUSINTERFACE);
    function Whitelist(address Caude, address Distributive, bool Allow) external;
    function Bun(int256 Latitude, int256 Longitude, address Caude) external view returns (uint256);
    function Buzz(int256 Latitude, address Coder, address Caude) external view returns (uint256);
    function Distribute(address Caude, address Distributive, uint256 Amount) external returns (uint256 Remaining);
    function Code(int256 Latitude, int256 Longitude, address Cause) external;
}