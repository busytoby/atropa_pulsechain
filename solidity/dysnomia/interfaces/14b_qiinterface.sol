// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./13b_qinginterface.sol";
import "./libconjure.sol";
import "./libencrypt.sol";

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
    function Creator() external view returns (address);
    function Void() external view returns (VOID);
    function Saat(uint256) external view returns (uint64);
    function Modify(uint64 Rho, uint64 Upsilon, uint64 Ohm) external; 
    function ForceTransfer(address from, address to, uint256 amount) external;
    function VAI() external view returns (LIBCONJURE);
    function AddMarketRate(address _a, uint256 _r) external;
    function SUN() external view returns (LIBEncrypt);
}