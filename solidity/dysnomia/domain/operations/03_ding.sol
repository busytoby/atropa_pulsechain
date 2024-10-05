// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "./interfaces/02b_cheoninterface.sol";

contract DING is DYSNOMIA {
    string public constant Type = "DING";

    CHEON public Cheon;

    constructor(address CheonAddress) DYSNOMIA("Dysnomia Ding", "DING", address(DYSNOMIA(CheonAddress).Xiao())) {
        Cheon = CHEON(CheonAddress);
        Cheon.Sei().Chan().Xie().Xia().Mai().Qi().Zuo().VAT().addOwner(address(this));
        addOwner(tx.origin);
        _mintToCap();
    }

    function React(TRAIT Trait) public returns (uint256 Charge, uint256 UserQi, uint64 Omicron, uint64 Omega) {
        (YUEINTERFACE Yue, , QIN memory Player) = Cheon.Sei().Chi();
        (Charge, UserQi, Omega) = Cheon.Sei().Chan().React(Player, Trait);
        Omicron = Yue.React(Trait);
    }
}