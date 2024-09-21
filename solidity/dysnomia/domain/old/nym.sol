// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "../../interfaces/12b_chointerface.sol";
import "../../include/acronym.sol";
import "../../include/uservote.sol";

contract Nym is DYSNOMIA {
    string public constant Type = "Acronym Game";

    CHOINTERFACE public Cho;
    uint256 public Waat;
    uint64 public RoundNumber;
    uint16 public AcronymCount;
    mapping(uint16 => ACRONYM) private Acronyms;
    mapping(uint64 => UserVote) private LastUserVote;
    uint64[] private _users;
    mapping(uint64 => User) private _players;
    bool public Active;
    string public Acronym;
    string public Description;
    uint256 private RoundStartTime;
    uint16 public Prize;
    uint8 private RoundMinutes;
    uint8 private MinPlayers;
    uint8 public MaxAcronymLength;

    constructor(address ChoAddress) DYSNOMIA(unicode"Champion", unicode"NYM", address(DYSNOMIA(ChoAddress).Xiao())) {
        Cho = CHOINTERFACE(ChoAddress);
        Waat = Cho.Luo();
        Cho.addOwner(address(this));
       
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

    function SetDescription(string memory _d) public onlyOwners {
        Description = _d;
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

    function GetPlayerBySoul(uint64 Soul) public onlyOwners view returns(User memory) {
        return _players[Soul];
    }

    event JoinedUser(uint64 Soul, string Username);
    function Join(address UserToken) public {
        User memory Alpha = Cho.Enter(UserToken);
        if(_players[Alpha.Soul].Soul == Alpha.Soul) return;
        _users.push(Alpha.Soul);
        LastUserVote[Alpha.Soul].Round = RoundNumber;
        emit JoinedUser(Alpha.Soul, Alpha.Username);
        Log(Alpha.Soul, Cho.Void().Nu().Aura(), string.concat("New User Joined :: ", Alpha.Username));
        _players[Alpha.Soul] = Alpha;
        (Alpha.On.Omicron, Alpha.On.Omega) = ReactPlayer(Alpha.Soul, Cho.Void().Nu().Aura());
        if(!Active && _users.length >= MinPlayers) NewRound();
    }

    function Leave() public {
        uint64 _soul = Cho.GetUserSoul();
        if(_players[_soul].Soul == _soul)
            _removeUserBySoul(_soul);
    }

    function _removeUserBySoul(uint64 Soul) internal {
        for(uint16 i = 0; i < _users.length; i++) {
            if(_users[i] == Soul) {
                delete LastUserVote[_users[i]];
                delete _players[_users[i]];
                _users[i] = _users[_users.length - 1];
                _users.pop();
                return;
            }
        }
    }

    event KickedUser(uint64 OperatorSoul, string OperatorUsername, uint64 UserSoul, string Username);
    function Kick(uint64 _soul) public onlyOwners {
        uint64 _operatorsoul = Cho.GetUserSoul();
        User memory Operator = _players[_operatorsoul];
        for(uint16 i = 0; i < _users.length; i++) {
            if(_users[i] == _soul) {
                emit KickedUser(Operator.Soul, Operator.Username, _users[i], _players[_users[i]].Username);
                Log(_users[i], Operator.Soul, string.concat(Operator.Username, " Kicked User :: ", _players[_users[i]].Username));
                _removeUserBySoul(_users[i]);
            }
        }
    }

    function Start() public onlyOwners {
        if(!Active && _users.length > 0) {
            uint64 _soul = Cho.GetUserSoul();
            User memory Alpha = _players[_soul];
            if(_players[Alpha.Soul].Soul == 0) revert NotPlaying(Alpha.Soul);
            NewRound();
            Active = true;
            NewAcronym();
        }
    }

    event InactiveUser(uint64 UserSoul, string Username);
    function NewRound() internal {
        for(uint16 i = 1; i <= AcronymCount; i++)
            delete Acronyms[i];
        AcronymCount = 0;

        for(uint16 i = 0; i < _users.length; i++) {
            if(LastUserVote[_users[i]].Round == 0) continue;
            if((LastUserVote[_users[i]].Round + 2) < RoundNumber) {
                emit InactiveUser(_users[i], _players[_users[i]].Username);
                Log(_users[i], 0, string.concat("Removed Inactive User :: ", _players[_users[i]].Username));
                _removeUserBySoul(_users[i]);
            }
        }

        if(_users.length >= MinPlayers) {
            Active = true;
            NewAcronym();
        } else
            Active = false;
    }

    function Vote(uint16 Id) public {
        uint64 _soul = Cho.GetUserSoul();
        assert(Id > 0 && Id <= AcronymCount);

        if(LastUserVote[_soul].Round <= RoundNumber) {
            LastUserVote[_soul].Submissions = 0;
            _mint(_players[_soul].On.Phi, 1 * 10 ** decimals());
        }
        LastUserVote[_soul].Vote = Id;
        LastUserVote[_soul].Round = RoundNumber;

        (Acronyms[Id].UserInfo.On.Omicron, Acronyms[Id].UserInfo.On.Omega) = ReactPlayer(Acronyms[Id].UserInfo.Soul, _soul);
        (Acronyms[Id].UserInfo.On.Omicron, Acronyms[Id].UserInfo.On.Omega) = Cho.React(Acronyms[Id].UserInfo.On.Omega);
        (_players[_soul].On.Omicron, _players[_soul].On.Omega) = Cho.ReactUser(_soul, Acronyms[Id].UserInfo.Soul);
        (_players[_soul].On.Omicron, _players[_soul].On.Omega) = Cho.React(_players[_soul].On.Omicron);

        if(block.timestamp >= (RoundStartTime + (RoundMinutes * 1 minutes))) EndRound();
    }

    event Winner(uint64 Soul, string Username, string Phrase, uint16 Prize);
    function EndRound() internal {
        uint16[] memory Tally = new uint16[](AcronymCount + 1);
        uint16 winners = 0;
        uint16 winningvotes = 0;
        for(uint16 i = 0; i < _users.length; i++)
            if(LastUserVote[_users[i]].Round == RoundNumber)
                Tally[LastUserVote[_users[i]].Vote] += 1;
        
        for(uint16 i = 1; i <= AcronymCount; i++)
            if(Tally[i] > winningvotes) {
                winningvotes = Tally[i];
                winners = 1;
            } else if(Tally[i] == winningvotes)
                winners += 1;

        if(winningvotes > 0)
            for(uint16 i = 1; i <= AcronymCount; i++)
                if(Tally[i] == winningvotes) {
                    Log(Acronyms[i].UserInfo.Soul, 0, string.concat("WINNER ", Acronyms[i].UserInfo.Username, " !! ", Acronyms[i].Phrase));
                    emit Winner(Acronyms[i].UserInfo.Soul, Acronyms[i].UserInfo.Username, Acronyms[i].Phrase, Prize / winners);
                    _mint(Acronyms[i].UserInfo.On.Phi, (Prize / winners) * 10 ** decimals());
                    (Acronyms[i].UserInfo.On.Omicron, Acronyms[i].UserInfo.On.Omega) = ReactPlayer(Acronyms[i].UserInfo.Soul, Cho.Void().Nu().Psi().Rho().Lai.Omega);
                    (Acronyms[i].UserInfo.On.Omicron, Acronyms[i].UserInfo.On.Omega) = ReactPlayer(Acronyms[i].UserInfo.Soul, Cho.Void().Nu().Psi().Rho().Le.Omicron ^ Acronyms[i].UserInfo.On.Omicron);
                }
        
        NewRound();
    }

    function GetAcronym(uint16 idx) public view returns(ACRONYM memory) {
        return Acronyms[idx];
    }

    event AcronymSubmission(uint64 Soul, string Username, uint16 Id, string Phrase);
    function Submit(User memory Alpha, string memory Beta) internal {
        ACRONYM memory Kappa;
        AcronymCount = AcronymCount + 1;
        Kappa.Phrase = Beta;
        Kappa.Id = AcronymCount;
        Kappa.UserInfo = Alpha;
        Log(Alpha.Soul, Cho.Void().Nu().Aura(), string.concat("<", Alpha.Username, "> Submitted :: [", Cho.CYUN().String(uint256(Kappa.Id)), "] ", Beta));
        emit AcronymSubmission(Alpha.Soul, Alpha.Username, Kappa.Id, Beta);
        (Alpha.On.Omicron, Alpha.On.Omega) = Cho.Reactor().ReactLai(Alpha.On.Omicron ^ Alpha.Soul);
        (_players[Alpha.Soul].On.Omicron, _players[Alpha.Soul].On.Omega) = ReactPlayer(Alpha.Soul, Alpha.On.Omega);
        (Kappa.UserInfo.On.Omicron, Kappa.UserInfo.On.Omega) = Cho.ReactUser(Alpha.Soul, Alpha.On.Omicron);
        Acronyms[AcronymCount] = Kappa;

        if(LastUserVote[Alpha.Soul].Submissions < 5) {
            LastUserVote[Alpha.Soul].Submissions = LastUserVote[Alpha.Soul].Submissions + 1;
            _mint(Alpha.On.Phi, 1 * 10 ** decimals());     
        } else
            (_players[Alpha.Soul].On.Omicron, _players[Alpha.Soul].On.Omega) = ReactPlayer(Alpha.Soul, Cho.Void().Nu().Psi().Rho().Le.Omega);

        if(block.timestamp >= (RoundStartTime + (RoundMinutes * 1 minutes))) EndRound();
    }

    event NewRoundAcronym(string Acronym);
    function NewAcronym() internal {
        Acronym = string(Cho.CYUN().RandomAcronym(MaxAcronymLength));
        Log(0, 0, string.concat("New Acronym :: ", Acronym));
        emit NewRoundAcronym(Acronym);
        RoundStartTime = block.timestamp;
        RoundNumber = RoundNumber + 1;
    }

    error NotPlaying(uint64 Soul);
    function Chat(string memory chatline) public {
        uint64 _soul = Cho.GetUserSoul();

        if(Cho.CYUN().CheckAcronym(Acronym, chatline))
            Submit(_players[_soul], chatline);
        else {
            Log(_soul, Cho.Void().Nu().Aura(), string.concat("<", _players[_soul].Username, "> ", chatline));
            (_players[_soul].On.Omicron, _players[_soul].On.Omega) = Cho.Reactor().ReactToTalk(_players[_soul]);
            (_players[_soul].On.Omicron, _players[_soul].On.Omega) = Cho.React(_players[_soul].On.Omega);
        }
    }

    event LogEvent(uint256 Waat, uint64 Soul, uint64 Aura, string LogLine);
    function Log(uint64 Soul, uint64 Aura, string memory LogLine) internal {
        emit LogEvent(Waat, Soul, Aura, LogLine);
    }

    function OperatorSendMSG(string memory chatline) public onlyOwners {
        Log(0, Cho.Void().Nu().Aura(), string.concat(chatline));
    }

    function ReactPlayer(uint64 Soul, uint64 Theta) public returns (uint64 Omicron, uint64 Omega) {
        if(_players[Soul].Soul == 0) revert NotPlaying(_players[Soul].Soul);
        (_players[Soul].On.Omicron, _players[Soul].On.Omega) = Cho.Reactor().ReactShioRod(Cho.Shio(), _players[Soul].On.Omicron ^ Theta);
        return Cho.Reactor().ReactShioRod(Cho.Shio(), Cho.Omicron() ^ _players[Soul].On.Omega);
    }
}