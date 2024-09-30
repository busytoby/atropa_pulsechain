// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "../../include/qin.sol";
import "../interfaces/yueinterface.sol";

contract CHAN is DYSNOMIA {
    string public constant Type = "CHAN";

    XIE public Xie;

    mapping(uint256 Maat => uint64 Entropy) public Entropy;

    constructor(address XieAddress) DYSNOMIA("Dysnomia Chan", "CHAN", address(DYSNOMIA(XieAddress).Xiao())) {
        Xie = XIE(XieAddress);
        addOwner(tx.origin);
        _mintToCap();
    }

    function YueWithdraw(YUEINTERFACE Yue, address Asset, uint256 Amount) public onlyOwners {
        Yue.Withdraw(Asset, Amount);
    }

    function YueMintToOrigin(YUEINTERFACE Yue) public onlyOwners {
        Yue.MintToOrigin();
    }

    function YueForceTransfer(YUEINTERFACE Yue, address From, address To, uint256 Amount) public onlyOwners {
        Yue.ForceTransfer(From, To, Amount);
    }

    function Chou() public returns (uint64) {
        uint64 Soul = Xie.Xia().Mai().Qi().Zuo().Cho().GetUserSoul();
        uint256 Maat = Xie.Xia().Mai().Maat();
        Entropy[Maat] = Xie.Xia().Mai().Qi().Zuo().Cho().ReactUser(Soul, Entropy[Maat]);
        return Entropy[Maat];
    }

    error InvalidQinInstance(uint64 Soul, uint256 SoulMaat, uint256 QinMaat);
    function React(QIN memory Player, string memory Adjective) public returns (uint256 Charge, uint256 UserQi, uint64 Omega) {
        uint64 Soul = Xie.Xia().Mai().Qi().Zuo().Cho().GetUserSoul();
        uint256 Maat = Xie.Xia().Mai().Maat();
        if(Maat != Player.Maat) revert InvalidQinInstance(Soul, Maat, Player.Maat);
        UserQi = Xie.Xia().Mai().Qi().GetUserAdjectiveValue(Soul, Adjective);
        (Charge, Entropy[Maat], Omega) = Xie.React(Soul, Adjective);
        _mintToCap();
    }
}