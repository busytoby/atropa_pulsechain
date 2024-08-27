// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../12_delegation.sol";

struct ACRONYM {
    uint16 Id;
    User UserInfo;
    string Phrase;
    uint16 Votes;
}

struct UserVote {
    uint16 Vote;
    uint64 Round;
}

contract Nym is DELEGATION {
    uint64 public RoundNumber;
    uint16 public AcronymCount;
    mapping(uint16 => ACRONYM) public Acronyms;
    mapping(uint64 => UserVote) public LastUserVote;
    User[] private _users;
    bool public Active;
    bytes private Acronym;
    uint256 public RoundStartTime;
    uint16 public Prize;
    uint8 public RoundMinutes;
    uint8 public MinPlayers;

    constructor(address VoidAddress) DELEGATION(unicode"Champion", unicode"NYM", VoidAddress) {
        maxSupply = 11111111111111111111;
        Active = false;
        RoundNumber = 0;
        AcronymCount = 0;
        Prize = 100;
        SetRoundMinutes(1);
        SetMinPlayers(1);
    }

    function SetRoundMinutes(uint8 _m) public onlyOwners {
        RoundMinutes = _m;
    }

    function SetMinPlayers(uint8 _m) public onlyOwners {
        MinPlayers = _m;
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
        "If More Than One Acronym Ties Then The 100 NYM Will Be Split ."
        "";
    }

    function Join(address UserToken) public {
        User memory Alpha = Enter(UserToken);
        _users.push(Alpha);
        LastUserVote[Alpha.Soul].Round = RoundNumber;
        On.Shio.Log(Saat[1], Saat[2], string.concat("New User Joined :: ", Alpha.Username));
        if(!Active && _users.length >= MinPlayers) NewRound();
    }

    function NewRound() public onlyOwners {
        for(uint16 i = 1; i < AcronymCount + 1; i++)
            delete Acronyms[i];
        AcronymCount = 0;

        for(uint256 i = 0; i < _users.length; i++) {
            if(LastUserVote[_users[i].Soul].Round == 0) continue;
            if(LastUserVote[_users[i].Soul].Round < RoundNumber - 2) {
                delete LastUserVote[_users[i].Soul];
                delete Delegates[_users[i].On.Phi];
                On.Shio.Log(Saat[1], Saat[2], string.concat("Removed Inactive User :: ", _users[i].Username));
                _users[i] = _users[_users.length - 1];
                _users.pop();
                i = i - 1;
            }
        }

        if(_users.length >= MinPlayers) {
            Active = true;
            NewAcronym();
        } else
            Active = false;
    }

    function GetVotes() public view returns (string[] memory Votable) {
        Votable = new string[](AcronymCount);
        for(uint16 i = 0; i < AcronymCount; i++)
            Votable[i] = Acronyms[i].Phrase;
    }

    function Vote(uint16 Id) public {
        assert(Id > 0 && Id <= AcronymCount);
        User memory Alpha = GetUser();

        if(LastUserVote[Alpha.Soul].Round <= RoundNumber) _mint(Alpha.On.Phi, 1 * 10 ** decimals());
        LastUserVote[Alpha.Soul].Vote = Id;
        LastUserVote[Alpha.Soul].Round = RoundNumber;
        Acronyms[Id].Votes = Acronyms[Id].Votes + 1;
        if(block.timestamp >= (RoundStartTime + (RoundMinutes * 1 minutes))) EndRound();
    }

    function EndRound() public onlyOwners {
        uint16[] memory Tally = new uint16[](AcronymCount + 1);
        uint16 winners = 0;
        uint16 winningvotes = 0;
        for(uint16 i = 0; i < _users.length; i++)
            if(LastUserVote[_users[i].Soul].Round == RoundNumber)
                Tally[LastUserVote[_users[i].Soul].Vote] += 1;
        
        for(uint16 i = 1; i < AcronymCount + 1; i++)
            if(Tally[i] > winningvotes) {
                winningvotes = Tally[i];
                winners = 1;
            } else if(Tally[i] == winningvotes)
                winners += 1;

        for(uint16 i = 1; i < AcronymCount + 1; i++)
            if(Tally[i] == winningvotes) {
                On.Shio.Log(Saat[1], Saat[2], string.concat("WINNER ", Acronyms[i].UserInfo.Username, " !! ", Acronyms[i].Phrase));
                _mint(Acronyms[i].UserInfo.On.Phi, (Prize / winners) * 10 ** decimals());
            }
        
        //NewRound();
    }

    function CaseInsensitiveCompare(bytes1 A, bytes1 B) public pure returns (bool) {
        return(A == B || uint8(A) == (uint8(B) + 32));
    }

    error CheckAcronymError(uint which, uint8 A, uint8 B);
    function CheckAcronym(bytes memory _acronym, string memory _Beta) public pure returns(bool) {
        bytes memory Beta = bytes(_Beta);
        if(!CaseInsensitiveCompare(Beta[0], _acronym[0])) revert CheckAcronymError(1, uint8(Beta[0]), uint8(_acronym[0]));
        uint8 _pos = 1;
        for(uint256 i = 1; i < Beta.length; i++) {
            if(uint8(Beta[i]) == 32)
                if(!CaseInsensitiveCompare(Beta[i+1], _acronym[_pos])) revert CheckAcronymError(2, uint8(Beta[i+1]), uint8(_acronym[_pos]));
                else {
                    _pos = _pos + 1;
                    i = i + 1;
                }
            if(_pos > _acronym.length) revert CheckAcronymError(3, _pos, uint8(_acronym.length));
        }
        if(_pos != _acronym.length) revert CheckAcronymError(4, _pos, uint8(_acronym.length));
        return true;
    }

    error InvalidAcronym(bytes Acronym, string Phrase);
    function Submit(string memory Beta) public {
        if(!CheckAcronym(Acronym, Beta)) revert InvalidAcronym(Acronym, Beta);

        User memory Alpha = GetUser();
        ACRONYM memory Kappa;
        AcronymCount = AcronymCount + 1;
        Kappa.Phrase = string(Beta);
        Kappa.Votes = 0;
        Kappa.Id = AcronymCount;
        Kappa.UserInfo = Alpha;
        Acronyms[AcronymCount] = Kappa;
        On.Shio.Log(Alpha.Soul, Void.Nu().Aura(), string.concat("<", Alpha.Username, "> Submitted :: ", string(Beta)));
        React(Alpha, Kappa.Id);
        _mint(Alpha.On.Phi, 1 * 10 ** decimals());        
        if(block.timestamp >= (RoundStartTime + (RoundMinutes * 1 minutes))) EndRound();
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
        RoundNumber = RoundNumber + 1;
    }

    function GetAcronym() public view returns(string memory) {
        return string(Acronym);
    }

    function Chat(string memory chatline) public override onlyOwners {
        User memory Alpha = GetUser();

        On.Shio.Log(Alpha.Soul, Void.Nu().Aura(), string.concat("<", Alpha.Username, "> ", chatline));
        React(Alpha, Xiao.Random());

        _mintToCap();
    }

    function OperatorSendMSG(string memory chatline) public override onlyOwners {
        On.Shio.Log(Saat[1], Void.Nu().Aura(), string.concat(chatline));
        _mintToCap();
    }
}