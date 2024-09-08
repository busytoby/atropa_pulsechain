// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../include/acronym.sol";
import "../include/user.sol";
import "../include/uservote.sol";
import "./10b_voidinterface.sol";

interface NYM {
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
    function Type() external view returns (string memory);
    function has(address _contract, string memory what) external view returns (bool does);
    //function Void() external returns (VOID);
    //function Saat(uint256) external returns (uint64);
    //function On() external returns (Bao memory);
    //function Delegates(address) external returns (User memory);
    function Chat(string memory chatline) external;
    function OperatorSendMSG(string memory chatline) external;
    function ActivePlayers() external view returns(uint256);
    function RoundNumber() external returns (uint64);
    function AcronymCount() external returns (uint16);
    //function Acronyms(uint16) external returns (ACRONYM memory);
    //function LastUserVote(uint64) external returns (UserVote memory);
    function Active() external returns (bool);
    function RoundStartTime() external returns (uint256);
    function Prize() external returns (uint16);
    //function RoundMinutes() external returns (uint8);
    //function MinPlayers() external returns (uint8);
    function SetRoundMinutes(uint8 _m) external;
    function SetMinPlayers(uint8 _m) external;
    function SetRules(string memory _r) external;
    function Rules() external view returns (string memory);
    function Join(address UserToken) external;
    //function GetVotes() external view returns (string[] memory Votable);
    function Vote(uint16 Id) external;
    function Submit(string memory Beta) external;
    function GetAcronym() external view returns(string memory);

}