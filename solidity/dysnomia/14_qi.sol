// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./include/user.sol";
import "./01_dysnomia_v2.sol";
import "./interfaces/13b_qinginterface.sol";

contract QI is DYSNOMIA {
    string public constant Type = "QI";

    CHOINTERFACE public Cho;
    mapping(uint256 QingWaat => mapping(uint64 UserSoul => mapping(uint256 QiWaat => uint256 YUBalance))) public _deposits;
    mapping(uint64 UserSoul => string[] Adjectives) private _userAdjectives;
    mapping(uint64 UserSoul => mapping(string Adjective => uint256 _uAidx)) private _userAdjectiveIndexes;
    mapping(uint256 QingWaat => string[] Adjectives) private _qingAdjectives;
    mapping(uint256 QingWaat => mapping(string Adjective => uint256 _qAidx)) private _qingAdjectiveIndexes;
    mapping(uint64 UserSoul => mapping(uint256 QiWaat => uint256 QingWaat)) private _userDeposits;
    mapping(uint256 QingWaat => mapping(uint256 QiWaat => uint256 YUBalance)) private _qiWaats;

    mapping(string => uint256) public Bhat;

    constructor(address ChoAddress) DYSNOMIA("DYSNOMIA Qi", "QI", address(DYSNOMIA(ChoAddress).Xiao())) {
        Cho = CHOINTERFACE(ChoAddress);
        addOwner(tx.origin);
    }

    function GetUserAdjectives(uint64 UserSoul) public view returns (string[] memory Adjectives) {
        return _userAdjectives[UserSoul];
    }

    function GetQingAdjectives(uint256 QingWaat) public view returns (string[] memory Adjectives) {
        return _qingAdjectives[QingWaat];
    }

    function AddLibraryOwner(string memory what) public onlyOwners {
        _addLibraryOwner(Cho.Void(), what);
    }

    error UnknownWaat(uint256 Waat);
    function Deposit(uint256 QingWaat, string memory Adjective, uint256 amount) public {
        if(Cho.Qu(QingWaat) == address(0x0)) revert UnknownWaat(QingWaat);
        if(Bhat[Adjective] == 0) Bhat[Adjective] = Cho.Luo();
        if(Cho.Qu(Bhat[Adjective]) == address(0x0)) revert UnknownWaat(Bhat[Adjective]);

        uint64 _soul = Cho.GetUserSoul();
        DYSNOMIA withdrawToken = DYSNOMIA(Cho.Addresses("Yu"));
        withdrawToken.transferFrom(msg.sender, address(this), amount);
        _deposits[QingWaat][_soul][Bhat[Adjective]] += amount;
        if(_userAdjectiveIndexes[_soul][Adjective] == 0) {
            _userAdjectiveIndexes[_soul][Adjective] = _userAdjectives[_soul].length;
            _userAdjectives[_soul].push(Adjective);
        }
        _userDeposits[_soul][Bhat[Adjective]] += amount;
        if(_qingAdjectiveIndexes[_soul][Adjective] == 0) {
            _qingAdjectiveIndexes[_soul][Adjective] = _qingAdjectives[_soul].length;
            _qingAdjectives[_soul].push(Adjective);
        }
        _qiWaats[QingWaat][Bhat[Adjective]] += amount;
    }

    error InsufficientYuBalance(uint256 QingWaat, string Adjective, uint256 QiWaat, uint256 Balance, uint256 Request);
    function Withdraw(uint256 QingWaat, string memory Adjective, uint256 amount) public {
        uint64 _soul = Cho.GetUserSoul();
        DYSNOMIA withdrawToken = DYSNOMIA(Cho.Addresses("Yu"));
        if(_deposits[QingWaat][_soul][Bhat[Adjective]] < amount) revert InsufficientYuBalance(QingWaat, Adjective, Bhat[Adjective], _deposits[QingWaat][_soul][Bhat[Adjective]], amount);
        withdrawToken.transfer(msg.sender, amount);
        _deposits[QingWaat][_soul][Bhat[Adjective]] -= amount;
        _userDeposits[_soul][Bhat[Adjective]] -= amount;
        _qiWaats[QingWaat][Bhat[Adjective]] -= amount;
    }
}