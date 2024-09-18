// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia_v2.sol";
import "./interfaces/16b_xiainterface.sol";

struct CHAN {
    uint256 vaat;
    // STUB
}

contract XIE is DYSNOMIA {
    string public constant Type = "XIE";

    XIA public Xia;

    mapping(address => CHAN) private _chan;

    mapping(string Adjective => uint256 Power) private _adjectivePowers;
    mapping(uint256 QingWaat => uint256 Power) private _qingPowers;

    TimeDeposit[] private _deposits;
    mapping(uint64 UserSoul => uint256[] DepositIds) private _userDepositIndexes;
    mapping(uint64 UserSoul => mapping(uint256 QingWaat => mapping(string Adjective => uint256 DepositId))) private _userQingAdjectiveDeposits;

    constructor(address XiaAddress) DYSNOMIA("Dysnomia Xie", "XIE", address(DYSNOMIA(XiaAddress).Xiao())) {
        Xia = XIA(XiaAddress);
        addOwner(tx.origin);
        _mintToCap();
    }

    function Register(address ChanAddress, uint256 QingWaat) public {
        // STUB
    }

    function React(string memory Adjective) public returns (uint256 Omicron, uint256 Omega) {
        // STUB

        // return Chan.SAVE(Adjective, Omicron, Omega)
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
    function Deposit(address Qing, string memory Adjective, uint256 amount) public {
        TimeDeposit memory _t;
        
        _t.waat = QING(Qing).Waat();
        if(_t.waat == 0) revert UnknownQing(Qing);
        
        address _checkQing = Xia.Mai().Qi().Zuo().Cho().Qu(_t.waat);
        if(_checkQing != Qing) revert WaatMismatch(Qing, _t.waat);

        uint64 _soul = Xia.Mai().Qi().Zuo().Cho().GetUserSoul();
        address WM = Xia.Mai().Qi().Zuo().Cho().Addresses("WM");
        DYSNOMIA withdrawToken = DYSNOMIA(WM);
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

        _mintToCap();
    }

    error NotOwner(uint256 DepositId);
    error ExceedsBalance(uint256 DepositId, uint256 Balance);
    function Withdraw(uint256 Id, uint256 Amount) public {
        uint64 _soul = Xia.Mai().Qi().Zuo().Cho().GetUserSoul();
        if(_deposits[Id].soul != _soul) revert NotOwner(Id);
        if(Amount > _deposits[Id].amount) revert ExceedsBalance(Id, _deposits[Id].amount);

        address WM = Xia.Mai().Qi().Zuo().Cho().Addresses("WM");
        DYSNOMIA withdrawToken = DYSNOMIA(WM);
        withdrawToken.transfer(msg.sender, Amount);
        _deposits[Id].amount -= Amount;

        _mintToCap();
    }
}