// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./include/user.sol";
import "./include/timedeposit.sol";
import "./01_dysnomia_v2.sol";
import "./interfaces/13b_qinginterface.sol";

contract QI is DYSNOMIA {
    string public constant Type = "QI";

    CHOINTERFACE public Cho;
    TimeDeposit[] private _deposits;
    string[] private _adjectives;
    mapping(uint64 UserSoul => uint256[] DepositIds) private _userDepositIndexes;
    mapping(uint256 QingWaat => uint256[] DepositIds) private _qingDepositIndexes;

    constructor(address ChoAddress) DYSNOMIA("DYSNOMIA Qi", "QI", address(DYSNOMIA(ChoAddress).Xiao())) {
        Cho = CHOINTERFACE(ChoAddress);
        addOwner(tx.origin);
        _mintToCap();
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
        _t.qing = Qing;
        
        uint256 QingWaat = QING(Qing).Waat();
        if(QingWaat == 0) revert UnknownQing(Qing);
        
        _t.qing == Cho.Qu(QingWaat);
        if(_t.qing != Qing) revert WaatMismatch(Qing, QingWaat);

        uint64 _soul = Cho.GetUserSoul();
        DYSNOMIA withdrawToken = DYSNOMIA(Cho.Addresses("Yu"));
        withdrawToken.transferFrom(msg.sender, address(this), amount);

        _t.soul = _soul;
        _t.depositId = _deposits.length;
        _t.amount = amount;
        _t.timestamp = block.timestamp;
        _deposits.push(_t);
        _adjectives.push(Adjective);
        _userDepositIndexes[_soul].push(_t.depositId);
        _qingDepositIndexes[QingWaat].push(_t.depositId);
        _mintToCap();
    }

    error NotOwner(uint256 DepositId);
    error ExceedsBalance(uint256 DepositId, uint256 Balance);
    function Withdraw(uint256 Id, uint256 Amount) public {
        uint64 _soul = Cho.GetUserSoul();
        if(_deposits[Id].soul != _soul) revert NotOwner(Id);
        if(Amount > _deposits[Id].amount) revert ExceedsBalance(Id, _deposits[Id].amount);

        DYSNOMIA withdrawToken = DYSNOMIA(Cho.Addresses("Yu"));
        withdrawToken.transfer(msg.sender, Amount);
        _deposits[Id].amount -= Amount;

        _mintToCap();
    }
}