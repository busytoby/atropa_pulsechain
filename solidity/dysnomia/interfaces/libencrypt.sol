// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./10b_voidinterface.sol";

interface LIBEncrypt {
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
    function Type() external view returns (string memory);
    function Zheng() external returns (ZHENG);
    function Void() external returns (VOID);
    function Encrypt(uint64 From, uint64 to, string memory Key, string memory Data) external returns (uint64 index);
    function Encapsulate(Bao memory From, uint64 Gamma, uint64 Rho, uint64 Upsilon, uint64 Ohm) external returns (uint64 Entropy, bytes memory Geng);
    function Encrypt(uint64 From, uint64 to, bytes memory Key, bytes memory Data) external returns (uint64 index);
    function Decrypt(uint64 From, uint64 to, uint64 Index, string memory Key) external returns (bytes memory);
    function Prune() external;
    function Decrypt(uint64 From, uint64 to, uint64 Index, bytes memory Key) external returns (bytes memory result);        
    function hashKey(bytes memory Key) external returns (uint64 result);
}