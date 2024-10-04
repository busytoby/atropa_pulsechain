// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia_v2.sol";
import "./include/trait.sol";
import "./interfaces/15b_maiinterface.sol";

contract XIA is DYSNOMIA {
    string public constant Type = "XIA";

    MAI public Mai;
    uint256 public e = 2593300000000;
    uint256 public Balance;
    TimeDeposit[] private _deposits;
    mapping(uint64 UserSoul => uint256[] DepositIds) private _userDepositIndexes;
    mapping(uint64 UserSoul => mapping(uint256 QingWaat => mapping(TRAIT Trait => uint256 DepositId))) private _userQingTraitDeposits;

    constructor(address MaiAddress) DYSNOMIA("Dysnomia Xia", "XIA", address(DYSNOMIA(MaiAddress).Xiao())) {
        Mai = MAI(MaiAddress);
        addOwner(tx.origin);
        _mintToCap();
    }

    function Amplify(uint256 Amount, uint256 Timestamp) public view returns (uint256 Bonus) {
        if(Timestamp >= block.timestamp) return 0;
        uint256 _dt = block.timestamp - Timestamp;
        Bonus = Amount * _dt;
        _dt /= 60;
        if(_dt == 0) return Bonus;
        Bonus *= _dt * 2;
        _dt /= 24;
        if(_dt == 0) return Bonus;
        Bonus *= _dt * 4;
        _dt /= 30;
        if(_dt == 0) return Bonus;
        Bonus *= _dt * 8;
        _dt /= 12;
        uint16 i = 16;
        while(_dt > 0) {
            Bonus *= _dt * i;
            i *= 2;
            _dt /= 2;
        }
    }

    function Charge(uint256 QingWaat, TRAIT Trait) public returns (uint256) {
        uint256 _b = Mai.Qi().GetQingAdjectiveValue(QingWaat, Trait);
        TimeDeposit memory _d = Mai.GetQingDeposit(QingWaat);
        uint256 _e = Amplify(_d.amount, _d.timestamp);
        _d = GetQingAdjectiveDeposit(QingWaat, Trait);
        uint256 _m = Amplify(_d.amount, _d.timestamp);
        return Xiao.modExp(_b, _e, _m);
    }

    function Decay(uint256 Power) public view returns (uint256) {
        if(Power > 2 * 10 ** 25) return Power / 2;
        if(Power <= MotzkinPrime) return Power;
        return Power - ((Power * 10 ** 20) / Lambda());
    }

    function Lambda() public view returns (uint256) {
        return (2 * 10 ** 38) / (10000000001 - (Balance / e));
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

    function GetQingAdjectiveDeposit(uint256 QingWaat, TRAIT Trait) public view returns (TimeDeposit memory) {
        uint64 _soul = Mai.Qi().Zuo().Cho().GetUserSoul();
        return GetDeposit(_userQingTraitDeposits[_soul][QingWaat][Trait]);
    }

    error WaatMismatch(address Qing, uint256 Waat);
    error UnknownQing(address Qing);
    error ExceedsMaxSystemDeposit(uint256 MaxDepositRemaining, uint256 RequestedDeposit);
    function Deposit(address Qing, TRAIT Trait, uint256 amount) public {
        TimeDeposit memory _t;
        
        if(Balance + amount > e * 10000000000) revert ExceedsMaxSystemDeposit((e * 10000000000) - Balance, amount);

        _t.waat = QINGINTERFACE(Qing).Waat();
        if(_t.waat == 0) revert UnknownQing(Qing);
        
        address _checkQing = Mai.Qi().Zuo().Cho().Qu(_t.waat);
        if(_checkQing != Qing) revert WaatMismatch(Qing, _t.waat);

        uint64 _soul = Mai.Qi().Zuo().Cho().GetUserSoul();
        SHIO Fomalhaute = Mai.Qi().Zuo().Cho().Void().Nu().Psi().Mu().Tau().Upsilon().GetRodByIdx(Mai.Qi().Zuo().Cho().Void().Nu().Psi().Mu().Tau().Xi()).Shio;
        Fomalhaute.transferFrom(msg.sender, address(this), amount);

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

        Balance += amount;
        _mintToCap();
    }

    error NotOwner(uint256 DepositId);
    error ExceedsBalance(uint256 DepositId, uint256 Balance);
    function Withdraw(uint256 Id, uint256 Amount) public {
        uint64 _soul = Mai.Qi().Zuo().Cho().GetUserSoul();
        if(_deposits[Id].soul != _soul) revert NotOwner(Id);
        if(Amount > _deposits[Id].amount) revert ExceedsBalance(Id, _deposits[Id].amount);

        SHIO Fomalhaute = Mai.Qi().Zuo().Cho().Void().Nu().Psi().Mu().Tau().Upsilon().GetRodByIdx(Mai.Qi().Zuo().Cho().Void().Nu().Psi().Mu().Tau().Xi()).Shio;
        Fomalhaute.transfer(msg.sender, Amount);
        _deposits[Id].amount -= Amount;

        Balance -= Amount;
        _mintToCap();
    }
}