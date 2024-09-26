// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia_v2.sol";
import "./interfaces/16b_xiainterface.sol";

struct Power {
    uint256 Charge;
    uint256 Block;
}

contract XIE is DYSNOMIA {
    string public constant Type = "XIE";

    XIA public Xia;

    mapping(string Adjective => Power) private _adjectivePowers;
    mapping(uint256 QingWaat => Power) private _qingPowers;
    uint256 private _lastTsoBlock;

    uint64 private _depositAmount = 239500800;
    TimeDeposit[] private _deposits;
    mapping(uint64 UserSoul => uint256[] DepositIds) private _userDepositIndexes;
    mapping(uint64 UserSoul => mapping(uint256 QingWaat => mapping(string Adjective => uint256 DepositId))) private _userQingAdjectiveDeposits;

    constructor(address XiaAddress) DYSNOMIA("Dysnomia Xie", "XIE", address(DYSNOMIA(XiaAddress).Xiao())) {
        Xia = XIA(XiaAddress);
        addOwner(tx.origin);
        _lastTsoBlock = block.number;
        _mintToCap();
    }

    function GetPowerByAdjective(string calldata Adjective) public view returns (uint256 Charge) {
        return _adjectivePowers[Adjective].Charge;
    }

    function GetPowerByQing(uint256 Waat) public view returns (uint256 Charge) {
        return _qingPowers[Waat].Charge;
    }

    function Tso(uint256 Max) public returns (uint256) { // return value > 0 indicates complete
        TimeDeposit memory _t;
        string memory _a;
        uint256 _p;
        uint256 _dc = Xia.Mai().Qi().GetDepositCount();
        for(; _lastTsoBlock < block.number; _lastTsoBlock++) {
            for(uint256 qa = 0; qa < _dc; qa++) {
                (_t, _a) = Xia.Mai().Qi().GetDeposit(qa);
                if(_adjectivePowers[_a].Block < block.number || _qingPowers[_t.waat].Block < block.number) {
                    if(_adjectivePowers[_a].Block == 0) _adjectivePowers[_a].Block = block.number;
                    if(_qingPowers[_t.waat].Block == 0) _qingPowers[_t.waat].Block = block.number;
                    if(_t.amount == 0) continue;
                    _p = Xia.Charge(_t.waat, _a);
                    for(; _adjectivePowers[_a].Block < block.number; _adjectivePowers[_a].Block++)
                        _adjectivePowers[_a].Charge = Xia.Decay(_adjectivePowers[_a].Charge);
                    for(; _qingPowers[_t.waat].Block < block.number; _qingPowers[_t.waat].Block++)
                        _qingPowers[_t.waat].Charge = Xia.Decay(_qingPowers[_t.waat].Charge);
                    _adjectivePowers[_a].Charge += _p;
                    _qingPowers[_t.waat].Charge += _p;
                    if(Max == 0) return 0;
                    --Max;
                }
            }
        }
        return Max;
    }

    function _reactUserQingAdjective(uint64 Soul, uint256 Waat, string memory Adjective) internal returns (uint256 Charge) {
        TimeDeposit memory _t;
        (_t, Adjective) = Xia.Mai().Qi().GetDeposit(_userQingAdjectiveDeposits[Soul][Waat][Adjective]);
        Charge = Xia.Charge(Waat, Adjective) % _t.amount;

        for(; _adjectivePowers[Adjective].Block < block.number; _adjectivePowers[Adjective].Block++) {
            if(_adjectivePowers[Adjective].Block == 0) _adjectivePowers[Adjective].Block = block.number;
            if(_t.amount == 0) break;
            if(_adjectivePowers[Adjective].Charge > 0) _adjectivePowers[Adjective].Charge = Xia.Decay(_adjectivePowers[Adjective].Charge);
        }

        for(; _qingPowers[Waat].Block < block.number; _qingPowers[Waat].Block++) {
            if(_qingPowers[Waat].Block == 0) _qingPowers[Waat].Block = block.number;
            if(_t.amount == 0) break;
            if(_qingPowers[Waat].Charge > 0) _qingPowers[Waat].Charge = Xia.Decay(_qingPowers[Waat].Charge);
        }

        _adjectivePowers[Adjective].Charge += Charge;
        _qingPowers[Waat].Charge += Charge;
    }

    function React(uint64 Soul, string memory Adjective) public returns (uint256 Charge, uint64 Omicron, uint64 Omega) {
        User memory Player = Xia.Mai().Qi().Zuo().Cho().GetUserBySoul(Soul);
        QIN memory PlayerQin = Xia.Mai().GetPlayerQin(Soul);
        Charge = _reactUserQingAdjective(Soul, PlayerQin.Location.Waat(), Adjective);

        (Omicron, Omega) = Xia.Mai().React(Player, Charge);
        Tso(48);
        _mintToCap();
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

    function GetQingAdjectiveDeposit(uint256 QingWaat, string memory Adjective) public view returns (TimeDeposit memory) {
        uint64 _soul = Xia.Mai().Qi().Zuo().Cho().GetUserSoul();
        return GetDeposit(_userQingAdjectiveDeposits[_soul][QingWaat][Adjective]);
    }

    error WaatMismatch(address Qing, uint256 Waat);
    error UnknownQing(address Qing);
    error ExceedsMaxSystemDeposit(uint256 MaxDepositRemaining, uint256 RequestedDeposit);
    function Deposit(address Qing, string memory Adjective) public {
        TimeDeposit memory _t;
        
        _t.waat = QING(Qing).Waat();
        if(_t.waat == 0) revert UnknownQing(Qing);
        
        address _checkQing = Xia.Mai().Qi().Zuo().Cho().Qu(_t.waat);
        if(_checkQing != Qing) revert WaatMismatch(Qing, _t.waat);

        uint64 _soul = Xia.Mai().Qi().Zuo().Cho().GetUserSoul();
        address WM = Xia.Mai().Qi().Zuo().Cho().Addresses("WM");
        DYSNOMIA withdrawToken = DYSNOMIA(WM);
        withdrawToken.transferFrom(msg.sender, address(this), _depositAmount);

        _t.soul = _soul;
        if(_userQingAdjectiveDeposits[_soul][_t.waat][Adjective] == 0)
            _t.depositId = _deposits.length;
        else 
            _t.depositId = _userQingAdjectiveDeposits[_soul][_t.waat][Adjective];
        _t.amount += _depositAmount;
        _t.timestamp = block.timestamp;
        _deposits.push(_t);
        _userDepositIndexes[_soul].push(_t.depositId);
        _userQingAdjectiveDeposits[_soul][_t.waat][Adjective] = _t.depositId;

        _mintToCap();
    }

    error NotOwner(uint256 DepositId);
    error ExceedsBalance(uint256 DepositId, uint256 Balance);
    function Withdraw(uint256 Id) public {
        uint64 _soul = Xia.Mai().Qi().Zuo().Cho().GetUserSoul();
        if(_deposits[Id].soul != _soul) revert NotOwner(Id);
        if(_depositAmount > _deposits[Id].amount) revert ExceedsBalance(Id, _deposits[Id].amount);

        address WM = Xia.Mai().Qi().Zuo().Cho().Addresses("WM");
        DYSNOMIA withdrawToken = DYSNOMIA(WM);
        withdrawToken.transfer(msg.sender, _depositAmount);
        _deposits[Id].amount -= _depositAmount;

        _mintToCap();
    }
}