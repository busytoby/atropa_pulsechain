// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "../../include/qin.sol";
import "../../include/trait.sol";
import "../interfaces/yueinterface.sol";

contract CHAN is DYSNOMIA {
    string public constant Type = "CHAN";

    XIE public Xie;

    mapping(address => address Yue) public Yan;
    mapping(uint256 Maat => uint64 Entropy) public Entropy;

    constructor(address XieAddress) DYSNOMIA("Dysnomia Chan", "CHAN", address(DYSNOMIA(XieAddress).Xiao())) {
        Xie = XIE(XieAddress);
        addOwner(tx.origin);
        _mintToCap();
    }

    error AlreadyAdded(address Origin, address Yue, address New);
    function AddYue(address Origin, address Yue) public onlyOwners {
        //if(Yan[Origin] != address(0x0)) revert AlreadyAdded(Origin, Yan[Origin], Yue);
        Yan[Origin] = Yue;
    }

    error NotOrigin(address YueOrigin, address Requestor);
    function TransferYue(address Yue, address NewOrigin) public {
        if(Yan[NewOrigin] != address(0x0)) revert AlreadyAdded(YUEINTERFACE(Yan[NewOrigin]).Origin(), Yue, NewOrigin);
        YUEINTERFACE _chi = YUEINTERFACE(Yue);
        if(_chi.Origin() != tx.origin) revert NotOrigin(_chi.Origin(), tx.origin);
        Yan[_chi.Origin()] = address(0x0);
        _chi.ChangeOrigin(NewOrigin);
        Yan[NewOrigin] = Yue;
    }

    function YueMintToOrigin(YUEINTERFACE Yue) public onlyOwners {
        Yue.MintToOrigin();
    }

    function Chou() public returns (uint64) {
        uint64 Soul = Xie.Xia().Mai().Qi().Zuo().Cho().GetUserSoul();
        uint256 Maat = Xie.Xia().Mai().Maat();
        Entropy[Maat] = Xie.Xia().Mai().Qi().Zuo().Cho().ReactUser(Soul, Entropy[Maat]);
        return Entropy[Maat];
    }

    error InvalidQinInstance(uint64 Soul, uint256 SoulMaat, uint256 QinMaat);
    function React(QIN memory Player, TRAIT Trait) public returns (uint256 Charge, uint256 UserQi, uint64 Omega) {
        uint64 Soul = Xie.Xia().Mai().Qi().Zuo().Cho().GetUserSoul();
        uint256 Maat = Xie.Xia().Mai().Maat();
        if(Maat != Player.Maat) revert InvalidQinInstance(Soul, Maat, Player.Maat);
        UserQi = Xie.Xia().Mai().Qi().GetUserTraitValue(Soul, Trait);
        (Charge, Entropy[Maat], Omega) = Xie.React(Soul, Trait);
        _mintToCap();
    }
}