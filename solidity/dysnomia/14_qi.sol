// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./include/user.sol";
import "./include/timedeposit.sol";
import "./include/trait.sol";
import "./01_dysnomia_v2.sol";
import "./interfaces/13b_qinginterface.sol";

contract QI is DYSNOMIA {
    string public constant Type = "QI";

    QINGINTERFACE public Zuo;
    TimeDeposit[] private _deposits;
    TRAIT[] private _traits;
    mapping(uint64 UserSoul => mapping(TRAIT => uint256 Sum)) _userSums;
    mapping(uint256 QingWaat => mapping(TRAIT => uint256 Sum)) _qingSums;
    mapping(uint64 UserSoul => uint256[] DepositIds) private _userDepositIndexes;
    mapping(uint256 QingWaat => uint256[] DepositIds) private _qingDepositIndexes;

    constructor(address ZuoQingAddress) DYSNOMIA("DYSNOMIA Qi", "QI", address(DYSNOMIA(ZuoQingAddress).Xiao())) {
        Zuo = QINGINTERFACE(ZuoQingAddress);
        addOwner(tx.origin);
        _mintToCap();
    }

    function GetUserAdjectiveValue(uint64 UserSoul, TRAIT Trait) public view returns (uint256 Sum) {
        return _userSums[UserSoul][Trait];
    }

    function GetQingAdjectiveValue(uint256 QingWaat, TRAIT Trait) public view returns (uint256 Sum) {
        return _qingSums[QingWaat][Trait];
    }

    function GetUserDepositCount(uint64 UserSoul) public view returns (uint256) {
        return _userDepositIndexes[UserSoul].length;
    }

    function GetUserDepositByIndex(uint64 UserSoul, uint256 Index) public view returns (TimeDeposit memory Stake, TRAIT Trait) {
        return GetDeposit(_userDepositIndexes[UserSoul][Index]);    
    }
    
    function GetQingDepositCount(uint256 QingWaat) public view returns (uint256) {
        return _qingDepositIndexes[QingWaat].length;
    }

    function GetQingDepositByIndex(uint256 QingWaat, uint256 Index) public view returns (TimeDeposit memory Stake, TRAIT Trait) {
        return GetDeposit(_qingDepositIndexes[QingWaat][Index]);
    }

    function GetDepositCount() public view returns (uint256) {
        return _deposits.length;
    }

    function GetDeposit(uint256 Id) public view returns (TimeDeposit memory Stake, TRAIT Trait) {
        return (_deposits[Id], _traits[Id]);
    }

    error WaatMismatch(address Qing, uint256 Waat);
    error UnknownQing(address Qing);
    error MinimumDepositAmount(uint256 Requested, uint256 Minimum);
    function Deposit(address Qing, TRAIT Trait, uint256 amount) public {        
        if(amount < 1 * 10 ** 12) revert MinimumDepositAmount(amount, 1 * 10 ** 12);
        TimeDeposit memory _t;
        
        _t.waat = QINGINTERFACE(Qing).Waat();
        if(_t.waat == 0) revert UnknownQing(Qing);
        
        address _checkQing = Zuo.Cho().Qu(_t.waat);
        if(_checkQing != Qing) revert WaatMismatch(Qing, _t.waat);

        uint64 _soul = Zuo.Cho().GetUserSoul();
        SHIO Eris = Zuo.Cho().Void().Nu().Psi().Mu().Tau().Upsilon().GetRodByIdx(Zuo.Cho().Void().Nu().Psi().Rho().Lai.Xi).Shio;
        Eris.transferFrom(msg.sender, address(this), amount);

        _t.soul = _soul;
        _t.depositId = _deposits.length;
        _t.amount = amount;
        _t.timestamp = block.timestamp;
        _deposits.push(_t);
        _traits.push(Trait);
        _userDepositIndexes[_soul].push(_t.depositId);
        _qingDepositIndexes[_t.waat].push(_t.depositId);
        _userSums[_soul][Trait] += amount;
        _qingSums[_t.waat][Trait] += amount;
        _mintToCap();
    }

    error NotOwner(uint256 DepositId);
    error ExceedsBalance(uint256 DepositId, uint256 Balance);
    error MinimumDepositOneYear(uint256 DepositTimestamp, uint256 MaturationTimestamp);
    function Withdraw(uint256 Id, uint256 Amount) public {
        uint64 _soul = Zuo.Cho().GetUserSoul();
        if(_deposits[Id].soul != _soul) revert NotOwner(Id);
        if(Amount > _deposits[Id].amount) revert ExceedsBalance(Id, _deposits[Id].amount);
        uint256 maturation = _deposits[Id].timestamp + 365 days;
        if(block.timestamp < maturation) revert MinimumDepositOneYear(_deposits[Id].timestamp, maturation);

        SHIO Eris = Zuo.Cho().Void().Nu().Psi().Mu().Tau().Upsilon().GetRodByIdx(Zuo.Cho().Void().Nu().Psi().Rho().Lai.Xi).Shio;
        Eris.transfer(msg.sender, Amount);
        _deposits[Id].amount -= Amount;
        _userSums[_soul][_traits[Id]] -= Amount;
        _qingSums[_deposits[Id].waat][_traits[Id]] -= Amount;

        _mintToCap();
    }
}