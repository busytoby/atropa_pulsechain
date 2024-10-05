// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "./interfaces/02b_cheoninterface.sol";

contract DING is DYSNOMIA {
    string public constant Type = "DING";

    CHEON public Cheon;

    constructor(address CheonAddress) DYSNOMIA("Dysnomia Ding", "DING", address(DYSNOMIA(CheonAddress).Xiao())) {
        Cheon = CHEON(CheonAddress);
        Cheon.Sei().Chan().Xie().Xia().Mai().Qi().Zuo().Cho().addOwner(address(this));
        addOwner(tx.origin);
        _mintToCap();
    }

    function GetTraitToken(TRAIT Trait) public view returns (DYSNOMIA Token) {
        if(Trait == TRAIT.YI) return DYSNOMIA(Cheon.Sei().Chan().Xie().Xia().Mai().Qi().Zuo().Cho().Addresses("YI"));
        if(Trait == TRAIT.ZHENG) return DYSNOMIA(Cheon.Sei().Chan().Xie().Xia().Mai().Qi().Zuo().Cho().Addresses("ZHENG"));
        if(Trait == TRAIT.ZHOU) return DYSNOMIA(Cheon.Sei().Chan().Xie().Xia().Mai().Qi().Zuo().Cho().Addresses("ZHOU"));
        if(Trait == TRAIT.YAU) return DYSNOMIA(Cheon.Sei().Chan().Xie().Xia().Mai().Qi().Zuo().Cho().Addresses("YAU"));
        if(Trait == TRAIT.YANG) return DYSNOMIA(Cheon.Sei().Chan().Xie().Xia().Mai().Qi().Zuo().Cho().Addresses("YANG"));
        if(Trait == TRAIT.SIU) return DYSNOMIA(Cheon.Sei().Chan().Xie().Xia().Mai().Qi().Zuo().Cho().Addresses("SIU"));
        if(Trait == TRAIT.VOID) return DYSNOMIA(Cheon.Sei().Chan().Xie().Xia().Mai().Qi().Zuo().Cho().Addresses("VOID"));
        if(Trait == TRAIT.LAU) return DYSNOMIA(Cheon.Sei().Chan().Xie().Xia().Mai().Qi().Zuo().Cho().Addresses("LAU"));
        if(Trait == TRAIT.CHO) return DYSNOMIA(Cheon.Sei().Chan().Xie().Xia().Mai().Qi().Zuo().Cho().Addresses("CHO"));
        if(Trait == TRAIT.QING) return DYSNOMIA(Cheon.Sei().Chan().Xie().Xia().Mai().Qi().Zuo().Cho().Addresses("QING"));
        if(Trait == TRAIT.QI) return DYSNOMIA(Cheon.Sei().Chan().Xie().Xia().Mai().Qi().Zuo().Cho().Addresses("QI"));
        if(Trait == TRAIT.MAI) return DYSNOMIA(Cheon.Sei().Chan().Xie().Xia().Mai().Qi().Zuo().Cho().Addresses("MAI"));
        assert(false);
    }

    function Train(TRAIT Trait) public returns (uint256 Charge, uint256 UserQi, uint256 Omicron, uint256 Omega) {
        (YUEINTERFACE Yue, , QIN memory Player) = Cheon.Sei().Chi();
        return _react(Yue, Player, Trait);
    }

    function _react(YUEINTERFACE Yue, QIN memory Player, TRAIT Trait) internal returns (uint256 Charge, uint256 UserQi, uint256 Omicron, uint256 Omega) {
        uint256 Hypogram;
        uint256 Epigram;
        uint256 Contour;
        (Charge, UserQi, Omega) = Cheon.Sei().Chan().React(Player, Trait);
        Omicron = Yue.React(Trait);
        (Hypogram, Epigram) = Yue.Bar(Trait);
        if(Trait != TRAIT.ZERO) Contour = GetTraitToken(Trait).balanceOf(Yue.Origin());
        else Contour = Player.Location.balanceOf(Yue.Origin());
        Omega = Xiao.modExp(Hypogram, Omega, Contour);
        Omicron = Xiao.modExp(Epigram, Omicron, Contour);
    }

    function React(uint64 Soul, TRAIT Trait) public onlyOwners returns (uint256 Charge, uint256 UserQi, uint256 Omicron, uint256 Omega) {
        (YUEINTERFACE Yue, , QIN memory Player) = Cheon.Sei().ChiBySoul(Soul);
        return _react(Yue, Player, Trait);
    }
}