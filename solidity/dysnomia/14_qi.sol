// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./include/user.sol";
import "./include/timedeposit.sol";
import "./01_dysnomia_v2.sol";
import "./interfaces/13b_qinginterface.sol";

contract QI is DYSNOMIA {
    string public constant Type = "QI";

    CHOINTERFACE public Cho;
    mapping(uint256 Id => TimeDeposit Stake) private _deposits;
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

    function GetDeposit(uint256 Id) public view returns (TimeDeposit memory Stake) {
        return _deposits[Id];
    }

    error UnknownWaat(uint256 Waat);
    error Max5555Days();
    function Deposit(uint256 QingWaat, string memory Adjective, uint256 amount) public {
        if(Cho.Qu(QingWaat) == address(0x0)) revert UnknownWaat(QingWaat);

        uint64 _soul = Cho.GetUserSoul();
        DYSNOMIA withdrawToken = DYSNOMIA(Cho.Addresses("Yu"));
        withdrawToken.transferFrom(msg.sender, address(this), amount);

        TimeDeposit memory _t;
        _t.soul = _soul;
        _t.adjective = Adjective;
        _t.depositId = Cho.Luo();
        _t.amount = amount;
        _t.timestamp = block.timestamp;
        _deposits[_t.depositId] = _t;
        _userDepositIndexes[_soul].push(_t.depositId);
        _qingDepositIndexes[QingWaat].push(_t.depositId);
        _mintToCap();
    }

    error NotOwner(uint256 DepositId);
    error ExceedsBalance(uint256 DepositId, uint256 Balance);
    error NotMature(uint256 DepositId, uint256 Maturation);
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