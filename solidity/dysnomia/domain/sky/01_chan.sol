// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "../../include/qin.sol";
import "../../interfaces/17b_xieinterface.sol";

contract CHAN is DYSNOMIA {
    string public constant Type = "CHAN";

    XIE public Xie;

    constructor(address XieAddress) DYSNOMIA("Dysnomia Chan", "CHAN", address(DYSNOMIA(XieAddress).Xiao())) {
        Xie = XIE(XieAddress);
        addOwner(tx.origin);
        _mintToCap();
    }

    error InvalidQinInstance(uint64 Soul, uint256 SoulMaat, uint256 QinMaat);
    function React(QIN memory Player, string memory Adjective) public returns (uint256 Charge, uint64 Omicron, uint64 Omega) {
        uint64 Soul = Xie.Xia().Mai().Qi().Zuo().Cho().GetUserSoul();
        uint256 Maat = Xie.Xia().Mai().Maat();
        if(Maat != Player.Maat) revert InvalidQinInstance(Soul, Maat, Player.Maat);
        uint256 UserQi = Xie.Xia().Mai().Qi().GetUserAdjectiveValue(Soul, Adjective);
        (Charge, Omicron, Omega) = Xie.React(Soul, Adjective);
        Charge = Charge % UserQi;
        _mintToCap();
    }
}