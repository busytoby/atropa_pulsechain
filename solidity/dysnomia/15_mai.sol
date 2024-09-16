// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia_v2.sol";
import "./interfaces/14b_qiinterface.sol";

struct QIN {
    uint256 Maat;
    QING Location;
    uint256 lastMove;
}

contract MAI is DYSNOMIA {
    string public constant Type = "MAI";

    QING public Zuo;
    mapping(uint64 Soul => QIN) private _players;
    mapping(uint256 Id => TimeDeposit Stake) private _deposits;
    mapping(uint64 UserSoul => uint256[] DepositIds) private _userDepositIndexes;
    mapping(uint64 UserSoul => mapping(uint256 QingWaat => uint256 DepositIds)) private _userQingDeposits;

    constructor(address ZuoQingAddress) DYSNOMIA("Dysnomia MAI", "MAI", address(DYSNOMIA(ZuoQingAddress).Xiao())) {
        Zuo = QING(ZuoQingAddress);
        addOwner(tx.origin);
        _mintToCap();
    }

    function Maat() public returns (uint256) {
        uint64 _soul = Zuo.Cho().GetUserSoul();
        if(_soul == 0) revert NotPlaying(_soul);
        if(_players[_soul].Maat == 0) _players[_soul].Maat = Zuo.Cho().Luo();
        _mintToCap();
        return Maat();
    }

    function GetUserDepositsIds(uint64 UserSoul) public view returns (uint256[] memory DepositIds) {
        return _userDepositIndexes[UserSoul];
    }

    function GetDeposit(uint256 Id) public view returns (TimeDeposit memory Stake) {
        return _deposits[Id];
    }

    error WaatMismatch(address Qing, uint256 Waat);
    error UnknownQing(address Qing);
    function Deposit(address Qing, uint256 amount) public {
        TimeDeposit memory _t;
        _t.qing = Qing;
        
        uint256 QingWaat = QING(Qing).Waat();
        if(QingWaat == 0) revert UnknownQing(Qing);
        
        _t.qing == Zuo.Cho().Qu(QingWaat);
        if(_t.qing != Qing) revert WaatMismatch(Qing, QingWaat);

        uint64 _soul = Zuo.Cho().GetUserSoul();
        DYSNOMIA withdrawToken = DYSNOMIA(_t.qing);
        withdrawToken.transferFrom(msg.sender, address(this), amount);

        _t.soul = _soul;
        if(_userQingDeposits[_soul][QingWaat] == 0)
            _t.depositId = Zuo.Cho().Luo();
        else 
            _t.depositId = _userQingDeposits[_soul][QingWaat];
        _t.amount = amount;
        _t.timestamp = block.timestamp;
        _deposits[_t.depositId] = _t;
        _userDepositIndexes[_soul].push(_t.depositId);
        _userQingDeposits[_soul][QingWaat] = _t.depositId;
        _mintToCap();
    }

    error NotOwner(uint256 DepositId);
    error ExceedsBalance(uint256 DepositId, uint256 Balance);
    function Withdraw(uint256 Id, uint256 Amount) public {
        uint64 _soul = Zuo.Cho().GetUserSoul();
        if(_deposits[Id].soul != _soul) revert NotOwner(Id);
        if(Amount > _deposits[Id].amount) revert ExceedsBalance(Id, _deposits[Id].amount);

        DYSNOMIA withdrawToken = DYSNOMIA(_deposits[Id].qing);
        withdrawToken.transfer(msg.sender, Amount);
        _deposits[Id].amount -= Amount;

        _mintToCap();
    }

    function _updateUserLocation(User memory Beta, address ToQing) internal {
        if(address(_players[Beta.Soul].Location) != address(0x0)) _players[Beta.Soul].Location.Leave();
        _players[Beta.Soul].Location = QING(ToQing);
        _players[Beta.Soul].Location.Join(Beta.On.Phi);
        _players[Beta.Soul].lastMove = block.timestamp;

        (uint64 Omicron, uint64 Omega) = Zuo.Cho().React(Beta.On.Omicron);
        (Omicron, Omega) = Zuo.Cho().ReactUser(Beta.Soul, Omega);
        (Omicron, Omega) = Zuo.ReactPlayer(Beta.Soul, Omicron);
        _mintToCap();
    }

    error NotPlaying(uint64 Soul);
    error OneMovePerBlock();
    function Move(string memory To) public {
        User memory Beta = Zuo.Cho().GetUser();
        if(Beta.Soul == 0) revert NotPlaying(Beta.Soul);
        if(_players[Beta.Soul].lastMove >= block.timestamp) revert OneMovePerBlock();
        uint256 From = _players[Beta.Soul].Location.Waat();
        address ToQing = Zuo.VAT().Alias(From, To);

        _updateUserLocation(Beta, ToQing);
    }

    function MovePlayer(uint64 Soul, address ToQing) public onlyOwners {
        User memory Beta = Zuo.Cho().GetUserBySoul(Soul);
        if(Beta.Soul == 0) revert NotPlaying(Beta.Soul);
        if(_players[Beta.Soul].Maat == 0) revert NotPlaying(Beta.Soul);

        _updateUserLocation(Beta, ToQing);
    }
}