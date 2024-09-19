// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia_v2.sol";
import "./interfaces/12b_chointerface.sol";
import "./interfaces/libyai.sol";

contract QING is DYSNOMIA {
    string public constant Type = "QING";

    CHOINTERFACE public Cho;
    DYSNOMIA public Asset;
    uint256 public Waat;
    uint16 public BouncerDivisor;
    uint256 public CoverCharge;
    bool public NoCROWS;
    mapping(address => bool) private _staff;
    mapping(address => uint256) private _list;
    mapping(uint64 => User) private _players;
    uint64[] private _users;

    constructor(address Integrative, address ChoAddress) DYSNOMIA("Mysterious Qing", "q", address(DYSNOMIA(ChoAddress).Xiao())) {
        Cho = CHOINTERFACE(ChoAddress);
        Waat = Cho.Luo();

        Asset = DYSNOMIA(Integrative);
        _staff[msg.sender] = true;
        setBouncerDivisor(32); // Default Based On Holding 25 CROWS
        setCoverCharge(0);
        NoCROWS = false;

        if(VAT().has(Integrative, "owner()")) addOwner(Asset.owner());
        if(VAT().has(Integrative, "name()") && VAT().has(Integrative, "symbol()")) Rename(string.concat(Asset.name(), " QING"), string.concat("q", Asset.symbol()));
        _mintToCap();
    }

    function Look() public view returns (uint64[] memory) {
        return _users;
    }

    event Withdrawal(uint64 Soul, uint64 Aura, address Token, uint256 amount);
    function Withdraw(address what, uint256 amount) public onlyOwners {
        User memory Alpha = Cho.GetUser();
        DYSNOMIA withdrawToken = DYSNOMIA(what);
        withdrawToken.transfer(msg.sender, amount);
        emit Withdrawal(Alpha.Soul, Cho.Void().Nu().Aura(), what, amount);
    }

    function VAT() public view returns (LIBYAI) {
        return LIBYAI(Cho.Void().GetLibraryAddress("yai"));
    }

    function AllowCROWS(bool _b) public onlyOwners {
        NoCROWS = _b;
    }

    function setBouncerDivisor(uint16 _d) public onlyBouncers {
        BouncerDivisor = _d;
    }

    function setCoverCharge(uint256 _c) public onlyBouncers {        
        CoverCharge = _c;
    }

    function setStaff(address _a, bool active) public onlyBouncers {        
        _staff[_a] = active;
    }

    function setGuestlist(address _a) public onlyBouncers {        
        _list[_a] = block.timestamp + 1 days;
    }

    modifier onlyBouncers() {
        _checkBouncer();
        _;
    }

    event KickedUser(uint64 OperatorSoul, string OperatorUsername, uint64 UserSoul, string Username, string Why);
    function Kick(uint64 Soul, string memory Why) public onlyBouncers {
        uint64 _operatorsoul = Cho.GetUserSoul();
        User memory Operator = _players[_operatorsoul];
        for(uint16 i = 0; i < _users.length; i++) {
            if(_users[i] == Soul) {
                emit KickedUser(Operator.Soul, Operator.Username, _users[i], _players[_users[i]].Username, Why);
                _removeUserBySoul(_users[i]);
            }
        }
    }

    function _removeUserBySoul(uint64 Soul) internal virtual {
        for(uint16 i = 0; i < _users.length; i++) {
            if(_users[i] == Soul) {
                delete _players[_users[i]];
                _users[i] = _users[_users.length - 1];
                _users.pop();
                return;
            }
        }
    }

    function bouncer(address cBouncer) public view returns (bool) {
        if(_staff[cBouncer]) return true;
        if(NoCROWS == false && DYSNOMIA(CROWSContract).balanceOf(cBouncer) >= 25 * 10 ** decimals()) return true;

        uint256 _ts = Asset.totalSupply();
        if(Asset.balanceOf(cBouncer) >= (_ts / BouncerDivisor)) return true;

        return Asset.owner(cBouncer);
    }

    error BouncerUnauthorized(address origin, address account, address what);
    function _checkBouncer() internal view {
        if(!bouncer(msg.sender) && !bouncer(tx.origin)) {
            uint64 _soul = Cho.GetUserSoul();
            if(!bouncer(_players[_soul].On.Phi))
                revert BouncerUnauthorized(tx.origin, msg.sender, address(this));
        }
    }

    event Joined(uint64 Soul, uint64 Aura, string Username);
    error AlreadyJoined(address UserToken);
    error CoverChargeUnauthorized(address AssetAddress, uint256 Amount);
    function Join(address UserToken) public {
        uint64 _soul = Cho.GetUserSoul();

        Cho.VerifyUserTokenPermissions(UserToken);

        if(_players[_soul].On.Phi == UserToken) revert AlreadyJoined(UserToken);
        if(_list[UserToken] < block.timestamp && CoverCharge > 0) {
            if(Asset.allowance(msg.sender, address(this)) <= CoverCharge) revert CoverChargeUnauthorized(address(Asset), CoverCharge + 1);
            bool paid = Asset.transferFrom(msg.sender, address(this), CoverCharge);
            if(!paid) revert CoverChargeUnauthorized(address(Asset), CoverCharge + 1);
        }

        if(_list[UserToken] < block.timestamp) {
            User memory Alpha = Cho.Enter(UserToken);
            emit Joined(Alpha.Soul, Cho.Void().Nu().Aura(), Alpha.Username);
            (Alpha.On.Omicron, Alpha.On.Omega) = ReactPlayer(Alpha.Soul, Cho.Void().Nu().Aura());
            _players[Alpha.Soul] = Alpha;
            _list[UserToken] = block.timestamp + 1 days;
            _users.push(Alpha.Soul);
        }
        Bounce();
        _mintToCap();
    }

    function Leave() public {
        uint64 _soul = Cho.GetUserSoul();
        if(_players[_soul].Soul == _soul)
            _removeUserBySoul(_soul);
    }

    function Connect(string memory name, address _qing) public onlyBouncers {
        VAT().Alias(Waat, name, _qing);
    }

    function Disconnect(string memory name) public onlyBouncers {
        VAT().Remove(Waat, name);
    }

    event Bounced(string Username);
    function Bounce() public {
        for(uint i = 0; i < _users.length; i++) {
            User memory Alpha = _players[_users[i]];
            if(_list[Alpha.On.Phi] < block.timestamp) {
                emit Bounced(Alpha.Username);
                delete _players[_users[i]];   
            }
        }
    }

    error Forbidden(address Asset);
    error NotAdmitted(uint64 Soul);
    function Chat(string memory chatline) public {
        uint64 _soul = Cho.GetUserSoul();
        if(VAT().IsForbidden(address(Asset))) revert Forbidden(address(Asset));
        if(_players[_soul].Soul == 0) revert NotAdmitted(_soul);
        emit LogEvent(Waat, _soul, Cho.Void().Nu().Aura(), _players[_soul].Username, chatline);
        (_players[_soul].On.Omicron, _players[_soul].On.Omega) = Cho.Reactor().ReactToTalk(_players[_soul]);
        Bounce();
        _mintToCap();
    }

    event LogEvent(uint256 Waat, uint64 Soul, uint64 Aura, string Username, string LogLine);
    function YAISendMSG(address UserToken, string memory Chatline) public {
        if(msg.sender != address(VAT())) revert Forbidden(address(this));
        LAU _user = LAU(UserToken);
        uint64 _soul = _user.Saat(1);
        Cho.VerifyUserTokenPermissions(UserToken);
        if(CoverCharge != 0 && _list[UserToken] < block.timestamp) revert NotAdmitted(_soul);
    
        emit LogEvent(Waat, _soul, _user.Saat(2), _user.Username(), Chatline);
    }

    function ReactPlayer(uint64 Soul, uint64 Theta) public returns (uint64 Omicron, uint64 Omega) {
        if(_players[Soul].Soul == 0) revert NotAdmitted(_players[Soul].Soul);
        (_players[Soul].On.Omicron, _players[Soul].On.Omega) = Cho.Reactor().ReactShioRod(Cho.Shio(), _players[Soul].On.Omicron ^ Theta);
        return Cho.Reactor().ReactShioRod(Cho.Shio(), Cho.Omicron() ^ _players[Soul].On.Omega);
    }
}
