// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../12_delegation.sol";
import "../../include/acronym.sol";
import "../../include/uservote.sol";

contract Nym is DELEGATION {
    string public constant Type = "Acronym Game";

    uint64 public RoundNumber;
    uint16 public AcronymCount;
    mapping(uint16 => ACRONYM) private Acronyms;
    mapping(uint64 => UserVote) private LastUserVote;
    User[] private _users;
    bool public Active;
    string public Acronym;
    uint256 private RoundStartTime;
    uint16 public Prize;
    uint8 private RoundMinutes;
    uint8 private MinPlayers;
    uint8 public MaxAcronymLength;

    constructor(address VoidAddress) DELEGATION(unicode"Champion", unicode"NYM", VoidAddress) {
        maxSupply = 11111111111111111111;
        Active = false;
        RoundNumber = 0;
        AcronymCount = 0;
        Prize = 100;
        SetRoundMinutes(10);
        SetMinPlayers(5);
        SetMaxAcronymLength(6);
        SetDescription(
            "Once At Least 5 Players Have Entered By Calling Enter(UserToken) With Their UserToken Address\n" 
            "The Game Will Start. A 2-6 Letter Acronym Will Be Generated And Can Always Be Retrieved By\n"
            "Calling GetAcronym(). Each Round Will Last For 10 Minutes While You Submit As Many Acronyms\n"
            "As You Want Within The Round By Calling Submit(Acronym String) And Observe As Others Submit\n"
            "Their Own Acronyms Within The Chat Channel While You Can Vote Or Change Your Vote For This\n"
            "Round's Acronym By Calling Vote(Acronym Number).  Players Who Do Not Vote For 2 Rounds Will\n"
            "Be Kicked Out Of The Delegation\n"
            "Earn Up To 5 NYM For Each Acryonym Submitted, 1 NYM For Voting Each Round,\n"
            "Or 100 NYM For Winning !\n"
            "If More Than One Acronym Ties Then The 100 NYM Will Be Split"
        );
    }

    function SetMaxAcronymLength(uint8 _m) public onlyOwners {
        MaxAcronymLength = _m;
    }

    function SetRoundMinutes(uint8 _m) public onlyOwners {
        RoundMinutes = _m;
    }

    function SetMinPlayers(uint8 _m) public onlyOwners {
        MinPlayers = _m;
    }

    function ActivePlayers() public view returns(uint256) {
        return _users.length;
    }

    function Join(address UserToken) public {
        User memory Alpha = Enter(UserToken);
        _users.push(Alpha);
        LastUserVote[Alpha.Soul].Round = RoundNumber;
        Log(Alpha.Soul, Saat[2], string.concat("New User Joined :: ", Alpha.Username));
        if(!Active && _users.length >= MinPlayers) NewRound();
    }

    function Kick(uint64 _soul) public onlyOwners {
        for(uint16 i = 0; i < _users.length; i++) {
            if(_users[i].Soul == _soul) {
                delete Delegates[_users[i].On.Phi];
                Log(_users[i].Soul, Saat[2], string.concat(Delegates[tx.origin].Username, " Kicked User :: ", _users[i].Username));
                _users[i] = _users[_users.length - 1];
                _users.pop();
            }
        }
    }

    function NewRound() internal {
        (On.Omicron, On.Omega) = On.Shio.React(On.Omicron ^ Void.Nu().Psi().Rho().Bang.Omicron);
        for(uint16 i = 1; i <= AcronymCount; i++)
            delete Acronyms[i];
        AcronymCount = 0;

        for(uint16 i = 0; i < _users.length; i++) {
            if(LastUserVote[_users[i].Soul].Round == 0) continue;
            if((LastUserVote[_users[i].Soul].Round + 2) < RoundNumber) {
                delete LastUserVote[_users[i].Soul];
                delete Delegates[_users[i].On.Phi];
                Log(_users[i].Soul, Saat[2], string.concat("Removed Inactive User :: ", _users[i].Username));
                _users[i] = _users[_users.length - 1];
                _users.pop();
            }
        }

        if(_users.length >= MinPlayers) {
            Active = true;
            NewAcronym();
        } else
            Active = false;
        (On.Omicron, On.Omega) = On.Shio.React(On.Omega ^ Void.Nu().Psi().Rho().Bang.Omega);
    }

    function Vote(uint16 Id) public {
        assert(Id > 0 && Id <= AcronymCount);
        User memory Alpha = GetUser();

        if(LastUserVote[Alpha.Soul].Round <= RoundNumber) {
            LastUserVote[Alpha.Soul].Submissions = 0;
            _mint(Alpha.On.Phi, 1 * 10 ** decimals());
        }
        LastUserVote[Alpha.Soul].Vote = Id;
        LastUserVote[Alpha.Soul].Round = RoundNumber;

        (Acronyms[Id].UserInfo.On.Omicron, Acronyms[Id].UserInfo.On.Omega) = React(Acronyms[Id].UserInfo, Alpha.Soul);
        (Alpha.On.Omicron, Alpha.On.Omega) = React(Alpha, Acronyms[Id].UserInfo.Soul);
        Delegates[tx.origin] = Alpha;
        if(block.timestamp >= (RoundStartTime + (RoundMinutes * 1 minutes))) EndRound();
    }

    function EndRound() internal {
        uint16[] memory Tally = new uint16[](AcronymCount + 1);
        uint16 winners = 0;
        uint16 winningvotes = 0;
        for(uint16 i = 0; i < _users.length; i++)
            if(LastUserVote[_users[i].Soul].Round == RoundNumber)
                Tally[LastUserVote[_users[i].Soul].Vote] += 1;
        
        for(uint16 i = 1; i <= AcronymCount; i++)
            if(Tally[i] > winningvotes) {
                winningvotes = Tally[i];
                winners = 1;
            } else if(Tally[i] == winningvotes)
                winners += 1;

        for(uint16 i = 1; i <= AcronymCount; i++)
            if(Tally[i] == winningvotes) {
                Log(Acronyms[i].UserInfo.Soul, Saat[2], string.concat("WINNER ", Acronyms[i].UserInfo.Username, " !! ", Acronyms[i].Phrase));
                _mint(Acronyms[i].UserInfo.On.Phi, (Prize / winners) * 10 ** decimals());
                (Acronyms[i].UserInfo.On.Omicron, Acronyms[i].UserInfo.On.Omega) = React(Acronyms[i].UserInfo, Void.Nu().Psi().Rho().Lai.Omega);
                (Acronyms[i].UserInfo.On.Omicron, Acronyms[i].UserInfo.On.Omega) = React(Acronyms[i].UserInfo, Void.Nu().Psi().Rho().Le.Omicron ^ Acronyms[i].UserInfo.On.Omicron);
            }
        
        NewRound();
    }

    error InvalidAcronym(string Acronym, string Phrase);
    function Submit(string memory Beta) public {
        if(!CYUN().CheckAcronym(Acronym, Beta)) revert InvalidAcronym(Acronym, Beta);

        User memory Alpha = GetUser();
        ACRONYM memory Kappa;
        AcronymCount = AcronymCount + 1;
        Kappa.Phrase = Beta;
        Kappa.Id = AcronymCount;
        Kappa.UserInfo = Alpha;
        Acronyms[AcronymCount] = Kappa;
        Log(Alpha.Soul, Void.Nu().Aura(), string.concat("<", Alpha.Username, "> Submitted :: [", CYUN().String(Kappa.Id), "] ", string(Beta)));
        (Alpha.On.Omicron, Alpha.On.Omega) = ReactShioRod(Void.Nu().Psi().Rho().Lai.Shio, On.Omicron ^ Alpha.Soul);
        (Alpha.On.Omicron, Alpha.On.Omega) = React(Alpha, On.Omega ^ Alpha.On.Omega);
        (Kappa.UserInfo.On.Omicron, Kappa.UserInfo.On.Omega) = React(Alpha, Kappa.UserInfo.On.Omicron ^ On.Omicron);
        Delegates[tx.origin] = Alpha;
        if(LastUserVote[Alpha.Soul].Submissions < 5) {
            LastUserVote[Alpha.Soul].Submissions = LastUserVote[Alpha.Soul].Submissions + 1;
            _mint(Alpha.On.Phi, 1 * 10 ** decimals());     
        }   
        if(block.timestamp >= (RoundStartTime + (RoundMinutes * 1 minutes))) EndRound();
    }

    function NewAcronym() internal {
        (On.Omicron, On.Omega) = On.Shio.React(On.Omicron ^ Saat[2]);
        Acronym = string(CYUN().RandomAcronym(MaxAcronymLength));
        (On.Omicron, On.Omega) = On.Shio.React(On.Omega ^ Saat[1]);
        Log(Saat[1], Saat[2], string.concat("New Acronym :: ", Acronym));
        RoundStartTime = block.timestamp;
        RoundNumber = RoundNumber + 1;
        (On.Omicron, On.Omega) = On.Shio.React(On.Omega ^ Void.Nu().Psi().Rho().Le.Omega);
    }

    function Chat(string memory chatline) public {
        User memory Alpha = GetUser();

        Log(Alpha.Soul, Void.Nu().Aura(), string.concat("<", Alpha.Username, "> ", chatline));
        (Alpha.On.Omicron, Alpha.On.Omega) = Void.Nu().Psi().Rho().Lai.Shio.React(Alpha.Soul);

        Delegates[tx.origin] = Alpha;
        _mintToCap();
    }
}