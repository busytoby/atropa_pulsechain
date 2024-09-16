// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./include/user.sol";
import "./01_dysnomia_v2.sol";
import "./interfaces/13b_qinginterface.sol";

struct TimeLockedQiDeposit {
    uint64 soul;
    string adjective;
    uint256 depositId;
    uint256 amount;
    uint256 maturation;
    uint256 withdrawnTimestamp;
}

contract QI is DYSNOMIA {
    string public constant Type = "QI";

    CHOINTERFACE public Cho;
    mapping(uint256 Id => TimeLockedQiDeposit Stake) private _deposits;
    mapping(uint64 UserSoul => uint256[] DepositIds) private _userDepositIndexes;
    mapping(uint256 QingWaat => uint256[] DepositIds) private _qingDepositIndexes;

    constructor(address ChoAddress) DYSNOMIA("DYSNOMIA Qi", "QI", address(DYSNOMIA(ChoAddress).Xiao())) {
        Cho = CHOINTERFACE(ChoAddress);
        addOwner(tx.origin);
        _mintToCap();
    }

    function GetUserDepositsIds(uint64 UserSoul) public view returns (uint256[] memory DepositIds) {
        return _userDepositIndexes[UserSoul];
    }

    function GetQingDepositIds(uint256 QingWaat) public view returns (uint256[] memory DepositIds) {
        return _qingDepositIndexes[QingWaat];
    }

    function GetDeposit(uint256 Id) public view returns (TimeLockedQiDeposit memory Stake) {
        return _deposits[Id];
    }

    error UnknownWaat(uint256 Waat);
    error Max5555Days();
    function Deposit(uint256 QingWaat, string memory Adjective, uint256 amount, uint16 Days) public {
        if(Days > 5555) revert Max5555Days();
        if(Cho.Qu(QingWaat) == address(0x0)) revert UnknownWaat(QingWaat);

        uint64 _soul = Cho.GetUserSoul();
        DYSNOMIA withdrawToken = DYSNOMIA(Cho.Addresses("Yu"));
        withdrawToken.transferFrom(msg.sender, address(this), amount);

        TimeLockedQiDeposit memory _t;
        _t.soul = _soul;
        _t.adjective = Adjective;
        _t.depositId = Cho.Luo();
        _t.amount = amount;
        _t.maturation = block.timestamp + Days * 1 days;
        _deposits[_t.depositId] = _t;
        _userDepositIndexes[_soul].push(_t.depositId);
        _qingDepositIndexes[QingWaat].push(_t.depositId);
        _mintToCap();
    }

    error NotOwner(uint256 DepositId);
    error AlreadyWithdrawn(uint256 DepositId, uint256 WithdrawnTimestamp);
    error NotMature(uint256 DepositId, uint256 Maturation);
    function Withdraw(uint256 Id) public {
        uint64 _soul = Cho.GetUserSoul();
        if(_deposits[Id].soul != _soul) revert NotOwner(Id);
        if(_deposits[Id].withdrawnTimestamp > 0) revert AlreadyWithdrawn(Id, _deposits[Id].withdrawnTimestamp);
        if(_deposits[Id].maturation < block.timestamp) revert NotMature(Id, _deposits[Id].maturation);

        DYSNOMIA withdrawToken = DYSNOMIA(Cho.Addresses("Yu"));
        withdrawToken.transfer(msg.sender, _deposits[Id].amount);

        _deposits[Id].withdrawnTimestamp = block.timestamp;
        _mintToCap();
    }
}