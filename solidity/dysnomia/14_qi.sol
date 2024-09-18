// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./include/user.sol";
import "./include/timedeposit.sol";
import "./01_dysnomia_v2.sol";
import "./interfaces/13b_qinginterface.sol";

contract QI is DYSNOMIA {
    string public constant Type = "QI";

    QING public Zuo;
    TimeDeposit[] private _deposits;
    string[] private _adjectives;
    mapping(uint64 UserSoul => mapping(string Adjective => uint256 Sum)) _userSums;
    mapping(uint256 QingWaat => mapping(string Adjective => uint256 Sum)) _qingSums;
    mapping(uint64 UserSoul => uint256[] DepositIds) private _userDepositIndexes;
    mapping(uint256 QingWaat => uint256[] DepositIds) private _qingDepositIndexes;

    constructor(address ZuoQingAddress) DYSNOMIA("DYSNOMIA Qi", "QI", address(DYSNOMIA(ZuoQingAddress).Xiao())) {
        Zuo = QING(ZuoQingAddress);
        addOwner(tx.origin);
        _mintToCap();
    }

    function GetUserAdjectiveValue(uint64 UserSoul, string memory Adjective) public view returns (uint256 Sum) {
        return _userSums[UserSoul][Adjective];
    }

    function GetQingAdjectiveValue(uint256 QingWaat, string memory Adjective) public view returns (uint256 Sum) {
        return _qingSums[QingWaat][Adjective];
    }

    function GetUserDepositCount(uint64 UserSoul) public view returns (uint256) {
        return _userDepositIndexes[UserSoul].length;
    }

    function GetUserDepositByIndex(uint64 UserSoul, uint256 Index) public view returns (TimeDeposit memory Stake, string memory Adjective) {
        return GetDeposit(_userDepositIndexes[UserSoul][Index]);    
    }
    
    function GetQingDepositCount(uint256 QingWaat) public view returns (uint256) {
        return _qingDepositIndexes[QingWaat].length;
    }

    function GetQingDepositByIndex(uint256 QingWaat, uint256 Index) public view returns (TimeDeposit memory Stake, string memory Adjective) {
        return GetDeposit(_qingDepositIndexes[QingWaat][Index]);
    }

    function GetDepositCount() public view returns (uint256) {
        return _deposits.length;
    }

    function GetDeposit(uint256 Id) public view returns (TimeDeposit memory Stake, string memory Adjective) {
        return (_deposits[Id], _adjectives[Id]);
    }

    error WaatMismatch(address Qing, uint256 Waat);
    error UnknownQing(address Qing);
    function Deposit(address Qing, string memory Adjective, uint256 amount) public {
        TimeDeposit memory _t;
        
        _t.waat = QING(Qing).Waat();
        if(_t.waat == 0) revert UnknownQing(Qing);
        
        address _checkQing = Zuo.Cho().Qu(_t.waat);
        if(_checkQing != Qing) revert WaatMismatch(Qing, _t.waat);

        uint64 _soul = Zuo.Cho().GetUserSoul();
        DYSNOMIA withdrawToken = DYSNOMIA(Zuo.Cho().Addresses("Yu"));
        withdrawToken.transferFrom(msg.sender, address(this), amount);

        _t.soul = _soul;
        _t.depositId = _deposits.length;
        _t.amount = amount;
        _t.timestamp = block.timestamp;
        _deposits.push(_t);
        _adjectives.push(Adjective);
        _userDepositIndexes[_soul].push(_t.depositId);
        _qingDepositIndexes[_t.waat].push(_t.depositId);
        _userSums[_soul][Adjective] += amount;
        _qingSums[_t.waat][Adjective] += amount;
        _mintToCap();
    }

    error NotOwner(uint256 DepositId);
    error ExceedsBalance(uint256 DepositId, uint256 Balance);
    function Withdraw(uint256 Id, uint256 Amount) public {
        uint64 _soul = Zuo.Cho().GetUserSoul();
        if(_deposits[Id].soul != _soul) revert NotOwner(Id);
        if(Amount > _deposits[Id].amount) revert ExceedsBalance(Id, _deposits[Id].amount);

        DYSNOMIA withdrawToken = DYSNOMIA(Zuo.Cho().Addresses("Yu"));
        withdrawToken.transfer(msg.sender, Amount);
        _deposits[Id].amount -= Amount;
        _userSums[_soul][_adjectives[Id]] -= Amount;
        _qingSums[_deposits[Id].waat][_adjectives[Id]] -= Amount;

        _mintToCap();
    }
}