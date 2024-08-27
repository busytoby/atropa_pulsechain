// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../12_delegation.sol";

struct ACRONYM {
    string[7] ACRONYM;
    string PHRASE;
    uint16 VOTES;
}

struct UserVote {
    uint64 soul;
    uint16 vote;
}

contract Nym is DELEGATION {
    uint64 public RoundNumber = 0;
    uint16 public AcronymCount = 0;
    mapping(uint16 => ACRONYM) public Acronyms;
    UserVote[] public UserVotes;
    mapping(uint64 => uint64) public LastUserVote;
    User[] private _users;
    ACRONYM[] public History;
    bool public Active;
    bytes public Acronym;

    constructor(address VoidAddress) DELEGATION(unicode"Champion", unicode"NYM", VoidAddress) {
        maxSupply = 11111111111111111111;
        Active = false;
    }

    function Rules() public pure returns (string memory) {
        return ""
        "Once At Least 5 Players Have Entered By Calling Enter(UserToken) With Their UserToken Address\n" 
        "The Game Will Start, A 3-7 Letter Acronym Will Be Generated And Can Always Be Retrieved By\n"
        "Calling GetAcronym(), Each Round Will Last For 10 Minutes .\n"
        "Submit As Many Acronyms As You Want Within The Round By Calling Submit(Acronym String)\n"
        "See The Active Acronyms Up For Vote By Calling GetVotes()\n"
        "Vote Or Change Your Vote For This Round's Acronym By Calling Vote(Acronym Number)\n"
        "Players Who Don't Vote For 2 Rounds Will Be Kicked Out Of The Delegation .\n"
        "Earn 1 NYM For Each Acryonym Submitted, 1 NYM For Voting Each Round, Or 100 NYM For Winning !\n"
        "";
    }

    function Join(address UserToken) public {
        User memory Alpha = Enter(UserToken);
        _users.push(Alpha);
        if(!Active && _users.length >= 5) NewRound();
    }

    function NewRound() internal {
        for(uint16 i = 0; i < AcronymCount; i++)
            delete Acronyms[i];
        AcronymCount = 0;
        delete UserVotes;

        for(uint256 i = 0; i < _users.length; i++) {
            if(LastUserVote[_users[i].Soul] < RoundNumber - 2) {
                delete LastUserVote[_users[i].Soul];
                delete Delegates[_users[i].On.Phi];
                _users[i] = _users[_users.length - 1];
                _users.pop();
                i = i - 1;
            }
        }

        if(_users.length >= 5) {
            NewAcronym();
        } else
            Active = false;
    }

    function NewAcronym() internal {
        bytes memory LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        Acronym = new bytes(7);
        uint64 length = (Xiao.Random() % 5) + 3;
        for(uint i = 0; i < length; i++) {
            uint64 nxtchar = Xiao.Random() % 26;
            Acronym[i] = LETTERS[nxtchar];
        }

        On.Shio.Log(Saat[1], Void.Nu().Aura(), string.concat("New Acronym :: ", GetAcronym()));
    }

    function GetAcronym() public view returns(string memory) {
        return string(Acronym);
    }

    function Chat(string memory chatline) public override onlyOwners {
        if(Delegates[tx.origin].Soul == 0) revert UserNotEntered(tx.origin);
        User memory Alpha = Delegates[tx.origin];

        string memory Username = GetUsername(Alpha);
        On.Shio.Log(Alpha.Soul, Void.Nu().Aura(), string.concat("<", Username, "> ", chatline));
        React(Alpha, Xiao.Random());
        
        _mintToCap();
    }

    function OperatorSendMSG(string memory chatline) public override onlyOwners {
        On.Shio.Log(Saat[1], Void.Nu().Aura(), string.concat(chatline));
        _mintToCap();
    }
}