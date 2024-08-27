// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../12_delegation.sol";

struct ACRONYM {
    uint16 Id;
    address UserAddress;
    string Phrase;
    uint16 Votes;
}

struct UserVote {
    uint16 Vote;
    uint64 Round;
}

contract Nym is DELEGATION {
    uint64 public RoundNumber = 0;
    uint16 public AcronymCount = 0;
    mapping(uint16 => ACRONYM) public Acronyms;
    mapping(uint64 => UserVote) public LastUserVote;
    User[] private _users;
    ACRONYM[] public History;
    bool public Active;
    bytes public Acronym;
    uint256 public RoundStartTime;

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
        On.Shio.Log(Saat[1], Saat[2], string.concat("New User Joined :: ", GetUsername(Alpha)));
        if(!Active && _users.length >= 5) NewRound();
    }

    function NewRound() internal {
        for(uint16 i = 0; i < AcronymCount; i++)
            delete Acronyms[i];
        AcronymCount = 0;

        for(uint256 i = 0; i < _users.length; i++) {
            if(LastUserVote[_users[i].Soul].Round < RoundNumber - 2) {
                delete LastUserVote[_users[i].Soul];
                delete Delegates[_users[i].On.Phi];
                On.Shio.Log(Saat[1], Saat[2], string.concat("Removed Inactive User :: ", GetUsername(_users[i])));
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

    function GetVotes() public view returns (ACRONYM[] memory Votable) {
        Votable = new ACRONYM[](AcronymCount);
        for(uint16 i = 0; i < AcronymCount; i++)
            Votable[i] = Acronyms[i];
    }

    function Vote(uint16 Id) public {
        assert(Id > 0 && Id <= AcronymCount);
        User memory Alpha = GetUser();

        if(LastUserVote[Alpha.Soul].Round <= RoundNumber) _mint(Alpha.On.Phi, 1 * 10 ** decimals());
        LastUserVote[Alpha.Soul].Vote = Id;
        LastUserVote[Alpha.Soul].Round = RoundNumber;
    }

    function CaseInsensitiveCompare(bytes1 A, bytes1 B) public pure returns (bool) {
        return(A != B && uint8(A) != (uint8(B) + 32));
    }

    function CheckAcronym(bytes memory _acronym, bytes memory Beta) public pure returns(bool) {
        if(!CaseInsensitiveCompare(Beta[0], _acronym[0])) return false;
        uint8 _pos = 1;
        for(uint256 i = 1; i < Beta.length; i++) {
            if(uint8(Beta[i]) == 32)
                if(!CaseInsensitiveCompare(Beta[i+1], _acronym[_pos])) return false;
                else {
                    _pos = _pos + 1;
                    i = i + 1;
                }
            if(_pos > _acronym.length) return false;
        }
        if(_pos != _acronym.length) return false;
        return true;
    }

    error InvalidAcronym(bytes Acronym, bytes Phrase);
    function Submit(bytes memory Beta) public {
        if(!CheckAcronym(Acronym, Beta)) revert InvalidAcronym(Acronym, Beta);

        User memory Alpha = GetUser();
        ACRONYM memory Kappa;
        AcronymCount = AcronymCount + 1;
        Kappa.Phrase = string(Beta);
        Kappa.Votes = 0;
        Kappa.Id = AcronymCount;
        Kappa.UserAddress = Alpha.On.Phi;
        Acronyms[AcronymCount] = Kappa;
        _mint(Kappa.UserAddress, 1 * 10 ** decimals());
    }

    function NewAcronym() internal {
        bytes memory LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        uint64 length = (Xiao.Random() % 5) + 3;
        Acronym = new bytes(length);
        for(uint i = 0; i < length; i++) {
            uint64 nxtchar = Xiao.Random() % 26;
            Acronym[i] = LETTERS[nxtchar];
        }

        On.Shio.Log(Saat[1], Saat[2], string.concat("New Acronym :: ", GetAcronym()));
        RoundStartTime = block.timestamp;
    }

    function GetAcronym() public view returns(string memory) {
        return string(Acronym);
    }

    function Chat(string memory chatline) public override onlyOwners {
        User memory Alpha = GetUser();

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