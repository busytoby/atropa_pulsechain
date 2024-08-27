// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./10b_voidinterface.sol";

interface LAU {
    function addOwner(address newOwner) external;
    function renounceOwnership(address toRemove) external;
    function owner(address cOwner) external view returns (bool);
    function mintToCap() external;
    function Type() external returns(string memory);
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