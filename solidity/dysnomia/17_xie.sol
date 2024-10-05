// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia_v2.sol";
import "./include/trait.sol";
import "./interfaces/16b_xiainterface.sol";

struct Power {
    uint256 Charge;
    uint256 Block;
}

contract XIE is DYSNOMIA {
    string public constant Type = "XIE";

    XIA public Xia;

    mapping(TRAIT Trait => Power) private _traitPowers;
    mapping(uint256 QingWaat => Power) private _qingPowers;

    TimeDeposit[] private _deposits;
    mapping(uint64 UserSoul => uint256[] DepositIds) private _userDepositIndexes;
    mapping(uint64 UserSoul => mapping(uint256 QingWaat => mapping(TRAIT Trait => uint256 DepositId))) private _userQingTraitDeposits;

    constructor(address XiaAddress) DYSNOMIA("Dysnomia Xie", "XIE", address(DYSNOMIA(XiaAddress).Xiao())) {
        Xia = XIA(XiaAddress);
        addOwner(tx.origin);
        _mintToCap();
    }

    function GetPowerByTrait(TRAIT Trait) public view returns (uint256 Charge) {
        return _traitPowers[Trait].Charge;
    }

    function GetPowerByQing(uint256 Waat) public view returns (uint256 Charge) {
        return _qingPowers[Waat].Charge;
    }

    function Tso() public returns (TRAIT Trait, uint256 Charge) { // return value > 0 indicates complete
        TimeDeposit memory _t;
        uint256 _dc = Xia.Mai().Qi().GetDepositCount();
        uint256 _rc = Xiao.Random() % _dc;
        uint256 _oc = _dc;

        while(_rc < _dc) {
            (_t, Trait) = Xia.Mai().Qi().GetDeposit(_rc);
            if(_t.amount > MotzkinPrime/30 && _deposits[_userQingTraitDeposits[_t.soul][_t.waat][Trait]].amount > MotzkinPrime/30) {
                if(_traitPowers[Trait].Block < block.number || _qingPowers[_t.waat].Block < block.number) {
                    if(_traitPowers[Trait].Block == 0) _traitPowers[Trait].Block = block.number;
                    if(_qingPowers[_t.waat].Block == 0) _qingPowers[_t.waat].Block = block.number;

                    Charge = Xia.Charge(_t.waat, Trait) % Xia.Amplify(_deposits[_userQingTraitDeposits[_t.soul][_t.waat][Trait]].amount, _deposits[_userQingTraitDeposits[_t.soul][_t.waat][Trait]].timestamp);
                    for(; _traitPowers[Trait].Block < block.number; _traitPowers[Trait].Block++)
                        _traitPowers[Trait].Charge = Xia.Decay(_traitPowers[Trait].Charge);
                    for(; _qingPowers[_t.waat].Block < block.number; _qingPowers[_t.waat].Block++)
                        _qingPowers[_t.waat].Charge = Xia.Decay(_qingPowers[_t.waat].Charge);
                    _traitPowers[Trait].Charge += Charge;
                    _qingPowers[_t.waat].Charge += Charge;
                    return (Trait, Charge);
                }
            }
            if(++_rc == _dc && _oc != _dc) return (TRAIT.ZERO, 2);
            if(_rc == _dc && _oc == _dc) {
                _dc = _rc;
                _rc = 0;
            }                
        }
        return (TRAIT.ZERO, 2);
    }

    function _reactUserQingTrait(uint64 Soul, uint256 Waat, TRAIT Trait) internal returns (uint256 Charge) {
        TimeDeposit memory _t;
        (_t, Trait) = Xia.Mai().Qi().GetDeposit(_userQingTraitDeposits[Soul][Waat][Trait]);
        Charge = Xia.Charge(Waat, Trait) % Xia.Amplify(_deposits[_userQingTraitDeposits[Soul][Waat][Trait]].amount, _deposits[_userQingTraitDeposits[Soul][Waat][Trait]].timestamp);

        for(; _traitPowers[Trait].Block < block.number; _traitPowers[Trait].Block++) {
            if(_traitPowers[Trait].Block == 0) _traitPowers[Trait].Block = block.number;
            if(_t.amount == 0) break;
            if(_traitPowers[Trait].Charge > 0) _traitPowers[Trait].Charge = Xia.Decay(_traitPowers[Trait].Charge);
        }

        for(; _qingPowers[Waat].Block < block.number; _qingPowers[Waat].Block++) {
            if(_qingPowers[Waat].Block == 0) _qingPowers[Waat].Block = block.number;
            if(_t.amount == 0) break;
            if(_qingPowers[Waat].Charge > 0) _qingPowers[Waat].Charge = Xia.Decay(_qingPowers[Waat].Charge);
        }

        _traitPowers[Trait].Charge += Charge;
        _qingPowers[Waat].Charge += Charge;
    }

    function React(uint64 Soul, TRAIT Trait) public returns (uint256 Charge, uint64 Omicron, uint64 Omega) {
        User memory Player = Xia.Mai().Qi().Zuo().Cho().GetUserBySoul(Soul);
        QIN memory PlayerQin = Xia.Mai().GetPlayerQin(Soul);
        Charge = _reactUserQingTrait(Soul, PlayerQin.Location.Waat(), Trait);

        (Omicron, Omega) = Xia.Mai().React(Player, Charge);
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

    function GetQingTraitDeposit(uint256 QingWaat, TRAIT Trait) public view returns (TimeDeposit memory) {
        uint64 _soul = Xia.Mai().Qi().Zuo().Cho().GetUserSoul();
        return GetDeposit(_userQingTraitDeposits[_soul][QingWaat][Trait]);
    }

    error WaatMismatch(address Qing, uint256 Waat);
    error UnknownQing(address Qing);
    error ExceedsMaxSystemDeposit(uint256 MaxDepositRemaining, uint256 RequestedDeposit);
    error MinimumDepositAmount(uint256 Requested, uint256 Minimum);
    function Deposit(address Qing, TRAIT Trait, uint256 amount) public {
        if(amount < MotzkinPrime) revert MinimumDepositAmount(amount, MotzkinPrime / 30);
        TimeDeposit memory _t;
        
        _t.waat = QINGINTERFACE(Qing).Waat();
        if(_t.waat == 0) revert UnknownQing(Qing);
        
        address _checkQing = Xia.Mai().Qi().Zuo().Cho().Qu(_t.waat);
        if(_checkQing != Qing) revert WaatMismatch(Qing, _t.waat);

        uint64 _soul = Xia.Mai().Qi().Zuo().Cho().GetUserSoul();
        SHIO Fornax = SHIO(Xia.Mai().Qi().Zuo().Cho().Void().Nu().Psi().Mu().Tau().Upsilon().Eta().Psi());
        Fornax.transferFrom(msg.sender, address(this), amount);

        _t.soul = _soul;
        if(_userQingTraitDeposits[_soul][_t.waat][Trait] == 0)
            _t.depositId = _deposits.length;
        else 
            _t.depositId = _userQingTraitDeposits[_soul][_t.waat][Trait];
        _t.amount += amount;
        _t.timestamp = block.timestamp;
        _deposits.push(_t);
        _userDepositIndexes[_soul].push(_t.depositId);
        _userQingTraitDeposits[_soul][_t.waat][Trait] = _t.depositId;

        _mintToCap();
    }

    error NotOwner(uint256 DepositId);
    error ExceedsBalance(uint256 DepositId, uint256 Balance);
    function Withdraw(uint256 Id, uint256 Amount) public {
        uint64 _soul = Xia.Mai().Qi().Zuo().Cho().GetUserSoul();
        if(_deposits[Id].soul != _soul) revert NotOwner(Id);
        if(Amount > _deposits[Id].amount) revert ExceedsBalance(Id, _deposits[Id].amount);

        SHIO Fornax = SHIO(Xia.Mai().Qi().Zuo().Cho().Void().Nu().Psi().Mu().Tau().Upsilon().Eta().Psi());
        Fornax.transfer(msg.sender, Amount);
        _deposits[Id].amount -= Amount;

        _mintToCap();
    }
}