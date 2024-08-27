// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../include/acronym.sol";
import "../include/user.sol";
import "../include/uservote.sol";
import "./10b_voidinterface.sol";

interface NYM {
    function Type() external returns (string memory);
    function Void() external returns (VOID);
    function Saat(uint256) external returns (uint64);
    function On() external returns (Bao memory);
    function Delegates(address) external returns (User memory);
    function React(User memory Alpha, uint64 Theta) external returns (uint64, uint64);
    function GetUsername(User memory Alpha) external view returns (string memory);
    function Chat(string memory chatline) external;
    function OperatorSendMSG(string memory chatline) external;
    function RoundNumber() external returns (uint64);
    function AcronymCount() external returns (uint16);
    function Acronyms(uint16) external returns (ACRONYM memory);
    function LastUserVote(uint64) external returns (UserVote memory);
    function Active() external returns (bool);
    function RoundStartTime() external returns (uint256);
    function Prize() external returns (uint16);
    function RoundMinutes() external returns (uint8);
    function MinPlayers() external returns (uint8);
    function SetRoundMinutes(uint8 _m) external;
    function SetMinPlayers(uint8 _m) external;
    function Rules() external pure returns (string memory);
    function Join(address UserToken) external;
    function GetVotes() external view returns (string[] memory Votable);
    function Vote(uint16 Id) external;
    function CaseInsensitiveCompare(bytes1 A, bytes1 B) external pure returns (bool);
    function CheckAcronym(bytes memory _acronym, string memory _Beta) external pure returns(bool);
    function Submit(string memory Beta) external;
    function GetAcronym() external view returns(string memory);

}