// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../12_cho.sol";
import "../../include/acronym.sol";
import "../../include/uservote.sol";

contract Nym is DYSNOMIA {
    string public constant Type = "Acronym Game";

    CHO public Cho;
    uint64[3] public Saat;
    Bao public On;
    uint64 public RoundNumber;
    uint16 public AcronymCount;
    mapping(uint16 => ACRONYM) private Acronyms;
    mapping(uint64 => UserVote) private LastUserVote;
    User[] private _users;
    bool public Active;
    string public Acronym;
    string public Description;
    uint256 private RoundStartTime;
    uint16 public Prize;
    uint8 private RoundMinutes;
    uint8 private MinPlayers;
    uint8 public MaxAcronymLength;

    constructor(address ChoAddress) DYSNOMIA(unicode"Champion", unicode"NYM", address(DYSNOMIA(ChoAddress).Xiao())) {
        Cho = CHO(ChoAddress);

        (Saat, On) = Cho.Void().Enter(__name, __symbol);
        On.Phi = address(this);
        On.Shio.addOwner(address(this));
        On.Shio.Rho().Rod.addOwner(address(this));
        On.Shio.Rho().Cone.addOwner(address(this));

        (On.Omicron, On.Omega) = _react(Cho.Void().Nu().Psi().Rho().Bang.Omicron);
        (On.Omega, On.Omicron) = Cho.Void().Nu().Psi().Rho().Bang.Shio.React(On.Omega);
        (On.Omicron, On.Omega) = _react(Cho.Void().Nu().Psi().Rho().Le.Omicron);

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

    event JoinedUser(uint64 Soul, string Username);
    function Join(address UserToken) public {
        User memory Alpha = Cho.Enter(UserToken);
        _users.push(Alpha);
        LastUserVote[Alpha.Soul].Round = RoundNumber;
        emit JoinedUser(Alpha.Soul, Alpha.Username);
        Log(Alpha.Soul, Saat[1], string.concat("New User Joined :: ", Alpha.Username));
        (Alpha.On.Omicron, Alpha.On.Omega) = React(Alpha, Saat[1]);
        if(!Active && _users.length >= MinPlayers) NewRound();
    }

    event KickedUser(uint64 OperatorSoul, string OperatorUsername, uint64 UserSoul, string Username);
    function Kick(uint64 _soul) public onlyOwners {
        User memory Operator = Cho.GetUser();
        for(uint16 i = 0; i < _users.length; i++) {
            if(_users[i].Soul == _soul) {
                emit KickedUser(Operator.Soul, Operator.Username, _users[i].Soul, _users[i].Username);
                Log(_users[i].Soul, Operator.Soul, string.concat(Operator.Username, " Kicked User :: ", _users[i].Username));
                _users[i] = _users[_users.length - 1];
                _users.pop();
            }
        }
    }

    event InactiveUser(uint64 UserSoul, string Username);
    function NewRound() internal {
        _react(On.Omicron ^ Cho.Void().Nu().Psi().Rho().Bang.Omicron);
        for(uint16 i = 1; i <= AcronymCount; i++)
            delete Acronyms[i];
        AcronymCount = 0;

        for(uint16 i = 0; i < _users.length; i++) {
            if(LastUserVote[_users[i].Soul].Round == 0) continue;
            if((LastUserVote[_users[i].Soul].Round + 2) < RoundNumber) {
                delete LastUserVote[_users[i].Soul];
                emit InactiveUser(_users[i].Soul, _users[i].Username);
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
        _react(On.Omega ^ Cho.Void().Nu().Psi().Rho().Bang.Omega);
    }

    function Vote(uint16 Id) public {
        assert(Id > 0 && Id <= AcronymCount);
        User memory Alpha = Cho.GetUser();

        if(LastUserVote[Alpha.Soul].Round <= RoundNumber) {
            LastUserVote[Alpha.Soul].Submissions = 0;
            _mint(Alpha.On.Phi, 1 * 10 ** decimals());
        }
        LastUserVote[Alpha.Soul].Vote = Id;
        LastUserVote[Alpha.Soul].Round = RoundNumber;

        (Acronyms[Id].UserInfo.On.Omicron, Acronyms[Id].UserInfo.On.Omega) = React(Acronyms[Id].UserInfo, Alpha.Soul);
        Cho.ReactUser(Alpha.Soul, Acronyms[Id].UserInfo.Soul);

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
                (Acronyms[i].UserInfo.On.Omicron, Acronyms[i].UserInfo.On.Omega) = React(Acronyms[i].UserInfo, Cho.Void().Nu().Psi().Rho().Lai.Omega);
                (Acronyms[i].UserInfo.On.Omicron, Acronyms[i].UserInfo.On.Omega) = React(Acronyms[i].UserInfo, Cho.Void().Nu().Psi().Rho().Le.Omicron ^ Acronyms[i].UserInfo.On.Omicron);
            }
        
        NewRound();
    }

    error InvalidAcronym(string Acronym, string Phrase);
    function Submit(string memory Beta) public {
        if(!Cho.CYUN().CheckAcronym(Acronym, Beta)) revert InvalidAcronym(Acronym, Beta);

        User memory Alpha = Cho.GetUser();
        ACRONYM memory Kappa;
        AcronymCount = AcronymCount + 1;
        Kappa.Phrase = Beta;
        Kappa.Id = AcronymCount;
        Kappa.UserInfo = Alpha;
        Acronyms[AcronymCount] = Kappa;
        Log(Alpha.Soul, Cho.Void().Nu().Aura(), string.concat("<", Alpha.Username, "> Submitted :: [", Cho.CYUN().String(Kappa.Id), "] ", string(Beta)));
        (Alpha.On.Omicron, Alpha.On.Omega) = Cho.Void().Nu().Psi().Rho().Lai.Shio.React(On.Omicron ^ Alpha.Soul);
        (Alpha.On.Omicron, Alpha.On.Omega) = React(Alpha, On.Omega ^ Alpha.On.Omega);
        Cho.ReactUser(Alpha.Soul, Alpha.On.Omicron ^ On.Omicron);

        if(LastUserVote[Alpha.Soul].Submissions < 5) {
            LastUserVote[Alpha.Soul].Submissions = LastUserVote[Alpha.Soul].Submissions + 1;
            _mint(Alpha.On.Phi, 1 * 10 ** decimals());     
        }   
        if(block.timestamp >= (RoundStartTime + (RoundMinutes * 1 minutes))) EndRound();
    }

    function NewAcronym() internal {
        _react(On.Omicron ^ Saat[2]);
        Acronym = string(Cho.CYUN().RandomAcronym(MaxAcronymLength));
        _react(On.Omega ^ Saat[1]);
        Log(Saat[1], Saat[2], string.concat("New Acronym :: ", Acronym));
        RoundStartTime = block.timestamp;
        RoundNumber = RoundNumber + 1;
        _react(On.Omega ^ Cho.Void().Nu().Psi().Rho().Le.Omega);
    }

    function Chat(string memory chatline) public {
        User memory Alpha = Cho.GetUser();

        Log(Alpha.Soul, Cho.Void().Nu().Aura(), string.concat("<", Alpha.Username, "> ", chatline));
        Cho.ReactUser(Alpha.Soul, Cho.Void().Nu().Psi().Rho().Lai.Omicron);
        (Alpha.On.Omicron, Alpha.On.Omega) = Cho.Void().Nu().Psi().Rho().Lai.Shio.React(Alpha.Soul);
        Cho.ReactUser(Alpha.Soul, Alpha.On.Omega);

        _mintToCap();
    }

    function Log(uint64 Soul, uint64 Aura, string memory LogLine) public onlyOwners {
        On.Shio.Log(Soul, Aura, LogLine);
    }

    function OperatorSendMSG(string memory chatline) public onlyOwners {
        _react(On.Omicron ^ Cho.Void().Nu().Aura());
        Log(Saat[1], Cho.Void().Nu().Aura(), string.concat(chatline));
        _mintToCap();
        _react(On.Omega ^ Saat[1]);
    }

    function _react(uint64 Eta) internal returns (uint64 Omicron, uint64 Omega) {
        (On.Omicron, On.Omega) = On.Shio.React(Eta);
        return(On.Omicron, On.Omega);
    }

    function React(User memory Alpha, uint64 Theta) public returns (uint64 Omicron, uint64 Omega) {
        (Alpha.On.Omicron, Alpha.On.Omega) = _react(Alpha.On.Omicron ^ Theta);
        return _react(On.Omicron ^ Alpha.On.Omega);
    }
}