// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia_v2.sol";
import "./include/qin.sol";
import "./interfaces/14b_qiinterface.sol";

contract MAI is DYSNOMIA {
    string public constant Type = "MAI";

    QIINTERFACE public Qi;
    mapping(uint64 Soul => QIN) private _players;
    TimeDeposit[] private _deposits;
    mapping(uint64 UserSoul => uint256[] DepositIds) private _userDepositIndexes;
    mapping(uint64 UserSoul => mapping(uint256 QingWaat => uint256 DepositId)) private _userQingDeposits;

    constructor(address QiAddress) DYSNOMIA("Dysnomia MAI", "MAI", address(DYSNOMIA(QiAddress).Xiao())) {
        Qi = QIINTERFACE(QiAddress);
        Qi.Zuo().Cho().addOwner(address(this));
        addOwner(tx.origin);
        addOwner(address(Qi.Zuo().VAT()));        
        _mintToCap();
    }

    function GetLocationByAddress(address UserToken) public view returns (QINGINTERFACE) {
        uint64 Soul = LAU(UserToken).Saat(1);
        return _players[Soul].Location;
    }

    function GetPlayerQin(uint64 Soul) public view returns (QIN memory) {
        if(address(_players[Soul].Location) == address(0x0)) revert UnknownQing(address(0x0));
        return _players[Soul];
    }

    function Maat() public returns (uint256) {
        uint64 _soul = Qi.Zuo().Cho().GetUserSoul();
        if(_players[_soul].Maat == 0) _players[_soul].Maat = Qi.Zuo().Cho().Luo();
        _mintToCap();
        return _players[_soul].Maat;
    }

    function GetUserDepositCount(uint64 UserSoul) public view returns (uint256) {
        return _userDepositIndexes[UserSoul].length;
    }

    function GetUserDepositByIndex(uint64 UserSoul, uint256 Index) public view returns (TimeDeposit memory Stake) {
        return GetDeposit(_userDepositIndexes[UserSoul][Index]);
    }

    function GetDepositCount() public view returns (uint256) {
        return _deposits.length;
    }

    function GetDeposit(uint256 Id) public view returns (TimeDeposit memory) {
        return _deposits[Id];
    }

    function GetQingDepositBySoul(uint64 Soul, uint256 QingWaat) public view returns (TimeDeposit memory) {
        return GetDeposit(_userQingDeposits[Soul][QingWaat]);
    }

    function GetQingDeposit(uint256 QingWaat) public view returns (TimeDeposit memory) {
        uint64 _soul = Qi.Zuo().Cho().GetUserSoul();
        return GetDeposit(_userQingDeposits[_soul][QingWaat]);
    }

    error WaatMismatch(address Qing, uint256 Waat);
    error UnknownQing(address Qing);
    error MinimumDepositAmount(uint256 Requested, uint256 Minimum);
    function Deposit(address Qing, uint256 amount) public {
        if(amount < MotzkinPrime) revert MinimumDepositAmount(amount, MotzkinPrime / 7776000); 
        TimeDeposit memory _t;
        
        _t.waat = QINGINTERFACE(Qing).Waat();
        if(_t.waat == 0) revert UnknownQing(Qing);
        
        address _checkQing = Qi.Zuo().Cho().Qu(_t.waat);
        if(_checkQing != Qing) revert WaatMismatch(Qing, _t.waat);

        uint64 _soul = Qi.Zuo().Cho().GetUserSoul();
        DYSNOMIA withdrawToken = DYSNOMIA(Qing);
        withdrawToken.transferFrom(msg.sender, address(this), amount);

        _t.soul = _soul;
        if(_userQingDeposits[_soul][_t.waat] == 0)
            _t.depositId = _deposits.length;
        else 
            _t.depositId = _userQingDeposits[_soul][_t.waat];
        _t.amount += amount;
        _t.timestamp = block.timestamp;
        _deposits.push(_t);
        _userDepositIndexes[_soul].push(_t.depositId);
        _userQingDeposits[_soul][_t.waat] = _t.depositId;
        _mintToCap();
    }

    error NotOwner(uint256 DepositId);
    error ExceedsBalance(uint256 DepositId, uint256 Balance);
    function Withdraw(uint256 Id, uint256 Amount) public {
        uint64 _soul = Qi.Zuo().Cho().GetUserSoul();
        if(_deposits[Id].soul != _soul) revert NotOwner(Id);
        if(Amount > _deposits[Id].amount) revert ExceedsBalance(Id, _deposits[Id].amount);

        address Qing = Qi.Zuo().Cho().Qu(_deposits[Id].waat);
        DYSNOMIA withdrawToken = DYSNOMIA(Qing);
        withdrawToken.transfer(msg.sender, Amount);
        _deposits[Id].amount -= Amount;

        _mintToCap();
    }

    function _updateUserLocation(User memory Beta, address ToQing) internal {
        if(address(_players[Beta.Soul].Location) != address(0x0)) _players[Beta.Soul].Location.Leave();
        _players[Beta.Soul].Location = QINGINTERFACE(ToQing);
        _players[Beta.Soul].Location.Join(Beta.On.Phi);
        _players[Beta.Soul].lastMove = block.timestamp;

        React(Beta, _players[Beta.Soul].Location.Waat());
        _mintToCap();
    }

    function React(User memory Beta, uint256 Gamma) public returns (uint64 Omicron, uint64 Omega) {
        (Omicron, Omega) = Qi.Zuo().Cho().React(uint64(Gamma % MotzkinPrime));
        Omicron = Qi.Zuo().Cho().ReactUser(Beta.Soul, Omicron);
        Omega = Qi.Zuo().ReactPlayer(Beta.Soul, Omega);
    }

    error OneMovePerBlock();
    function Move(string memory To) public onlyOwners {
        User memory Beta = Qi.Zuo().Cho().GetUser();
        if(_players[Beta.Soul].lastMove >= block.timestamp) revert OneMovePerBlock();
        uint256 From = _players[Beta.Soul].Location.Waat();
        address ToQing = Qi.Zuo().VAT().Alias(From, To);

        _updateUserLocation(Beta, ToQing);
    }

    function MovePlayer(uint64 Soul, address ToQing) public onlyOwners {
        User memory Beta = Qi.Zuo().Cho().GetUserBySoul(Soul);
        _updateUserLocation(Beta, ToQing);
    }
}