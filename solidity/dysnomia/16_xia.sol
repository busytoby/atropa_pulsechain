// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia_v2.sol";
import "./interfaces/15b_maiinterface.sol";

contract XIA is DYSNOMIA {
    string public constant Type = "XIA";

    MAI public Mai;
    uint256 public e = 751401541040690000000000000000;
    uint256 public Balance;
    TimeDeposit[] private _deposits;
    mapping(uint64 UserSoul => uint256[] DepositIds) private _userDepositIndexes;
    mapping(uint64 UserSoul => mapping(uint256 QingWaat => mapping(string Adjective => uint256 DepositId))) private _userQingAdjectiveDeposits;

    constructor(address MaiAddress) DYSNOMIA("Dysnomia Xia", "XIA", address(DYSNOMIA(MaiAddress).Xiao())) {
        Mai = MAI(MaiAddress);
        addOwner(tx.origin);
        _mintToCap();
    }

    function Charge(uint256 QingWaat, string memory adjective) public returns (uint256) {
        uint256 _b = Mai.Qi().GetQingAdjectiveValue(QingWaat, adjective);
        uint256 _e = Mai.GetQingDeposit(QingWaat).amount;
        uint256 _m = GetQingAdjectiveDeposit(QingWaat, adjective).amount;
        return Xiao.modExp(_b, _e, _m);
    }

    function Decay(uint256 Power) public view returns (uint256) {
        if(Power < MotzkinPrime) return Power;
        if(Power > 2 * 10 ** 48) return Power / 2;

        return Power - ((Power * 10 ** 20) / Lambda());
    }

    function Lambda() public view returns (uint256) {
        return (2 * 10 ** 38) / (1001 - (Balance / e));
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
        uint64 _soul = Mai.Qi().Zuo().Cho().GetUserSoul();
        return GetDeposit(_userQingAdjectiveDeposits[_soul][QingWaat][Adjective]);
    }

    error WaatMismatch(address Qing, uint256 Waat);
    error UnknownQing(address Qing);
    error ExceedsMaxSystemDeposit(uint256 MaxDepositRemaining, uint256 RequestedDeposit);
    function Deposit(address Qing, string memory Adjective, uint256 amount) public {
        TimeDeposit memory _t;
        
        if(Balance + amount > e * 1000) revert ExceedsMaxSystemDeposit((e * 1000) - Balance, amount);

        _t.waat = QING(Qing).Waat();
        if(_t.waat == 0) revert UnknownQing(Qing);
        
        address _checkQing = Mai.Qi().Zuo().Cho().Qu(_t.waat);
        if(_checkQing != Qing) revert WaatMismatch(Qing, _t.waat);

        uint64 _soul = Mai.Qi().Zuo().Cho().GetUserSoul();
        address Freebies = Mai.Qi().Zuo().Cho().Addresses("Freebies");
        DYSNOMIA withdrawToken = DYSNOMIA(Freebies);
        withdrawToken.transferFrom(msg.sender, address(this), amount);

        _t.soul = _soul;
        if(_userQingAdjectiveDeposits[_soul][_t.waat][Adjective] == 0)
            _t.depositId = _deposits.length;
        else 
            _t.depositId = _userQingAdjectiveDeposits[_soul][_t.waat][Adjective];
        _t.amount += amount;
        _t.timestamp = block.timestamp;
        _deposits.push(_t);
        _userDepositIndexes[_soul].push(_t.depositId);
        _userQingAdjectiveDeposits[_soul][_t.waat][Adjective] = _t.depositId;

        Balance += amount;
        _mintToCap();
    }

    error NotOwner(uint256 DepositId);
    error ExceedsBalance(uint256 DepositId, uint256 Balance);
    function Withdraw(uint256 Id, uint256 Amount) public {
        uint64 _soul = Mai.Qi().Zuo().Cho().GetUserSoul();
        if(_deposits[Id].soul != _soul) revert NotOwner(Id);
        if(Amount > _deposits[Id].amount) revert ExceedsBalance(Id, _deposits[Id].amount);

        address Freebies = Mai.Qi().Zuo().Cho().Addresses("Freebies");
        DYSNOMIA withdrawToken = DYSNOMIA(Freebies);
        withdrawToken.transfer(msg.sender, Amount);
        _deposits[Id].amount -= Amount;

        Balance -= Amount;
        _mintToCap();
    }
}